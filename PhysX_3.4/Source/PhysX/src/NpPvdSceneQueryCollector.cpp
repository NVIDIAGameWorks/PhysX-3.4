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

#include "NpScene.h"

#if PX_SUPPORT_PVD
using namespace physx;
using namespace Sq;
using namespace Vd;

static const char* gName_PvdRaycast[2]			= { "SceneQueries.Raycasts",		"BatchedQueries.Raycasts" };
static const char* gName_PvdSweep[2]			= { "SceneQueries.Sweeps",			"BatchedQueries.Sweeps" };
static const char* gName_PvdOverlap[2]			= { "SceneQueries.Overlaps",		"BatchedQueries.Overlaps" };
static const char* gName_PvdSqHit[2]			= { "SceneQueries.Hits",			"BatchedQueries.Hits" };
static const char* gName_PxTransform[2]			= { "SceneQueries.PoseList",		"BatchedQueries.PoseList" };
static const char* gName_PxFilterData[2]		= { "SceneQueries.FilterDataList",	"BatchedQueries.FilterDataList" };
static const char* gName_PxGeometryHolder[2]	= { "SceneQueries.GeometryList",	"BatchedQueries.GeometryList" };

PvdSceneQueryCollector::PvdSceneQueryCollector(Scb::Scene& scene, bool isBatched) :
	mAccumulatedRaycastQueries	(gName_PvdRaycast),
	mAccumulatedSweepQueries	(gName_PvdSweep),
	mAccumulatedOverlapQueries	(gName_PvdOverlap),
	mPvdSqHits					(gName_PvdSqHit),
	mPoses						(gName_PxTransform),
	mFilterData					(gName_PxFilterData),
	mScene						(scene),
	mGeometries0				(gName_PxGeometryHolder),
	mGeometries1				(gName_PxGeometryHolder),
	mInUse						(0),
	mIsBatched					(isBatched)
{
}

void PvdSceneQueryCollector::release()
{
	physx::pvdsdk::PvdDataStream* stream = mScene.getScenePvdClient().getDataStream();
	if(stream && stream->isConnected())
	{
		const Ps::Array<PxGeometryHolder>& geoms = getPrevFrameGeometries();
		for(PxU32 k=0; k<geoms.size(); ++k)
			stream->destroyInstance(&geoms[k]);

		clearGeometryArrays();
	}
}

template<class QueryResultT, class PvdHitType>
static void collectBatchedHits(const QueryResultT* results, Ps::Array<PvdHitType>& accumulated, Ps::Array<PvdSqHit>& pvdSqHits, PxU32 nb, PxU32 startIdx, const char* arrayName)
{
	for(PxU32 i=0; i<nb; i++)
	{
		const QueryResultT& result = results[i];
		if(result.queryStatus != PxBatchQueryStatus::eSUCCESS)
			continue;

		PvdHitType& query = accumulated[startIdx + i];
		const PxU32 nbAnyHits = result.getNbAnyHits();
		if(query.mHits.mCount != nbAnyHits)
		{
			query.mHits = PvdReference(arrayName, pvdSqHits.size(), nbAnyHits);

			for(PxU32 j=0; j<nbAnyHits; j++)
				pvdSqHits.pushBack(PvdSqHit(result.getAnyHit(j)));
		}
	}
}

void PvdSceneQueryCollector::collectAllBatchedHits(const PxRaycastQueryResult* r, PxU32 nbR, PxU32 idxR, const PxOverlapQueryResult* o, PxU32 nbO, PxU32 idxO, const PxSweepQueryResult* s, PxU32 nbS, PxU32 idxS)
{
	collectBatchedHits(r, mAccumulatedRaycastQueries, mPvdSqHits, nbR, idxR, getArrayName(mPvdSqHits));
	collectBatchedHits(o, mAccumulatedOverlapQueries, mPvdSqHits, nbO, idxO, getArrayName(mPvdSqHits));
	collectBatchedHits(s, mAccumulatedSweepQueries, mPvdSqHits, nbS, idxS, getArrayName(mPvdSqHits));
}

template<class SDKHitType, class PvdHitType>
static void accumulate(PvdHitType& query, Ps::Array<PvdHitType>& accumulated, const char* arrayName, Ps::Array<PvdSqHit>& dst, const SDKHitType* src, PxU32 nb, const PxQueryFilterData& fd)
{
	query.mFilterFlags = fd.flags;
	query.mHits = PvdReference(arrayName, dst.size(), nb);

	PX_ASSERT(PxU32(-1) != nb);
	for(PxU32 i=0; i<nb; i++)
		dst.pushBack(PvdSqHit(src[i]));

	accumulated.pushBack(query);
}

static PX_FORCE_INLINE void clampNbHits(PxU32& hitsNum, const PxQueryFilterData& fd, bool multipleHits)
{
	if((fd.flags & PxQueryFlag::eANY_HIT) || !multipleHits)
		hitsNum = hitsNum > 0 ? 1u : 0;
}

template<class Type> static void pushBackT(Ps::Array<Type>& array, const Type& item, PvdReference& ref, const char* arrayName)
{
	ref = PvdReference(arrayName, array.size(), 1);
	array.pushBack(item);
}

void PvdSceneQueryCollector::raycast(const PxVec3& origin, const PxVec3& unitDir, PxReal distance, const PxRaycastHit* hit, PxU32 hitsNum, const PxQueryFilterData& fd, bool multipleHits)
{
	Ps::Mutex::ScopedLock lock(mMutex);

	PvdRaycast raycastQuery;
	raycastQuery.mOrigin		= origin;
	raycastQuery.mUnitDir		= unitDir;
	raycastQuery.mDistance		= distance;
	raycastQuery.mFilterData	= fd.data;
	if(fd.flags & PxQueryFlag::eANY_HIT)	raycastQuery.mType = QueryID::QUERY_RAYCAST_ANY_OBJECT;
	else if(multipleHits)					raycastQuery.mType = QueryID::QUERY_RAYCAST_ALL_OBJECTS;
	else									raycastQuery.mType = QueryID::QUERY_RAYCAST_CLOSEST_OBJECT;
	clampNbHits(hitsNum, fd, multipleHits);

	accumulate(raycastQuery, mAccumulatedRaycastQueries, getArrayName(mPvdSqHits), mPvdSqHits, hit, hitsNum, fd);
}

void PvdSceneQueryCollector::sweep(const PxGeometry& geometry, const PxTransform& pose, const PxVec3& unitDir, PxReal distance, const PxSweepHit* hit, PxU32 hitsNum, const PxQueryFilterData& fd, bool multipleHits)
{
	Ps::Mutex::ScopedLock lock(mMutex);

	PvdSweep sweepQuery;
	pushBackT(getGeometries(mInUse), PxGeometryHolder(geometry), sweepQuery.mGeometries, getArrayName(getGeometries(mInUse)));	// PT: TODO: optimize this. We memcopy once to the stack, then again to the array....
	pushBackT(mPoses, pose, sweepQuery.mPoses, getArrayName(mPoses));
	pushBackT(mFilterData, fd.data, sweepQuery.mFilterData, getArrayName(mFilterData));

	const PxGeometryType::Enum type = geometry.getType();	// PT: TODO: QueryID::QUERY_LINEAR_xxx_SWEEP_ALL_OBJECTS are never used!
	if(type==PxGeometryType::eBOX)												sweepQuery.mType = QueryID::QUERY_LINEAR_OBB_SWEEP_CLOSEST_OBJECT;
	else if(type==PxGeometryType::eSPHERE || type==PxGeometryType::eCAPSULE)	sweepQuery.mType = QueryID::QUERY_LINEAR_CAPSULE_SWEEP_CLOSEST_OBJECT;
	else if(type==PxGeometryType::eCONVEXMESH)									sweepQuery.mType = QueryID::QUERY_LINEAR_CONVEX_SWEEP_CLOSEST_OBJECT;
	else																		PX_ASSERT(0);
	sweepQuery.mUnitDir		= unitDir;
	sweepQuery.mDistance	= distance;
	clampNbHits(hitsNum, fd, multipleHits);

	accumulate(sweepQuery, mAccumulatedSweepQueries, getArrayName(mPvdSqHits), mPvdSqHits, hit, hitsNum, fd);
}

void PvdSceneQueryCollector::overlapMultiple(const PxGeometry& geometry, const PxTransform& pose, const PxOverlapHit* hit, PxU32 hitsNum, const PxQueryFilterData& fd)
{
	Ps::Mutex::ScopedLock lock(mMutex);

	PvdOverlap overlapQuery;
	pushBackT(getGeometries(mInUse), PxGeometryHolder(geometry), overlapQuery.mGeometries, getArrayName(getGeometries(mInUse)));	// PT: TODO: optimize this. We memcopy once to the stack, then again to the array....

	const PxGeometryType::Enum type = geometry.getType();
	if(type==PxGeometryType::eBOX)				overlapQuery.mType = pose.q.isIdentity() ? QueryID::QUERY_OVERLAP_AABB_ALL_OBJECTS : QueryID::QUERY_OVERLAP_OBB_ALL_OBJECTS;
	else if(type==PxGeometryType::eSPHERE)		overlapQuery.mType = QueryID::QUERY_OVERLAP_SPHERE_ALL_OBJECTS;
	else if(type==PxGeometryType::eCAPSULE)		overlapQuery.mType = QueryID::QUERY_OVERLAP_CAPSULE_ALL_OBJECTS;
	else if(type==PxGeometryType::eCONVEXMESH)	overlapQuery.mType = QueryID::QUERY_OVERLAP_CONVEX_ALL_OBJECTS;
	else										PX_ASSERT(0);
	overlapQuery.mPose			= pose;
	overlapQuery.mFilterData	= fd.data;

	accumulate(overlapQuery, mAccumulatedOverlapQueries, getArrayName(mPvdSqHits), mPvdSqHits, hit, hitsNum, fd);
}
#endif
