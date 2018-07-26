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
// Copyright (c) 2008-2018 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

#include "ScBodySim.h"
#include "ScScene.h"
#include "ScConstraintSim.h"
#include "ScConstraintInteraction.h"
#include "ScArticulationSim.h"
#include "PxsContext.h"
#include "PxsRigidBody.h"
#include "ScShapeIterator.h"
#include "ScShapeSim.h"
#include "ScConstraintCore.h"
#include "ScPhysics.h"
#include "DyArticulation.h"
#include "PxsSimpleIslandManager.h"
#include "PxsSimulationController.h"

#if PX_USE_PARTICLE_SYSTEM_API
#include "PtContext.h"
#endif

using namespace physx;
using namespace physx::Dy;
using namespace Sc;

#define PX_FREEZE_INTERVAL 1.5f
#define PX_FREE_EXIT_THRESHOLD 4.f
#define PX_FREEZE_TOLERANCE 0.25f

#define PX_SLEEP_DAMPING	0.5f
#define PX_FREEZE_SCALE		0.9f

Sc::BodySim::BodySim(Scene& scene, BodyCore& core) :
	RigidSim					(scene, core),
	mLLBody						(&core.getCore()),
	mNodeIndex					(IG_INVALID_NODE),
	mInternalFlags				(0),
	mVelModState				(VMF_GRAVITY_DIRTY),
	mActiveListIndex			(SC_NOT_IN_SCENE_INDEX),
	mArticulation				(NULL),
	mConstraintGroup			(NULL)
	{

	// For 32-bit, sizeof(BodyCore) = 160 bytes, sizeof(BodySim) = 192 bytes
	mLLBody.sleepLinVelAcc = PxVec3(0);
	mLLBody.sleepAngVelAcc = PxVec3(0);
	mLLBody.freezeCount = PX_FREEZE_INTERVAL;
	mLLBody.accelScale = 1.f;
	mLLBody.solverIterationCounts = core.getCore().solverIterationCounts;
	core.getCore().numCountedInteractions = 0;
	core.getCore().numBodyInteractions = 0;
	mLLBody.mInternalFlags = 0;
	if (core.getActorFlags()&PxActorFlag::eDISABLE_GRAVITY)
		mLLBody.mInternalFlags |= PxsRigidBody::eDISABLE_GRAVITY;
	if (core.getFlags() & PxRigidBodyFlag::eENABLE_SPECULATIVE_CCD)
		mLLBody.mInternalFlags |= PxsRigidBody::eSPECULATIVE_CCD;

	//If a body pending insertion was given a force/torque then it will have 
	//the dirty flags stored in a separate structure.  Copy them across
	//so we can use them now that the BodySim is constructed.
	SimStateData* simStateData = core.getSimStateData(false);
	bool hasPendingForce = false;
	if(simStateData)
	{
		VelocityMod* velmod = simStateData->getVelocityModData();
		hasPendingForce = (velmod->flags != 0) && 
			(!velmod->getLinearVelModPerSec().isZero() || !velmod->getAngularVelModPerSec().isZero() ||
			 !velmod->getLinearVelModPerStep().isZero() || !velmod->getAngularVelModPerStep().isZero());
		mVelModState = velmod->flags;	
		velmod->flags = 0;
	}

	// PT: don't read the core ptr we just wrote, use input param
	// PT: at time of writing we get a big L2 here because even though bodycore has been prefetched, the wake counter is 160 bytes away
	const bool isAwake =	(core.getWakeCounter() > 0) || 
							(!core.getLinearVelocity().isZero()) ||
							(!core.getAngularVelocity().isZero()) || 
							hasPendingForce;

	const bool isKine = isKinematic();

	IG::SimpleIslandManager* simpleIslandManager = scene.getSimpleIslandManager();
	if (!isArticulationLink())
	{
		mNodeIndex = simpleIslandManager->addRigidBody(&mLLBody, isKine, isAwake);
	}
	else
	{
		if(mArticulation)
		{
			const ArticulationLinkHandle articLinkhandle = mArticulation->getLinkHandle(*this);
			IG::NodeIndex index = mArticulation->getIslandNodeIndex();
			mNodeIndex.setIndices(index.index(), articLinkhandle & (DY_ARTICULATION_MAX_SIZE-1));
		}
	}

	//If a user add force or torque before the body is inserted into the scene,
	//this logic will make sure pre solver stage apply external force/torque to the body
	if(hasPendingForce && !isArticulationLink())
		scene.getVelocityModifyMap().growAndSet(mNodeIndex.index());

	PX_ASSERT(mActiveListIndex == SC_NOT_IN_SCENE_INDEX);

	setActive(isAwake, ActorSim::AS_PART_OF_CREATION);

	if (isAwake)
	{
		scene.addToActiveBodyList(*this);
		PX_ASSERT(isActive());
	}
	else
	{
		mActiveListIndex = SC_NOT_IN_ACTIVE_LIST_INDEX;
		PX_ASSERT(!isActive());

		simpleIslandManager->deactivateNode(mNodeIndex);
	}

	if (isKine)
	{
		initKinematicStateBase(core, true);

		const SimStateData* kd = core.getSimStateData(true);
		if (!kd)
		{
			core.setupSimStateData(scene.getSimStateDataPool(), true, false);
			notifyPutToSleep();  // sleep state of kinematics is fully controlled by the simulation controller not the island manager
		}
		else
		{
			PX_ASSERT(kd->isKine());
			PX_ASSERT(kd->getKinematicData()->targetValid);  // the only reason for the kinematic data to exist at that point already is if the target has been set
			PX_ASSERT(isAwake);  // the expectation is that setting a target also sets the wake counter to a positive value
			postSetKinematicTarget();
		}
	}
}

Sc::BodySim::~BodySim()
{
	Sc::Scene &scene = getScene();
	const bool active = isActive();

	getBodyCore().tearDownSimStateData(scene.getSimStateDataPool(), isKinematic() ? true : false);

	PX_ASSERT(!readInternalFlag(BF_ON_DEATHROW)); // Before 3.0 it could happen that destroy could get called twice. Assert to make sure this is fixed.
	raiseInternalFlag(BF_ON_DEATHROW);

	scene.removeBody(*this);
	PX_ASSERT(!getConstraintGroup());  // Removing from scene should erase constraint group node if it existed

	if(mArticulation)
		mArticulation->removeBody(*this);

	//Articulations are represented by a single node, so they must only be removed by the articulation and not the links!
	if(mArticulation == NULL && mNodeIndex.articulationLinkId() == 0) //If it wasn't an articulation link, then we can remove it
		scene.getSimpleIslandManager()->removeNode(mNodeIndex);

#if PX_USE_PARTICLE_SYSTEM_API
	PX_ASSERT(!scene.getParticleContext() || !scene.getParticleContext()->getBodyTransformVaultFast().isInVault(mLLBody.getCore()));
#endif

	PX_ASSERT(mActiveListIndex != SC_NOT_IN_SCENE_INDEX);

	if (active)
		scene.removeFromActiveBodyList(*this);

	mActiveListIndex = SC_NOT_IN_SCENE_INDEX;

	mCore.setSim(NULL);
}

//--------------------------------------------------------------
//
// Actor implementation
//
//--------------------------------------------------------------

void Sc::BodySim::onActivate()
{
	PX_ASSERT((!isKinematic()) || notInScene() || readInternalFlag(BF_KINEMATIC_MOVED));	// kinematics should only get activated when a target is set.
																							// exception: object gets newly added, then the state change will happen later

	BodyCore& core = getBodyCore();
	if (!isArticulationLink())
	{
		mLLBody.mInternalFlags &= (~PxsRigidBody::eFROZEN);
		// Put in list of activated bodies. The list gets cleared at the end of a sim step after the sleep callbacks have been fired.
		getScene().onBodyWakeUp(this);
	}

	if (core.getFlags() & PxRigidBodyFlag::eENABLE_POSE_INTEGRATION_PREVIEW)
	{
		PX_ASSERT(!getScene().isInPosePreviewList(*this));
		getScene().addToPosePreviewList(*this);
	}
	createSqBounds();
}

void Sc::BodySim::updateCached(Cm::BitMapPinned* shapeChangedMap)
{
	if(!(mLLBody.mInternalFlags & PxsRigidBody::eFROZEN))
	{
		Sc::ShapeSim* sim;
		for(Sc::ShapeIterator iterator(*this); (sim = iterator.getNext())!=NULL;)
			sim->updateCached(0, shapeChangedMap);

	}
}

void Sc::BodySim::updateCached(PxsTransformCache& transformCache, Bp::BoundsArray& boundsArray)
{
	PX_ASSERT(!(mLLBody.mInternalFlags & PxsRigidBody::eFROZEN));	// PT: should not be called otherwise

	Sc::ShapeSim* sim;
	for(Sc::ShapeIterator iterator(*this); (sim = iterator.getNext())!=NULL;)
		sim->updateCached(transformCache, boundsArray);
}

void Sc::BodySim::updateContactDistance(PxReal* contactDistance, const PxReal dt, Bp::BoundsArray& boundsArray)
{
	if (getLowLevelBody().getCore().mFlags & PxRigidBodyFlag::eENABLE_SPECULATIVE_CCD
		&& !(getLowLevelBody().mInternalFlags & PxcRigidBody::eFROZEN))
	{
		Sc::ShapeSim* sim;

		const PxVec3 linVel = getLowLevelBody().getLinearVelocity();
		const PxVec3 aVel = getLowLevelBody().getAngularVelocity();
		const PxReal inflation = linVel.magnitude() * dt;

		for (Sc::ShapeIterator iterator(*this); (sim = iterator.getNext()) != NULL;)
			sim->updateContactDistance(contactDistance, inflation, aVel, dt, boundsArray);
	}
}

void Sc::BodySim::onDeactivate()
{
	PX_ASSERT((!isKinematic()) || notInScene() || !readInternalFlag(BF_KINEMATIC_MOVED));	// kinematics should only get deactivated when no target is set.
																							// exception: object gets newly added, then the state change will happen later
	BodyCore& core = getBodyCore();
	if (!readInternalFlag(BF_ON_DEATHROW))
	{
		// Set velocity to 0.
		// Note: this is also fine if the method gets called because the user puts something to sleep (this behavior is documented in the API)
		PX_ASSERT(core.getWakeCounter() == 0.0f);
		const PxVec3 zero(0.f, 0.f, 0.f);
		core.setLinearVelocityInternal(zero);
		core.setAngularVelocityInternal(zero);
	
		setForcesToDefaults(!(mLLBody.mInternalFlags & PxsRigidBody::eDISABLE_GRAVITY));
	}

	if (!isArticulationLink())  // Articulations have their own sleep logic.
		getScene().onBodySleep(this);

	if (core.getFlags() & PxRigidBodyFlag::eENABLE_POSE_INTEGRATION_PREVIEW)
	{
		PX_ASSERT(getScene().isInPosePreviewList(*this));
		getScene().removeFromPosePreviewList(*this);
	}
	destroySqBounds();
}

//--------------------------------------------------------------
//
// BodyCore interface implementation
//
//--------------------------------------------------------------

void Sc::BodySim::notifyAddSpatialAcceleration()
{
	//The dirty flag is stored separately in the BodySim so that we query the dirty flag before going to 
	//the expense of querying the simStateData for the velmod values.
	raiseVelocityModFlag(VMF_ACC_DIRTY);

	getScene().getVelocityModifyMap().growAndSet(getNodeIndex().index());
}

void Sc::BodySim::notifyClearSpatialAcceleration()
{
	//The dirty flag is stored separately in the BodySim so that we query the dirty flag before going to 
	//the expense of querying the simStateData for the velmod values.
	raiseVelocityModFlag(VMF_ACC_DIRTY);
	getScene().getVelocityModifyMap().growAndSet(getNodeIndex().index());
}

void Sc::BodySim::notifyAddSpatialVelocity()
{
	//The dirty flag is stored separately in the BodySim so that we query the dirty flag before going to 
	//the expense of querying the simStateData for the velmod values.
	raiseVelocityModFlag(VMF_VEL_DIRTY);
	getScene().getVelocityModifyMap().growAndSet(getNodeIndex().index());
}

void Sc::BodySim::notifyClearSpatialVelocity()
{
	//The dirty flag is stored separately in the BodySim so that we query the dirty flag before going to 
	//the expense of querying the simStateData for the velmod values.
	raiseVelocityModFlag(VMF_VEL_DIRTY);
	getScene().getVelocityModifyMap().growAndSet(getNodeIndex().index());
}

void Sc::BodySim::postActorFlagChange(PxU32 oldFlags, PxU32 newFlags)
{
	// PT: don't convert to bool if not needed
	const PxU32 wasWeightless = oldFlags & PxActorFlag::eDISABLE_GRAVITY;
	const PxU32 isWeightless = newFlags & PxActorFlag::eDISABLE_GRAVITY;

	if (isWeightless != wasWeightless)
	{
		if (mVelModState == 0) raiseVelocityModFlag(VMF_GRAVITY_DIRTY);

		if (isWeightless)
			mLLBody.mInternalFlags |= PxsRigidBody::eDISABLE_GRAVITY;
		else
			mLLBody.mInternalFlags &= (~PxsRigidBody::eDISABLE_GRAVITY);
	}
}

void Sc::BodySim::postBody2WorldChange()
{
	mLLBody.saveLastCCDTransform();

#if PX_USE_PARTICLE_SYSTEM_API
	if (getScene().getParticleContext()->getBodyTransformVaultFast().isInVault(*mLLBody.mCore))
		getScene().getParticleContext()->getBodyTransformVaultFast().teleportBody(*mLLBody.mCore);
#endif

	notifyShapesOfTransformChange();
}

void Sc::BodySim::postSetWakeCounter(PxReal t, bool forceWakeUp)
{
	if ((t > 0.0f) || forceWakeUp)
		notifyNotReadyForSleeping();
	else
	{
		const bool readyForSleep = checkSleepReadinessBesidesWakeCounter();
		if (readyForSleep)
			notifyReadyForSleeping();
	}
}

void Sc::BodySim::postSetKinematicTarget()
{
	PX_ASSERT(getBodyCore().getSimStateData(true));
	PX_ASSERT(getBodyCore().getSimStateData(true)->isKine());
	PX_ASSERT(getBodyCore().getSimStateData(true)->getKinematicData()->targetValid);

	raiseInternalFlag(BF_KINEMATIC_MOVED);	// Important to set this here already because trigger interactions need to have this information when being activated.
}

static void updateBPGroup(ElementSim* current)
{
	while(current)
	{
		static_cast<ShapeSim*>(current)->updateBPGroup();
		current = current->mNextInActor;
	}
}

void Sc::BodySim::postSwitchToKinematic()
{
	initKinematicStateBase(getBodyCore(), false);

	// - interactions need to get refiltered to make sure that kinematic-kinematic and kinematic-static pairs get suppressed
	// - unlike postSwitchToDynamic(), constraint interactions are not marked dirty here because a transition to kinematic will put the object asleep which in turn 
	//   triggers onDeactivate() on the constraint pairs that are active. If such pairs get deactivated, they will get removed from the list of active breakable
	//   constraints automatically.
	setActorsInteractionsDirty(InteractionDirtyFlag::eBODY_KINEMATIC, NULL, InteractionFlag::eFILTERABLE);

	getScene().getSimpleIslandManager()->setKinematic(mNodeIndex);

	updateBPGroup(getElements_());
}

void Sc::BodySim::postSwitchToDynamic()
{
	mScene.getSimpleIslandManager()->setDynamic(mNodeIndex);

	setForcesToDefaults(true);

	if(getConstraintGroup())
		getConstraintGroup()->markForProjectionTreeRebuild(mScene.getProjectionManager());

	// - interactions need to get refiltered to make sure that former kinematic-kinematic and kinematic-static pairs get enabled
	// - switching from kinematic to dynamic does not change the sleep state of the body. The constraint interactions are marked dirty
	//   to check later whether they need to be activated plus potentially tracked for constraint break testing. This special treatment
	//   is necessary because constraints between two kinematic bodies are considered inactive, no matter whether one of the kinematics
	//   is active (has a target) or not.
	setActorsInteractionsDirty(InteractionDirtyFlag::eBODY_KINEMATIC, NULL, InteractionFlag::eFILTERABLE | InteractionFlag::eCONSTRAINT);

	clearInternalFlag(BF_KINEMATIC_MOVE_FLAGS);

	if(isActive())
		mScene.swapInActiveBodyList(*this);

	//
	updateBPGroup(getElements_());
}

void Sc::BodySim::postPosePreviewChange(const PxU32 posePreviewFlag)
{
	if (isActive())
	{
		if (posePreviewFlag & PxRigidBodyFlag::eENABLE_POSE_INTEGRATION_PREVIEW)
			getScene().addToPosePreviewList(*this);
		else
			getScene().removeFromPosePreviewList(*this);
	}
	else
		PX_ASSERT(!getScene().isInPosePreviewList(*this));
}

//--------------------------------------------------------------
//
// Sleeping
//
//--------------------------------------------------------------

void Sc::BodySim::setActive(bool active, PxU32 infoFlag)
{
	PX_ASSERT(!active || isDynamicRigid());  // Currently there should be no need to activate an actor that does not take part in island generation

	const PxU32 asPartOfCreation = infoFlag & ActorSim::AS_PART_OF_CREATION;
	if (asPartOfCreation || isActive() != active)
	{
		PX_ASSERT(!asPartOfCreation || (getActorInteractionCount() == 0)); // On creation or destruction there should be no interactions

		if (active)
		{
			if (!asPartOfCreation)
			{
				// Inactive => Active
				getScene().addToActiveBodyList(*this);
			}

			onActivate();

			activateInteractions(infoFlag);

			PX_ASSERT(asPartOfCreation || isActive());
		}
		else
		{
			if (!asPartOfCreation)
			{
				// Active => Inactive
				getScene().removeFromActiveBodyList(*this);
			}

			deactivateInteractions(infoFlag);

			onDeactivate();

			PX_ASSERT(asPartOfCreation || (!isActive()));
		}
	}
}

void Sc::BodySim::activateInteractions(PxU32 /*infoFlag*/)
{
	const PxU32 nbInteractions = getActorInteractionCount();

	for(PxU32 i=0; i<nbInteractions; ++i)
	{
		Ps::prefetchLine(mInteractions[PxMin(i+1,nbInteractions-1)]);
		Interaction* interaction = mInteractions[i];

		bool isNotIGControlled = interaction->getType() != Sc::InteractionType::eOVERLAP &&
			interaction->getType() != Sc::InteractionType::eMARKER;

		if (!interaction->readInteractionFlag(InteractionFlag::eIS_ACTIVE) && (isNotIGControlled))
		{
			const bool proceed = interaction->onActivate(NULL);
			if (proceed && (interaction->getType() < InteractionType::eTRACKED_IN_SCENE_COUNT))
				getScene().notifyInteractionActivated(interaction);
		}
	}
}

void Sc::BodySim::deactivateInteractions(PxU32 infoFlag)
{
	const PxU32 nbInteractions = getActorInteractionCount();

	for(PxU32 i=0; i<nbInteractions; ++i)
	{
		Ps::prefetchLine(mInteractions[PxMin(i+1,nbInteractions-1)]);
		Interaction* interaction = mInteractions[i];

		bool isNotIGControlled = interaction->getType() != Sc::InteractionType::eOVERLAP &&
			interaction->getType() != Sc::InteractionType::eMARKER;

		if (interaction->readInteractionFlag(InteractionFlag::eIS_ACTIVE) && isNotIGControlled)
		{
			const bool proceed = interaction->onDeactivate(infoFlag);
			if (proceed && (interaction->getType() < InteractionType::eTRACKED_IN_SCENE_COUNT))
				getScene().notifyInteractionDeactivated(interaction);
		}
	}
}

void Sc::BodySim::wakeUp()
{
	setActive(true);
	notifyWakeUp(true);
}

void Sc::BodySim::putToSleep()
{
	PX_ASSERT(getBodyCore().getWakeCounter() == 0.0f);
	PX_ASSERT(getBodyCore().getLinearVelocity().isZero());
	PX_ASSERT(getBodyCore().getAngularVelocity().isZero());
#ifdef _DEBUG
	// pending forces should have been cleared at this point
	const SimStateData* sd = getBodyCore().getSimStateData(false);
	if (sd)
	{
		const VelocityMod* vm = sd->getVelocityModData();
		PX_ASSERT(vm->linearPerSec.isZero() && vm->linearPerStep.isZero() && vm->angularPerSec.isZero() && vm->angularPerStep.isZero());
	}
#endif

	setActive(false);
	notifyPutToSleep();

	clearInternalFlag(InternalFlags(BF_KINEMATIC_SETTLING | BF_KINEMATIC_SETTLING_2));	// putToSleep is used when a kinematic gets removed from the scene while the sim is running and then gets re-inserted immediately.
												// We can move this code when we look into the open task of making buffered re-insertion more consistent with the non-buffered case.
}

void Sc::BodySim::internalWakeUp(PxReal wakeCounterValue)
{
	if(mArticulation)
		mArticulation->internalWakeUp(wakeCounterValue);
	else
		internalWakeUpBase(wakeCounterValue);
}

void Sc::BodySim::internalWakeUpArticulationLink(PxReal wakeCounterValue)
{
	PX_ASSERT(mArticulation);
	internalWakeUpBase(wakeCounterValue);
}

void Sc::BodySim::internalWakeUpBase(PxReal wakeCounterValue)	//this one can only increase the wake counter, not decrease it, so it can't be used to put things to sleep!
{
	if ((!isKinematic()) && (getBodyCore().getWakeCounter() < wakeCounterValue))
	{
		PX_ASSERT(wakeCounterValue > 0.0f);
		getBodyCore().setWakeCounterFromSim(wakeCounterValue);

		//we need to update the gpu body sim because we reset the wake counter for the body core
		mScene.getSimulationController()->addDynamic(&mLLBody, mNodeIndex.index());
		setActive(true);
		notifyWakeUp(false);
		mLLBody.mInternalFlags &= (~PxsRigidBody::eFROZEN);
	}
}

void Sc::BodySim::notifyReadyForSleeping()
{
	if(mArticulation == NULL)
		getScene().getSimpleIslandManager()->deactivateNode(mNodeIndex);
}

void Sc::BodySim::notifyNotReadyForSleeping()
{
	getScene().getSimpleIslandManager()->activateNode(mNodeIndex);
}

void Sc::BodySim::notifyWakeUp(bool /*wakeUpInIslandGen*/)
{
	getScene().getSimpleIslandManager()->activateNode(mNodeIndex);
}

void Sc::BodySim::notifyPutToSleep()
{
	getScene().getSimpleIslandManager()->putNodeToSleep(mNodeIndex);
}

void Sc::BodySim::resetSleepFilter()
{
	mLLBody.sleepAngVelAcc = PxVec3(0.0f);
	mLLBody.sleepLinVelAcc = PxVec3(0.0f);
}

//This function will be called by CPU sleepCheck code
PxReal Sc::BodySim::updateWakeCounter(PxReal dt, PxReal energyThreshold, const Cm::SpatialVector& motionVelocity)
{
	// update the body's sleep state and 
	BodyCore& core = getBodyCore();

	const PxReal wakeCounterResetTime = ScInternalWakeCounterResetValue;

	PxReal wc = core.getWakeCounter();
	
	{

		PxVec3 bcSleepLinVelAcc = mLLBody.sleepLinVelAcc;
		PxVec3 bcSleepAngVelAcc = mLLBody.sleepAngVelAcc;

		if(wc < wakeCounterResetTime * 0.5f || wc < dt)
		{
			const PxTransform& body2World = getBody2World();

			// calculate normalized energy: kinetic energy divided by mass
			const PxVec3 t = core.getInverseInertia();
			const PxVec3 inertia(t.x > 0.f ? 1.0f/t.x : 1.f, t.y > 0.f ? 1.0f/t.y : 1.f, t.z > 0.f ? 1.0f/t.z : 1.f);

			PxVec3 sleepLinVelAcc =motionVelocity.linear;
			PxVec3 sleepAngVelAcc = body2World.q.rotateInv(motionVelocity.angular);

			
			bcSleepLinVelAcc += sleepLinVelAcc;
			bcSleepAngVelAcc += sleepAngVelAcc;

			PxReal invMass = core.getInverseMass();
			if(invMass == 0.f)
				invMass = 1.f;

			const PxReal angular = bcSleepAngVelAcc.multiply(bcSleepAngVelAcc).dot(inertia) * invMass;
			const PxReal linear = bcSleepLinVelAcc.magnitudeSquared();
			PxReal normalizedEnergy = 0.5f * (angular + linear);

			// scale threshold by cluster factor (more contacts => higher sleep threshold)
			const PxReal clusterFactor = PxReal(1 + getNumCountedInteractions());
			const PxReal threshold = clusterFactor*energyThreshold;
		
			if (normalizedEnergy >= threshold)
			{
				PX_ASSERT(isActive());
				resetSleepFilter();
				const float factor = threshold == 0.f ? 2.0f : PxMin(normalizedEnergy/threshold, 2.0f);
				PxReal oldWc = wc;
				wc = factor * 0.5f * wakeCounterResetTime + dt * (clusterFactor - 1.0f);
				core.setWakeCounterFromSim(wc);
				if (oldWc == 0.0f)  // for the case where a sleeping body got activated by the system (not the user) AND got processed by the solver as well
					notifyNotReadyForSleeping();
				
				return wc;
			}
		}

		mLLBody.sleepLinVelAcc = bcSleepLinVelAcc;
		mLLBody.sleepAngVelAcc = bcSleepAngVelAcc;
	}

	wc = PxMax(wc-dt, 0.0f);
	core.setWakeCounterFromSim(wc);
	return wc;
}

//--------------------------------------------------------------
//
// Kinematics
//
//--------------------------------------------------------------

PX_FORCE_INLINE void Sc::BodySim::initKinematicStateBase(BodyCore&, bool asPartOfCreation)
{
	PX_ASSERT(!readInternalFlag(BF_KINEMATIC_MOVED));

	if (!asPartOfCreation && isActive())
		getScene().swapInActiveBodyList(*this);

	//mLLBody.setAccelerationV(Cm::SpatialVector::zero());

	// Need to be before setting setRigidBodyFlag::KINEMATIC

	if (getConstraintGroup())
		getConstraintGroup()->markForProjectionTreeRebuild(getScene().getProjectionManager());
}

void Sc::BodySim::calculateKinematicVelocity(PxReal oneOverDt)
{
	PX_ASSERT(isKinematic());
	
	/*------------------------------------------------\
	| kinematic bodies are moved directly by the user and are not influenced by external forces
	| we simply determine the distance moved since the last simulation frame and 
	| assign the appropriate delta to the velocity. This vel will be used to shove dynamic
	| objects in the solver.
	| We have to do this like so in a delayed way, because when the user sets the target pos the dt is not
	| yet known.
	\------------------------------------------------*/
	PX_ASSERT(isActive());

	BodyCore& core = getBodyCore();

	if (readInternalFlag(BF_KINEMATIC_MOVED))
	{
		clearInternalFlag(InternalFlags(BF_KINEMATIC_SETTLING | BF_KINEMATIC_SETTLING_2));
		const SimStateData* kData = core.getSimStateData(true);
		PX_ASSERT(kData);
		PX_ASSERT(kData->isKine());
		PX_ASSERT(kData->getKinematicData()->targetValid);
		PxVec3 linVelLL, angVelLL;
		const PxTransform targetPose = kData->getKinematicData()->targetPose;
		const PxTransform& currBody2World = getBody2World();

		//the kinematic target pose is now the target of the body (CoM) and not the actor.

		PxVec3 deltaPos = targetPose.p;
		deltaPos -= currBody2World.p;
		linVelLL = deltaPos * oneOverDt;

		PxQuat q = targetPose.q * currBody2World.q.getConjugate();

		if (q.w < 0)	//shortest angle.
			q = -q;

		PxReal angle;
 		PxVec3 axis;
		q.toRadiansAndUnitAxis(angle, axis);
		angVelLL = axis * angle * oneOverDt;

		core.setLinearVelocity(linVelLL);
		core.setAngularVelocity(angVelLL);

		// Moving a kinematic should trigger a wakeUp call on a higher level.
		PX_ASSERT(core.getWakeCounter()>0);
		PX_ASSERT(isActive());
		
	}
	else
	{
		core.setLinearVelocity(PxVec3(0));
		core.setAngularVelocity(PxVec3(0));
	}
}

void Sc::BodySim::updateKinematicPose()
{
	/*------------------------------------------------\
	| kinematic bodies are moved directly by the user and are not influenced by external forces
	| we simply determine the distance moved since the last simulation frame and 
	| assign the appropriate delta to the velocity. This vel will be used to shove dynamic
	| objects in the solver.
	| We have to do this like so in a delayed way, because when the user sets the target pos the dt is not
	| yet known.
	\------------------------------------------------*/

	PX_ASSERT(isKinematic());
	PX_ASSERT(isActive());

	BodyCore& core = getBodyCore();

	if (readInternalFlag(BF_KINEMATIC_MOVED))
	{
		clearInternalFlag(InternalFlags(BF_KINEMATIC_SETTLING | BF_KINEMATIC_SETTLING_2));
		const SimStateData* kData = core.getSimStateData(true);
		PX_ASSERT(kData);
		PX_ASSERT(kData->isKine());
		PX_ASSERT(kData->getKinematicData()->targetValid);
		
		const PxTransform targetPose = kData->getKinematicData()->targetPose;
		getBodyCore().getCore().body2World = targetPose;
	}
}

bool Sc::BodySim::deactivateKinematic()
{
	BodyCore& core = getBodyCore();
	if(readInternalFlag(BF_KINEMATIC_SETTLING_2))
	{
		clearInternalFlag(BF_KINEMATIC_SETTLING_2);
		core.setWakeCounterFromSim(0);	// For sleeping objects the wake counter must be 0. This needs to hold for kinematics too.
		notifyReadyForSleeping();
		notifyPutToSleep();
		setActive(false);
		return true;
	}
	else if (readInternalFlag(BF_KINEMATIC_SETTLING))
	{
		clearInternalFlag(BF_KINEMATIC_SETTLING);
		raiseInternalFlag(BF_KINEMATIC_SETTLING_2);
	}
	else
	{
		clearInternalFlag(BF_KINEMATIC_MOVED);
		raiseInternalFlag(BF_KINEMATIC_SETTLING);
	}
	return false;
}

//--------------------------------------------------------------
//
// Miscellaneous
//
//--------------------------------------------------------------

void Sc::BodySim::updateForces(PxReal dt, PxsRigidBody** updatedBodySims, PxU32* updatedBodyNodeIndices, PxU32& index, Cm::SpatialVector* acceleration, bool simUsesAdaptiveForce)
{
	PxVec3 linAcc(0.0f), angAcc(0.0f);

	const bool accDirty = readVelocityModFlag(VMF_ACC_DIRTY);
	const bool velDirty = readVelocityModFlag(VMF_VEL_DIRTY);

	BodyCore& bodyCore = getBodyCore();
	SimStateData* simStateData = NULL;

	//if we change the logic like this, which means we don't need to have two seperate variables in the pxgbodysim to represent linAcc and angAcc. However, this
	//means angAcc will be always 0
	if( (accDirty || velDirty) &&  ((simStateData = bodyCore.getSimStateData(false)) != NULL) )
	{
		VelocityMod* velmod = simStateData->getVelocityModData();

		//we don't have support for articulation yet
		if (updatedBodySims)
		{
			updatedBodySims[index] = &getLowLevelBody();
			updatedBodyNodeIndices[index++] = getNodeIndex().index();
		}

		if(velDirty)
		{
			getBodyCore().updateVelocities(velmod->getLinearVelModPerStep(), velmod->getAngularVelModPerStep());
		}
		
		if (accDirty)
		{
			linAcc = velmod->getLinearVelModPerSec();
			angAcc = velmod->getAngularVelModPerSec();

			if (acceleration)
			{
				acceleration->linear = linAcc;
				acceleration->angular = angAcc;
			}
			else
			{
				PxReal scale = dt;
				if (simUsesAdaptiveForce)
				{
					if (getScene().getSimpleIslandManager()->getAccurateIslandSim().getIslandStaticTouchCount(mNodeIndex) != 0)
					{
						scale *= mLLBody.accelScale;
					}
				}
				getBodyCore().updateVelocities(linAcc*scale, angAcc*scale);
			}
		}		
	}

	setForcesToDefaults(readVelocityModFlag(VMF_ACC_DIRTY));
}

bool Sc::BodySim::isConnectedTo(const ActorSim& other, bool& collisionDisabled) const
{
	const Sc::ActorSim* actorToMatch;
	PxU32 size;
	Interaction** interactions;

	if(getActorInteractionCount() <= other.getActorInteractionCount())
	{
		size = getActorInteractionCount();
		interactions = getActorInteractions();
		actorToMatch = &other;
	}
	else
	{
		size = other.getActorInteractionCount();
		interactions = other.getActorInteractions();
		actorToMatch = this;
	}

	while(size--)
	{
		Interaction* interaction = *interactions++;
		if (interaction->getType() == InteractionType::eCONSTRAINTSHADER)
		{
			ConstraintInteraction* csi = static_cast<ConstraintInteraction*>(interaction);
			if ((&csi->getActor0() == actorToMatch) || (&csi->getActor1() == actorToMatch))
			{
				collisionDisabled = !((csi->getConstraint()->getCore().getFlags() & PxConstraintFlag::eCOLLISION_ENABLED));
				return true;
			}
		}
	}

	collisionDisabled = false;
	return false;
}

void Sc::BodySim::onConstraintDetach()
{
	PX_ASSERT(readInternalFlag(BF_HAS_CONSTRAINTS));

	PxU32 size = getActorInteractionCount();
	Interaction** interactions = getActorInteractions();
	unregisterCountedInteraction();

	while(size--)
	{
		const Interaction* interaction = *interactions++;
		if(interaction->getType() == InteractionType::eCONSTRAINTSHADER)
			return;
	}

	clearInternalFlag(BF_HAS_CONSTRAINTS);  // There are no other constraint interactions left
}

void Sc::BodySim::setArticulation(Sc::ArticulationSim* a, PxReal wakeCounter, bool asleep, PxU32 bodyIndex)
{
	mArticulation = a; 
	if(a)
	{
		IG::NodeIndex index = mArticulation->getIslandNodeIndex();
		mNodeIndex.setIndices(index.index(), bodyIndex);
		getBodyCore().setWakeCounterFromSim(wakeCounter);

		if (getFlagsFast() & PxRigidBodyFlag::eENABLE_SPECULATIVE_CCD)
			getScene().setSpeculativeCCDArticulationLink(mNodeIndex.index());

		if (!asleep)
		{
			setActive(true);
			notifyWakeUp(false);
		}
		else
		{
			notifyReadyForSleeping();
			notifyPutToSleep();
			setActive(false);
		}
	}
	else
	{
		mNodeIndex.setIndices(IG_INVALID_NODE, 1);
	}
}

void Sc::BodySim::createSqBounds()
{
	if(!isActive() || usingSqKinematicTarget())
		return;

	PX_ASSERT(!isFrozen());
	
	Sc::ShapeSim* sim;
	for(Sc::ShapeIterator iterator(*this); NULL != (sim = iterator.getNext());)
		sim->createSqBounds();
}

void Sc::BodySim::destroySqBounds()
{
	Sc::ShapeSim* sim;
	for(Sc::ShapeIterator iterator(*this); NULL != (sim = iterator.getNext());)
		sim->destroySqBounds();
}

void Sc::BodySim::freezeTransforms(Cm::BitMapPinned* shapeChangedMap)
{
	Sc::ShapeSim* sim;
	for(Sc::ShapeIterator iterator(*this); NULL != (sim = iterator.getNext());)
	{
		sim->updateCached(PxsTransformFlag::eFROZEN, shapeChangedMap);
		sim->destroySqBounds();
	}
}

