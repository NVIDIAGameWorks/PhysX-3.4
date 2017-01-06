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

#include "ScbAggregate.h"
#include "ScbActor.h"

using namespace physx;

void Scb::Aggregate::addActor(Scb::Actor& actor)
{
	const ControlState::Enum state = getControlState();

	if(!isBufferingSpecial(state))
	{
		actor.getActorCore().setAggregateID(mAggregateID);
		PvdAttachActorToAggregate( this, &actor );
		PvdUpdateProperties( this );
	}
	else if((state != ControlState::eREMOVE_PENDING))  // If the aggregate is pending for deletion, adding/removing an actor should not be double buffered because the aggregateID must not be set for the actors
	{
		// if available, search in list of removed actors to cover the remove-add case
		Scb::AggregateBuffer* PX_RESTRICT bufferedData = getBufferedData();
		if(bufferedData->removeBufferIdx != 0xffffffff)
		{
			Scb::Actor** removeBuffer = getScbScene()->getActorBuffer(bufferedData->removeBufferIdx);
			for(PxU32 i=0; i<bufferedData->removeCount; i++)
			{
				if(removeBuffer[i] == &actor)
				{
					removeBuffer[i] = removeBuffer[bufferedData->removeCount - 1];
					PX_ASSERT(bufferedData->removeCount > 0);
					bufferedData->removeCount--;
					break;
				}
			}
		}

		Scb::Actor** actorBuffer;
		if(bufferedData->addBufferIdx == 0xffffffff)
			actorBuffer = getScbScene()->allocActorBuffer(mMaxNbActors, bufferedData->addBufferIdx);
		else
			actorBuffer = getScbScene()->getActorBuffer(bufferedData->addBufferIdx);

		PX_ASSERT(bufferedData->addCount < mMaxNbActors);
		actorBuffer[bufferedData->addCount] = &actor;
		bufferedData->addCount++;

		if(state != ControlState::eINSERT_PENDING)
			markUpdated(BF_ADD_ACTOR);
		else
		{
			// Not a great solution but aggregates are special in the sense that even in the pending insert case, data needs to be double buffered
			// (see isBufferingSpecial() for details)
			setBufferFlag(BF_ADD_ACTOR);
		}
	}
}

void Scb::Aggregate::removeActor(Scb::Actor& actor, bool reinsert)
{
	const ControlState::Enum state = getControlState();
	const ControlState::Enum actorState = actor.getControlState();

	if(!isBufferingSpecial(state))
	{
		Sc::ActorCore& ac = actor.getActorCore();
		ac.setAggregateID(PX_INVALID_U32);
		
		if(getScbSceneForAPI() && reinsert)
			ac.reinsertShapes();
	}
	else if((state != ControlState::eREMOVE_PENDING))
	{
		// if available, search in list of added actors to cover the add-remove case
		Scb::AggregateBuffer* PX_RESTRICT bufferedData = getBufferedData();
		if(bufferedData->addBufferIdx != 0xffffffff)
		{
			Scb::Actor** addBuffer = getScbScene()->getActorBuffer(bufferedData->addBufferIdx);
			for(PxU32 i=0; i<bufferedData->addCount; i++)
			{
				if(addBuffer[i] == &actor)
				{
					addBuffer[i] = addBuffer[bufferedData->addCount - 1];
					PX_ASSERT(bufferedData->addCount > 0);
					bufferedData->addCount--;
					return;  // It's fine to abort here because the aggregateID has not been set yet
				}
			}
		}

		Scb::Actor** actorBuffer;
		if(bufferedData->removeBufferIdx == 0xffffffff)
			actorBuffer = getScbScene()->allocActorBuffer(mMaxNbActors, bufferedData->removeBufferIdx);
		else
			actorBuffer = getScbScene()->getActorBuffer(bufferedData->removeBufferIdx);

		PX_ASSERT(bufferedData->removeCount < mMaxNbActors);
		actorBuffer[bufferedData->removeCount] = &actor;
		bufferedData->removeCount++;

		markUpdated(BF_REMOVE_ACTOR);
	}

	//Update pvd status if not buffer OR the actor release while aggregate is already in scene
	if(!isBufferingSpecial(state) 
		|| ((actorState == ControlState::eIN_SCENE || actorState == ControlState::eREMOVE_PENDING ) 
			&& state != ControlState::eINSERT_PENDING
			&& !reinsert))
	{
		PvdDetachActorFromAggregate( this, &actor );
		PvdUpdateProperties( this );
	}
}
