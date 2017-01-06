// This code contains NVIDIA Confidential Information and is disclosed to you
// under a form of NVIDIA software license agreement provided separately to you.
//
// Notice
// NVIDIA Corporation and its licensors retain all intellectual property and
// proprietary rights in and to this software and related documentation and
// any modifications thereto. Any use, reproduction, disclosure, or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA Corporation is strictly prohibited.
//
// ALL NVIDIA DESIGN SPECIFICATIONS, CODE ARE PROVIDED "AS IS.". NVIDIA MAKES
// NO WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES OF NONINFRINGEMENT,
// MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Information and code furnished is believed to be accurate and reliable.
// However, NVIDIA Corporation assumes no responsibility for the consequences of use of such
// information or for any infringement of patents or other rights of third parties that may
// result from its use. No license is granted by implication or otherwise under any patent
// or patent rights of NVIDIA Corporation. Details are subject to change without notice.
// This code supersedes and replaces all information previously supplied.
// NVIDIA Corporation products are not authorized for use as critical
// components in life support devices or systems without express written approval of
// NVIDIA Corporation.
//
// Copyright (c) 2008-2017 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

#ifndef PARTICLE_EMITTER_H
#define PARTICLE_EMITTER_H

#include "ParticleFactory.h"
#include "PxTkRandom.h"

using namespace PxToolkit;
//----------------------------------------------------------------------------//

#define PARTICLE_EMITTER_RANDOMIZE_EMISSION 1

//----------------------------------------------------------------------------//

class ParticleEmitter : public SampleAllocateable
{
public:

	struct Shape
	{
		enum Enum
		{
			eELLIPSE	= 0,
			eRECTANGLE	= 1,
		};
	};

						ParticleEmitter(Shape::Enum shape, PxReal extentX, PxReal extentY, PxReal spacing);
	virtual				~ParticleEmitter();

	// Relative to mFrameBody, or relative to global frame, if mFrameBody == NULL
	void				setLocalPose(const PxTransform& pose)		{ mLocalPose = pose; }
	PxTransform			getLocalPose()						const	{ return mLocalPose; } 

	void				setFrameRigidBody(PxRigidDynamic* rigidBody){ mFrameBody = rigidBody; }
	PxRigidDynamic*		getFrameRigidBody()					const	{ return mFrameBody; }

	void 				setRandomPos(PxVec3 t)						{ mRandomPos = t; }
	PxVec3 				getRandomPos()						const	{ return mRandomPos; }

	void 				setRandomAngle(PxReal t)					{ mRandomAngle = t; }
	PxReal 				getRandomAngle()					const	{ return mRandomAngle; }

	void 				setVelocity(PxReal t)						{ mVelocity = t; }
	PxReal 				getVelocity()						const	{ return mVelocity; }

	// Used for two way interaction, zero meaning, there is none
	void				setParticleMass(PxReal m)						{ mParticleMass = m; }
	PxReal 				getParticleMass()						const	{ return mParticleMass; }

	void				step(ParticleData& particles, PxReal dt, const PxVec3& externalAcceleration, PxReal maxParticleVelocity);

protected:

	virtual	void		stepInternal(ParticleData& particles, PxReal dt, const PxVec3& externalAcceleration, PxReal maxParticleVelocity) = 0;

	void				initStep(ParticleData& particles, PxReal dt);
	void				finalizeStep();

	PX_INLINE	void	computePositionNoise(PxVec3& posNoise);
				void	computeSiteVelocity(PxVec3& siteVel, const PxVec3& sitePos);
	PX_INLINE	bool	isOutsideShape(PxU32 x, PxU32 y, PxReal offset) { return mShape == Shape::eELLIPSE && outsideEllipse(x, y, offset); }

	static		bool	spawnParticle(
							ParticleData& data,
							PxU32& num, 
							const PxU32 max, 
							const PxVec3& position, 
							const PxVec3& velocity);

	static PX_INLINE PxReal randInRange(PxReal a,PxReal b);
	static PX_INLINE PxU32 randInRange(PxU32 range);
#if PARTICLE_EMITTER_RANDOMIZE_EMISSION
	static PxToolkit::BasicRandom mRandom;
#endif

protected:

	PxVec3				mRandomPos;
	PxReal				mRandomAngle;
	PxReal				mVelocity;
	PxReal				mParticleMass;

	//derived quantities
	PxU32				mNumSites;
	PxU32				mNumX;
	PxU32				mNumY;
	PxReal				mSpacingX;
	PxReal				mSpacingY;
	PxReal				mSpacingZ;

	//only needed during step computations.
	PxVec3				mAxisX;
	PxVec3				mAxisY;
	PxVec3				mAxisZ;	
	PxVec3				mBasePos;

private:

	PX_INLINE	bool	outsideEllipse(PxU32 x, PxU32 y, PxReal offset);
	void				updateDerivedBase();

private:

	PxTransform			mLocalPose;
	PxRigidDynamic*		mFrameBody;

	PxReal				mExtentX;
	PxReal				mExtentY;
	Shape::Enum			mShape;

	//state derived quantities
	PxReal				mEllipseRadius2;
	PxReal				mEllipseConstX0;
	PxReal				mEllipseConstX1;
	PxReal				mEllipseConstY0;
	PxReal				mEllipseConstY1;

	//only needed during step computations.
	PxVec3				mBodyAngVel;
	PxVec3				mBodyLinVel;
	PxVec3				mBodyCenter;
	PxTransform			mGlobalPose;
	PxVec3				mLinMomentum;
	PxVec3				mAngMomentum;
};

//----------------------------------------------------------------------------//

PX_INLINE bool ParticleEmitter::outsideEllipse(PxU32 x, PxU32 y, PxReal offset)
{
	PxReal cX = (x + offset - mEllipseConstX0)*mEllipseConstX1;
	PxReal cY = (y 		    - mEllipseConstY0)*mEllipseConstY1;
	return ( cX*cX + cY*cY >= mEllipseRadius2);
}

//----------------------------------------------------------------------------//

PX_INLINE void ParticleEmitter::computePositionNoise(PxVec3& posRand)
{
	posRand.x = randInRange(-mRandomPos.x, mRandomPos.x);
	posRand.y = randInRange(-mRandomPos.y, mRandomPos.y);
	posRand.z = randInRange(-mRandomPos.z, mRandomPos.z);
}

//----------------------------------------------------------------------------//

PX_INLINE PxReal ParticleEmitter::randInRange(PxReal a, PxReal b)
{
#if PARTICLE_EMITTER_RANDOMIZE_EMISSION
	return Rand(a, b);
#else
	return a + (b-a)/2.0f;
#endif
}

//----------------------------------------------------------------------------//

PX_INLINE PxU32 ParticleEmitter::randInRange(PxU32 range)
{
#if PARTICLE_EMITTER_RANDOMIZE_EMISSION
	PxU32 retval = Rand();
	if(range > 0x7fff)
	{
		retval = (retval << 15) | Rand();
		retval = (retval << 15) | Rand();
	}
	return retval % range;
#else
	static PxU32 noRandomVal = 0;
	return noRandomVal++;
#endif
}

//----------------------------------------------------------------------------//

#endif // PARTICLE_EMITTER_H
