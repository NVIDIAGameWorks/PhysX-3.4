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


#ifndef PX_PHYSICS_SCP_INTERACTION
#define PX_PHYSICS_SCP_INTERACTION

#include "foundation/Px.h"
#include "ScInteractionFlags.h"
#include "ScScene.h"
#include "ScActorSim.h"
#include "PsUserAllocated.h"
#include "PsUtilities.h"
#include "PsFoundation.h"

namespace physx
{

#define PX_INVALID_INTERACTION_ACTOR_ID 0xffffffff
#define PX_INVALID_INTERACTION_SCENE_ID 0xffffffff

namespace Sc
{
	// Interactions are used for connecting actors into activation
	// groups. An interaction always connects exactly two actors. 
	// An interaction is implicitly active if at least one of the two 
	// actors it connects is active.

	class Interaction : public Ps::UserAllocated
	{
		PX_NOCOPY(Interaction)

	protected:
										Interaction(ActorSim& actor0, ActorSim& actor1, InteractionType::Enum interactionType, PxU8 flags);
		virtual							~Interaction() { PX_ASSERT(!readInteractionFlag(InteractionFlag::eIN_DIRTY_LIST)); }

	public:
		// Interactions automatically register themselves in the actors here
		PX_FORCE_INLINE bool			registerInActors(void* data = NULL);

		// Interactions automatically unregister themselves from the actors here
		PX_FORCE_INLINE void			unregisterFromActors();

		PX_FORCE_INLINE	ActorSim&		getActor0() const { return mActor0; }
		PX_FORCE_INLINE	ActorSim&		getActor1() const { return mActor1; }

		PX_FORCE_INLINE Scene&			getScene() const { return mActor0.getScene(); }

		// PT: TODO: why do we have both virtual functions AND a type in there?
		PX_FORCE_INLINE	InteractionType::Enum getType() const { return InteractionType::Enum(mInteractionType); }

		PX_FORCE_INLINE PxU8			readInteractionFlag(PxU8 flag) const { return PxU8(mInteractionFlags & flag); }
		PX_FORCE_INLINE void			raiseInteractionFlag(InteractionFlag::Enum flag) { mInteractionFlags |= flag; }
		PX_FORCE_INLINE void			clearInteractionFlag(InteractionFlag::Enum flag) { mInteractionFlags &= ~flag; }

		PX_FORCE_INLINE	bool			isRegistered() const { return mSceneId != PX_INVALID_INTERACTION_SCENE_ID; }

		/**
		\brief Mark the interaction as dirty. This will put the interaction into a list that is processed once per simulation step.

		@see InteractionDirtyFlag
		*/
		PX_FORCE_INLINE void			setDirty(PxU32 dirtyFlags);

		/**
		\brief Clear all flags that mark the interaction as dirty and optionally remove the interaction from the list of dirty interactions.

		@see InteractionDirtyFlag
		*/
		/*PX_FORCE_INLINE*/ void			setClean(bool removeFromList);

		PX_FORCE_INLINE Ps::IntBool		needsRefiltering() const { return (getDirtyFlags() & InteractionDirtyFlag::eFILTER_STATE); }

		PX_FORCE_INLINE Ps::IntBool		isElementInteraction() const;

		// Called when an interaction is activated or created.
		// Return true if activation should proceed else return false (for example: joint interaction between two kinematics should not get activated)
		virtual			bool			onActivate(void* data) = 0;

		// Called when an interaction is deactivated.
		// Return true if deactivation should proceed else return false (for example: joint interaction between two kinematics can ignore deactivation because it always is deactivated)
		virtual			bool			onDeactivate(PxU32 infoFlag) = 0;

		PX_FORCE_INLINE	void			setInteractionId(PxU32 id) { mSceneId = id; }
		PX_FORCE_INLINE	PxU32			getInteractionId() const { return mSceneId; }

		PX_FORCE_INLINE	void			setActorId(ActorSim* actor, PxU32 id);
		PX_FORCE_INLINE void			invalidateActorId(ActorSim* actor);
		PX_FORCE_INLINE	PxU32			getActorId(const ActorSim* actor) const;
		

		PX_FORCE_INLINE PxU8			getDirtyFlags() const { return mDirtyFlags; }

	private:
						void			addToDirtyList();
						void			removeFromDirtyList();

						ActorSim&		mActor0;
						ActorSim&		mActor1;

						PxU32			mSceneId;	// PT: TODO: merge this with mInteractionType

		// PT: TODO: are those IDs even worth caching? Since the number of interactions per actor is (or should be) small,
		// we could just do a linear search and save memory here...
						PxU32			mActorId0;	// PT: id of this interaction within mActor0's mInteractions array
						PxU32			mActorId1;	// PT: id of this interaction within mActor1's mInteractions array
	protected:
						PxU8			mInteractionType;	// PT: stored on a byte to save space, should be InteractionType enum
						PxU8			mInteractionFlags;
						PxU8			mDirtyFlags;  // see DirtyFlag enum
						PxU8			mPadding;
	};

} // namespace Sc

//////////////////////////////////////////////////////////////////////////

PX_FORCE_INLINE bool Sc::Interaction::registerInActors(void* data)
{
	bool active = onActivate(data);

	mActor0.registerInteraction(this);
	mActor1.registerInteraction(this);

	return active;
}


PX_FORCE_INLINE void Sc::Interaction::unregisterFromActors()
{
	mActor0.unregisterInteraction(this);
	mActor1.unregisterInteraction(this);
}

PX_FORCE_INLINE void Sc::Interaction::invalidateActorId(ActorSim* actor)
{
	if (&mActor0 == actor)
		mActorId0 = PX_INVALID_INTERACTION_ACTOR_ID;
	else
		mActorId1 = PX_INVALID_INTERACTION_ACTOR_ID;
}


PX_FORCE_INLINE	void Sc::Interaction::setActorId(ActorSim* actor, PxU32 id)
{
	PX_ASSERT(id != PX_INVALID_INTERACTION_ACTOR_ID);
	if (&mActor0 == actor)
		mActorId0 = id;
	else
		mActorId1 = id;
}


PX_FORCE_INLINE	PxU32 Sc::Interaction::getActorId(const ActorSim* actor) const
{
	if (&mActor0 == actor)
		return mActorId0;
	else
		return mActorId1;
}


PX_FORCE_INLINE Ps::IntBool Sc::Interaction::isElementInteraction() const
{
	Ps::IntBool res = readInteractionFlag(InteractionFlag::eELEMENT_ELEMENT);

#if PX_USE_PARTICLE_SYSTEM_API
	PX_ASSERT(	(res && 
				((getType() == InteractionType::eOVERLAP)	|| 
				(getType() == InteractionType::eTRIGGER)	|| 
				(getType() == InteractionType::eMARKER)     || 
				(getType() == InteractionType::ePARTICLE_BODY))) ||
				(!res && 
				((getType() == InteractionType::eCONSTRAINTSHADER)	|| 
				(getType() == InteractionType::eARTICULATION))));	
#else
	PX_ASSERT(	(res && 
				((getType() == InteractionType::eOVERLAP)	|| 
				(getType() == InteractionType::eTRIGGER)	|| 
				(getType() == InteractionType::eMARKER))) ||
				(!res && 
				((getType() == InteractionType::eCONSTRAINTSHADER)	|| 
				(getType() == InteractionType::eARTICULATION))));
#endif
	return res;
}


PX_FORCE_INLINE void Sc::Interaction::setDirty(PxU32 dirtyFlags)
{
	PX_ASSERT(getType() != InteractionType::eARTICULATION);

	mDirtyFlags |= Ps::to8(dirtyFlags);
	if (!readInteractionFlag(InteractionFlag::eIN_DIRTY_LIST))
	{
		addToDirtyList();
		raiseInteractionFlag(InteractionFlag::eIN_DIRTY_LIST);
	}
}


//PX_FORCE_INLINE void Sc::Interaction::setClean(bool removeFromList)
//{
//	if (readInteractionFlag(InteractionFlag::eIN_DIRTY_LIST))
//	{
//		if (removeFromList)  // if we process all dirty interactions anyway, then we can just clear the list at the end and save the work here.
//			removeFromDirtyList();
//		clearInteractionFlag(InteractionFlag::eIN_DIRTY_LIST);
//	}
//
//	mDirtyFlags = 0;
//}


}

#endif // PX_PHYSICS_SCP_INTERACTION
