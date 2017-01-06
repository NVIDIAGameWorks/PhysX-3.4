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


#ifndef PX_PHYSICS_NP_VOLUMECACHE
#define PX_PHYSICS_NP_VOLUMECACHE

#include "PxVolumeCache.h"
#include "PsUserAllocated.h"
#include "CmPhysXCommon.h"
#include "PsArray.h"

namespace physx
{

	struct MultiQueryInput;

namespace Sq { class SceneQueryManager; }


// internal implementation for PxVolumeCache
class NpVolumeCache : public PxVolumeCache, public Ps::UserAllocated
{
public:
									NpVolumeCache(Sq::SceneQueryManager* sqm, PxU32 maxNbStatic, PxU32 maxNbDynamic);
	virtual							~NpVolumeCache();
	virtual			bool			isValid() const;
					bool			isValid(PxU32 isDynamic) const;

	virtual			FillStatus		fill(const PxGeometry& cacheVolume, const PxTransform& pose);
					FillStatus		fillInternal(PxU32 isDynamic, const PxOverlapHit* buffer = NULL, PxI32 count = 0);

	virtual			bool			getCacheVolume(PxGeometryHolder& resultVolume, PxTransform& resultPose);
	virtual			PxI32			getNbCachedShapes();

	virtual			void			invalidate();
	virtual			void			release();

	virtual			void			forEach(Iterator& iter); 

	virtual			void			setMaxNbStaticShapes(PxU32 maxCount);
	virtual			PxU32			getMaxNbStaticShapes();
	virtual			void			setMaxNbDynamicShapes(PxU32 maxCount);
	virtual			PxU32			getMaxNbDynamicShapes();

	template<typename HitType>
					bool			multiQuery(
										const MultiQueryInput& multiInput, // type specific input data
										PxHitCallback<HitType>& hitCall, PxHitFlags hitFlags,
										const PxQueryFilterData& filterData, PxQueryFilterCallback* filterCall,
										PxF32 inflation = 0.0f) const;

	virtual			bool			raycast(
										const PxVec3& origin, const PxVec3& unitDir, const PxReal distance,
										PxRaycastCallback& hitCall, PxHitFlags hitFlags,
										const PxQueryFilterData& filterData, PxQueryFilterCallback* filterCall) const;

	virtual			bool			sweep(
										const PxGeometry& geometry, const PxTransform& pose, const PxVec3& unitDir, const PxReal distance,
										PxSweepCallback& hitCall, PxHitFlags hitFlags,
										const PxQueryFilterData& filterData, PxQueryFilterCallback* filterCall,
										const PxReal inflation) const;

	virtual			bool			overlap(
										const PxGeometry& geometry, const PxTransform& pose,
										PxOverlapCallback& hitCall, const PxQueryFilterData& filterData,
										PxQueryFilterCallback* filterCall) const;


					void			onOriginShift(const PxVec3& shift);


	PxGeometryHolder				mCacheVolume;
	PxTransform						mCachePose;
	PxU32							mMaxShapeCount[2];
	Sq::SceneQueryManager*			mSQManager;
	mutable Ps::Array<PxActorShape> mCache[2]; // AP todo: improve memory management, could we have one allocation for both?
	PxU32							mStaticTimestamp;
	PxU32							mDynamicTimestamp;
	bool							mIsInvalid[2]; // invalid for reasons other than timestamp, such as overflow on previous fill
};

}

#endif // PX_PHYSICS_NP_SCENE
