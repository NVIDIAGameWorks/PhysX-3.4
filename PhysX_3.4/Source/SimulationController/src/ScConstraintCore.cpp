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


#include "PsFoundation.h"

#include "ScPhysics.h"
#include "ScBodyCore.h"
#include "ScConstraintCore.h"
#include "ScConstraintSim.h"

using namespace physx;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Sc::ConstraintCore::ConstraintCore(PxConstraintConnector& connector, const PxConstraintShaderTable& shaders, PxU32 dataSize)
:	mFlags(PxConstraintFlag::eDRIVE_LIMITS_ARE_FORCES)
,	mAppliedForce(PxVec3(0))
,	mAppliedTorque(PxVec3(0))
,	mConnector(&connector)
,	mProject(shaders.project)
,	mSolverPrep(shaders.solverPrep)
,	mVisualize(shaders.visualize)
,	mDataSize(dataSize)
,	mLinearBreakForce(PX_MAX_F32)
,	mAngularBreakForce(PX_MAX_F32)
,	mMinResponseThreshold(0)
,	mSim(NULL)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Sc::ConstraintCore::~ConstraintCore()
{
}

void Sc::ConstraintCore::setFlags(PxConstraintFlags flags)
{
	PxConstraintFlags old = mFlags;
	flags = flags | (old & PxConstraintFlag::eGPU_COMPATIBLE);
	if(flags != old)
	{		
		mFlags = flags;
		if(getSim())
			getSim()->postFlagChange(old, flags);
	}
}

void Sc::ConstraintCore::getForce(PxVec3& force, PxVec3& torque) const
{
	if(!mSim)
	{
		force = PxVec3(0,0,0);
		torque = PxVec3(0,0,0);
	}
	else
		mSim->getForce(force, torque);

}

void Sc::ConstraintCore::setBodies(RigidCore* r0v, RigidCore* r1v)
{
	if(mSim)
		mSim->postBodiesChange(r0v, r1v);
}

bool Sc::ConstraintCore::updateConstants(void* addr)
{
	if (getSim())
	{
		getSim()->setConstantsLL(addr);
		return true;
	}
	return false;
}

void Sc::ConstraintCore::setBreakForce(PxReal linear, PxReal angular)
{
	mLinearBreakForce = linear;
	mAngularBreakForce = angular;

	if (getSim())
		getSim()->setBreakForceLL(linear, angular);
}

void Sc::ConstraintCore::getBreakForce(PxReal& linear, PxReal& angular) const
{
	linear = mLinearBreakForce;
	angular = mAngularBreakForce;
}

void Sc::ConstraintCore::setMinResponseThreshold(PxReal threshold)
{
	mMinResponseThreshold = threshold;

	if (getSim())
		getSim()->setMinResponseThresholdLL(threshold);
}

PxConstraint* Sc::ConstraintCore::getPxConstraint()
{
	return gOffsetTable.convertScConstraint2Px(this);
}

const PxConstraint* Sc::ConstraintCore::getPxConstraint() const
{
	return gOffsetTable.convertScConstraint2Px(this);
}

void Sc::ConstraintCore::breakApart()
{
	// TODO: probably want to do something with the interaction here
	// as well as remove the constraint from LL.

	mFlags |= PxConstraintFlag::eBROKEN;
}

void Sc::ConstraintCore::prepareForSetBodies()
{
	if(mSim)
		mSim->preBodiesChange();
}
