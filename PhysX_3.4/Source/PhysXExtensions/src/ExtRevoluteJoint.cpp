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


#include "ExtRevoluteJoint.h"
#include "PsUtilities.h"
#include "ExtConstraintHelper.h"
#include "CmRenderOutput.h"
#include "PsMathUtils.h"
#include "CmVisualization.h"
#include "CmUtils.h"

#include "common/PxSerialFramework.h"

using namespace physx;
using namespace Ext;

namespace physx
{
	PxRevoluteJoint* PxRevoluteJointCreate(PxPhysics& physics,
		PxRigidActor* actor0, const PxTransform& localFrame0,
		PxRigidActor* actor1, const PxTransform& localFrame1);
}

PxRevoluteJoint* physx::PxRevoluteJointCreate(PxPhysics& physics,
											  PxRigidActor* actor0, const PxTransform& localFrame0,
											  PxRigidActor* actor1, const PxTransform& localFrame1)
{
	PX_CHECK_AND_RETURN_NULL(localFrame0.isSane(), "PxRevoluteJointCreate: local frame 0 is not a valid transform"); 
	PX_CHECK_AND_RETURN_NULL(localFrame1.isSane(), "PxRevoluteJointCreate: local frame 1 is not a valid transform"); 
	PX_CHECK_AND_RETURN_NULL(actor0 != actor1, "PxRevoluteJointCreate: actors must be different");
	PX_CHECK_AND_RETURN_NULL((actor0 && actor0->is<PxRigidBody>()) || (actor1 && actor1->is<PxRigidBody>()), "PxRevoluteJointCreate: at least one actor must be dynamic");

	RevoluteJoint* j;
	PX_NEW_SERIALIZED(j,RevoluteJoint)(physics.getTolerancesScale(), actor0, localFrame0, actor1, localFrame1);
	if(j->attach(physics, actor0, actor1))
		return j;

	PX_DELETE(j);
	return NULL;
}


PxReal RevoluteJoint::getAngle() const
{
	PxQuat q = getRelativeTransform().q, swing, twist;
	Ps::separateSwingTwist(q, swing, twist);
	PxReal angle = twist.getAngle();
	if(q.x<0)
		angle = 2*PxPi - angle;
	if(angle>PxPi)
		angle-=2*PxPi;
	return angle;
	
}

PxReal RevoluteJoint::getVelocity() const
{
	return getRelativeAngularVelocity().magnitude();
}



PxJointAngularLimitPair RevoluteJoint::getLimit()	const
{ 
	return data().limit;	
}

void RevoluteJoint::setLimit(const PxJointAngularLimitPair& limit)
{ 
	PX_CHECK_AND_RETURN(limit.isValid(), "PxRevoluteJoint::setLimit: limit invalid");
	PX_CHECK_AND_RETURN(limit.lower>-PxPi && limit.upper<PxPi , "PxRevoluteJoint::twist limit must be strictly -*PI and PI");
	PX_CHECK_AND_RETURN(limit.upper - limit.lower < PxTwoPi, "PxRevoluteJoint::twist limit range must be strictly less than 2*PI");
	data().limit = limit; 
	markDirty();	
}

PxReal RevoluteJoint::getDriveVelocity() const
{ 
	return data().driveVelocity;	
}

void RevoluteJoint::setDriveVelocity(PxReal velocity)
{ 
	PX_CHECK_AND_RETURN(PxIsFinite(velocity), "PxRevoluteJoint::setDriveVelocity: invalid parameter");
	data().driveVelocity = velocity; 
	markDirty(); 
}

PxReal RevoluteJoint::getDriveForceLimit() const
{ 
	return data().driveForceLimit;	
}

void RevoluteJoint::setDriveForceLimit(PxReal forceLimit)
{ 
	PX_CHECK_AND_RETURN(PxIsFinite(forceLimit), "PxRevoluteJoint::setDriveForceLimit: invalid parameter");
	data().driveForceLimit = forceLimit; 
	markDirty(); 
}

PxReal RevoluteJoint::getDriveGearRatio() const
{ 
	return data().driveGearRatio;	
}

void RevoluteJoint::setDriveGearRatio(PxReal gearRatio)
{ 
	PX_CHECK_AND_RETURN(PxIsFinite(gearRatio) && gearRatio>0, "PxRevoluteJoint::setDriveGearRatio: invalid parameter");
	data().driveGearRatio = gearRatio; 
	markDirty(); 
}

void RevoluteJoint::setProjectionAngularTolerance(PxReal tolerance)
{ 
	PX_CHECK_AND_RETURN(PxIsFinite(tolerance) && tolerance>=0 && tolerance<=PxPi, "PxRevoluteJoint::setProjectionAngularTolerance: invalid parameter");
	data().projectionAngularTolerance = tolerance;
	markDirty();	
}

PxReal RevoluteJoint::getProjectionAngularTolerance() const	
{ 
	return data().projectionAngularTolerance; 
}

void RevoluteJoint::setProjectionLinearTolerance(PxReal tolerance)
{ 
	PX_CHECK_AND_RETURN(PxIsFinite(tolerance) && tolerance >=0, "PxRevoluteJoint::setProjectionLinearTolerance: invalid parameter");
	data().projectionLinearTolerance = tolerance;
	markDirty(); 
}

PxReal RevoluteJoint::getProjectionLinearTolerance() const
{ 
	return data().projectionLinearTolerance;		
}

PxRevoluteJointFlags RevoluteJoint::getRevoluteJointFlags(void)	const
{ 
	return data().jointFlags; 
}

void RevoluteJoint::setRevoluteJointFlags(PxRevoluteJointFlags flags)
{ 
	data().jointFlags = flags; 
}

void RevoluteJoint::setRevoluteJointFlag(PxRevoluteJointFlag::Enum flag, bool value)
{
	if(value)
		data().jointFlags |= flag;
	else
		data().jointFlags &= ~flag;
	markDirty();
}




void* Ext::RevoluteJoint::prepareData()
{
	data().tqHigh =  PxTan(data().limit.upper/4);
	data().tqLow = PxTan(data().limit.lower/4);
	data().tqPad = PxTan(data().limit.contactDistance/4);

	return RevoluteJointT::prepareData();
}


namespace
{

void RevoluteJointProject(const void* constantBlock,
						  PxTransform& bodyAToWorld,
						  PxTransform& bodyBToWorld,
						  bool projectToA)
{
	using namespace joint;

	const RevoluteJointData& data = *reinterpret_cast<const RevoluteJointData*>(constantBlock);

	PxTransform cA2w, cB2w, cB2cA, projected;
	computeDerived(data, bodyAToWorld, bodyBToWorld, cA2w, cB2w, cB2cA);

	bool linearTrunc, angularTrunc;
	projected.p = truncateLinear(cB2cA.p, data.projectionLinearTolerance, linearTrunc);

	PxQuat swing, twist, projSwing;
	Ps::separateSwingTwist(cB2cA.q,swing,twist);
	projSwing = truncateAngular(swing, PxSin(data.projectionAngularTolerance/2), PxCos(data.projectionAngularTolerance/2), angularTrunc);
	
	if(linearTrunc || angularTrunc)
	{
		projected.q = projSwing * twist;
		projectTransforms(bodyAToWorld, bodyBToWorld, cA2w, cB2w, projected, data, projectToA);
	}
}

void RevoluteJointVisualize(PxConstraintVisualizer& viz,
		 					const void* constantBlock,
							const PxTransform& body0Transform,
							const PxTransform& body1Transform,
							PxU32 flags)
{
	const RevoluteJointData& data = *reinterpret_cast<const RevoluteJointData*>(constantBlock);

	const PxTransform& t0 = body0Transform * data.c2b[0];
	const PxTransform& t1 = body1Transform * data.c2b[1];

	if(flags & PxConstraintVisualizationFlag::eLOCAL_FRAMES)
		viz.visualizeJointFrames(t0, t1);

	if((flags & PxConstraintVisualizationFlag::eLIMITS) && (data.jointFlags & PxRevoluteJointFlag::eLIMIT_ENABLED))
	{
		// PT: TODO: refactor this with the solver prep code
		PxQuat cB2cAq = t0.q.getConjugate() * t1.q;
		PxQuat twist(cB2cAq.x,0,0,cB2cAq.w);

		PxReal magnitude = twist.normalize();
		PxReal tqPhi = physx::intrinsics::fsel(magnitude - 1e-6f, twist.x / (1.0f + twist.w), 0.f);

		PxReal quarterAngle = tqPhi;
		PxReal lower = data.tqLow;
		PxReal upper = data.tqHigh;
		PxReal pad = data.tqPad;

		if(data.limit.isSoft())
			pad = 0;

		bool active = false;
		PX_ASSERT(lower<upper);
		if(quarterAngle < lower+pad)
			active = true;
		if(quarterAngle > upper-pad)
			active = true;

		viz.visualizeAngularLimit(t0, data.limit.lower, data.limit.upper, active);
	}
}
}

bool Ext::RevoluteJoint::attach(PxPhysics &physics, PxRigidActor* actor0, PxRigidActor* actor1)
{
	mPxConstraint = physics.createConstraint(actor0, actor1, *this, sShaders, sizeof(RevoluteJointData));
	return mPxConstraint!=NULL;
}

void RevoluteJoint::exportExtraData(PxSerializationContext& stream)
{
	if(mData)
	{
		stream.alignData(PX_SERIAL_ALIGN);
		stream.writeData(mData, sizeof(RevoluteJointData));
	}
	stream.writeName(mName);
}

void RevoluteJoint::importExtraData(PxDeserializationContext& context)
{
	if(mData)
		mData = context.readExtraData<RevoluteJointData, PX_SERIAL_ALIGN>();
	context.readName(mName);
}

void RevoluteJoint::resolveReferences(PxDeserializationContext& context)
{
	setPxConstraint(resolveConstraintPtr(context, getPxConstraint(), getConnector(), sShaders));	
}

RevoluteJoint* RevoluteJoint::createObject(PxU8*& address, PxDeserializationContext& context)
{
	RevoluteJoint* obj = new (address) RevoluteJoint(PxBaseFlag::eIS_RELEASABLE);
	address += sizeof(RevoluteJoint);	
	obj->importExtraData(context);
	obj->resolveReferences(context);
	return obj;
}

// global function to share the joint shaders with API capture	
const PxConstraintShaderTable* Ext::GetRevoluteJointShaderTable() 
{ 
	return &RevoluteJoint::getConstraintShaderTable();
}

//~PX_SERIALIZATION
PxConstraintShaderTable Ext::RevoluteJoint::sShaders = { Ext::RevoluteJointSolverPrep, RevoluteJointProject, RevoluteJointVisualize, PxConstraintFlag::Enum(0) };
