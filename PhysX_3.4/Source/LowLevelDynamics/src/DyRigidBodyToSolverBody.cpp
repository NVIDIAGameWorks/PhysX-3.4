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


#include "CmUtils.h"
#include "DySolverBody.h"
#include "PxsRigidBody.h"
#include "PxvDynamics.h"

namespace physx
{

namespace Dy
{
//This method returns values of 0 when the inertia is 0. This is a bit of a hack but allows us to 
//represent kinematic objects' velocities in our new format
PX_FORCE_INLINE PxVec3 computeSafeSqrtInertia(const PxVec3& v)
{
	return PxVec3(v.x == 0.f ? 0.f : PxSqrt(v.x), v.y == 0.f ? 0.f : PxSqrt(v.y), v.z == 0.f ? 0.f : PxSqrt(v.z));
}

void copyToSolverBodyData(const PxVec3& linearVelocity, const PxVec3& angularVelocity, const PxReal invMass, const PxVec3& invInertia, const PxTransform& globalPose,
	const PxReal maxDepenetrationVelocity, const PxReal maxContactImpulse, const PxU32 nodeIndex, const PxReal reportThreshold, PxSolverBodyData& data, PxU32 lockFlags)
{
	data.nodeIndex = nodeIndex;

	PxVec3 safeSqrtInvInertia = computeSafeSqrtInertia(invInertia);

	PxMat33 rotation(globalPose.q);

	Cm::transformInertiaTensor(safeSqrtInvInertia, rotation, data.sqrtInvInertia);

	// Copy simple properties
	data.linearVelocity = linearVelocity;
	data.angularVelocity = angularVelocity;

	if (lockFlags)
	{
		if (lockFlags & PxRigidDynamicLockFlag::eLOCK_LINEAR_X)
			data.linearVelocity.x = 0.f;
		if (lockFlags & PxRigidDynamicLockFlag::eLOCK_LINEAR_Y)
			data.linearVelocity.y = 0.f;
		if (lockFlags & PxRigidDynamicLockFlag::eLOCK_LINEAR_Z)
			data.linearVelocity.z = 0.f;

		//KS - technically, we can zero the inertia columns and produce stiffer constraints. However, this can cause numerical issues with the 
		//joint solver, which is fixed by disabling joint preprocessing and setting minResponseThreshold to some reasonable value > 0. However, until
		//this is handled automatically, it's probably better not to zero these inertia rows
		if (lockFlags & PxRigidDynamicLockFlag::eLOCK_ANGULAR_X)
		{
			data.angularVelocity.x = 0.f;
			//data.sqrtInvInertia.column0 = PxVec3(0.f);
		}
		if (lockFlags & PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y)
		{
			data.angularVelocity.y = 0.f;
			//data.sqrtInvInertia.column1 = PxVec3(0.f);
		}
		if (lockFlags & PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z)
		{
			data.angularVelocity.z = 0.f;
			//data.sqrtInvInertia.column2 = PxVec3(0.f);
		}
	}


	PX_ASSERT(linearVelocity.isFinite());
	PX_ASSERT(angularVelocity.isFinite());

	data.invMass = invMass;
	data.penBiasClamp = maxDepenetrationVelocity;
	data.maxContactImpulse = maxContactImpulse;
	data.body2World = globalPose;
	data.lockFlags = lockFlags;

	data.reportThreshold = reportThreshold;
}

}

}
