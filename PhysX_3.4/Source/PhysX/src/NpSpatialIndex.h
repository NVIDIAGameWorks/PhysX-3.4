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


#ifndef NP_SPATIALINDEX
#define NP_SPATIALINDEX

#include "PxSpatialIndex.h"
#include "PsUserAllocated.h"
#include "CmPhysXCommon.h"

namespace physx
{
namespace Sq
{
	class IncrementalPruner;
}

class NpSpatialIndex: public PxSpatialIndex, public Ps::UserAllocated
{
public:
								NpSpatialIndex();
								~NpSpatialIndex();

	virtual	PxSpatialIndexItemId	insert(PxSpatialIndexItem& item,
										   const PxBounds3& bounds);

	virtual	void					update(PxSpatialIndexItemId id,
										   const PxBounds3& bounds);

	virtual	void					remove(PxSpatialIndexItemId id);

	virtual void					overlap(const PxBounds3& aabb,
											PxSpatialOverlapCallback& callback)		const;

	virtual void					raycast(const PxVec3& origin, 
											const PxVec3& unitDir, 
											PxReal maxDist, 
											PxSpatialLocationCallback& callback)	const;

	virtual	void					sweep(const PxBounds3& aabb, 
										  const PxVec3& unitDir, 
										  PxReal maxDist, 
										  PxSpatialLocationCallback& callback)		const;

	virtual void					flush()	{ flushUpdates(); }
	virtual void					rebuildFull();
	virtual void					setIncrementalRebuildRate(PxU32 rate);
	virtual void					rebuildStep();
	virtual void					release();
private:
	
	// const so that we can call it from const methods
	void							flushUpdates() const;

	mutable bool			mPendingUpdates;
	Sq::IncrementalPruner*	mPruner;
};


}
#endif


