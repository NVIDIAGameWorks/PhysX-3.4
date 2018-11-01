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
// Copyright (c) 2008-2018 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

#include "BpBroadPhase.h"
#include "BpBroadPhaseSap.h"
#include "BpBroadPhaseMBP.h"
#include "PxSceneDesc.h"
#include "BpSimpleAABBManager.h"
#include "CmBitMap.h"

using namespace physx;
using namespace Bp;
using namespace Cm;

BroadPhase* BroadPhase::create(
	const PxBroadPhaseType::Enum bpType,
	const PxU32 maxNbRegions,
	const PxU32 maxNbBroadPhaseOverlaps,
	const PxU32 maxNbStaticShapes,
	const PxU32 maxNbDynamicShapes,
	PxU64 contextID)
{
	PX_ASSERT(bpType==PxBroadPhaseType::eMBP || bpType == PxBroadPhaseType::eSAP);

	if(bpType==PxBroadPhaseType::eMBP)
		return PX_NEW(BroadPhaseMBP)(maxNbRegions, maxNbBroadPhaseOverlaps, maxNbStaticShapes, maxNbDynamicShapes, contextID);
	else
		return PX_NEW(BroadPhaseSap)(maxNbBroadPhaseOverlaps, maxNbStaticShapes, maxNbDynamicShapes, contextID);
}

#if PX_CHECKED
bool BroadPhaseUpdateData::isValid(const BroadPhaseUpdateData& updateData, const BroadPhase& bp)
{
	return (updateData.isValid() && bp.isValid(updateData));
}

static bool testHandles(PxU32 size, const BpHandle* handles, const PxU32 capacity, const Bp::FilterGroup::Enum* groups, const PxBounds3* bounds, BitMap& bitmap)
{
	if(!handles && size)
		return false;

/*	ValType minVal=0;
	ValType maxVal=0xffffffff;*/

	for(PxU32 i=0;i<size;i++)
	{
		const BpHandle h = handles[i];

		if(h>=capacity)
			return false;

		// Array in ascending order of id.
		if(i>0 && (h < handles[i-1]))
			return false;

		if(groups && groups[h]==FilterGroup::eINVALID)
			return false;

		bitmap.set(h);

		if(bounds)
		{
			for(PxU32 j=0;j<3;j++)
			{
				//Max must be greater than min.
				if(bounds[h].minimum[j]>bounds[h].maximum[j])
					return false;
#if 0
				//Bounds have an upper limit.
				if(bounds[created[i]].getMax(j)>=maxVal)
					return false;

				//Bounds have a lower limit.
				if(bounds[created[i]].getMin(j)<=minVal)
					return false;

				//Max must be odd.
				if(4 != (bounds[created[i]].getMax(j) & 4))
					return false;

				//Min must be even.
				if(0 != (bounds[created[i]].getMin(j) & 4))
					return false;
#endif
			}
		}
	}
	return true;
}

static bool testBitmap(const BitMap& bitmap, PxU32 size, const BpHandle* handles)
{
	while(size--)
	{
		const BpHandle h = *handles++;
		if(bitmap.test(h))
			return false;
	}
	return true;
}

bool BroadPhaseUpdateData::isValid() const 
{
	const PxBounds3* bounds = getAABBs();
	const PxU32 boxesCapacity = getCapacity();
	const Bp::FilterGroup::Enum* groups = getGroups();

	BitMap createdBitmap;	createdBitmap.resizeAndClear(boxesCapacity);
	BitMap updatedBitmap;	updatedBitmap.resizeAndClear(boxesCapacity);
	BitMap removedBitmap;	removedBitmap.resizeAndClear(boxesCapacity);

	if(!testHandles(getNumCreatedHandles(), getCreatedHandles(), boxesCapacity, groups, bounds, createdBitmap))
		return false;
	if(!testHandles(getNumUpdatedHandles(), getUpdatedHandles(), boxesCapacity, groups, bounds, updatedBitmap))
		return false;
	if(!testHandles(getNumRemovedHandles(), getRemovedHandles(), boxesCapacity, NULL, NULL, removedBitmap))
		return false;

	if(1)
	{
		// Created/updated
		if(!testBitmap(createdBitmap, getNumUpdatedHandles(), getUpdatedHandles()))
			return false;
		// Created/removed
		if(!testBitmap(createdBitmap, getNumRemovedHandles(), getRemovedHandles()))
			return false;
		// Updated/removed
		if(!testBitmap(updatedBitmap, getNumRemovedHandles(), getRemovedHandles()))
			return false;
	}
	return true;
}
#endif
