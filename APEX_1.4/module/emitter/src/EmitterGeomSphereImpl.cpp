/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "Apex.h"
#include "ApexUsingNamespace.h"
#include "EmitterGeomSphereImpl.h"
//#include "ApexSharedSerialization.h"
#include "RenderDebugInterface.h"
#include "RenderDebugInterface.h"
#include "ApexPreview.h"
#include "EmitterGeomSphereParams.h"

namespace nvidia
{
namespace emitter
{



EmitterGeomSphereImpl::EmitterGeomSphereImpl(NvParameterized::Interface* params)
{
	NvParameterized::Handle eh(*params);
	const NvParameterized::Definition* paramDef;
	const char* enumStr = 0;

	mGeomParams = (EmitterGeomSphereParams*)params;
	mRadius = &(mGeomParams->parameters().radius);
	mHemisphere = &(mGeomParams->parameters().hemisphere);

	//error check
	mGeomParams->getParameterHandle("emitterType", eh);
	mGeomParams->getParamEnum(eh, enumStr);
	paramDef = eh.parameterDefinition();

	mType = EmitterType::ET_RATE;
	for (int i = 0; i < paramDef->numEnumVals(); ++i)
	{
		if (!nvidia::strcmp(paramDef->enumVal(i), enumStr))
		{
			mType = (EmitterType::Enum)i;
			break;
		}
	}
}

EmitterGeom* EmitterGeomSphereImpl::getEmitterGeom()
{
	return this;
}


#ifdef WITHOUT_DEBUG_VISUALIZE
void EmitterGeomSphereImpl::visualize(const PxTransform& , RenderDebugInterface&)
{
}
#else
void EmitterGeomSphereImpl::visualize(const PxTransform& pose, RenderDebugInterface& renderDebug)
{
	const float radius = *mRadius;
	const float radiusSquared = radius * radius;
	const float hemisphere = *mHemisphere;
	const float sphereCapBaseHeight = -radius + 2 * radius * hemisphere;
	const float sphereCapBaseRadius = PxSqrt(radiusSquared - sphereCapBaseHeight * sphereCapBaseHeight);
	using RENDER_DEBUG::DebugColors;
	RENDER_DEBUG_IFACE(&renderDebug)->pushRenderState();
	RENDER_DEBUG_IFACE(&renderDebug)->setPose(pose);
	RENDER_DEBUG_IFACE(&renderDebug)->setCurrentColor(RENDER_DEBUG_IFACE(&renderDebug)->getDebugColor(DebugColors::DarkGreen));
	RENDER_DEBUG_IFACE(&renderDebug)->debugSphere(PxVec3(0.0f), *mRadius);
	if(hemisphere > 0.0f) 
	{
		RENDER_DEBUG_IFACE(&renderDebug)->setCurrentColor(RENDER_DEBUG_IFACE(&renderDebug)->getDebugColor(DebugColors::DarkPurple));
		PxMat44 circlePose = PxMat44(PxIdentity);
		circlePose.setPosition(PxVec3(0.0f, sphereCapBaseHeight, 0.0f));
		RENDER_DEBUG_IFACE(&renderDebug)->setPose(circlePose);
		RENDER_DEBUG_IFACE(&renderDebug)->debugCircle(PxVec3(0.0f), sphereCapBaseRadius, 3);
		RENDER_DEBUG_IFACE(&renderDebug)->debugLine(PxVec3(0.0f), PxVec3(0.0f, radius - sphereCapBaseHeight, 0.0f));
		for(float t = 0.0f; t < 2 * PxPi; t += PxPi / 3)
		{
			PxVec3 offset(PxSin(t) * sphereCapBaseRadius, 0.0f, PxCos(t) * sphereCapBaseRadius);
			RENDER_DEBUG_IFACE(&renderDebug)->debugLine(offset, PxVec3(0.0f, radius - sphereCapBaseHeight, 0.0f));
		}
		RENDER_DEBUG_IFACE(&renderDebug)->setPose(PxIdentity);
	}
	RENDER_DEBUG_IFACE(&renderDebug)->popRenderState();
}
#endif

#ifdef WITHOUT_DEBUG_VISUALIZE
void EmitterGeomSphereImpl::drawPreview(float , RenderDebugInterface*) const
{
}
#else
void EmitterGeomSphereImpl::drawPreview(float scale, RenderDebugInterface* renderDebug) const
{
	const float radius = *mRadius;
	const float radiusSquared = radius * radius;
	const float hemisphere = *mHemisphere;
	const float sphereCapBaseHeight = -radius + 2 * radius * hemisphere;
	const float sphereCapBaseRadius = PxSqrt(radiusSquared - sphereCapBaseHeight * sphereCapBaseHeight);
	using RENDER_DEBUG::DebugColors;
	RENDER_DEBUG_IFACE(renderDebug)->pushRenderState();
	RENDER_DEBUG_IFACE(renderDebug)->setCurrentColor(RENDER_DEBUG_IFACE(renderDebug)->getDebugColor(DebugColors::DarkGreen),
	                             RENDER_DEBUG_IFACE(renderDebug)->getDebugColor(DebugColors::DarkGreen));
	RENDER_DEBUG_IFACE(renderDebug)->debugSphere(PxVec3(0.0f), *mRadius * scale);
	if(hemisphere > 0.0f)
	{
		RENDER_DEBUG_IFACE(renderDebug)->setCurrentColor(RENDER_DEBUG_IFACE(renderDebug)->getDebugColor(DebugColors::DarkPurple));
		PxMat44 circlePose = PxMat44(PxIdentity);
		circlePose.setPosition(PxVec3(0.0f, sphereCapBaseHeight, 0.0f));
		RENDER_DEBUG_IFACE(renderDebug)->setPose(circlePose);
		RENDER_DEBUG_IFACE(renderDebug)->debugCircle(PxVec3(0.0f), sphereCapBaseRadius, 3);
		RENDER_DEBUG_IFACE(renderDebug)->debugLine(PxVec3(0.0f), PxVec3(0.0f, radius - sphereCapBaseHeight, 0.0f));
		for(float t = 0.0f; t < 2 * PxPi; t += PxPi / 3)
		{
			PxVec3 offset(PxSin(t) * sphereCapBaseRadius, 0.0f, PxCos(t) * sphereCapBaseRadius);
			RENDER_DEBUG_IFACE(renderDebug)->debugLine(offset,  PxVec3(0.0f, radius - sphereCapBaseHeight, 0.0f));
		}
		RENDER_DEBUG_IFACE(renderDebug)->setPose(PxIdentity);
	}
	RENDER_DEBUG_IFACE(renderDebug)->popRenderState();
}
#endif

void EmitterGeomSphereImpl::setEmitterType(EmitterType::Enum t)
{
	mType = t;

	NvParameterized::Handle eh(*mGeomParams);
	const NvParameterized::Definition* paramDef;

	//error check
	mGeomParams->getParameterHandle("emitterType", eh);
	paramDef = eh.parameterDefinition();

	mGeomParams->setParamEnum(eh, paramDef->enumVal((int)mType));
}

float EmitterGeomSphereImpl::computeEmitterVolume() const
{
	PX_ASSERT(*mHemisphere >= 0.0f);
	PX_ASSERT(*mHemisphere <= 1.0f);
	float radius = *mRadius;
	float hemisphere = 2 * radius * (*mHemisphere);
	bool moreThanHalf = true;
	if (hemisphere > radius)
	{
		hemisphere -= radius;
		moreThanHalf = false;
	}
	const float halfSphereVolume = 2.0f / 3.0f * PxPi * radius * radius * radius;
	const float sphereCapVolume = 1.0f / 3.0f * PxPi * hemisphere * hemisphere * (3 * radius - hemisphere);
	if (moreThanHalf)
	{
		return halfSphereVolume + sphereCapVolume;
	}
	else
	{
		return sphereCapVolume;
	}
}

PxVec3 EmitterGeomSphereImpl::randomPosInFullVolume(const PxMat44& pose, QDSRand& rand) const
{
	PX_ASSERT(*mHemisphere >= 0.0f);
	PX_ASSERT(*mHemisphere <= 1.0f);

	const float radius = *mRadius;
	const float radiusSquared = radius * radius;
	const float hemisphere = *mHemisphere;
	const float sphereCapBaseHeight = -radius + 2 * radius * hemisphere;
	const float sphereCapBaseRadius = PxSqrt(radiusSquared - sphereCapBaseHeight * sphereCapBaseHeight);
	const float horizontalExtents = hemisphere < 0.5f ? radius : sphereCapBaseRadius;
	/* bounding box for a sphere cap */
	const PxBounds3 boundingBox(PxVec3(-horizontalExtents, sphereCapBaseHeight, -horizontalExtents),
									   PxVec3(horizontalExtents, radius, horizontalExtents));

	PxVec3 pos;
	do
	{
		pos = rand.getScaled(boundingBox.minimum, boundingBox.maximum);
	}
	while (pos.magnitudeSquared() > radiusSquared);

	return pose.transform(pos);
}


bool EmitterGeomSphereImpl::isInEmitter(const PxVec3& pos, const PxMat44& pose) const
{
	const PxVec3 localPos = pose.inverseRT().transform(pos);
	const float radius = *mRadius;
	const float radiusSquared = radius * radius;
	const float hemisphere = *mHemisphere;
	const float sphereCapBaseHeight = -radius + 2 * radius * hemisphere;
	const float sphereCapBaseRadius = PxSqrt(radiusSquared - sphereCapBaseHeight * sphereCapBaseHeight);
	const float horizontalExtents = hemisphere < 0.5f ? radius : sphereCapBaseRadius;
	/* bounding box for a sphere cap */
	const PxBounds3 boundingBox(PxVec3(-horizontalExtents, sphereCapBaseHeight, -horizontalExtents),
	                                   PxVec3(horizontalExtents, radius, horizontalExtents));

	bool isInSphere = localPos.magnitudeSquared() <= radiusSquared;
	bool isInBoundingBox = boundingBox.contains(localPos);

	return isInSphere & isInBoundingBox;
}


void EmitterGeomSphereImpl::computeFillPositions(physx::Array<PxVec3>& positions,
        physx::Array<PxVec3>& velocities,
        const PxTransform& pose,
		const PxVec3& scale,
        float objRadius,
        PxBounds3& outBounds,
        QDSRand&) const
{
	// we're not doing anything with the velocities array
	PX_UNUSED(velocities);
	PX_UNUSED(scale);

	const float radius = *mRadius;
	const float radiusSquared = radius * radius;
	const float hemisphere = *mHemisphere;
	const float sphereCapBaseHeight = -radius + 2 * radius * hemisphere;
	const float sphereCapBaseRadius = PxSqrt(radiusSquared - sphereCapBaseHeight * sphereCapBaseHeight);
	const float horizontalExtents = hemisphere < 0.5f ? radius : sphereCapBaseRadius;

	uint32_t numX = (uint32_t)PxFloor(horizontalExtents / objRadius);
	numX -= numX % 2;
	uint32_t numY = (uint32_t)PxFloor((radius - sphereCapBaseHeight) / objRadius);
	numY -= numY % 2;
	uint32_t numZ = (uint32_t)PxFloor(horizontalExtents / objRadius);
	numZ -= numZ % 2;

	const float radiusMinusObjRadius = radius - objRadius;
	const float radiusMinusObjRadiusSquared = radiusMinusObjRadius * radiusMinusObjRadius;
	for (float x = -(numX * objRadius); x <= radiusMinusObjRadius; x += 2 * objRadius)
	{
		for (float y = sphereCapBaseHeight; y <= radiusMinusObjRadius; y += 2 * objRadius)
		{
			for (float z = -(numZ * objRadius); z <= radiusMinusObjRadius; z += 2 * objRadius)
			{
				const PxVec3 p(x, y, z);
				if (p.magnitudeSquared() < radiusMinusObjRadiusSquared)
				{
					positions.pushBack(pose.transform(PxVec3(x, y, z)));
					outBounds.include(positions.back());
				}
			}
		}
	}
}

}
} // namespace nvidia::apex
