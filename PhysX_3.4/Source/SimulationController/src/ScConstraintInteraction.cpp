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


#include "ScConstraintInteraction.h"
#include "ScConstraintSim.h"
#include "ScBodySim.h"
#include "ScScene.h"
#include "PxsRigidBody.h"
#include "PxsSimpleIslandManager.h"


using namespace physx;


Sc::ConstraintInteraction::ConstraintInteraction(ConstraintSim* constraint, RigidSim& r0, RigidSim& r1) :
	ActorInteraction	(r0, r1, InteractionType::eCONSTRAINTSHADER, InteractionFlag::eCONSTRAINT),
	mConstraint			(constraint)
{
	registerInActors();

	BodySim* b0 = mConstraint->getBody(0);
	BodySim* b1 = mConstraint->getBody(1);

	if (b0)
		b0->onConstraintAttach();
	if (b1)
		b1->onConstraintAttach();

	IG::SimpleIslandManager* simpleIslandManager = getScene().getSimpleIslandManager();
	mEdgeIndex = simpleIslandManager->addConstraint(&mConstraint->getLowLevelConstraint(), b0 ? b0->getNodeIndex() : IG::NodeIndex(), b1 ? b1->getNodeIndex() : IG::NodeIndex(), this);

}


Sc::ConstraintInteraction::~ConstraintInteraction()
{
	PX_ASSERT(!readInteractionFlag(InteractionFlag::eIN_DIRTY_LIST));
	PX_ASSERT(!getDirtyFlags());
	PX_ASSERT(!mConstraint->readFlag(ConstraintSim::eCHECK_MAX_FORCE_EXCEEDED));
}


PX_FORCE_INLINE void Sc::ConstraintInteraction::removeFromActiveBreakableList(Scene& s)
{
	if (mConstraint->readFlag(ConstraintSim::eBREAKABLE | ConstraintSim::eCHECK_MAX_FORCE_EXCEEDED) == (ConstraintSim::eBREAKABLE | ConstraintSim::eCHECK_MAX_FORCE_EXCEEDED))
		s.removeActiveBreakableConstraint(mConstraint);
}


void Sc::ConstraintInteraction::destroy()
{
	setClean(true);  // removes the pair from the dirty interaction list etc.

	Scene& scene = getScene();

	removeFromActiveBreakableList(scene);

	if(mEdgeIndex != IG_INVALID_EDGE)
		scene.getSimpleIslandManager()->removeConnection(mEdgeIndex);
	mEdgeIndex = IG_INVALID_EDGE;

	unregisterFromActors();

	BodySim* b0 = mConstraint->getBody(0);
	BodySim* b1 = mConstraint->getBody(1);

	if (b0)
		b0->onConstraintDetach();  // Note: Has to be done AFTER the interaction has unregistered from the actors
	if (b1)
		b1->onConstraintDetach();  // Note: Has to be done AFTER the interaction has unregistered from the actors

	clearInteractionFlag(InteractionFlag::eIS_ACTIVE);  // ensures that broken constraints do not go into the list of active breakable constraints anymore
}


void Sc::ConstraintInteraction::updateState()
{
	PX_ASSERT(!mConstraint->isBroken());
	PX_ASSERT(getDirtyFlags() & InteractionDirtyFlag::eBODY_KINEMATIC);  // at the moment this should be the only reason for this method being called

	// at least one of the bodies got switched from kinematic to dynamic. This will not have changed the sleep state of the interactions, so the
	// constraint interactions are just marked dirty and processed as part of the dirty interaction update system.
	// 
	// -> need to check whether to activate the constraint and whether constraint break testing
	//    is now necessary
	//
	// the transition from dynamic to kinematic will always trigger an onDeactivate() (because the body gets deactivated)
	// and thus there is no need to consider that case here.
	//

	onActivate(NULL);	// note: this will not activate if the necessary conditions are not met, so it can be called even if the pair has been deactivated again before the
					//       simulation step started
}


bool Sc::ConstraintInteraction::onActivate(void*)
{
	PX_ASSERT(!mConstraint->isBroken());

	BodySim* b0 = mConstraint->getBody(0);
	BodySim* b1 = mConstraint->getBody(1);

	bool b0Vote = !b0 || b0->isActive();
	bool b1Vote = !b1 || b1->isActive();

	bool b0Dynamic = b0 && (!b0->isKinematic());
	bool b1Dynamic = b1 && (!b1->isKinematic());

	//
	// note: constraints between kinematics and kinematics/statics are always inactive and must not be activated
	//
	if ((b0Vote || b1Vote) && (b0Dynamic || b1Dynamic))
	{
		raiseInteractionFlag(InteractionFlag::eIS_ACTIVE);

		if (mConstraint->readFlag(ConstraintSim::eBREAKABLE | ConstraintSim::eCHECK_MAX_FORCE_EXCEEDED) == ConstraintSim::eBREAKABLE)
			getScene().addActiveBreakableConstraint(mConstraint, this);

		return true;
	}
	else
		return false;
}


bool Sc::ConstraintInteraction::onDeactivate(PxU32)
{
	const BodySim* b0 = mConstraint->getBody(0);
	const BodySim* b1 = mConstraint->getBody(1);

	bool b0Dynamic = b0 && (!b0->isKinematic());
	bool b1Dynamic = b1 && (!b1->isKinematic());

	PX_ASSERT(	(!b0 && b1 && !b1->isActive()) || 
				(!b1 && b0 && !b0->isActive()) ||
				((b0 && b1 && (!b0->isActive() || !b1->isActive()))) );

	//
	// note: constraints between kinematics and kinematics/statics should always get deactivated
	//
	if (((!b0 || !b0->isActive()) && (!b1 || !b1->isActive())) || (!b0Dynamic && !b1Dynamic))
	{
		removeFromActiveBreakableList(getScene());

		clearInteractionFlag(InteractionFlag::eIS_ACTIVE);
		
		return true;
	}
	else
		return false;
}
