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


#include "NpCast.h"
#include "NpWriteCheck.h"
#include "NpReadCheck.h"

namespace physx
{

// PX_SERIALIZATION
void NpArticulationJoint::resolveReferences(PxDeserializationContext& context)
{
	context.translatePxBase(mParent);
	context.translatePxBase(mChild);
	// temporary 3.3.1 -> 3.3.2 backwards compatibility patch; remove in 3.4.
	PxU32 version = context.getPhysXVersion();
	if(version < 0x03030200)
		mJoint.getScArticulationJoint().setDriveType(PxArticulationJointDriveType::eTARGET);
}

NpArticulationJoint* NpArticulationJoint::createObject(PxU8*& address, PxDeserializationContext& context)
{
	NpArticulationJoint* obj = new (address) NpArticulationJoint(PxBaseFlags(0));
	address += sizeof(NpArticulationJoint);	
	obj->importExtraData(context);
	obj->resolveReferences(context);
	return obj;
}
// ~PX_SERIALIZATION

NpArticulationJoint::NpArticulationJoint(NpArticulationLink& parent, 
										 const PxTransform& parentFrame,
										 NpArticulationLink& child, 
										 const PxTransform& childFrame) 
: PxArticulationJoint(PxConcreteType::eARTICULATION_JOINT, PxBaseFlag::eOWNS_MEMORY)
, mJoint(parentFrame, childFrame)
, mParent(&parent)
, mChild(&child)
{}


NpArticulationJoint::~NpArticulationJoint()
{
}


void NpArticulationJoint::release()
{
	NpPhysics::getInstance().notifyDeletionListenersUserRelease(this, NULL);

	if (mJoint.getScbSceneForAPI())
		mJoint.getScbSceneForAPI()->removeArticulationJoint(mJoint);

	mJoint.destroy();
}



PxTransform NpArticulationJoint::getParentPose() const
{
	NP_READ_CHECK(getOwnerScene());

	return mParent->getCMassLocalPose().transform(mJoint.getParentPose());
}

void NpArticulationJoint::setParentPose(const PxTransform& t)
{
	PX_CHECK_AND_RETURN(t.isSane(), "NpArticulationJoint::setParentPose t is not valid.");
	
	NP_WRITE_CHECK(getOwnerScene());
	if(mParent==NULL)
		return;

	mJoint.setParentPose(mParent->getCMassLocalPose().transformInv(t.getNormalized()));
}



PxTransform NpArticulationJoint::getChildPose() const
{
	NP_READ_CHECK(getOwnerScene());

	return mChild->getCMassLocalPose().transform(mJoint.getChildPose());
}

void NpArticulationJoint::setChildPose(const PxTransform& t)
{
	PX_CHECK_AND_RETURN(t.isSane(), "NpArticulationJoint::setChildPose t is not valid.");

	NP_WRITE_CHECK(getOwnerScene());

	mJoint.setChildPose(mChild->getCMassLocalPose().transformInv(t.getNormalized()));
}



void NpArticulationJoint::setTargetOrientation(const PxQuat& p)
{
	PX_CHECK_AND_RETURN(mJoint.getDriveType() != PxArticulationJointDriveType::eTARGET || p.isUnit(), "NpArticulationJoint::setTargetOrientation, quat orientation is not valid.");
	PX_CHECK_AND_RETURN(mJoint.getDriveType() != PxArticulationJointDriveType::eERROR || (p.getImaginaryPart().isFinite() && p.w == 0), "NpArticulationJoint::setTargetOrientation rotation vector orientation is not valid.");

	NP_WRITE_CHECK(getOwnerScene());

	mJoint.setTargetOrientation(p);
}


PxQuat NpArticulationJoint::getTargetOrientation() const
{
	NP_READ_CHECK(getOwnerScene());

	return mJoint.getTargetOrientation();
}


void NpArticulationJoint::setTargetVelocity(const PxVec3& v)
{
	PX_CHECK_AND_RETURN(v.isFinite(), "NpArticulationJoint::setTargetVelocity v is not valid.");

	NP_WRITE_CHECK(getOwnerScene());

	mJoint.setTargetVelocity(v);
}


PxArticulationJointDriveType::Enum	NpArticulationJoint::getDriveType() const
{
	NP_READ_CHECK(getOwnerScene());
	return mJoint.getDriveType();
}

void  NpArticulationJoint::setDriveType(PxArticulationJointDriveType::Enum driveType)
{
	NP_WRITE_CHECK(getOwnerScene());
	mJoint.setDriveType(driveType);
}


PxVec3 NpArticulationJoint::getTargetVelocity() const
{
	NP_READ_CHECK(getOwnerScene());

	return mJoint.getTargetVelocity();
}


void NpArticulationJoint::setStiffness(PxReal s)
{
	PX_CHECK_AND_RETURN(PxIsFinite(s) && s >= 0.0f, "PxArticulationJoint::setStiffness: spring coefficient must be >= 0!");

	NP_WRITE_CHECK(getOwnerScene());

	mJoint.setStiffness(s);
}


PxReal NpArticulationJoint::getStiffness() const
{
	NP_READ_CHECK(getOwnerScene());

	return mJoint.getStiffness();
}


void NpArticulationJoint::setDamping(PxReal d)
{
	PX_CHECK_AND_RETURN(PxIsFinite(d) && d >= 0.0f, "PxArticulationJoint::setDamping: damping coefficient must be >= 0!");

	NP_WRITE_CHECK(getOwnerScene());

	mJoint.setDamping(d);
}


PxReal NpArticulationJoint::getDamping() const
{
	NP_READ_CHECK(getOwnerScene());

	return mJoint.getDamping();
}


void NpArticulationJoint::setSwingLimitContactDistance(PxReal d)
{
	PX_CHECK_AND_RETURN(PxIsFinite(d) && d >= 0.0f, "PxArticulationJoint::setSwingLimitContactDistance: padding coefficient must be > 0!");

	NP_WRITE_CHECK(getOwnerScene());

	mJoint.setSwingLimitContactDistance(d);
}


PxReal NpArticulationJoint::getSwingLimitContactDistance() const
{
	NP_READ_CHECK(getOwnerScene());

	return mJoint.getSwingLimitContactDistance();
}


void NpArticulationJoint::setTwistLimitContactDistance(PxReal d)
{
	PX_CHECK_AND_RETURN(PxIsFinite(d) && d >= 0.0f, "PxArticulationJoint::setTwistLimitContactDistance: padding coefficient must be > 0!");

	NP_WRITE_CHECK(getOwnerScene());

	mJoint.setTwistLimitContactDistance(d);
}


PxReal NpArticulationJoint::getTwistLimitContactDistance() const
{
	NP_READ_CHECK(getOwnerScene());

	return mJoint.getTwistLimitContactDistance();
}


void NpArticulationJoint::setInternalCompliance(PxReal r)
{
	PX_CHECK_AND_RETURN(PxIsFinite(r) && r>0, "PxArticulationJoint::setInternalCompliance: compliance must be > 0");

	NP_WRITE_CHECK(getOwnerScene());

	mJoint.setInternalCompliance(r);
}



PxReal NpArticulationJoint::getInternalCompliance() const
{
	NP_READ_CHECK(getOwnerScene());

	return mJoint.getInternalCompliance();
}


void NpArticulationJoint::setExternalCompliance(PxReal r)
{
	PX_CHECK_AND_RETURN(PxIsFinite(r) && r>0, "PxArticulationJoint::setExternalCompliance: compliance must be > 0");
	NP_WRITE_CHECK(getOwnerScene());

	mJoint.setExternalCompliance(r);
}


PxReal NpArticulationJoint::getExternalCompliance() const
{
	NP_READ_CHECK(getOwnerScene());

	return mJoint.getExternalCompliance();
}


void NpArticulationJoint::setSwingLimit(PxReal yLimit, PxReal zLimit)
{
	PX_CHECK_AND_RETURN(PxIsFinite(yLimit) && PxIsFinite(zLimit) && yLimit > 0 && zLimit > 0 && yLimit < PxPi && zLimit < PxPi, 
		"PxArticulationJoint::setSwingLimit: values must be >0 and < Pi");
	NP_WRITE_CHECK(getOwnerScene());

	mJoint.setSwingLimit(yLimit, zLimit);
}


void NpArticulationJoint::getSwingLimit(PxReal &yLimit, PxReal &zLimit) const
{
	NP_READ_CHECK(getOwnerScene());

	mJoint.getSwingLimit(yLimit, zLimit);
}


void NpArticulationJoint::setTangentialStiffness(PxReal stiffness)
{
	PX_CHECK_AND_RETURN(PxIsFinite(stiffness) && stiffness >= 0, "PxArticulationJoint::setTangentialStiffness: stiffness must be > 0");

	NP_WRITE_CHECK(getOwnerScene());

	mJoint.setTangentialStiffness(stiffness);
}


PxReal NpArticulationJoint::getTangentialStiffness() const
{
	NP_READ_CHECK(getOwnerScene());

	return mJoint.getTangentialStiffness();
}


void NpArticulationJoint::setTangentialDamping(PxReal damping)
{
	PX_CHECK_AND_RETURN(PxIsFinite(damping) && damping >= 0, "PxArticulationJoint::setTangentialDamping: damping must be > 0");
	NP_WRITE_CHECK(getOwnerScene());

	mJoint.setTangentialDamping(damping);
}


PxReal NpArticulationJoint::getTangentialDamping() const
{
	NP_READ_CHECK(getOwnerScene());

	return mJoint.getTangentialDamping();
}



void NpArticulationJoint::setSwingLimitEnabled(bool e)
{
	NP_WRITE_CHECK(getOwnerScene());

	mJoint.setSwingLimitEnabled(e);
}


bool NpArticulationJoint::getSwingLimitEnabled() const
{
	NP_READ_CHECK(getOwnerScene());

	return mJoint.getSwingLimitEnabled();
}


void NpArticulationJoint::setTwistLimit(PxReal lower, PxReal upper)
{
	PX_CHECK_AND_RETURN(PxIsFinite(lower) && PxIsFinite(upper) && lower<upper && lower>-PxPi && upper < PxPi, "PxArticulationJoint::setTwistLimit: illegal parameters");

	NP_WRITE_CHECK(getOwnerScene());

	mJoint.setTwistLimit(lower, upper);
}


void NpArticulationJoint::getTwistLimit(PxReal &lower, PxReal &upper) const
{
	NP_READ_CHECK(getOwnerScene());

	mJoint.getTwistLimit(lower, upper);
}


void NpArticulationJoint::setTwistLimitEnabled(bool e)
{
	NP_WRITE_CHECK(getOwnerScene());

	mJoint.setTwistLimitEnabled(e);
}


bool NpArticulationJoint::getTwistLimitEnabled() const
{
	NP_READ_CHECK(getOwnerScene());

	return mJoint.getTwistLimitEnabled();
}


NpScene* NpArticulationJoint::getOwnerScene() const
{
	return mJoint.getScbScene() ? static_cast<NpScene *>(mJoint.getScbScene()->getPxScene()) : NULL;
}


void NpArticulationJointGetBodiesFromScb(Scb::ArticulationJoint&c, Scb::Body*&b0, Scb::Body*&b1)
{
	NpArticulationJoint* np = const_cast<NpArticulationJoint*>(getNpArticulationJoint(&c));

	NpArticulationLink& a0 = np->getParent(), & a1 = np->getChild();
	b0 = &a0.getScbBodyFast();
	b1 = &a1.getScbBodyFast();
}


}
