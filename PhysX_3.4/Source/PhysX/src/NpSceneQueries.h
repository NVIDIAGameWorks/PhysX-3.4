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

#ifndef PX_PHYSICS_NP_SCENEQUERIES
#define PX_PHYSICS_NP_SCENEQUERIES

#include "PxScene.h"
#include "PxQueryReport.h"
#include "PsIntrinsics.h"
#include "CmPhysXCommon.h"
#include "SqSceneQueryManager.h"
#include "GuTriangleMesh.h"
#include "GuRaycastTests.h"
#include "GuSweepTests.h"
#include "GuOverlapTests.h"
#include "ScbScene.h"

#if PX_SUPPORT_PVD
#include "NpPvdSceneQueryCollector.h"
#endif

namespace physx { namespace Sq {

	struct QueryID { enum Enum {
		QUERY_RAYCAST_ANY_OBJECT,
		QUERY_RAYCAST_CLOSEST_OBJECT,
		QUERY_RAYCAST_ALL_OBJECTS,

		QUERY_OVERLAP_SPHERE_ALL_OBJECTS,
		QUERY_OVERLAP_AABB_ALL_OBJECTS,
		QUERY_OVERLAP_OBB_ALL_OBJECTS,
		QUERY_OVERLAP_CAPSULE_ALL_OBJECTS,
		QUERY_OVERLAP_CONVEX_ALL_OBJECTS,

		QUERY_LINEAR_OBB_SWEEP_CLOSEST_OBJECT,
		QUERY_LINEAR_CAPSULE_SWEEP_CLOSEST_OBJECT,
		QUERY_LINEAR_CONVEX_SWEEP_CLOSEST_OBJECT
	}; };
}

struct MultiQueryInput
{
	const PxVec3* rayOrigin; // only valid for raycasts
	const PxVec3* unitDir; // only valid for raycasts and sweeps
	PxReal maxDistance; // only valid for raycasts and sweeps
	const PxGeometry* geometry; // only valid for overlaps and sweeps
	const PxTransform* pose; // only valid for overlaps and sweeps
	PxReal inflation; // only valid for sweeps

	// Raycast constructor
	MultiQueryInput(const PxVec3& aRayOrigin, const PxVec3& aUnitDir, PxReal aMaxDist)
	{
		Ps::prefetchLine(&aRayOrigin);
		Ps::prefetchLine(&aUnitDir);
		rayOrigin = &aRayOrigin;
		unitDir = &aUnitDir;
		maxDistance = aMaxDist;
		geometry = NULL;
		pose = NULL;
		inflation = 0.0f;
	}

	// Overlap constructor
	MultiQueryInput(const PxGeometry* aGeometry, const PxTransform* aPose)
	{
		Ps::prefetchLine(aGeometry);
		Ps::prefetchLine(aPose);
		geometry = aGeometry;
		pose = aPose;
		inflation = 0.0f;
		rayOrigin = unitDir = NULL;
	}

	// Sweep constructor
	MultiQueryInput(
		const PxGeometry* aGeometry, const PxTransform* aPose,
		const PxVec3& aUnitDir, const PxReal aMaxDist, const PxReal aInflation)
	{
		Ps::prefetchLine(aGeometry);
		Ps::prefetchLine(aPose);
		Ps::prefetchLine(&aUnitDir);
		rayOrigin = NULL;
		maxDistance = aMaxDist;
		unitDir = &aUnitDir;
		geometry = aGeometry;
		pose = aPose;
		inflation = aInflation;
	}

	PX_FORCE_INLINE const PxVec3& getDir() const { PX_ASSERT(unitDir); return *unitDir; }
	PX_FORCE_INLINE const PxVec3& getOrigin() const { PX_ASSERT(rayOrigin); return *rayOrigin; }
};

struct BatchQueryFilterData
{
	void*							filterShaderData;
	PxU32							filterShaderDataSize;
	PxBatchQueryPreFilterShader		preFilterShader;	
	PxBatchQueryPostFilterShader	postFilterShader;	
	#if PX_SUPPORT_PVD
	Vd::PvdSceneQueryCollector*	collector; // gets set to bq collector
	#endif
	BatchQueryFilterData(void* fsData, PxU32 fsSize, PxBatchQueryPreFilterShader preFs, PxBatchQueryPostFilterShader postFs)
		: filterShaderData(fsData), filterShaderDataSize(fsSize), preFilterShader(preFs), postFilterShader(postFs)
	{
		#if PX_SUPPORT_PVD
		collector = NULL;
		#endif
	}
};

class PxGeometry;

class NpSceneQueries : public PxScene
{
	PX_NOCOPY(NpSceneQueries)

public:
	NpSceneQueries(const PxSceneDesc& desc);
	~NpSceneQueries();

	template<typename QueryHit>
					bool							multiQuery(
														const MultiQueryInput& in,
														PxHitCallback<QueryHit>& hits, PxHitFlags hitFlags, const PxQueryCache* cache,
														const PxQueryFilterData& filterData, PxQueryFilterCallback* filterCall,
														BatchQueryFilterData* bqFd) const;

	// Synchronous scene queries
	virtual			bool							raycast(
														const PxVec3& origin, const PxVec3& unitDir, const PxReal distance,	// Ray data
														PxRaycastCallback& hitCall, PxHitFlags hitFlags,
														const PxQueryFilterData& filterData, PxQueryFilterCallback* filterCall,
														const PxQueryCache* cache) const;

	virtual			bool							sweep(
														const PxGeometry& geometry, const PxTransform& pose,	// GeomObject data
														const PxVec3& unitDir, const PxReal distance,	// Ray data
														PxSweepCallback& hitCall, PxHitFlags hitFlags,
														const PxQueryFilterData& filterData, PxQueryFilterCallback* filterCall,
														const PxQueryCache* cache, const PxReal inflation) const;

	virtual			bool							overlap(
														const PxGeometry& geometry, const PxTransform& transform,	// GeomObject data
														PxOverlapCallback& hitCall, 
														const PxQueryFilterData& filterData, PxQueryFilterCallback* filterCall) const;

	PX_FORCE_INLINE	PxU64							getContextId()				const	{ return PxU64(reinterpret_cast<size_t>(this));	}
	PX_FORCE_INLINE	Scb::Scene&						getScene()							{ return mScene;								}
	PX_FORCE_INLINE	const Scb::Scene&				getScene()					const	{ return mScene;								}
	PX_FORCE_INLINE	PxU32							getFlagsFast()				const	{ return mScene.getFlags();						}
	PX_FORCE_INLINE	Sq::SceneQueryManager&			getSceneQueryManagerFast()			{ return mSQManager;							}

					Scb::Scene						mScene;
					Sq::SceneQueryManager			mSQManager;

					const Gu::GeomRaycastTable&		mCachedRaycastFuncs;
					const Gu::GeomSweepFuncs&		mCachedSweepFuncs;
					const Gu::GeomOverlapTable*		mCachedOverlapFuncs;

#if PX_SUPPORT_PVD
public:
					//Scene query and hits for pvd, collected in current frame
					mutable Vd::PvdSceneQueryCollector		mSingleSqCollector;
					mutable Vd::PvdSceneQueryCollector		mBatchedSqCollector;

PX_FORCE_INLINE				Vd::PvdSceneQueryCollector&	getSingleSqCollector() const {return mSingleSqCollector;}
PX_FORCE_INLINE				Vd::PvdSceneQueryCollector&	getBatchedSqCollector() const {return mBatchedSqCollector;}
#endif // PX_SUPPORT_PVD
};

#if PX_SUPPORT_EXTERN_TEMPLATE
//explicit template instantiation declaration
extern template
bool NpSceneQueries::multiQuery<PxRaycastHit>(const MultiQueryInput&, PxHitCallback<PxRaycastHit>&, PxHitFlags, const PxQueryCache*, const PxQueryFilterData&, PxQueryFilterCallback*, BatchQueryFilterData*) const;

extern template
bool NpSceneQueries::multiQuery<PxOverlapHit>(const MultiQueryInput&, PxHitCallback<PxOverlapHit>&, PxHitFlags, const PxQueryCache*, const PxQueryFilterData&, PxQueryFilterCallback*, BatchQueryFilterData*) const;

extern template
bool NpSceneQueries::multiQuery<PxSweepHit>(const MultiQueryInput&, PxHitCallback<PxSweepHit>&, PxHitFlags, const PxQueryCache*, const PxQueryFilterData&, PxQueryFilterCallback*, BatchQueryFilterData*) const;
#endif

namespace Sq { class AABBPruner; class AABBTreeRuntimeNode; class AABBTree; }

#if PX_VC 
    #pragma warning(push)
	#pragma warning( disable : 4324 ) // Padding was added at the end of a structure because of a __declspec(align) value.
#endif

#if PX_VC 
     #pragma warning(pop) 
#endif

} // namespace physx, sq

#endif
