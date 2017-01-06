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

#include "PtCollisionMethods.h"
#if PX_USE_PARTICLE_SYSTEM_API

using namespace physx;
using namespace Pt;

namespace
{

PX_FORCE_INLINE void collideWithPlane(ParticleCollData& collData, PxReal proxRadius)
{
	// In plane space the normal is (1,0,0) and d is 0. This simplifies the computations below.
	PxReal entryTime = -FLT_MAX;

	PxReal planeDistNewPos = collData.localNewPos.x;
	PxReal planeDistOldPos = collData.localOldPos.x;

	bool isContained = false;
	bool hasDC = false;
	bool hasProx = false;
	bool parallelMotion = false;

	// Test the old pos for containment
	if(planeDistOldPos <= 0.0f)
		isContained = true;

	// Test proximity
	if(planeDistNewPos <= proxRadius)
	{
		if(planeDistNewPos > 0.0f)
			hasProx = true;

		// Test discrete collision
		if(planeDistNewPos <= collData.restOffset)
			hasDC = true;
	}

	if(!(hasProx || hasDC || isContained))
		return; // We know that the old position is outside the surface and that the new position is
	            // not within the proximity region.

	PxVec3 planeNormal;
	planeNormal = PxVec3(1.0f, 0.0f, 0.0f);

	// Test continuous collision
	PxVec3 motion = collData.localNewPos - collData.localOldPos;
	PxReal projMotion = motion.x;
	if(projMotion == 0.0f) // parallel
	{
		if(planeDistNewPos > 0.0f)
			parallelMotion = true;
	}
	else
	{
		PxReal hitTime = -planeDistOldPos / projMotion;
		if(projMotion < 0.0f) // entry point
			entryTime = hitTime;
	}

	if(isContained)
	{
		// Treat the case where the old pos is inside the skeleton as
		// a continous collision with time 0

		collData.localFlags |= ParticleCollisionFlags::L_CC;
		collData.ccTime = 0.0f;
		collData.localSurfaceNormal = planeNormal;

		// Push the particle to the surface (such that distance to surface is equal to the collision radius)
		collData.localSurfacePos = collData.localOldPos;
		collData.localSurfacePos.x += (collData.restOffset - planeDistOldPos);
	}
	else
	{
		// check for continuous collision
		// only add a proximity/discrete case if there are no continous collisions
		// for this shape or any other shape before

		bool ccHappened = ((0.0f <= entryTime) && (entryTime < collData.ccTime) && (!parallelMotion));
		if(ccHappened)
		{
			collData.localSurfaceNormal = planeNormal;

			// collData.localSurfacePos = collData.localOldPos + (motion*entryTime);
			// collData.localSurfacePos.x += collData.restOffset;
			PxVec3 relativePOSITION = motion * entryTime;
			computeContinuousTargetPosition(collData.localSurfacePos, collData.localOldPos, relativePOSITION,
			                                collData.localSurfaceNormal, collData.restOffset);

			collData.ccTime = entryTime;
			collData.localFlags |= ParticleCollisionFlags::L_CC;
		}
		else if(!(collData.localFlags & ParticleCollisionFlags::CC))
		{
			// No other collision shape has caused a continuous collision so far

			PX_ASSERT(hasProx | hasDC);

			if(hasProx) // proximity
				collData.localFlags |= ParticleCollisionFlags::L_PROX;
			if(hasDC) // discrete collision
				collData.localFlags |= ParticleCollisionFlags::L_DC;

			collData.localSurfaceNormal = planeNormal;

			// Move contact point such that the projected distance to the surface is equal
			// to the collision radius
			collData.localSurfacePos = collData.localNewPos;
			collData.localSurfacePos.x += (collData.restOffset - planeDistNewPos);
		}
	}
}
}

void physx::Pt::collideWithPlane(ParticleCollData* particleCollData, PxU32 numCollData,
                                 const Gu::GeometryUnion& planeShape, PxReal proxRadius)
{
	PX_ASSERT(particleCollData);
	PX_ASSERT(planeShape.getType() == PxGeometryType::ePLANE);
	PX_UNUSED(planeShape);

	for(PxU32 p = 0; p < numCollData; p++)
	{
		::collideWithPlane(particleCollData[p], proxRadius);
	}
}

#endif // PX_USE_PARTICLE_SYSTEM_API
