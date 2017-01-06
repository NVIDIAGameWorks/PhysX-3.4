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


#ifndef PX_PHYSICS_SCP_ACTOR_ELEMENT_PAIR
#define PX_PHYSICS_SCP_ACTOR_ELEMENT_PAIR

#include "ScActorSim.h"
#include "ScElementSim.h"

namespace physx
{
namespace Sc
{

	// Class shared by all element-element pairs where one element should not be distinguished from
	// its actor (for example: rb-fluid, ...)
	class ActorElementPair
	{
	public:
		PX_INLINE ActorElementPair(ActorSim& actor, ElementSim& element, PxPairFlags pairFlag);
		PX_INLINE ~ActorElementPair();
		
		PX_INLINE ElementSim& getElement() const { return mElement; }
		PX_INLINE ActorSim& getActor() const { return mActor; }

		PX_INLINE void incRefCount() { ++mRefCount; PX_ASSERT(mRefCount>0); }
		PX_INLINE PxU32 decRefCount() { PX_ASSERT(mRefCount>0); return --mRefCount; }
		PX_INLINE PxU32 getRefCount() const { return mRefCount; }

		PX_INLINE PxPairFlags getPairFlags() const { return mPairFlags; }
		PX_INLINE void setPairFlags(PxPairFlags pairFlags) { mPairFlags = pairFlags; }

		PX_INLINE bool isFilterPair() const { return mIsFilterPair; }
		PX_INLINE void markAsFilterPair(bool filterPair) { mIsFilterPair = filterPair; }

		PX_INLINE bool isSuppressed() const { return mIsSuppressed; }
		PX_INLINE void markAsSuppressed(bool suppress) { mIsSuppressed = suppress; }

		PX_INLINE bool isKilled() const { return mIsKilled; }
		PX_INLINE void markAsKilled(bool killed) { mIsKilled = killed; }

		PX_INLINE bool hasBeenRefiltered(PxU32 sceneTimestamp);

	private:
		ActorElementPair& operator=(const ActorElementPair&);
		ActorSim& mActor;
		ElementSim& mElement;
		PxPairFlags mPairFlags;
		PxU32 mRefilterTimestamp;
		PxU16 mRefCount;
		bool mIsFilterPair;
		bool mIsSuppressed;
		bool mIsKilled;
	};

} // namespace Sc


Sc::ActorElementPair::ActorElementPair(ActorSim& actor, ElementSim& element, PxPairFlags pairFlag) : 
	mActor(actor),
	mElement(element),
	mPairFlags(pairFlag),
	mRefilterTimestamp(0),
	mRefCount(0),
	mIsFilterPair(false),
	mIsSuppressed(false),
	mIsKilled(false)
{ 
}


Sc::ActorElementPair::~ActorElementPair()
{
}


PX_INLINE bool Sc::ActorElementPair::hasBeenRefiltered(PxU32 sceneTimestamp)
{
	if (mRefilterTimestamp != sceneTimestamp)
	{
		mRefilterTimestamp = sceneTimestamp;
		return false;
	}
	else
		return true;
}

}

#endif
