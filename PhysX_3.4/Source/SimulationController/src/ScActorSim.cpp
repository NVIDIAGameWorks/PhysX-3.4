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

#include "CmPhysXCommon.h"
#include "ScActorSim.h"
#include "ScActorCore.h"
#include "ScElementSim.h"
#include "ScScene.h"
#include "ScInteraction.h"

using namespace physx;

Sc::ActorSim::ActorSim(Scene& scene, ActorCore& core) :
	mFirstElement					(NULL),
	mScene							(scene),
	mCore							(core)
{
	core.setSim(this);
}

Sc::ActorSim::~ActorSim()
{
	mInteractions.releaseMem(*this);
}

void Sc::ActorSim::registerInteraction(Interaction* interaction)
{
	const PxU32 id = mInteractions.size();
	mInteractions.pushBack(interaction, *this);
	interaction->setActorId(this, id);
}

void Sc::ActorSim::unregisterInteraction(Interaction* interaction)
{
	const PxU32 i = interaction->getActorId(this);
	PX_ASSERT(i < mInteractions.size());
	mInteractions.replaceWithLast(i); 
	if (i<mInteractions.size())
		mInteractions[i]->setActorId(this, i);
}

void Sc::ActorSim::onElementAttach(ElementSim& element)
{
	element.mNextInActor = mFirstElement;
	mFirstElement = &element;
}

void Sc::ActorSim::onElementDetach(ElementSim& element)
{
	PX_ASSERT(mFirstElement);	// PT: else we shouldn't be called
	ElementSim* currentElem = mFirstElement;
	ElementSim* previousElem = NULL;
	while(currentElem)
	{
		if(currentElem==&element)
		{
			if(previousElem)
				previousElem->mNextInActor = currentElem->mNextInActor;
			else
				mFirstElement = currentElem->mNextInActor;
			return;
		}
		previousElem = currentElem;
		currentElem = currentElem->mNextInActor;
	}
	PX_ASSERT(0);
}

// PT: TODO: refactor with Sc::ParticlePacketShape::reallocInteractions
void Sc::ActorSim::reallocInteractions(Sc::Interaction**& mem, PxU32& capacity, PxU32 size, PxU32 requiredMinCapacity)
{
	Interaction** newMem;
	PxU32 newCapacity;

	if(requiredMinCapacity==0)
	{
		newCapacity = 0;
		newMem = 0;
	}
	else if(requiredMinCapacity<=INLINE_INTERACTION_CAPACITY)
	{
		newCapacity = INLINE_INTERACTION_CAPACITY;
		newMem = mInlineInteractionMem;
	}
	else
	{
		newCapacity = Ps::nextPowerOfTwo(requiredMinCapacity-1);
		newMem = reinterpret_cast<Interaction**>(mScene.allocatePointerBlock(newCapacity));
	}

	PX_ASSERT(newCapacity >= requiredMinCapacity && requiredMinCapacity>=size);

	if(mem)
	{
		PxMemCopy(newMem, mem, size*sizeof(Interaction*));

		if(mem!=mInlineInteractionMem)
			mScene.deallocatePointerBlock(reinterpret_cast<void**>(mem), capacity);
	}
	
	capacity = newCapacity;
	mem = newMem;
}

void Sc::ActorSim::postDominanceGroupChange()
{
	//force all related interactions to refresh, so they fetch new dominance values.
	setActorsInteractionsDirty(InteractionDirtyFlag::eDOMINANCE, NULL, InteractionFlag::eRB_ELEMENT);
}

void Sc::ActorSim::setActorsInteractionsDirty(InteractionDirtyFlag::Enum flag, const ActorSim* other, PxU8 interactionFlag)
{
	PxU32 size = getActorInteractionCount();
	Interaction** interactions = getActorInteractions();
	while(size--)
	{
		Interaction* interaction = *interactions++;
		if ((!other || other == &interaction->getActor0() || other == &interaction->getActor1()) &&
			(interaction->readInteractionFlag(interactionFlag)))
			interaction->setDirty(flag);
	}
}
