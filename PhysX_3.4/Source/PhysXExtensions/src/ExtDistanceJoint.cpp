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


#include "ExtDistanceJoint.h"
#include "common/PxSerialFramework.h"

using namespace physx;
using namespace Ext;

namespace physx
{
	PxDistanceJoint* PxDistanceJointCreate(PxPhysics& physics,
		PxRigidActor* actor0, const PxTransform& localFrame0,
		PxRigidActor* actor1, const PxTransform& localFrame1);
}

PxDistanceJoint* physx::PxDistanceJointCreate(PxPhysics& physics,
											  PxRigidActor* actor0, const PxTransform& localFrame0,
											  PxRigidActor* actor1, const PxTransform& localFrame1)
{
	PX_CHECK_AND_RETURN_NULL(localFrame0.isSane(), "PxDistanceJointCreate: local frame 0 is not a valid transform"); 
	PX_CHECK_AND_RETURN_NULL(localFrame1.isSane(), "PxDistanceJointCreate: local frame 1 is not a valid transform"); 
	PX_CHECK_AND_RETURN_NULL(actor0 != actor1, "PxDistanceJointCreate: actors must be different");
	PX_CHECK_AND_RETURN_NULL((actor0 && actor0->is<PxRigidBody>()) || (actor1 && actor1->is<PxRigidBody>()), "PxD6JointCreate: at least one actor must be dynamic");

	DistanceJoint* j;
	PX_NEW_SERIALIZED(j,DistanceJoint)(physics.getTolerancesScale(), actor0, localFrame0, actor1, localFrame1);
	if(j->attach(physics, actor0, actor1))
		return j;

	PX_DELETE(j);
	return NULL;
}

PxReal DistanceJoint::getDistance() const
{
	return getRelativeTransform().p.magnitudeSquared();
}

void DistanceJoint::setMinDistance(PxReal distance)	
{ 
	PX_CHECK_AND_RETURN(PxIsFinite(distance), "PxDistanceJoint::setMinDistance: invalid parameter");
	data().minDistance = distance;
	markDirty();
}

PxReal DistanceJoint::getMinDistance() const
{ 
	return data().minDistance;		
}

void DistanceJoint::setMaxDistance(PxReal distance)	
{ 
	PX_CHECK_AND_RETURN(PxIsFinite(distance), "PxDistanceJoint::setMaxDistance: invalid parameter");
	data().maxDistance = distance;
	markDirty();
}

PxReal DistanceJoint::getMaxDistance() const	
{ 
	return data().maxDistance;			
}

void DistanceJoint::setTolerance(PxReal tolerance) 
{ 
	PX_CHECK_AND_RETURN(PxIsFinite(tolerance), "PxDistanceJoint::setTolerance: invalid parameter");
	data().tolerance = tolerance;
	markDirty();
}

PxReal DistanceJoint::getTolerance() const
{ 
	return data().tolerance;			
}

void DistanceJoint::setStiffness(PxReal stiffness)
{ 
	PX_CHECK_AND_RETURN(PxIsFinite(stiffness), "PxDistanceJoint::setStiffness: invalid parameter");
	data().stiffness = stiffness;
	markDirty();
}

PxReal DistanceJoint::getStiffness() const	
{ 
	return data().stiffness;
}

void DistanceJoint::setDamping(PxReal damping)
{ 
	PX_CHECK_AND_RETURN(PxIsFinite(damping), "PxDistanceJoint::setDamping: invalid parameter");
	data().damping = damping;
	markDirty();	
}

PxReal DistanceJoint::getDamping() const
{ 
	return data().damping;
}

PxDistanceJointFlags DistanceJoint::getDistanceJointFlags(void) const
{ 
	return data().jointFlags;		
}

void DistanceJoint::setDistanceJointFlags(PxDistanceJointFlags flags) 
{ 
	data().jointFlags = flags; 
	markDirty();	
}

void DistanceJoint::setDistanceJointFlag(PxDistanceJointFlag::Enum flag, bool value)
{
	if(value)
		data().jointFlags |= flag;
	else
		data().jointFlags &= ~flag;
	markDirty();
}



namespace
{
static void DistanceJointVisualize(PxConstraintVisualizer& viz, const void* constantBlock, const PxTransform& body0Transform, const PxTransform& body1Transform, PxU32 flags)
{
	const DistanceJointData& data = *reinterpret_cast<const DistanceJointData*>(constantBlock);

	const PxTransform cA2w = body0Transform.transform(data.c2b[0]);
	const PxTransform cB2w = body1Transform.transform(data.c2b[1]);

	if(flags & PxConstraintVisualizationFlag::eLOCAL_FRAMES)
		viz.visualizeJointFrames(cA2w, cB2w);

	// PT: we consider the following is part of the joint's "limits" since that's the only available flag we have
	if(flags & PxConstraintVisualizationFlag::eLIMITS)
	{
		const bool enforceMax = (data.jointFlags & PxDistanceJointFlag::eMAX_DISTANCE_ENABLED);
		const bool enforceMin = (data.jointFlags & PxDistanceJointFlag::eMIN_DISTANCE_ENABLED);
		if(!enforceMin && !enforceMax)
			return;

		PxVec3 dir = cB2w.p - cA2w.p;
		const float currentDist = dir.normalize();

		PxU32 color = 0x00ff00;
		if(enforceMax && currentDist>data.maxDistance)
			color = 0xff0000;
		if(enforceMin && currentDist<data.minDistance)
			color = 0x0000ff;

		viz.visualizeLine(cA2w.p, cB2w.p, color);
	}
}

void DistanceJointProject(const void* /*constantBlock*/,
						  PxTransform& /*bodyAToWorld*/,
						  PxTransform& /*bodyBToWorld*/,
						  bool /*projectToA*/)
{
	// TODO
}


}

bool Ext::DistanceJoint::attach(PxPhysics &physics, PxRigidActor* actor0, PxRigidActor* actor1)
{
	mPxConstraint = physics.createConstraint(actor0, actor1, *this, sShaders, sizeof(DistanceJointData));
	return mPxConstraint!=NULL;
}

void DistanceJoint::exportExtraData(PxSerializationContext& stream)
{
	if(mData)
	{
		stream.alignData(PX_SERIAL_ALIGN);
		stream.writeData(mData, sizeof(DistanceJointData));
	}
	stream.writeName(mName);
}

void DistanceJoint::importExtraData(PxDeserializationContext& context)
{
	if(mData)
		mData = context.readExtraData<DistanceJointData, PX_SERIAL_ALIGN>();

	context.readName(mName);
}

void DistanceJoint::resolveReferences(PxDeserializationContext& context)
{
	setPxConstraint(resolveConstraintPtr(context, getPxConstraint(), getConnector(), sShaders));	
}

DistanceJoint* DistanceJoint::createObject(PxU8*& address, PxDeserializationContext& context)
{
	DistanceJoint* obj = new (address) DistanceJoint(PxBaseFlag::eIS_RELEASABLE);
	address += sizeof(DistanceJoint);	
	obj->importExtraData(context);
	obj->resolveReferences(context);
	return obj;
}

// global function to share the joint shaders with API capture	
const PxConstraintShaderTable* Ext::GetDistanceJointShaderTable() 
{ 
	return &DistanceJoint::getConstraintShaderTable();
}

//~PX_SERIALIZATION
PxConstraintShaderTable Ext::DistanceJoint::sShaders = { Ext::DistanceJointSolverPrep, DistanceJointProject, DistanceJointVisualize, PxConstraintFlag::Enum(0) };
