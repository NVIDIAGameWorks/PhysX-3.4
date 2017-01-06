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


#ifndef PX_PHYSICS_NP_QUERYSHARED
#define PX_PHYSICS_NP_QUERYSHARED

#include "foundation/PxMemory.h"

namespace physx
{

using namespace Cm;

template <class ActorT>
PX_FORCE_INLINE bool applyClientFilter(ActorT* actor, const PxQueryFilterData& filterData, const NpSceneQueries& scene)
{
	if(filterData.clientId != actor->getOwnerClient())
	{
		const bool passForeignShapes = scene.getClientBehaviorFlags(filterData.clientId) & PxClientBehaviorFlag::eREPORT_FOREIGN_OBJECTS_TO_SCENE_QUERY;
		const bool reportToForeignClients = actor->getClientBehaviorFlags() & PxActorClientBehaviorFlag::eREPORT_TO_FOREIGN_CLIENTS_SCENE_QUERY;
		if(!(passForeignShapes && reportToForeignClients))
			return false;
	}
	return true;
}

PX_FORCE_INLINE bool applyFilterEquation(const Scb::Shape& scbShape, const PxFilterData& queryFd)
{
	// if the filterData field is non-zero, and the bitwise-AND value of filterData AND the shape's
	// queryFilterData is zero, the shape is skipped.
	if(queryFd.word0 | queryFd.word1 | queryFd.word2 | queryFd.word3)
	{
		const PxFilterData& objFd = scbShape.getScShape().getQueryFilterData();
		const PxU32 keep = (queryFd.word0 & objFd.word0) | (queryFd.word1 & objFd.word1) | (queryFd.word2 & objFd.word2) | (queryFd.word3 & objFd.word3);
		if(!keep)
			return false;
	}
	return true;
}

//========================================================================================================================
// these partial template specializations are used to generalize the query code to be reused for all permutations of
// hit type=(raycast, overlap, sweep) x query type=(ANY, SINGLE, MULTIPLE)
template <typename HitType> struct HitTypeSupport { enum { IsRaycast = 0, IsSweep = 0, IsOverlap = 0 }; };
template <> struct HitTypeSupport<PxRaycastHit>
{
	enum { IsRaycast = 1, IsSweep = 0, IsOverlap = 0 };
	static PX_FORCE_INLINE PxReal getDistance(const PxQueryHit& hit) { return static_cast<const PxRaycastHit&>(hit).distance; }
};
template <> struct HitTypeSupport<PxSweepHit>
{
	enum { IsRaycast = 0, IsSweep = 1, IsOverlap = 0 };
	static PX_FORCE_INLINE PxReal getDistance(const PxQueryHit& hit) { return static_cast<const PxSweepHit&>(hit).distance; }
};
template <> struct HitTypeSupport<PxOverlapHit>
{
	enum { IsRaycast = 0, IsSweep = 0, IsOverlap = 1 };
	static PX_FORCE_INLINE PxReal getDistance(const PxQueryHit&) { return -1.0f; }
};

#define HITDIST(hit) HitTypeSupport<HitType>::getDistance(hit)

template<typename HitType>
static PxU32 clipHitsToNewMaxDist(HitType* ppuHits, PxU32 count, PxReal newMaxDist)
{
	PxU32 i=0;
	while(i!=count)
	{
		if(HITDIST(ppuHits[i]) > newMaxDist)
			ppuHits[i] = ppuHits[--count];
		else
			i++;
	}
	return count;
}

} // namespace physx

#endif // PX_PHYSICS_NP_QUERYSHARED
