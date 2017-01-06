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

#include "PxSceneQueryExt.h"

using namespace physx;

bool PxSceneQueryExt::raycastAny(	const PxScene& scene,
									const PxVec3& origin, const PxVec3& unitDir, const PxReal distance,
									PxSceneQueryHit& hit, const PxSceneQueryFilterData& filterData,
									PxSceneQueryFilterCallback* filterCall, const PxSceneQueryCache* cache,
									PxClientID queryClient)
{
	PxSceneQueryFilterData fdAny = filterData;
	fdAny.flags |= PxQueryFlag::eANY_HIT;
	fdAny.clientId = queryClient;
	PxRaycastBuffer buf;
	scene.raycast(origin, unitDir, distance, buf, PxHitFlags(), fdAny, filterCall, cache);
	hit = buf.block;
	return buf.hasBlock;
}

bool PxSceneQueryExt::raycastSingle(const PxScene& scene,
									const PxVec3& origin, const PxVec3& unitDir, const PxReal distance,
									PxSceneQueryFlags outputFlags, PxRaycastHit& hit,
									const PxSceneQueryFilterData& filterData,
									PxSceneQueryFilterCallback* filterCall, const PxSceneQueryCache* cache,
									PxClientID queryClient)
{
	PxRaycastBuffer buf;
	PxQueryFilterData fd1 = filterData; fd1.clientId = queryClient;
	scene.raycast(origin, unitDir, distance, buf, outputFlags, fd1, filterCall, cache);
	hit = buf.block;
	return buf.hasBlock;
}

PxI32 PxSceneQueryExt::raycastMultiple(	const PxScene& scene,
										const PxVec3& origin, const PxVec3& unitDir, const PxReal distance,
										PxSceneQueryFlags outputFlags,
										PxRaycastHit* hitBuffer, PxU32 hitBufferSize, bool& blockingHit,
										const PxSceneQueryFilterData& filterData,
										PxSceneQueryFilterCallback* filterCall, const PxSceneQueryCache* cache,
										PxClientID queryClient)
{
	PxRaycastBuffer buf(hitBuffer, hitBufferSize);
	PxQueryFilterData fd1 = filterData; fd1.clientId = queryClient;
	scene.raycast(origin, unitDir, distance, buf, outputFlags, fd1, filterCall, cache);
	blockingHit = buf.hasBlock;
	if(blockingHit)
	{
		if(buf.nbTouches < hitBufferSize)
		{
			hitBuffer[buf.nbTouches] = buf.block;
			return PxI32(buf.nbTouches+1);
		}
		else // overflow, drop the last touch
		{
			hitBuffer[hitBufferSize-1] = buf.block;
			return -1;
		}
	} else
		// no block
		return PxI32(buf.nbTouches);
}

bool PxSceneQueryExt::sweepAny(	const PxScene& scene,
								const PxGeometry& geometry, const PxTransform& pose, const PxVec3& unitDir, const PxReal distance,
								PxSceneQueryFlags queryFlags,
								PxSceneQueryHit& hit,
								const PxSceneQueryFilterData& filterData,
								PxSceneQueryFilterCallback* filterCall,
								const PxSceneQueryCache* cache,
								PxClientID queryClient,
								PxReal inflation)
{
	PxSceneQueryFilterData fdAny = filterData;
	fdAny.flags |= PxQueryFlag::eANY_HIT;
	fdAny.clientId = queryClient;
	PxSweepBuffer buf;
	scene.sweep(geometry, pose, unitDir, distance, buf, queryFlags, fdAny, filterCall, cache, inflation);
	hit = buf.block;
	return buf.hasBlock;
}

bool PxSceneQueryExt::sweepSingle(	const PxScene& scene,
									const PxGeometry& geometry, const PxTransform& pose, const PxVec3& unitDir, const PxReal distance,
									PxSceneQueryFlags outputFlags,
									PxSweepHit& hit,
									const PxSceneQueryFilterData& filterData,
									PxSceneQueryFilterCallback* filterCall,
									const PxSceneQueryCache* cache,
									PxClientID queryClient, PxReal inflation)
{
	PxSweepBuffer buf;
	PxQueryFilterData fd1 = filterData; fd1.clientId = queryClient;
	scene.sweep(geometry, pose, unitDir, distance, buf, outputFlags, fd1, filterCall, cache, inflation);
	hit = buf.block;
	return buf.hasBlock;
}

PxI32 PxSceneQueryExt::sweepMultiple(	const PxScene& scene,
										const PxGeometry& geometry, const PxTransform& pose, const PxVec3& unitDir, const PxReal distance,
										PxSceneQueryFlags outputFlags, PxSweepHit* hitBuffer, PxU32 hitBufferSize, bool& blockingHit,
										const PxSceneQueryFilterData& filterData,
										PxSceneQueryFilterCallback* filterCall, const PxSceneQueryCache* cache,
										PxClientID queryClient, PxReal inflation)
{
	PxQueryFilterData fd1 = filterData; fd1.clientId = queryClient;
	PxSweepBuffer buf(hitBuffer, hitBufferSize);
	scene.sweep(geometry, pose, unitDir, distance, buf, outputFlags, fd1, filterCall, cache, inflation);
	blockingHit = buf.hasBlock;
	if(blockingHit)
	{
		if(buf.nbTouches < hitBufferSize)
		{
			hitBuffer[buf.nbTouches] = buf.block;
			return PxI32(buf.nbTouches+1);
		}
		else // overflow, drop the last touch
		{
			hitBuffer[hitBufferSize-1] = buf.block;
			return -1;
		}
	} else
		// no block
		return PxI32(buf.nbTouches);
}

PxI32 PxSceneQueryExt::overlapMultiple(	const PxScene& scene,
										const PxGeometry& geometry, const PxTransform& pose,
										PxOverlapHit* hitBuffer, PxU32 hitBufferSize,
										const PxSceneQueryFilterData& filterData,
										PxSceneQueryFilterCallback* filterCall,
										PxClientID queryClient)
{
	PxQueryFilterData fd1 = filterData; fd1.clientId = queryClient;
	fd1.flags |= PxQueryFlag::eNO_BLOCK;
	PxOverlapBuffer buf(hitBuffer, hitBufferSize);
	scene.overlap(geometry, pose, buf, fd1, filterCall);
	if(buf.hasBlock)
	{
		if(buf.nbTouches < hitBufferSize)
		{
			hitBuffer[buf.nbTouches] = buf.block;
			return PxI32(buf.nbTouches+1);
		}
		else // overflow, drop the last touch
		{
			hitBuffer[hitBufferSize-1] = buf.block;
			return -1;
		}
	} else
		// no block
		return PxI32(buf.nbTouches);
}

bool PxSceneQueryExt::overlapAny(	const PxScene& scene,
									const PxGeometry& geometry, const PxTransform& pose,
									PxOverlapHit& hit,
									const PxSceneQueryFilterData& filterData,
									PxSceneQueryFilterCallback* filterCall,
									PxClientID queryClient)
{
	PxSceneQueryFilterData fdAny = filterData;
	fdAny.flags |= (PxQueryFlag::eANY_HIT | PxQueryFlag::eNO_BLOCK);
	fdAny.clientId = queryClient;
	PxOverlapBuffer buf;
	scene.overlap(geometry, pose, buf, fdAny, filterCall);
	hit = buf.block;
	return buf.hasBlock;
}
