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

#ifndef PX_PHYSICS_SCP_ACTOR_SIM
#define PX_PHYSICS_SCP_ACTOR_SIM

#include "PsUserAllocated.h"
#include "CmPhysXCommon.h"
#include "CmUtils.h"
#include "PxActor.h"
#include "ScInteractionFlags.h"
#include "ScActorCore.h"

namespace physx
{

class PxActor;

namespace Sc
{
	class Interaction;
	class ElementSim;
	
	class ActorSim : public Ps::UserAllocated
	{
		friend class Scene;  // the scene is allowed to set the scene array index
		friend class Interaction;
		PX_NOCOPY(ActorSim)

	public:
		enum ActivityChangeInfoFlag
		{
			AS_PART_OF_CREATION				= (1 << 0),
			AS_PART_OF_ISLAND_GEN			= (1 << 1)
		};

											ActorSim(Scene&, ActorCore&);
		virtual								~ActorSim();

		// Get the scene the actor resides in
		PX_FORCE_INLINE	Scene&				getScene()					const	{ return mScene; }

		// Get the number of interactions connected to the actor
		PX_FORCE_INLINE	PxU32				getActorInteractionCount()	const	{ return mInteractions.size(); }

		// Prepares the actor for less than n interactions
						void				setInteractionCountHint(PxU32 n)	{ mInteractions.reserve(n, *this); }

		// Get an iterator to the interactions connected to the actor
		PX_FORCE_INLINE	Interaction**		getActorInteractions()		const	{ return mInteractions.begin();	}

		// Get first element in the actor (linked list)
		PX_FORCE_INLINE	ElementSim*			getElements_()						{ return mFirstElement;		}
		PX_FORCE_INLINE	const ElementSim*	getElements_()				const	{ return mFirstElement;		}

		// Get the type ID of the actor
		PX_FORCE_INLINE	PxActorType::Enum	getActorType()				const	{ return mCore.getActorCoreType();	}

		// Returns true if the actor is a dynamic rigid body (including articulation links)
		PX_FORCE_INLINE	bool				isDynamicRigid()			const	{ const PxActorType::Enum type = getActorType(); return type == PxActorType::eRIGID_DYNAMIC || type == PxActorType::eARTICULATION_LINK; }

						void				onElementAttach(ElementSim& element);
						void				onElementDetach(ElementSim& element);

		virtual			void				postActorFlagChange(PxU32, PxU32) {}

						void				postDominanceGroupChange();

						void				setActorsInteractionsDirty(InteractionDirtyFlag::Enum flag, const ActorSim* other, PxU8 interactionFlag);

		PX_FORCE_INLINE	ActorCore&			getActorCore() const { return mCore; }

	private:
		//These are called from interaction creation/destruction
						void				registerInteraction(Interaction* interaction);
						void				unregisterInteraction(Interaction* interaction);

						void				reallocInteractions(Sc::Interaction**& mem, PxU32& capacity, PxU32 size, PxU32 requiredMinCapacity);
	protected:
		// dsequeira: interaction arrays are a major cause of small allocations, so we don't want to delegate them to the heap allocator
		// it's not clear this inline array is really needed, we should take it out and see whether the cache perf is worse

		static const PxU32 INLINE_INTERACTION_CAPACITY = 4;
						Interaction*		mInlineInteractionMem[INLINE_INTERACTION_CAPACITY];

		Cm::OwnedArray<Sc::Interaction*, Sc::ActorSim, PxU32, &Sc::ActorSim::reallocInteractions>
											mInteractions;

						ElementSim*			mFirstElement;

						Scene&				mScene;

						ActorCore&			mCore;
	};

} // namespace Sc

}

#endif
