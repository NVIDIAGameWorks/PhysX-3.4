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

#ifndef SQ_PRUNERTREEMAP_H
#define SQ_PRUNERTREEMAP_H

#include "SqTypedef.h"
#include "PsArray.h"

namespace physx
{
namespace Sq
{
	static const PxU32 INVALID_NODE_ID = 0xFFffFFff;
	static const PxU32 INVALID_POOL_ID = 0xFFffFFff;

	// Maps pruning pool indices to AABB-tree indices (i.e. locates the object's box in the aabb-tree nodes pool)
	// 
	// The map spans pool indices from 0..N-1, where N is the number of pool entries when the map was created from a tree.
	//
	// It maps: 
	//		to node indices in the range 0..M-1, where M is the number of nodes in the tree the map was created from,
	//   or to INVALID_NODE_ID if the pool entry was removed or pool index is outside input domain.
	//
	// The map is the inverse of the tree mapping: (node[map[poolID]].primitive == poolID) is true at all times.

	class AABBTreeUpdateMap 
	{
	public:
												AABBTreeUpdateMap()		{}
												~AABBTreeUpdateMap()	{}

						void					release()
												{
													mMapping.reset();
												}

						// indices offset used when indices are shifted from objects (used for merged trees)
						void					initMap(PxU32 numPoolObjects, const Sq::AABBTree& tree);

						void					invalidate(PoolIndex poolIndex, PoolIndex replacementPoolIndex, Sq::AABBTree& tree);

		PX_FORCE_INLINE TreeNodeIndex operator[](PxU32 poolIndex) const
												{ 
													return poolIndex < mMapping.size() ? mMapping[poolIndex] : INVALID_NODE_ID;
												}
	private:
		// maps from prunerIndex (index in the PruningPool) to treeNode index
		// this will only map to leaf tree nodes
					Ps::Array<TreeNodeIndex>	mMapping;
	};

}
}

#endif
