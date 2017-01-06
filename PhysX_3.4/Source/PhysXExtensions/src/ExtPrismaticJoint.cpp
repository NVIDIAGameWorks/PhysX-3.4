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


#include "ExtPrismaticJoint.h"
#include "ExtConstraintHelper.h"
#include "CmRenderOutput.h"
#include "CmVisualization.h"

#include "common/PxSerialFramework.h"

using namespace physx;
using namespace Ext;

PxPrismaticJoint* physx::PxPrismaticJointCreate(PxPhysics& physics,
												PxRigidActor* actor0, const PxTransform& localFrame0,
												PxRigidActor* actor1, const PxTransform& localFrame1)
{
	PX_CHECK_AND_RETURN_NULL(localFrame0.isSane(), "PxPrismaticJointCreate: local frame 0 is not a valid transform"); 
	PX_CHECK_AND_RETURN_NULL(localFrame1.isSane(), "PxPrismaticJointCreate: local frame 1 is not a valid transform"); 
	PX_CHECK_AND_RETURN_NULL((actor0 && actor0->is<PxRigidBody>()) || (actor1 && actor1->is<PxRigidBody>()), "PxPrismaticJointCreate: at least one actor must be dynamic");
	PX_CHECK_AND_RETURN_NULL(actor0 != actor1, "PxPrismaticJointCreate: actors must be different");

	PrismaticJoint* j;
	PX_NEW_SERIALIZED(j,PrismaticJoint)(physics.getTolerancesScale(), actor0, localFrame0, actor1, localFrame1);

	if(j->attach(physics, actor0, actor1))
		return j;

	PX_DELETE(j);
	return NULL;
}

void PrismaticJoint::setProjectionAngularTolerance(PxReal tolerance)
{ 
	PX_CHECK_AND_RETURN(PxIsFinite(tolerance) && tolerance >=0 && tolerance <= PxPi, "PxPrismaticJoint::setProjectionAngularTolerance: invalid parameter");
	data().projectionAngularTolerance = tolerance;	
	markDirty();	
}

PxReal PrismaticJoint::getProjectionAngularTolerance() const
{ 
	return data().projectionAngularTolerance; 
}

void PrismaticJoint::setProjectionLinearTolerance(PxReal tolerance) 
{ 
	PX_CHECK_AND_RETURN(PxIsFinite(tolerance) && tolerance >=0, "PxPrismaticJoint::setProjectionLinearTolerance: invalid parameter");
	data().projectionLinearTolerance = tolerance;	
	markDirty(); 
}

PxReal PrismaticJoint::getProjectionLinearTolerance() const	
{ 
	return data().projectionLinearTolerance;		
}

PxPrismaticJointFlags PrismaticJoint::getPrismaticJointFlags(void) const
{ 
	return data().jointFlags;		
}

void PrismaticJoint::setPrismaticJointFlags(PxPrismaticJointFlags flags)
{ 
	data().jointFlags = flags; markDirty();	
}

void PrismaticJoint::setPrismaticJointFlag(PxPrismaticJointFlag::Enum flag, bool value)
{
	if(value)
		data().jointFlags |= flag;
	else
		data().jointFlags &= ~flag;
	markDirty();
}


PxJointLinearLimitPair PrismaticJoint::getLimit() const
{ 
	return data().limit;	
}

void PrismaticJoint::setLimit(const PxJointLinearLimitPair& limit)
{ 
	PX_CHECK_AND_RETURN(limit.isValid(), "PxPrismaticJoint::setLimit: invalid parameter");
	data().limit = limit;
	markDirty();
}



namespace
{


void PrismaticJointVisualize(PxConstraintVisualizer& viz,
							 const void* constantBlock,
							 const PxTransform& body0Transform,
							 const PxTransform& body1Transform,
							 PxU32 /*flags*/)
{
	const PrismaticJointData& data = *reinterpret_cast<const PrismaticJointData*>(constantBlock);

	const PxTransform& t0 = body0Transform * data.c2b[0];
	const PxTransform& t1 = body1Transform * data.c2b[1];

	viz.visualizeJointFrames(t0, t1);

	PxVec3 axis = t0.rotate(PxVec3(1.f,0,0));
	PxReal ordinate = axis.dot(t0.transformInv(t1.p)-t0.p);

	if(data.jointFlags & PxPrismaticJointFlag::eLIMIT_ENABLED)
	{
		viz.visualizeLinearLimit(t0, t1, data.limit.lower, ordinate < data.limit.lower + data.limit.contactDistance);
		viz.visualizeLinearLimit(t0, t1, data.limit.upper, ordinate > data.limit.upper - data.limit.contactDistance);
	}

}



void PrismaticJointProject(const void* constantBlock,
						   PxTransform& bodyAToWorld,
						   PxTransform& bodyBToWorld,
						   bool projectToA)
{
	using namespace joint;

	const PrismaticJointData& data = *reinterpret_cast<const PrismaticJointData*>(constantBlock);

	PxTransform cA2w, cB2w, cB2cA, projected;
	computeDerived(data, bodyAToWorld, bodyBToWorld, cA2w, cB2w, cB2cA);

	PxVec3 v(0,cB2cA.p.y,cB2cA.p.z);
	bool linearTrunc, angularTrunc;
	projected.p = truncateLinear(v, data.projectionLinearTolerance, linearTrunc);
	projected.q = truncateAngular(cB2cA.q, PxSin(data.projectionAngularTolerance/2), PxCos(data.projectionAngularTolerance/2), angularTrunc);
	
	if(linearTrunc || angularTrunc)
	{
		projected.p.x = cB2cA.p.x;
		projectTransforms(bodyAToWorld, bodyBToWorld, cA2w, cB2w, projected, data, projectToA);
	}
}
}

bool Ext::PrismaticJoint::attach(PxPhysics &physics, PxRigidActor* actor0, PxRigidActor* actor1)
{
	mPxConstraint = physics.createConstraint(actor0, actor1, *this, sShaders, sizeof(PrismaticJointData));
	return mPxConstraint!=NULL;
}

void PrismaticJoint::exportExtraData(PxSerializationContext& stream)
{
	if(mData)
	{
		stream.alignData(PX_SERIAL_ALIGN);
		stream.writeData(mData, sizeof(PrismaticJointData));
	}
	stream.writeName(mName);
}

void PrismaticJoint::importExtraData(PxDeserializationContext& context)
{
	if(mData)
		mData = context.readExtraData<PrismaticJointData, PX_SERIAL_ALIGN>();
	context.readName(mName);
}

void PrismaticJoint::resolveReferences(PxDeserializationContext& context)
{
	setPxConstraint(resolveConstraintPtr(context, getPxConstraint(), getConnector(), sShaders));	
}

PrismaticJoint* PrismaticJoint::createObject(PxU8*& address, PxDeserializationContext& context)
{
	PrismaticJoint* obj = new (address) PrismaticJoint(PxBaseFlag::eIS_RELEASABLE);
	address += sizeof(PrismaticJoint);	
	obj->importExtraData(context);
	obj->resolveReferences(context);
	return obj;
}

// global function to share the joint shaders with API capture	
const PxConstraintShaderTable* Ext::GetPrismaticJointShaderTable() 
{ 
	return &PrismaticJoint::getConstraintShaderTable();
}

//~PX_SERIALIZATION
PxConstraintShaderTable Ext::PrismaticJoint::sShaders = { Ext::PrismaticJointSolverPrep, PrismaticJointProject, PrismaticJointVisualize, PxConstraintFlag::Enum(0) };


