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


#ifndef PX_PHYSICS_SCP_BODYSIM
#define PX_PHYSICS_SCP_BODYSIM

#include "PsUtilities.h"
#include "PsIntrinsics.h"
#include "ScRigidSim.h"
#include "PxvDynamics.h"
#include "ScBodyCore.h"
#include "ScSimStateData.h"
#include "ScConstraintGroupNode.h"
#include "PxRigidDynamic.h"
#include "DyArticulation.h"
#include "PxsRigidBody.h"
#include "PxsSimpleIslandManager.h"

namespace physx
{
namespace Bp
{
	class BoundsArray;
}
	class PxsTransformCache;
	class PxsSimulationController;
namespace Sc
{
	#define SC_NOT_IN_SCENE_INDEX		0xffffffff  // the body is not in the scene yet
	#define SC_NOT_IN_ACTIVE_LIST_INDEX	0xfffffffe  // the body is in the scene but not in the active list

	class Scene;
	class ConstraintSim;
	class ArticulationSim;

	static const PxReal ScInternalWakeCounterResetValue = 20.0f*0.02f;

	class BodySim : public RigidSim
	{
	public:
		enum InternalFlags
		{
			//BF_DISABLE_GRAVITY		= 1 << 0,	// Don't apply the scene's gravity

			BF_HAS_STATIC_TOUCH		= 1 << 1,	// Set when a body is part of an island with static contacts. Needed to be able to recalculate adaptive force if this changes
			BF_KINEMATIC_MOVED		= 1 << 2,	// Set when the kinematic was moved

			BF_ON_DEATHROW			= 1 << 3,	// Set when the body is destroyed

			BF_IS_IN_SLEEP_LIST		= 1 << 4,	// Set when the body is added to the list of bodies which were put to sleep
			BF_IS_IN_WAKEUP_LIST	= 1 << 5,	// Set when the body is added to the list of bodies which were woken up
			BF_SLEEP_NOTIFY			= 1 << 6,	// A sleep notification should be sent for this body (and not a wakeup event, even if the body is part of the woken list as well)
			BF_WAKEUP_NOTIFY		= 1 << 7,	// A wake up notification should be sent for this body (and not a sleep event, even if the body is part of the sleep list as well)

			BF_HAS_CONSTRAINTS		= 1 << 8,	// Set if the body has one or more constraints
			BF_KINEMATIC_SETTLING	= 1 << 9,	// Set when the body was moved kinematically last frame
			BF_KINEMATIC_SETTLING_2 = 1 << 10,
			BF_KINEMATIC_MOVE_FLAGS = BF_KINEMATIC_MOVED | BF_KINEMATIC_SETTLING | BF_KINEMATIC_SETTLING_2 //Used to clear kinematic masks in 1 call

			// PT: WARNING: flags stored on 16-bits now.
		};

	public:
												BodySim(Scene&, BodyCore&);
		virtual									~BodySim();

						void					notifyAddSpatialAcceleration();
						void					notifyClearSpatialAcceleration();
						void					notifyAddSpatialVelocity();
						void					notifyClearSpatialVelocity();
						void					updateCached(Cm::BitMapPinned* shapeChangedMap);
						void					updateCached(PxsTransformCache& transformCache, Bp::BoundsArray& boundsArray);
						void					updateContactDistance(PxReal* contactDistance, const PxReal dt, Bp::BoundsArray& boundsArray);

		// hooks for actions in body core when it's attached to a sim object. Generally
		// we get called after the attribute changed.
			
		virtual			void					postActorFlagChange(PxU32 oldFlags, PxU32 newFlags);
						void					postBody2WorldChange();
						void					postSetWakeCounter(PxReal t, bool forceWakeUp);
						void					postSetKinematicTarget();
						void					postSwitchToKinematic();
						void					postSwitchToDynamic();
						void					postPosePreviewChange(const PxU32 posePreviewFlag);  // called when PxRigidBodyFlag::eENABLE_POSE_INTEGRATION_PREVIEW changes

		PX_FORCE_INLINE const PxTransform&		getBody2World()		const	{ return getBodyCore().getCore().body2World;		}
		PX_FORCE_INLINE const PxTransform&		getBody2Actor()		const	{ return getBodyCore().getCore().getBody2Actor();	}
		PX_FORCE_INLINE const PxsRigidBody&		getLowLevelBody()	const	{ return mLLBody;									}
		PX_FORCE_INLINE	PxsRigidBody&			getLowLevelBody()			{ return mLLBody;									}
						void					wakeUp();  // note: for user API call purposes only, i.e., use from BodyCore. For simulation internal purposes there is internalWakeUp().
						void					putToSleep();

		static			PxU32					getRigidBodyOffset()		{ return  PxU32(PX_OFFSET_OF_RT(BodySim, mLLBody));}

		//---------------------------------------------------------------------------------
		// Actor implementation
		//---------------------------------------------------------------------------------
	protected:
		virtual			void					onActivate();
		virtual			void					onDeactivate();

	private:
		//---------------------------------------------------------------------------------
		// Constraint projection
		//---------------------------------------------------------------------------------
	public:
		PX_FORCE_INLINE	ConstraintGroupNode*	getConstraintGroup()								{ return mConstraintGroup; }
		PX_FORCE_INLINE	void					setConstraintGroup(ConstraintGroupNode* node)		{ mConstraintGroup = node; }

		//// A list of active projection trees in the scene might be better
		//PX_FORCE_INLINE void					projectPose() { PX_ASSERT(mConstraintGroup); ConstraintGroupNode::projectPose(*mConstraintGroup); }

		//---------------------------------------------------------------------------------
		// Kinematics
		//---------------------------------------------------------------------------------
	public:
		PX_FORCE_INLINE bool					isKinematic()								const	{ return getBodyCore().getFlags() & PxRigidBodyFlag::eKINEMATIC;	}
		PX_FORCE_INLINE bool					isArticulationLink()						const	{ return getActorType() == PxActorType::eARTICULATION_LINK; }
						void					calculateKinematicVelocity(PxReal oneOverDt);
						void					updateKinematicPose();
						bool					deactivateKinematic();

	private:
		PX_FORCE_INLINE void					initKinematicStateBase(BodyCore&, bool asPartOfCreation);

		//---------------------------------------------------------------------------------
		// Sleeping
		//---------------------------------------------------------------------------------
	public:
		PX_FORCE_INLINE	bool					isActive() const { return (mActiveListIndex < SC_NOT_IN_ACTIVE_LIST_INDEX); }
						void					setActive(bool active, PxU32 infoFlag=0);  // see ActivityChangeInfoFlag

						void					activateInteractions(PxU32 infoFlag);
						void					deactivateInteractions(PxU32 infoFlag);

		PX_FORCE_INLINE PxU32					getActiveListIndex() const { return mActiveListIndex; }  // if the body is active, the index is smaller than SC_NOT_IN_ACTIVE_LIST_INDEX
		PX_FORCE_INLINE void					setActiveListIndex(PxU32 index) { mActiveListIndex = index; }

						void					internalWakeUp(PxReal wakeCounterValue=ScInternalWakeCounterResetValue);
						void					internalWakeUpArticulationLink(PxReal wakeCounterValue);	// called by ArticulationSim to wake up this link

						PxReal					updateWakeCounter(PxReal dt, PxReal energyThreshold, const Cm::SpatialVector& motionVelocity);

						void					resetSleepFilter();
						void					notifyReadyForSleeping();			// inform the sleep island generation system that the body is ready for sleeping
						void					notifyNotReadyForSleeping();		// inform the sleep island generation system that the body is not ready for sleeping
		PX_FORCE_INLINE bool					checkSleepReadinessBesidesWakeCounter();  // for API triggered changes to test sleep readiness

		PX_FORCE_INLINE void					registerCountedInteraction() { mLLBody.getCore().numCountedInteractions++; PX_ASSERT(mLLBody.getCore().numCountedInteractions); }
		PX_FORCE_INLINE void					unregisterCountedInteraction() { PX_ASSERT(mLLBody.getCore().numCountedInteractions); mLLBody.getCore().numCountedInteractions--;}
		PX_FORCE_INLINE PxU32					getNumCountedInteractions()	const { return mLLBody.getCore().numCountedInteractions; }

		PX_FORCE_INLINE Ps::IntBool				isFrozen()	const	{ return Ps::IntBool(mLLBody.mInternalFlags & PxsRigidBody::eFROZEN);	}
		PX_FORCE_INLINE void					setFrozen()			{ mLLBody.mInternalFlags |= PxsRigidBody::eFROZEN;						}
		PX_FORCE_INLINE void					clearFrozen()		{ mLLBody.mInternalFlags &= (~PxsRigidBody::eFROZEN);					}
	private:
		PX_FORCE_INLINE	void					notifyWakeUp(bool wakeUpInIslandGen = false);					// inform the sleep island generation system that the object got woken up
		PX_FORCE_INLINE	void					notifyPutToSleep();				// inform the sleep island generation system that the object was put to sleep
		PX_FORCE_INLINE void					internalWakeUpBase(PxReal wakeCounterValue);

		//---------------------------------------------------------------------------------
		// External velocity changes
		//---------------------------------------------------------------------------------
	public:

						void					updateForces(PxReal dt, PxsRigidBody** updatedBodySims, PxU32* updatedBodyNodeIndices, 
													PxU32& index, Cm::SpatialVector* acceleration, bool simUsesAdaptiveForce);
	private:
		PX_FORCE_INLINE void					raiseVelocityModFlag(VelocityModFlags f)				{ mVelModState |= f;					}
		PX_FORCE_INLINE void					clearVelocityModFlag(VelocityModFlags f)				{ mVelModState &= ~f;					}
		PX_FORCE_INLINE bool					readVelocityModFlag(VelocityModFlags f)					{ return (mVelModState & f) != 0;		}
		PX_FORCE_INLINE void					setForcesToDefaults(bool enableGravity);

		//---------------------------------------------------------------------------------
		// Miscellaneous
		//---------------------------------------------------------------------------------
	public:
		PX_FORCE_INLINE	PxU16					getInternalFlag()								const	{ return mInternalFlags;  }
		PX_FORCE_INLINE bool					readInternalFlag(InternalFlags flag)			const	{ return (mInternalFlags & flag) != 0;	}
		PX_FORCE_INLINE void					raiseInternalFlag(InternalFlags flag)					{ mInternalFlags |= flag;				}
		PX_FORCE_INLINE void					clearInternalFlag(InternalFlags flag)					{ mInternalFlags &= ~flag;				}
		PX_FORCE_INLINE PxU32					getFlagsFast()									const	{ return getBodyCore().getFlags();		}

		PX_FORCE_INLINE void					incrementBodyConstraintCounter()						{ mLLBody.mCore->numBodyInteractions++;					}
		PX_FORCE_INLINE void					decrementBodyConstraintCounter()						{ PX_ASSERT(mLLBody.mCore->numBodyInteractions>0); mLLBody.mCore->numBodyInteractions--; }

		PX_FORCE_INLINE	BodyCore&				getBodyCore()									const	{ return static_cast<BodyCore&>(getRigidCore());		}

		PX_INLINE		ArticulationSim*		getArticulation()								const	{ return mArticulation; }
						void 					setArticulation(ArticulationSim* a, PxReal wakeCounter, bool asleep, PxU32 bodyIndex);

		PX_FORCE_INLINE IG::NodeIndex			getNodeIndex() const									{ return mNodeIndex; }

						bool					isConnectedTo(const ActorSim& other, bool& collisionDisabled) const;  // Check if connected to specified object by a constraint
		PX_FORCE_INLINE void					onConstraintAttach()									{ raiseInternalFlag(BF_HAS_CONSTRAINTS); registerCountedInteraction(); }
						void					onConstraintDetach();

		PX_FORCE_INLINE	void					onOriginShift(const PxVec3& shift)						{ mLLBody.mLastTransform.p -= shift; }

		PX_FORCE_INLINE	bool					notInScene()									const	{ return mActiveListIndex == SC_NOT_IN_SCENE_INDEX; }

		PX_FORCE_INLINE bool					usingSqKinematicTarget()						const	
		{ 	
			PxU32 ktFlags(PxRigidBodyFlag::eUSE_KINEMATIC_TARGET_FOR_SCENE_QUERIES | PxRigidBodyFlag::eKINEMATIC);
			return (getFlagsFast()&ktFlags) == ktFlags;
		}

		PX_FORCE_INLINE	PxU32					getNbShapes()									const	{ return this->mNumElements; }

						void					createSqBounds();
						void					destroySqBounds();
						void					freezeTransforms(Cm::BitMapPinned* shapeChangedMap);
						void					invalidateSqBounds();

	private:

		//---------------------------------------------------------------------------------
		// Base body
		//---------------------------------------------------------------------------------
						PxsRigidBody			mLLBody;

		//---------------------------------------------------------------------------------
		// Island manager
		//---------------------------------------------------------------------------------
						IG::NodeIndex			mNodeIndex;

		//---------------------------------------------------------------------------------
		// External velocity changes
		//---------------------------------------------------------------------------------
		// VelocityMod data allocated on the fly when the user applies velocity changes
		// which need to be accumulated.
		// VelMod dirty flags stored in BodySim so we can save ourselves the expense of looking at 
		// the separate velmod data if no forces have been set.
						PxU16					mInternalFlags;
						PxU8					mVelModState;


		//---------------------------------------------------------------------------------
		// Sleeping
		//---------------------------------------------------------------------------------
						PxU32					mActiveListIndex;	// Used by Scene to track active bodies

		//---------------------------------------------------------------------------------
		// Articulation
		//---------------------------------------------------------------------------------
						ArticulationSim*		mArticulation;				// NULL if not in an articulation

		//---------------------------------------------------------------------------------
		// Joints & joint groups
		//---------------------------------------------------------------------------------

		// This is a tree data structure that gives us the projection order of joints in which this body is the tree root.
		// note: the link of the root body is not necces. the root link due to the re-rooting of the articulation!
						ConstraintGroupNode*	mConstraintGroup;
	};

} // namespace Sc



PX_FORCE_INLINE void Sc::BodySim::setForcesToDefaults(bool enableGravity)
{
	SimStateData* simStateData = getBodyCore().getSimStateData(false);
	if(simStateData) 
	{
		VelocityMod* velmod = simStateData->getVelocityModData();
		velmod->clear();
	}

	if (enableGravity)
		mVelModState = VMF_GRAVITY_DIRTY;	// We want to keep the gravity flag to make sure the acceleration gets changed to gravity-only
											// in the next step (unless the application adds new forces of course)
	else
		mVelModState = 0;
}


PX_FORCE_INLINE bool Sc::BodySim::checkSleepReadinessBesidesWakeCounter()
{
	const BodyCore& bodyCore = getBodyCore();
	const SimStateData* simStateData = bodyCore.getSimStateData(false);
	const VelocityMod* velmod = simStateData ? simStateData->getVelocityModData() : NULL;

	bool readyForSleep = bodyCore.getLinearVelocity().isZero() && bodyCore.getAngularVelocity().isZero();
	if (readVelocityModFlag(VMF_ACC_DIRTY))
	{
		readyForSleep = readyForSleep && (!velmod || velmod->getLinearVelModPerSec().isZero());
		readyForSleep = readyForSleep && (!velmod || velmod->getAngularVelModPerSec().isZero());
	}
	if (readVelocityModFlag(VMF_VEL_DIRTY))
	{
		readyForSleep = readyForSleep && (!velmod || velmod->getLinearVelModPerStep().isZero());
		readyForSleep = readyForSleep && (!velmod || velmod->getAngularVelModPerStep().isZero());
	}

	return readyForSleep;
}


}

#endif
