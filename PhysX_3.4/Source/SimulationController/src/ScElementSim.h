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


#ifndef PX_PHYSICS_SCP_ELEMENT_SIM
#define PX_PHYSICS_SCP_ELEMENT_SIM

#include "PsUserAllocated.h"
#include "PxFiltering.h"
#include "PxvConfig.h"
#include "ScActorSim.h"
#include "ScInteraction.h"
#include "BpSimpleAABBManager.h"
#include "ScObjectIDTracker.h"

namespace physx
{
namespace Sc
{
	class ElementSimInteraction;

	struct ElementType
	{
		enum Enum
		{
			eSHAPE = 0,
#if PX_USE_PARTICLE_SYSTEM_API
			ePARTICLE_PACKET,
#endif
#if PX_USE_CLOTH_API
			eCLOTH,
#endif
			eCOUNT,
			eTRIGGER = eCOUNT
		};
	};

	PX_COMPILE_TIME_ASSERT(ElementType::eCOUNT <= 4);			//2 bits reserved for type on win32 and win64 (8 bits on other platforms)

	/*
	A ElementSim is a part of a ActorSim. It contributes to the activation framework by adding its 
	interactions to the actor. */
	PX_ALIGN_PREFIX(16)
	class ElementSim : public Ps::UserAllocated
	{
		PX_NOCOPY(ElementSim)

	public:
		class ElementInteractionIterator
		{
			public:
				PX_FORCE_INLINE			ElementInteractionIterator(const ElementSim& e, PxU32 nbInteractions, Interaction** interactions) :
					mInteractions(interactions), mInteractionsLast(interactions + nbInteractions), mElement(&e) {}
				ElementSimInteraction*	getNext();

			private:
				Interaction**					mInteractions;
				Interaction**					mInteractionsLast;
				const ElementSim*				mElement;
		};

		class ElementInteractionReverseIterator
		{
			public:
				PX_FORCE_INLINE			ElementInteractionReverseIterator(const ElementSim& e, PxU32 nbInteractions, Interaction** interactions) :
					mInteractions(interactions), mInteractionsLast(interactions + nbInteractions), mElement(&e) {}
				ElementSimInteraction*	getNext();

			private:
				Interaction**					mInteractions;
				Interaction**					mInteractionsLast;
				const ElementSim*				mElement;
		};


												ElementSim(ActorSim& actor, ElementType::Enum type);
		virtual									~ElementSim();

		// Get an iterator to the interactions connected to the element
		PX_FORCE_INLINE	ElementInteractionIterator getElemInteractions()	const	{ return ElementInteractionIterator(*this, mActor.getActorInteractionCount(), mActor.getActorInteractions()); }
		PX_FORCE_INLINE	ElementInteractionReverseIterator getElemInteractionsReverse()	const	{ return ElementInteractionReverseIterator(*this, mActor.getActorInteractionCount(), mActor.getActorInteractions()); }

		PX_FORCE_INLINE	ActorSim&				getActor()					const	{ return mActor; }

		PX_FORCE_INLINE	Scene&					getScene()					const	{ return mActor.getScene();	}

		PX_FORCE_INLINE	ElementType::Enum		getElementType()			const	{ return ElementType::Enum(mType); }

		PX_FORCE_INLINE PxU32					getElementID()				const	{ return mElementID;	}
		PX_FORCE_INLINE bool					isInBroadPhase()			const	{ return mInBroadPhase;	}
		
						void					addToAABBMgr(PxReal contactDistance, PxU32 group, bool isTrigger);
						void					removeFromAABBMgr();

		//---------- Filtering ----------
		virtual			void					getFilterInfo(PxFilterObjectAttributes& filterAttr, PxFilterData& filterData) const = 0;

		PX_FORCE_INLINE void					setFilterObjectAttributeType(PxFilterObjectAttributes& attr, PxFilterObjectType::Enum type) const;
		//-------------------------------

						void					setElementInteractionsDirty(InteractionDirtyFlag::Enum flag, PxU8 interactionFlag);

		PX_FORCE_INLINE	void					initID()
		{
			Scene& scene = getScene();
			mElementID = scene.getElementIDPool().createID();
			scene.getBoundsArray().initEntry(mElementID);
		}

		PX_FORCE_INLINE	void					releaseID()
		{
			getScene().getElementIDPool().releaseID(mElementID);
		}

	public:
						ElementSim*				mNextInActor;
	private:
						ActorSim&				mActor;

						PxU32					mElementID : 29;
						PxU32					mType : 2;
						PxU32					mInBroadPhase : 1;
	}
	PX_ALIGN_SUFFIX(16);
} // namespace Sc

// SFD: duplicated attribute generation in SqFiltering.h
PX_FORCE_INLINE void Sc::ElementSim::setFilterObjectAttributeType(PxFilterObjectAttributes& attr, PxFilterObjectType::Enum type) const
{
	PX_ASSERT((attr & (PxFilterObjectType::eMAX_TYPE_COUNT-1)) == 0);
	attr |= type;
}


}

#endif
