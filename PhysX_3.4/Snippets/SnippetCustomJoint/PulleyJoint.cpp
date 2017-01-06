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


#include "PulleyJoint.h"
#include <assert.h>
#include "PxConstraint.h"

using namespace physx;


PxConstraintShaderTable PulleyJoint::sShaderTable = { &PulleyJoint::solverPrep, &PulleyJoint::project, &PulleyJoint::visualize, PxConstraintFlag::Enum(0) };

PulleyJoint::PulleyJoint(PxPhysics& physics, PxRigidBody& body0, const PxTransform& localFrame0, const PxVec3& attachment0,
											 PxRigidBody& body1, const PxTransform& localFrame1, const PxVec3& attachment1)
{
	mConstraint = physics.createConstraint(&body0, &body1, *this, PulleyJoint::sShaderTable, sizeof(PulleyJointData));

	mBody[0] = &body0;
	mBody[1] = &body1;

	// keep these around in case the CoM gets relocated
	mLocalPose[0] = localFrame0.getNormalized();
	mLocalPose[1] = localFrame1.getNormalized();

	// the data which will be fed to the joint solver and projection shaders
	mData.attachment0 = attachment0;
	mData.attachment1 = attachment1;
	mData.distance = 1.0f;
	mData.ratio = 1.0f;
	mData.c2b[0] = body0.getCMassLocalPose().transformInv(mLocalPose[0]);
	mData.c2b[1] = body1.getCMassLocalPose().transformInv(mLocalPose[1]);
}

void PulleyJoint::release()
{
	mConstraint->release();
}

///////////////////////////////////////////// attribute accessors and mutators


void PulleyJoint::setAttachment0(const PxVec3& pos)
{
	mData.attachment0 = pos;
	mConstraint->markDirty();
}

PxVec3 PulleyJoint::getAttachment0() const
{
	return mData.attachment0;
}

void PulleyJoint::setAttachment1(const PxVec3& pos)
{
	mData.attachment1 = pos;
	mConstraint->markDirty();
}

PxVec3 PulleyJoint::getAttachment1() const
{
	return mData.attachment1;
}

void PulleyJoint::setDistance(float totalDistance)
{
	mData.distance = totalDistance;
	mConstraint->markDirty();
}

float PulleyJoint::getDistance() const
{
	return mData.distance;
}
	
void PulleyJoint::setRatio(float ratio)
{
	mData.ratio = ratio;
	mConstraint->markDirty();
}

float PulleyJoint::getRatio() const
{
	return mData.ratio;
}


///////////////////////////////////////////// PxConstraintConnector methods

void* PulleyJoint::prepareData()
{
	return &mData;
}

void  PulleyJoint::onConstraintRelease()
{
	delete this;
}

void PulleyJoint::onComShift(PxU32 actor)
{
	mData.c2b[actor] = mBody[actor]->getCMassLocalPose().transformInv(mLocalPose[actor]); 
	mConstraint->markDirty();
}

void  PulleyJoint::onOriginShift(const PxVec3& shift)
{
	mData.attachment0 -= shift;
	mData.attachment1 -= shift;
}

void* PulleyJoint::getExternalReference(PxU32& typeID)
{
	typeID = TYPE_ID;
	return this;
}


///////////////////////////////////////////// work functions


PxU32 PulleyJoint::solverPrep(Px1DConstraint* constraints,
							  PxVec3& body0WorldOffset,
							  PxU32 maxConstraints,
							  PxConstraintInvMassScale&,
							  const void* constantBlock,
							  const PxTransform& bA2w,
							  const PxTransform& bB2w)
{		
	PX_UNUSED(maxConstraints);

	const PulleyJointData& data = *reinterpret_cast<const PulleyJointData*>(constantBlock);

	PxTransform cA2w = bA2w.transform(data.c2b[0]);
	PxTransform cB2w = bB2w.transform(data.c2b[1]);

	body0WorldOffset = cB2w.p - bA2w.p;

	PxVec3 directionA = data.attachment0 - cA2w.p;
	PxReal distanceA = directionA.normalize();

	PxVec3 directionB = data.attachment1 - cB2w.p;
	PxReal distanceB = directionB.normalize();

	directionB *= data.ratio;

	PxReal totalDistance = distanceA + distanceB;

	// compute geometric error:
	PxReal geometricError = (data.distance - totalDistance); 

	Px1DConstraint *c = constraints;
	// constraint is breakable, so we need to output forces
	c->flags = Px1DConstraintFlag::eOUTPUT_FORCE;

	if (geometricError < 0.0f) 
	{
		c->maxImpulse = PX_MAX_F32;
		c->minImpulse = 0;
		c->geometricError = geometricError; 
	}
	else if(geometricError > 0.0f)
	{
		c->maxImpulse = 0;
		c->minImpulse = -PX_MAX_F32;
		c->geometricError = geometricError; 
	}

	c->linear0 = directionA;		c->angular0 = (cA2w.p - bA2w.p).cross(c->linear0);		
	c->linear1 = -directionB;		c->angular1 = (cB2w.p - bB2w.p).cross(c->linear1);		

	return 1;
}

void PulleyJoint::project(const void* constantBlock,
						  PxTransform& bodyAToWorld,
						  PxTransform& bodyBToWorld,
						  bool projectToA)
{
	PX_UNUSED(constantBlock);
	PX_UNUSED(bodyAToWorld);
	PX_UNUSED(bodyBToWorld);
	PX_UNUSED(projectToA);
}

void PulleyJoint::visualize(PxConstraintVisualizer&		viz,
							const void*					constantBlock,
							const PxTransform&			body0Transform,
							const PxTransform&			body1Transform,
							PxU32						flags)
{
	PX_UNUSED(flags);
	const PulleyJointData& data = *reinterpret_cast<const PulleyJointData*>(constantBlock);

	PxTransform cA2w = body0Transform * data.c2b[0];
	PxTransform cB2w = body1Transform * data.c2b[1];
	viz.visualizeJointFrames(cA2w, cB2w);
	viz.visualizeJointFrames(PxTransform(data.attachment0), PxTransform(data.attachment1));
}

