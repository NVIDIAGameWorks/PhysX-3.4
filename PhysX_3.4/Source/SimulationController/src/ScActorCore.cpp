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


#include "ScActorCore.h"
#include "ScActorSim.h"
#include "ScShapeIterator.h"
#include "ScShapeCore.h"
#include "ScShapeSim.h"
#include "ScBodySim.h"

using namespace physx;

Sc::ActorCore::ActorCore(PxActorType::Enum actorType, PxU8 actorFlags, PxClientID owner, PxU8 behavior, PxDominanceGroup dominanceGroup) :
	mSim					(NULL),
	mAggregateIDOwnerClient	((PxU32(owner)<<24)|0x00ffffff),
	mActorFlags				(actorFlags),
	mActorType				(PxU8(actorType)),
	mClientBehaviorFlags	(behavior),
	mDominanceGroup			(dominanceGroup)
{
	PX_ASSERT((actorType & 0xff) == actorType);
}

Sc::ActorCore::~ActorCore()
{
}

void Sc::ActorCore::setActorFlags(PxActorFlags af)	
{ 
	const PxActorFlags old = mActorFlags;
	if(af!=old)
	{
		mActorFlags = af;

		if(mSim)
			mSim->postActorFlagChange(old, af);
	}
}	

void Sc::ActorCore::setDominanceGroup(PxDominanceGroup g)
{
	PX_ASSERT(g<128);
	mDominanceGroup = g;
	if(mSim)
		mSim->postDominanceGroupChange();
}

void Sc::ActorCore::reinsertShapes()
{
	PX_ASSERT(mSim);
	if(!mSim)
		return;

	ShapeSim* sim = NULL;
	for (ShapeIterator shapeIterator(*mSim); NULL != (sim = shapeIterator.getNext()); )
		sim->reinsertBroadPhase();
}
