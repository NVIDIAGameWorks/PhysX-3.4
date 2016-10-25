/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __PARTICLE_IOS_COMMON_CODE_H__
#define __PARTICLE_IOS_COMMON_CODE_H__

#include "ParticleIosCommon.h"


namespace nvidia
{
namespace pxparticleios
{

#ifdef __CUDACC__

#define SIM_FETCH(name, idx) tex1Dfetch(KERNEL_TEX_REF(name), idx)
#define SIM_FLOAT4 float4
#define SIM_INT_AS_FLOAT(x) *(const float*)(&x)
#define SIM_INJECTOR_ARRAY const InjectorParamsArray&
#define SIM_FETCH_INJECTOR(injectorArray, injParams, injector) injectorArray.fetchElem(INPLACE_STORAGE_ARGS_VAL, injParams, injector)

PX_CUDA_CALLABLE PX_INLINE float splitFloat4(PxVec3& v3, const SIM_FLOAT4& f4)
{
	v3.x = f4.x;
	v3.y = f4.y;
	v3.z = f4.z;
	return f4.w;
}
PX_CUDA_CALLABLE PX_INLINE SIM_FLOAT4 combineFloat4(const PxVec3& v3, float w)
{
	return make_float4(v3.x, v3.y, v3.z, w);
}

struct PxInternalParticleFlagGpu
{
	enum Enum
	{
		//reserved	(1<<0),
		//reserved	(1<<1),
		//reserved	(1<<2),
		//reserved	(1<<3),
		//reserved	(1<<4),
		//reserved	(1<<5),
		eCUDA_NOTIFY_CREATE					= (1 << 6),
		eCUDA_NOTIFY_SET_POSITION			= (1 << 7),
	};
};
struct PxParticleFlag
{
	enum Enum
	{
		eVALID								= (1 << 0),
		eCOLLISION_WITH_STATIC				= (1 << 1),
		eCOLLISION_WITH_DYNAMIC				= (1 << 2),
		eCOLLISION_WITH_DRAIN				= (1 << 3),
		eSPATIAL_DATA_STRUCTURE_OVERFLOW	= (1 << 4),
	};
};
struct PxParticleFlagGpu
{
	uint16_t api;	// PxParticleFlag
	uint16_t low;	// PxInternalParticleFlagGpu
};

#else

#define SIM_FETCH(name, idx) mem##name[idx]
#define SIM_FLOAT4 PxVec4
#define SIM_INT_AS_FLOAT(x) *(const float*)(&x)
#define SIM_INJECTOR_ARRAY const Px3InjectorParams*
#define SIM_FETCH_INJECTOR(injectorArray, injParams, injector) { injParams = injectorArray[injector]; }

PX_INLINE float splitFloat4(PxVec3& v3, const SIM_FLOAT4& f4)
{
	v3 = f4.getXYZ();
	return f4.w;
}
PX_INLINE SIM_FLOAT4 combineFloat4(const PxVec3& v3, float w)
{
	return PxVec4(v3.x, v3.y, v3.z, w);
}

#endif


APEX_CUDA_CALLABLE PX_INLINE float calcParticleBenefit(
	const Px3InjectorParams& inj, const PxVec3& eyePos,
	const PxVec3& pos, const PxVec3& vel, float life)
{
	float benefit = inj.mLODBias;
	//distance term
	float distance = (eyePos - pos).magnitude();
	benefit += inj.mLODDistanceWeight * (1.0f - PxMin(1.0f, distance / inj.mLODMaxDistance));
	//velocity term, TODO: clamp velocity
	float velMag = vel.magnitude();
	benefit += inj.mLODSpeedWeight * velMag;
	//life term
	benefit += inj.mLODLifeWeight * life;

	return PxClamp(benefit, 0.0f, 1.0f);
}



INPLACE_TEMPL_VA_ARGS_DEF(typename FieldAccessor)
APEX_CUDA_CALLABLE PX_INLINE float simulateParticle(
	INPLACE_STORAGE_ARGS_DEF, SIM_INJECTOR_ARRAY injectorArray,
    float deltaTime,
    PxVec3 eyePos,
    bool isNewParticle,
    unsigned int srcIdx,
    unsigned int dstIdx,
    SIM_FLOAT4* memPositionMass,
    SIM_FLOAT4* memVelocityLife,
	SIM_FLOAT4* memCollisionNormalFlags,
	uint32_t* memUserData,
    IofxActorIDIntl* memIofxActorIDs,
    float* memLifeSpan,
    float* memLifeTime,
	float* memDensity,
    unsigned int* memInjector,
    FieldAccessor& fieldAccessor,
	unsigned int &injIndex,
	const GridDensityParams params,
#ifdef __CUDACC__
    SIM_FLOAT4* memPxPosition,
    SIM_FLOAT4* memPxVelocity,
	SIM_FLOAT4* memPxCollision,
	float* memPxDensity,
    unsigned int* memNvFlags
#else
	PxVec3& position,
	PxVec3& velocity,
	PxVec3& collisionNormal,
	uint32_t& particleFlags,
	float& density
#endif
)
{
	CPU_INPLACE_STORAGE_ARGS_UNUSED
	PX_UNUSED(memCollisionNormalFlags);
	PX_UNUSED(memDensity);
	PX_UNUSED(fieldAccessor);
	PX_UNUSED(params);

	float mass;
#ifdef __CUDACC__
	PxVec3 position;
	PxVec3 velocity;
	PxVec3 collisionNormal;
	uint32_t  particleFlags;
	float  density;
#endif

	//read
	float lifeSpan = SIM_FETCH(LifeSpan, srcIdx);
	unsigned int injector = SIM_FETCH(Injector, srcIdx);
	IofxActorIDIntl iofxActorID = IofxActorIDIntl(SIM_FETCH(IofxActorIDs, srcIdx));

	float lifeTime = lifeSpan;
	if (!isNewParticle)
	{
		lifeTime = SIM_FETCH(LifeTime, srcIdx);
		lifeTime = PxMax(lifeTime - deltaTime, 0.0f);

#ifndef __CUDACC__
		mass = memPositionMass[srcIdx].w;

		/* Apply field sampler velocity */
		fieldAccessor(srcIdx, velocity);
#endif
	}
	else
	{
		collisionNormal = PxVec3(0.0f);
		particleFlags = 0;
		density = 0.0f;
		splitFloat4(velocity, SIM_FETCH(VelocityLife, srcIdx));
#ifdef __CUDACC__
	}
	{
#endif
		mass = splitFloat4(position, SIM_FETCH(PositionMass, srcIdx));
	}

#ifdef __CUDACC__
	if (isNewParticle)
	{
		memPxPosition[dstIdx] = combineFloat4(position, 0);
		memPxVelocity[dstIdx] = combineFloat4(velocity, 0);

		PxParticleFlagGpu flags;
		flags.api = PxParticleFlag::eVALID;
		flags.low = PxInternalParticleFlagGpu::eCUDA_NOTIFY_CREATE;
		memNvFlags[dstIdx] = *((unsigned int*)&flags);
	}
	else
	{
		const SIM_FLOAT4 pxPosition = SIM_FETCH(PxPosition, srcIdx);
		const SIM_FLOAT4 pxVelocity = SIM_FETCH(PxVelocity, srcIdx);

		if (memPxDensity) 
		{
			density = SIM_FETCH(PxDensity, srcIdx);
			memPxDensity[dstIdx] = density;
		}
		splitFloat4(position, pxPosition);
		splitFloat4(velocity, pxVelocity);

		PxParticleFlagGpu flags;
		*((uint32_t*)&flags) = SIM_FETCH(NvFlags, srcIdx);

		/* Apply field sampler velocity */
		fieldAccessor(srcIdx, velocity);
		memPxVelocity[dstIdx] = combineFloat4(velocity, pxVelocity.w);

		splitFloat4(collisionNormal, SIM_FETCH(PxCollision, srcIdx));
		particleFlags = flags.api;

		if (dstIdx != srcIdx)
		{
			memPxPosition[dstIdx] = pxPosition;

			flags.low |= PxInternalParticleFlagGpu::eCUDA_NOTIFY_SET_POSITION;
			memNvFlags[dstIdx] = *((uint32_t*)&flags);
		}
	}
#endif

	Px3InjectorParams injParams;
	SIM_FETCH_INJECTOR(injectorArray, injParams, injector);
	injIndex = injParams.mLocalIndex;
	// injParams.mLODBias == FLT_MAX if injector was released!
	// and IOFX returns IofxActorIDIntl::NO_VOLUME for homeless/dead particles
	bool validActorID = (injParams.mLODBias < FLT_MAX)
		&& (isNewParticle || (iofxActorID.getVolumeID() != IofxActorIDIntl::NO_VOLUME))
		&& position.isFinite() && velocity.isFinite();
	if (!validActorID)
	{
		iofxActorID.setActorClassID(IofxActorIDIntl::IPX_ACTOR);
		injIndex = UINT32_MAX;
	}

	//write
	memLifeTime[dstIdx] = lifeTime;
	memPositionMass[dstIdx] = combineFloat4(position, mass);
	memVelocityLife[dstIdx] = combineFloat4(velocity, lifeTime / lifeSpan);

	const uint32_t collisionFlags = (particleFlags & uint32_t(PxParticleFlag::eCOLLISION_WITH_STATIC | PxParticleFlag::eCOLLISION_WITH_DYNAMIC));
	memCollisionNormalFlags[dstIdx] = combineFloat4(collisionNormal, SIM_INT_AS_FLOAT(collisionFlags));
	if (memDensity != 0)
	{
		memDensity[dstIdx] = density;
	}

	if (!validActorID || dstIdx != srcIdx)
	{
		memIofxActorIDs[dstIdx] = iofxActorID;
	}
	if (dstIdx != srcIdx)
	{
		memLifeSpan[dstIdx] = lifeSpan;
		memInjector[dstIdx] = injector;

		memUserData[dstIdx] = SIM_FETCH(UserData,srcIdx);
	}

	float benefit = -FLT_MAX;
	if (validActorID && lifeTime > 0.0f)
	{
		benefit = calcParticleBenefit(injParams, eyePos, position, velocity, lifeTime / lifeSpan);
	}
	return benefit;
}

}
} // namespace nvidia

#endif
