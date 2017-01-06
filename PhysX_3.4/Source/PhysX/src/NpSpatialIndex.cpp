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

#include "NpSpatialIndex.h"
#include "PsFPU.h"
#include "SqPruner.h"
#include "PxBoxGeometry.h"
#include "PsFoundation.h"
#include "GuBounds.h"

using namespace physx;
using namespace Sq;
using namespace Gu;

NpSpatialIndex::NpSpatialIndex()
: mPendingUpdates(false)
{
	mPruner = createAABBPruner(true);
}

NpSpatialIndex::~NpSpatialIndex()
{
	PX_DELETE(mPruner);
}

PxSpatialIndexItemId NpSpatialIndex::insert(PxSpatialIndexItem& item, const PxBounds3& bounds)
{
	PX_SIMD_GUARD;
	PX_CHECK_AND_RETURN_VAL(bounds.isValid(), "PxSpatialIndex::insert: bounds are not valid.", PX_SPATIAL_INDEX_INVALID_ITEM_ID);

	PrunerHandle output;
	PrunerPayload payload;
	payload.data[0] = reinterpret_cast<size_t>(&item);
	mPruner->addObjects(&output, &bounds, &payload, 1, false);
	mPendingUpdates = true;
	return output;
}
	
void NpSpatialIndex::update(PxSpatialIndexItemId id, const PxBounds3& bounds)
{
	PX_SIMD_GUARD;
	PX_CHECK_AND_RETURN(bounds.isValid(), "PxSpatialIndex::update: bounds are not valid.");

	PxBounds3* b;
	mPruner->getPayload(id, b);
	*b = bounds;
	mPruner->updateObjectsAfterManualBoundsUpdates(&id, 1);

	mPendingUpdates = true;
}

void NpSpatialIndex::remove(PxSpatialIndexItemId id)
{
	PX_SIMD_GUARD;

	mPruner->removeObjects(&id, 1);
	mPendingUpdates = true;
}

namespace
{
	struct OverlapCallback: public PrunerCallback
	{
		OverlapCallback(PxSpatialOverlapCallback& callback) : mUserCallback(callback) {}

		virtual PxAgain invoke(PxReal& /*distance*/, const PrunerPayload& userData)
		{
			PxSpatialIndexItem& item = *reinterpret_cast<PxSpatialIndexItem*>(userData.data[0]);
			return mUserCallback.onHit(item);
		}

		PxSpatialOverlapCallback &mUserCallback;
	private:
		OverlapCallback& operator=(const OverlapCallback&);
	};

	struct LocationCallback: public PrunerCallback
	{
		LocationCallback(PxSpatialLocationCallback& callback) : mUserCallback(callback) {}

		virtual PxAgain invoke(PxReal& distance, const PrunerPayload& userData)
		{
			PxReal oldDistance = distance, shrunkDistance = distance;
			PxSpatialIndexItem& item = *reinterpret_cast<PxSpatialIndexItem*>(userData.data[0]);
			PxAgain result = mUserCallback.onHit(item, distance, shrunkDistance);

			if(shrunkDistance>distance)
				Ps::getFoundation().error(PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__, "PxSpatialLocationCallback: distance may not be extended.");

			if(!result)
				return false;

			distance = PxMin(oldDistance, distance);
			return true;
		}

		PxSpatialLocationCallback& mUserCallback;

	private:
		LocationCallback& operator=(const LocationCallback&);
	};
}

void NpSpatialIndex::flushUpdates() const
{
	if(mPendingUpdates)
		mPruner->commit();
	mPendingUpdates = false;
}

void NpSpatialIndex::overlap(const PxBounds3& aabb, PxSpatialOverlapCallback& callback) const
{
	PX_SIMD_GUARD;
	PX_CHECK_AND_RETURN(aabb.isValid(), "PxSpatialIndex::overlap: aabb is not valid.");

	flushUpdates();
	OverlapCallback cb(callback);
	PxBoxGeometry boxGeom(aabb.getExtents());
	PxTransform xf(aabb.getCenter());
	ShapeData shapeData(boxGeom, xf, 0.0f); // temporary rvalue not compatible with PX_NOCOPY 
	mPruner->overlap(shapeData, cb);
}

void NpSpatialIndex::raycast(const PxVec3& origin, const PxVec3& unitDir, PxReal maxDist, PxSpatialLocationCallback& callback) const
{
	PX_SIMD_GUARD;

	PX_CHECK_AND_RETURN(origin.isFinite(),								"PxSpatialIndex::raycast: origin is not valid.");
	PX_CHECK_AND_RETURN(unitDir.isFinite() && unitDir.isNormalized(),	"PxSpatialIndex::raycast: unitDir is not valid.");
	PX_CHECK_AND_RETURN(maxDist > 0.0f,									"PxSpatialIndex::raycast: distance must be positive");

	flushUpdates();
	LocationCallback cb(callback);
	mPruner->raycast(origin, unitDir, maxDist, cb);
}

void NpSpatialIndex::sweep(const PxBounds3& aabb, const PxVec3& unitDir, PxReal maxDist, PxSpatialLocationCallback& callback) const
{
	PX_SIMD_GUARD;

	PX_CHECK_AND_RETURN(aabb.isValid(),									"PxSpatialIndex::sweep: aabb is not valid.");
	PX_CHECK_AND_RETURN(unitDir.isFinite() && unitDir.isNormalized(),	"PxSpatialIndex::sweep: unitDir is not valid.");
	PX_CHECK_AND_RETURN(maxDist > 0.0f,									"PxSpatialIndex::sweep: distance must be positive");

	flushUpdates();
	LocationCallback cb(callback);
	PxBoxGeometry boxGeom(aabb.getExtents());
	PxTransform xf(aabb.getCenter());
	ShapeData shapeData(boxGeom, xf, 0.0f); // temporary rvalue not compatible with PX_NOCOPY 
	mPruner->sweep(shapeData, unitDir, maxDist, cb);
}


void NpSpatialIndex::rebuildFull()
{
	PX_SIMD_GUARD;

	mPruner->purge();
	mPruner->commit();
	mPendingUpdates = false;
}

void NpSpatialIndex::setIncrementalRebuildRate(PxU32 rate)
{
	mPruner->setRebuildRateHint(rate);
}

void NpSpatialIndex::rebuildStep()
{
	PX_SIMD_GUARD;
	mPruner->buildStep();
	mPendingUpdates = true;
}

void NpSpatialIndex::release()
{
	delete this;
}

PxSpatialIndex* physx::PxCreateSpatialIndex()
{
	return PX_NEW(NpSpatialIndex)();
}

