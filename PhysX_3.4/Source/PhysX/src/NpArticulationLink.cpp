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


#include "NpArticulationLink.h"
#include "NpArticulationJoint.h"
#include "NpWriteCheck.h"
#include "NpReadCheck.h"
#include "ScbArticulation.h"
#include "CmVisualization.h"
#include "CmConeLimitHelper.h"
#include "CmUtils.h"
#include "NpArticulation.h"

using namespace physx;

// PX_SERIALIZATION
void NpArticulationLink::requires(PxProcessPxBaseCallback& c)
{
	NpArticulationLinkT::requires(c);
	
	if(mInboundJoint)
		c.process(*mInboundJoint);
}

void NpArticulationLink::exportExtraData(PxSerializationContext& stream)
{
	NpArticulationLinkT::exportExtraData(stream);
	Cm::exportInlineArray(mChildLinks, stream);
}

void NpArticulationLink::importExtraData(PxDeserializationContext& context)
{
	NpArticulationLinkT::importExtraData(context);
	Cm::importInlineArray(mChildLinks, context);
}

void NpArticulationLink::resolveReferences(PxDeserializationContext& context)
{	
    context.translatePxBase(mRoot);
    context.translatePxBase(mInboundJoint);
    context.translatePxBase(mParent);
       
    NpArticulationLinkT::resolveReferences(context);

    const PxU32 nbLinks = mChildLinks.size();
    for(PxU32 i=0;i<nbLinks;i++)
        context.translatePxBase(mChildLinks[i]);
}

NpArticulationLink* NpArticulationLink::createObject(PxU8*& address, PxDeserializationContext& context)
{
	NpArticulationLink* obj = new (address) NpArticulationLink(PxBaseFlags(0));
	address += sizeof(NpArticulationLink);	
	obj->importExtraData(context);
	obj->resolveReferences(context);
	return obj;
}
//~PX_SERIALIZATION

NpArticulationLink::NpArticulationLink(const PxTransform& bodyPose, NpArticulation& root, NpArticulationLink* parent)
: NpArticulationLinkT(PxConcreteType::eARTICULATION_LINK, PxBaseFlag::eOWNS_MEMORY, PxActorType::eARTICULATION_LINK, bodyPose)
, mRoot(&root)
, mInboundJoint(NULL)
, mParent(parent)
{
	PX_ASSERT(mBody.getScbType() == ScbType::eBODY);
	mBody.setScbType(ScbType::eBODY_FROM_ARTICULATION_LINK);

	mRoot->addToLinkList(*this);

	if (parent)
		parent->addToChildList(*this);
}


NpArticulationLink::~NpArticulationLink()
{
}

void NpArticulationLink::releaseInternal()
{
	NpPhysics::getInstance().notifyDeletionListenersUserRelease(this, userData);

	NpArticulationLinkT::release();

	mRoot->removeLinkFromList(*this);

	if (mParent)
		mParent->removeFromChildList(*this);

	if (mInboundJoint)
		mInboundJoint->release();

	NpScene* npScene = NpActor::getAPIScene(*this);
	if (npScene)
		npScene->getScene().removeActor(mBody, true, false);

	mBody.destroy();
}


void NpArticulationLink::release()
{
	NP_WRITE_CHECK(NpActor::getOwnerScene(*this));

	PX_CHECK_AND_RETURN(mRoot->getRoot() != this || NpActor::getOwnerScene(*this) == NULL, "PxArticulationLink::release(): root link may not be released while articulation is in a scene");

	//! this function doesn't get called when the articulation root is released
	// therefore, put deregistration code etc. into dtor, not here

	if (mChildLinks.empty())
	{
		releaseInternal();
	}
	else
	{
		Ps::getFoundation().error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__, "PxArticulationLink::release(): Only leaf articulation links can be released. Release call failed");
	}
}



PxTransform NpArticulationLink::getGlobalPose() const
{
	NP_READ_CHECK(NpActor::getOwnerScene(*this));

	//!!!AL TODO: Need to start from root and compute along the branch to reflect double buffered state of root link
	return getScbBodyFast().getBody2World() * getScbBodyFast().getBody2Actor().getInverse();
}


PxArticulation& NpArticulationLink::getArticulation() const
{
	NP_READ_CHECK(NpActor::getOwnerScene(*this));
	return *mRoot;
}


PxArticulationJoint* NpArticulationLink::getInboundJoint() const
{
	NP_READ_CHECK(NpActor::getOwnerScene(*this));
	return mInboundJoint;
}


PxU32 NpArticulationLink::getNbChildren() const
{
	NP_READ_CHECK(NpActor::getOwnerScene(*this));
	return mChildLinks.size();
}


PxU32 NpArticulationLink::getChildren(PxArticulationLink** userBuffer, PxU32 bufferSize, PxU32 startIndex) const
{
	NP_READ_CHECK(NpActor::getOwnerScene(*this));
	return Cm::getArrayOfPointers(userBuffer, bufferSize, startIndex, mChildLinks.begin(), mChildLinks.size());
}


void NpArticulationLink::setCMassLocalPose(const PxTransform& pose)
{
	NP_WRITE_CHECK(NpActor::getOwnerScene(*this));
	PX_CHECK_AND_RETURN(pose.isSane(), "PxArticulationLink::setCMassLocalPose: invalid parameter");

	PxTransform p = pose.getNormalized();
	PxTransform oldpose = getScbBodyFast().getBody2Actor();
	PxTransform comShift = p.transformInv(oldpose);

	NpArticulationLinkT::setCMassLocalPoseInternal(p);

	if(mInboundJoint)
	{
		Scb::ArticulationJoint &j = mInboundJoint->getScbArticulationJoint();
		j.setChildPose(comShift.transform(j.getChildPose()));
	}

	for(PxU32 i=0; i<mChildLinks.size(); i++)
	{
		Scb::ArticulationJoint &j = static_cast<NpArticulationJoint*>(mChildLinks[i]->getInboundJoint())->getScbArticulationJoint();
		j.setParentPose(comShift.transform(j.getParentPose()));
	}
}


void NpArticulationLink::addForce(const PxVec3& force, PxForceMode::Enum mode, bool autowake)
{
	NpScene* scene = NpActor::getOwnerScene(*this);
	PX_UNUSED(scene);

	PX_CHECK_AND_RETURN(force.isFinite(), "NpArticulationLink::addForce: force is not valid.");
	NP_WRITE_CHECK(scene);
	PX_CHECK_AND_RETURN(scene, "NpArticulationLink::addForce: articulation link must be in a scene!");

	addSpatialForce(&force, 0, mode);

	mRoot->wakeUpInternal((!force.isZero()), autowake);
}


void NpArticulationLink::addTorque(const PxVec3& torque, PxForceMode::Enum mode, bool autowake)
{
	NpScene* scene = NpActor::getOwnerScene(*this);
	PX_UNUSED(scene);

	PX_CHECK_AND_RETURN(torque.isFinite(), "NpArticulationLink::addTorque: force is not valid.");
	NP_WRITE_CHECK(scene);
	PX_CHECK_AND_RETURN(scene, "NpArticulationLink::addTorque: articulation link must be in a scene!");

	addSpatialForce(0, &torque, mode);

	mRoot->wakeUpInternal((!torque.isZero()), autowake);
}

void NpArticulationLink::clearForce(PxForceMode::Enum mode)
{
	NpScene* scene = NpActor::getOwnerScene(*this);
	PX_UNUSED(scene);
	NP_WRITE_CHECK(scene);
	PX_CHECK_AND_RETURN(scene, "NpArticulationLink::clearForce: articulation link must be in a scene!");

	clearSpatialForce(mode, true, false);
}

void NpArticulationLink::clearTorque(PxForceMode::Enum mode)
{
	NpScene* scene = NpActor::getOwnerScene(*this);
	PX_UNUSED(scene);
	NP_WRITE_CHECK(scene);
	PX_CHECK_AND_RETURN(scene, "NpArticulationLink::clearTorque: articulation link must be in a scene!");

	clearSpatialForce(mode, false, true);
}

void NpArticulationLink::setGlobalPose(const PxTransform& pose)
{
	// clow: no need to test inputs here, it's done in the setGlobalPose function already
	setGlobalPose(pose, true);
}

void NpArticulationLink::setGlobalPose(const PxTransform& pose, bool autowake)
{
	NpScene* scene = NpActor::getOwnerScene(*this);

	PX_CHECK_AND_RETURN(pose.isValid(), "NpArticulationLink::setGlobalPose pose is not valid.");

	NP_WRITE_CHECK(scene);
	
#if PX_CHECKED
	if(scene)
		scene->checkPositionSanity(*this, pose, "PxArticulationLink::setGlobalPose");
#endif

	PxTransform body2World = pose * getScbBodyFast().getBody2Actor();
	getScbBodyFast().setBody2World(body2World, false);

	if (scene && autowake)
		mRoot->wakeUpInternal(false, true);
}


void NpArticulationLink::setLinearVelocity(const PxVec3& velocity, bool autowake)
{
	NpScene* scene = NpActor::getOwnerScene(*this);

	PX_CHECK_AND_RETURN(velocity.isFinite(), "NpArticulationLink::setLinearVelocity velocity is not valid.");

	NP_WRITE_CHECK(scene);
	
	getScbBodyFast().setLinearVelocity(velocity);

	if (scene)
		mRoot->wakeUpInternal((!velocity.isZero()), autowake);
}


void NpArticulationLink::setAngularVelocity(const PxVec3& velocity, bool autowake)
{
	NpScene* scene = NpActor::getOwnerScene(*this);

	PX_CHECK_AND_RETURN(velocity.isFinite(), "NpArticulationLink::setAngularVelocity velocity is not valid.");

	NP_WRITE_CHECK(scene);

	getScbBodyFast().setAngularVelocity(velocity);

	if (scene)
		mRoot->wakeUpInternal((!velocity.isZero()), autowake);
}


#if PX_ENABLE_DEBUG_VISUALIZATION
void NpArticulationLink::visualize(Cm::RenderOutput& out, NpScene* scene)
{
	NpArticulationLinkT::visualize(out, scene);

	if (getScbBodyFast().getActorFlags() & PxActorFlag::eVISUALIZATION)
	{
		PX_ASSERT(getScene());
		PxReal scale = getScene()->getVisualizationParameter(PxVisualizationParameter::eSCALE);

		PxReal massAxes = scale * getScene()->getVisualizationParameter(PxVisualizationParameter::eBODY_MASS_AXES);
		if (massAxes != 0)
		{
			PxU32 color = 0xff;
			color = (color<<16 | color<<8 | color);
			PxVec3 dims = invertDiagInertia(getScbBodyFast().getInverseInertia());
			dims = getDimsFromBodyInertia(dims, 1.0f / getScbBodyFast().getInverseMass());

			out << color << getScbBodyFast().getBody2World() << Cm::DebugBox(dims * 0.5f);
		}	
		PxReal frameScale = scale * getScene()->getVisualizationParameter(PxVisualizationParameter::eJOINT_LOCAL_FRAMES);
		PxReal limitScale = scale * getScene()->getVisualizationParameter(PxVisualizationParameter::eJOINT_LIMITS);
		if ( frameScale != 0.0f || limitScale != 0.0f )
		{
			Cm::ConstraintImmediateVisualizer viz( frameScale, limitScale, out );
			visualizeJoint( viz );
		}
	}
}

void NpArticulationLink::visualizeJoint(PxConstraintVisualizer& jointViz)
{
	NpArticulationLink* parent = getParent();
	if(parent)
	{
		PxTransform cA2w = getGlobalPose().transform(mInboundJoint->getChildPose());
		PxTransform cB2w = parent->getGlobalPose().transform(mInboundJoint->getParentPose());
	
		jointViz.visualizeJointFrames(cA2w, cB2w);

		PxTransform parentFrame = cB2w;

		if(cA2w.q.dot(cB2w.q)<0)
			cB2w.q = -cB2w.q;

		PxTransform cB2cA = cA2w.transformInv(cB2w);	

		PxQuat swing, twist;
		Ps::separateSwingTwist(cB2cA.q,swing,twist);

		PxMat33 cA2w_m(cA2w.q), cB2w_m(cB2w.q);
	
		PxReal tqPhi = Ps::tanHalf(twist.x, twist.w);		// always support (-pi, +pi)
		
		PxReal lower, upper, yLimit, zLimit;

		mInboundJoint->getTwistLimit(lower, upper);
		mInboundJoint->getSwingLimit(yLimit, zLimit);
		PxReal swingPad = mInboundJoint->getSwingLimitContactDistance(), twistPad = mInboundJoint->getTwistLimitContactDistance();
		jointViz.visualizeAngularLimit(parentFrame, lower, upper, PxAbs(tqPhi) > PxTan(upper-twistPad));

		PxVec3 tanQSwing = PxVec3(0, Ps::tanHalf(swing.z,swing.w), -Ps::tanHalf(swing.y,swing.w));
		Cm::ConeLimitHelper coneHelper(PxTan(yLimit/4), PxTan(zLimit/4), PxTan(swingPad/4));
		jointViz.visualizeLimitCone(parentFrame, PxTan(yLimit/4), PxTan(zLimit/4), !coneHelper.contains(tanQSwing));
	}	
}
#endif  // PX_ENABLE_DEBUG_VISUALIZATION
