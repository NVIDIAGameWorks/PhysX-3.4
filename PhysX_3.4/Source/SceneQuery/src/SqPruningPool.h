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

#ifndef SQ_PRUNINGPOOL_H
#define SQ_PRUNINGPOOL_H

#include "SqPruner.h"
#include "SqTypedef.h"
#include "SqBounds.h"

namespace physx
{
namespace Sq
{
	// This class is designed to maintain a two way mapping between pair(PrunerPayload,AABB) and PrunerHandle
	// Internally there's also an index for handles (AP: can be simplified?)
	// This class effectively stores bounded pruner payloads, returns a PrunerHandle and allows O(1)
	// access to them using a PrunerHandle
	// Supported operations are add, remove, update bounds
	class PruningPool 
	{
		public:
												PruningPool();
												~PruningPool();

		PX_FORCE_INLINE	const PrunerPayload&	getPayload(PrunerHandle handle) const { return mObjects[getIndex(handle)];	}

		PX_FORCE_INLINE	const PrunerPayload&	getPayload(PrunerHandle handle, PxBounds3*& bounds)	const
												{
													const PoolIndex index = getIndex(handle);
													bounds = mWorldBoxes + index;
													return mObjects[index];
												}

						void					shiftOrigin(const PxVec3& shift);

		// PT: adds 'count' objects to the pool. Needs 'count' bounds and 'count' payloads passed as input. Writes out 'count' handles
		// in 'results' array. Function returns number of successfully added objects, ideally 'count' but can be less in case we run
		// out of memory.
						PxU32					addObjects(PrunerHandle* results, const PxBounds3* bounds, const PrunerPayload* payload, PxU32 count);

		// this function will swap the last object with the hole formed by removed PrunerHandle object
		// and return the removed last object's index in the pool
						PoolIndex				removeObject(PrunerHandle h);

		// Data access
		PX_FORCE_INLINE	PoolIndex				getIndex(PrunerHandle h)const	{ return mHandleToIndex[h];	}
		PX_FORCE_INLINE	PrunerPayload*			getObjects()			const	{ return mObjects;			}
		PX_FORCE_INLINE	PxU32					getNbActiveObjects()	const	{ return mNbObjects;		}
		PX_FORCE_INLINE	const PxBounds3*		getCurrentWorldBoxes()	const	{ return mWorldBoxes;		}
		PX_FORCE_INLINE	PxBounds3*				getCurrentWorldBoxes()			{ return mWorldBoxes;		}

		PX_FORCE_INLINE	const PxBounds3&		getWorldAABB(PrunerHandle h) const
												{
													return mWorldBoxes[getIndex(h)];
												}

		PX_FORCE_INLINE	void					updateObjectsAndInflateBounds(const PrunerHandle* handles, const PxU32* indices, const PxBounds3* newBounds, PxU32 count)
												{
													for(PxU32 i=0; i<count; i++)
													{
														const PoolIndex poolIndex = getIndex(handles[i]);
														PX_ASSERT(poolIndex!=INVALID_PRUNERHANDLE);
//														if(poolIndex!=INVALID_PRUNERHANDLE)
															Sq::inflateBounds(mWorldBoxes[poolIndex], newBounds[indices[i]]);
													}
												}

						void					preallocate(PxU32 entries);
//	protected:

						PxU32					mNbObjects;			//!< Current number of objects
						PxU32					mMaxNbObjects;		//!< Max. number of objects (capacity for mWorldBoxes, mObjects)

						//!< these arrays are parallel
						PxBounds3*				mWorldBoxes;		//!< List of world boxes, stores mNbObjects, capacity=mMaxNbObjects
						PrunerPayload*			mObjects;			//!< List of objects, stores mNbObjects, capacity=mMaxNbObjects
//	private:			
						PoolIndex*				mHandleToIndex;		//!< Maps from PrunerHandle to internal index (payload index in mObjects)
						PrunerHandle*			mIndexToHandle;		//!< Inverse map from objectIndex to PrunerHandle

				// this is the head of a list of holes formed in mHandleToIndex
				// by removed handles
				// the rest of the list is stored in holes in mHandleToIndex (in place)
						PrunerHandle			mFirstRecycledHandle;

						bool					resize(PxU32 newCapacity);
	};

} // namespace Sq

}

#endif // SQ_PRUNINGPOOL_H
