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

#include "TestMotionGenerator.h"

//----------------------------------------------------------------------------//
MotionGenerator::MotionGenerator() 
:	mLinear(0.0f),
	mAngular(0.0f),
	mTransform(PxTransform(PxIdentity))
{
}

//----------------------------------------------------------------------------//
MotionGenerator::MotionGenerator(const PxTransform &pose, const PxVec3& linear, const PxVec3& angular) 
:	mLinear(linear),
	mAngular(angular),
	mTransform(pose)
{
}

//----------------------------------------------------------------------------//
PxVec3 MotionGenerator::getLinearVelocity( float time )
{
	float t = time - (int(time) & ~0xf);
	const float scale = 0.25f * PxPi;

	if(t > 0 && t < 2)
		return -scale * sinf(t * 0.5f * PxPi) * mLinear;

	if(t > 8 && t < 10)
		return +scale * sinf(t * 0.5f * PxPi) * mLinear;

	return PxVec3(0.0f);
}

//----------------------------------------------------------------------------//
PxVec3 MotionGenerator::getAngularVelocity( float time )
{
	float t = time - (int(time) & ~0xf);

	if(t > 4 && t < 6)
		return +PxPi * mAngular;

	if(t > 12 && t < 14)
		return -PxPi * mAngular;

	return PxVec3(0.0f);
}

static PxQuat computeQuatFromAngularVelocity(const PxVec3 &omega)
{
	PxReal angle = omega.magnitude();

	if (angle < 1e-5f) 
	{
		return PxQuat(PxIdentity);
	} else {
		PxReal s = sin( 0.5f * angle ) / angle;
		PxReal x = omega[0] * s;
		PxReal y = omega[1] * s;
		PxReal z = omega[2] * s;
		PxReal w = cos( 0.5f * angle );
		return PxQuat(x,y,z,w);
	}
}
//----------------------------------------------------------------------------//
const PxTransform& MotionGenerator::update(float time, float dt)
{
	PxVec3 dw = dt * getAngularVelocity(time);
	PxQuat dq = computeQuatFromAngularVelocity(dw);

	mTransform.q = (dq * mTransform.q).getNormalized();
	mTransform.p += dt * getLinearVelocity(time);
	
	return mTransform;
}





