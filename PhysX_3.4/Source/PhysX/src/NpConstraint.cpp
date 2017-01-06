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


#include "PxConstraint.h"
#include "NpConstraint.h"
#include "NpPhysics.h"
#include "NpRigidDynamic.h"
#include "NpRigidStatic.h"
#include "NpArticulationLink.h"
#include "ScbConstraint.h"
#include "ScbNpDeps.h"

using namespace physx;

void NpConstraint::setConstraintFunctions(PxConstraintConnector& n, const PxConstraintShaderTable& shaders)
{
	mConstraint.getScConstraint().setConstraintFunctions(n, shaders);
	
	//update mConnectorArray, since mActor0 or mActor1 should be in external reference
	bool bNeedUpdate = false;
	if(mActor0)
	{
		NpActor& npActor = NpActor::getFromPxActor(*mActor0);
		if(npActor.findConnector(NpConnectorType::eConstraint, this) == 0xffffffff)
		{
			bNeedUpdate = true;	
			npActor.addConnector(NpConnectorType::eConstraint, this, "PxConstraint: Add to rigid actor 0: Constraint already added");
		}
	}

	if(mActor1)
	{
		NpActor& npActor = NpActor::getFromPxActor(*mActor1);
		if(npActor.findConnector(NpConnectorType::eConstraint, this) == 0xffffffff)
		{
			bNeedUpdate = true;	
			npActor.addConnector(NpConnectorType::eConstraint, this, "PxConstraint: Add to rigid actor 1: Constraint already added");
		}
	}

	if(bNeedUpdate)
	{
		NpScene* newScene = getSceneFromActors(mActor0, mActor1);
		NpScene* oldScene = getNpScene();

		if (oldScene != newScene)
		{
			if (oldScene)
			{
				oldScene->removeFromConstraintList(*this);
				oldScene->getScene().removeConstraint(getScbConstraint());
			}
			if (newScene)
			{
				newScene->addToConstraintList(*this);
				newScene->getScene().addConstraint(getScbConstraint());
			}
		}
	}
}

NpConstraint::NpConstraint(PxRigidActor* actor0, PxRigidActor* actor1, PxConstraintConnector& connector, const PxConstraintShaderTable& shaders, PxU32 dataSize)
:	PxConstraint(PxConcreteType::eCONSTRAINT, PxBaseFlag::eOWNS_MEMORY)
,	mActor0		(actor0)
,	mActor1		(actor1)
,	mConstraint	(connector, shaders, dataSize)
,	mIsDirty	(true)
{
	
	mConstraint.setFlags(shaders.flag);
	if(actor0)
		NpActor::getFromPxActor(*actor0).addConnector(NpConnectorType::eConstraint, this, "PxConstraint: Add to rigid actor 0: Constraint already added");
	if(actor1)
		NpActor::getFromPxActor(*actor1).addConnector(NpConnectorType::eConstraint, this, "PxConstraint: Add to rigid actor 1: Constraint already added");

	NpScene* s = getSceneFromActors(actor0, actor1);
	if (s)
	{
		s->addToConstraintList(*this);
		s->getScene().addConstraint(mConstraint);
	}
}


NpConstraint::~NpConstraint()
{
	if(getBaseFlags()&PxBaseFlag::eOWNS_MEMORY)
		mConstraint.getPxConnector()->onConstraintRelease();

	NpFactory::getInstance().onConstraintRelease(this);
}

void NpConstraint::release()
{
	NpScene* npScene = getNpScene();
	NP_WRITE_CHECK(npScene);

	NpPhysics::getInstance().notifyDeletionListenersUserRelease(this, NULL);

	if(mActor0)
		NpActor::getFromPxActor(*mActor0).removeConnector(*mActor0, NpConnectorType::eConstraint, this, "PxConstraint: Add to rigid actor 0: Constraint already added");
	if(mActor1)
		NpActor::getFromPxActor(*mActor1).removeConnector(*mActor1, NpConnectorType::eConstraint, this, "PxConstraint: Add to rigid actor 1: Constraint already added");

	if (npScene)
	{
		npScene->removeFromConstraintList(*this);
		npScene->getScene().removeConstraint(getScbConstraint());
	}

	mConstraint.destroy();
}

// PX_SERIALIZATION
void NpConstraint::resolveReferences(PxDeserializationContext& context)
{	
	context.translatePxBase(mActor0);
	context.translatePxBase(mActor1);
}

NpConstraint* NpConstraint::createObject(PxU8*& address, PxDeserializationContext& context)
{
	NpConstraint* obj = new (address) NpConstraint(PxBaseFlags(0));
	address += sizeof(NpConstraint);	
	obj->importExtraData(context);
	obj->resolveReferences(context);
	return obj;
}

// ~PX_SERIALIZATION

PxScene* NpConstraint::getScene() const
{
	return getNpScene();
}

void NpConstraint::getActors(PxRigidActor*& actor0, PxRigidActor*& actor1) const
{
	NP_READ_CHECK(getNpScene());
	actor0 = mActor0;
	actor1 = mActor1;
}

void NpConstraint::setActors(PxRigidActor* actor0, PxRigidActor* actor1)
{
	NP_WRITE_CHECK(getNpScene());

	PX_CHECK_AND_RETURN((actor0 && !actor0->is<PxRigidStatic>()) || (actor1 && !actor1->is<PxRigidStatic>()), "PxConstraint: at least one actor must be non-static");
	PX_SIMD_GUARD;

	if(mActor0)
		NpActor::getFromPxActor(*mActor0).removeConnector(*mActor0, NpConnectorType::eConstraint, this, "PxConstraint: Add to rigid actor 0: Constraint already added");
	if(mActor1)
		NpActor::getFromPxActor(*mActor1).removeConnector(*mActor1, NpConnectorType::eConstraint, this, "PxConstraint: Add to rigid actor 1: Constraint already added");

	if(actor0)
		NpActor::getFromPxActor(*actor0).addConnector(NpConnectorType::eConstraint, this, "PxConstraint: Add to rigid actor 0: Constraint already added");
	if(actor1)
		NpActor::getFromPxActor(*actor1).addConnector(NpConnectorType::eConstraint, this, "PxConstraint: Add to rigid actor 1: Constraint already added");

	mActor0 = actor0;
	mActor1 = actor1;

	NpScene* newScene = getSceneFromActors(actor0, actor1);
	NpScene* oldScene = getNpScene();

	if (oldScene != newScene)
	{
		if (oldScene)
		{
			oldScene->removeFromConstraintList(*this);
			oldScene->getScene().removeConstraint(getScbConstraint());
		}

		getScbConstraint().setBodies(getScbRigidObject(actor0), getScbRigidObject(actor1));

		if (newScene)
		{
			newScene->addToConstraintList(*this);
			newScene->getScene().addConstraint(getScbConstraint());
		}
	}
	else
		getScbConstraint().setBodies(getScbRigidObject(actor0), getScbRigidObject(actor1));
}

void NpConstraint::markDirty()
{
	mIsDirty = true;
}

void NpConstraint::setFlags(PxConstraintFlags flags)
{
	NP_WRITE_CHECK(getNpScene());
	// check for eBROKEN which is a read only flag
	PX_CHECK_AND_RETURN(!(flags & PxConstraintFlag::eBROKEN), "PxConstraintFlag::eBROKEN is a read only flag");

	PX_CHECK_AND_RETURN(!(flags & PxConstraintFlag::eGPU_COMPATIBLE), "PxConstraintFlag::eGPU_COMPATIBLE is an internal flag and is illegal to set via the API");

	PX_SIMD_GUARD;

	mConstraint.setFlags(flags);
}

PxConstraintFlags NpConstraint::getFlags() const
{
	NP_READ_CHECK(getNpScene());
	return mConstraint.getFlags();
}


void NpConstraint::setFlag(PxConstraintFlag::Enum flag, bool value)
{
	NP_WRITE_CHECK(getNpScene());

	// check for eBROKEN which is a read only flag
	PX_CHECK_AND_RETURN(flag != PxConstraintFlag::eBROKEN, "PxConstraintFlag::eBROKEN is a read only flag");

	PX_CHECK_AND_RETURN(flag != PxConstraintFlag::eGPU_COMPATIBLE, "PxConstraintFlag::eGPU_COMPATIBLE is an internal flag and is illegal to set via the API");

	PX_SIMD_GUARD;

	PxConstraintFlags f = mConstraint.getFlags();

	mConstraint.setFlags(value ? f|flag : f&~flag);
}


void NpConstraint::getForce(PxVec3& linear, PxVec3& angular) const
{
	NP_READ_CHECK(getNpScene());
	mConstraint.getForce(linear, angular);
}
	
void NpConstraint::updateConstants()
{
	if(!mIsDirty)
		return;

	//ML - we can't just set dirty to false because this fails with constraints that were created while the scene was buffering!
	if(mConstraint.updateConstants(mConstraint.getPxConnector()->prepareData()))
		mIsDirty = false;
#if PX_SUPPORT_PVD
	Scb::Scene* scbScene = mConstraint.getScbSceneForAPI();
	//Changed to use the visual scenes update system which respects
	//the debugger's connection type flag.
	if( scbScene && !scbScene->isPhysicsBuffering() )
		scbScene->getScenePvdClient().updatePvdProperties( &mConstraint );
#endif
}

void NpConstraint::setBreakForce(PxReal linear, PxReal angular)
{
	NP_WRITE_CHECK(getNpScene());
	PX_SIMD_GUARD;
	mConstraint.setBreakForce(linear, angular);
}

void NpConstraint::getBreakForce(PxReal& linear, PxReal& angular) const
{
	NP_READ_CHECK(getNpScene());
	PX_SIMD_GUARD;
	mConstraint.getBreakForce(linear, angular);
}


void NpConstraint::setMinResponseThreshold(PxReal threshold)
{
	PX_CHECK_AND_RETURN(PxIsFinite(threshold) && threshold>=0, "PxConstraint::setMinResponseThreshold: threshold must be non-negative");
	NP_WRITE_CHECK(getNpScene());
	PX_SIMD_GUARD;
	mConstraint.setMinResponseThreshold(threshold);
}

PxReal NpConstraint::getMinResponseThreshold() const
{
	NP_READ_CHECK(getNpScene());
	PX_SIMD_GUARD;
	return mConstraint.getMinResponseThreshold();
}


bool NpConstraint::isValid() const
{
	NP_READ_CHECK(getNpScene());
	bool isValid0 = mActor0 && !mActor0->is<PxRigidStatic>();
	bool isValid1 = mActor1 && !mActor1->is<PxRigidStatic>();
	return isValid0 || isValid1;
}

void* NpConstraint::getExternalReference(PxU32& typeID)
{
	NP_READ_CHECK(getNpScene());
	PxConstraintConnector* connector = mConstraint.getPxConnector();
	return connector->getExternalReference(typeID);
}

void NpConstraint::comShift(PxRigidActor* actor)
{
	PX_ASSERT(actor == mActor0 || actor == mActor1);
	PxConstraintConnector* connector = mConstraint.getPxConnector();
	if(actor == mActor0)
		connector->onComShift(0);
	if(actor == mActor1)
		connector->onComShift(1);
}

void NpConstraint::actorDeleted(PxRigidActor* actor)
{
	// the actor cannot be deleted without also removing it from the scene,
	// which means that the joint will also have been removed from the scene,
	// so we can just reset the actor here.
	PX_ASSERT(actor == mActor0 || actor == mActor1);

	if(actor == mActor0)
		mActor0 = 0;
	else
		mActor1 = 0;
}



NpScene* NpConstraint::getNpScene() const
{
	return mConstraint.getScbSceneForAPI() ? static_cast<NpScene*>(mConstraint.getScbSceneForAPI()->getPxScene()) : NULL;
}

Scb::RigidObject* NpConstraint::getScbRigidObject(PxRigidActor* a)
{
	if(!a)
		return NULL;

	PxType type = a->getConcreteType();

	if (type == PxConcreteType::eRIGID_DYNAMIC)
		return &(static_cast<NpRigidDynamic*>(a)->getScbBodyFast());
	else if (type == PxConcreteType::eARTICULATION_LINK)
		return &(static_cast<NpArticulationLink*>(a)->getScbBodyFast());
	else
	{
		PX_ASSERT(type == PxConcreteType::eRIGID_STATIC);
		return &(static_cast<NpRigidStatic*>(a)->getScbRigidStaticFast());
	}
}

void physx::NpConstraintGetRigidObjectsFromScb(const Scb::Constraint&c, Scb::RigidObject*&b0, Scb::RigidObject*&b1)
{
	const size_t offset = size_t(&(reinterpret_cast<NpConstraint*>(0)->getScbConstraint()));
	const NpConstraint* np = reinterpret_cast<const NpConstraint*>(reinterpret_cast<const char*>(&c)-offset);

	PxRigidActor* a0, * a1;
	np->getActors(a0, a1);
	b0 = NpConstraint::getScbRigidObject(a0);
	b1 = NpConstraint::getScbRigidObject(a1);
}

NpScene* NpConstraint::getSceneFromActors() const
{
	return getSceneFromActors(mActor0, mActor1);
}

PX_FORCE_INLINE NpScene* NpConstraint::getSceneFromActors(const PxRigidActor* actor0, const PxRigidActor* actor1)
{
	NpScene* s0 = NULL;
	NpScene* s1 = NULL;

	if (actor0 && (!(actor0->getActorFlags() & PxActorFlag::eDISABLE_SIMULATION)))
		s0 = static_cast<NpScene*>(actor0->getScene());
	if (actor1 && (!(actor1->getActorFlags() & PxActorFlag::eDISABLE_SIMULATION)))
		s1 = static_cast<NpScene*>(actor1->getScene());

#if PX_CHECKED
	if ((s0 && s1) && (s0 != s1))
		Ps::getFoundation().error(PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, "Adding constraint to scene: Actors belong to different scenes, undefined behavior expected!");
#endif

	if ((!actor0 || s0) && (!actor1 || s1))
		return s0 ? s0 : s1;
	else
		return NULL;
}
