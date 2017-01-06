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

#include "SqAABBTreeUpdateMap.h"
#include "SqAABBTree.h"

using namespace physx;
using namespace Sq;

static const PxU32 SHRINK_THRESHOLD = 1024;

void AABBTreeUpdateMap::initMap(PxU32 nbObjects, const AABBTree& tree)
{
	if(!nbObjects)	
	{
		release();
		return;
	}

	// Memory management
	{
		const PxU32 mapSize = nbObjects;
		const PxU32 targetCapacity = mapSize + (mapSize>>2);

		PxU32 currentCapacity = mMapping.capacity();
		if( ( targetCapacity < (currentCapacity>>1) ) && ( (currentCapacity-targetCapacity) > SHRINK_THRESHOLD ) )
		{
			// trigger reallocation of a smaller array, there is enough memory to save
			currentCapacity = 0;
		}

		if(mapSize > currentCapacity)
		{
			// the mapping values are invalid and reset below in any case
			// so there is no need to copy the values at all
			mMapping.reset();
			mMapping.reserve(targetCapacity);	// since size is 0, reserve will also just allocate
		}

		mMapping.forceSize_Unsafe(mapSize);

		for(PxU32 i=0;i<mapSize;i++)
			mMapping[i] = INVALID_NODE_ID;
	}

	const PxU32 nbNodes = tree.getNbNodes();
	const AABBTreeRuntimeNode*	nodes = tree.getNodes();
	const PxU32* indices = tree.getIndices();
	for(TreeNodeIndex i=0;i<nbNodes;i++)
	{
		if(nodes[i].isLeaf())
		{
			const PxU32 nbPrims = nodes[i].getNbRuntimePrimitives();
			// PT: with multiple primitives per node, several mapping entries will point to the same node.
			PX_ASSERT(nbPrims<=16);
			for(PxU32 j=0;j<nbPrims;j++)
			{
				const PxU32 index = nodes[i].getPrimitives(indices)[j];
				PX_ASSERT(index<nbObjects);
				mMapping[index] = i;
			}
		}
	}
}

void AABBTreeUpdateMap::invalidate(PoolIndex prunerIndex0, PoolIndex prunerIndex1, AABBTree& tree)
{
	// prunerIndex0 and prunerIndex1 are both indices into the pool, not handles
	// prunerIndex0 is the index in the pruning pool for the node that was just removed
	// prunerIndex1 is the index in the pruning pool for the node
	const TreeNodeIndex nodeIndex0 = prunerIndex0<mMapping.size() ? mMapping[prunerIndex0] : INVALID_NODE_ID;
	const TreeNodeIndex nodeIndex1 = prunerIndex1<mMapping.size() ? mMapping[prunerIndex1] : INVALID_NODE_ID;

	//printf("map invalidate pi0:%x ni0:%x\t",prunerIndex0,nodeIndex0);
	//printf("  replace with pi1:%x ni1:%x\n",prunerIndex1,nodeIndex1);

	// if nodeIndex0 exists:
	//		invalidate node 0 
	//		invalidate map prunerIndex0
	// if nodeIndex1 exists: 
	//		point node 1 to prunerIndex0
	//		map prunerIndex0 to node 1
	//		invalidate map prunerIndex1

	// eventually: 
	// - node 0 is invalid
	// - prunerIndex0 is mapped to node 1 or
	//		is not mapped if prunerIndex1 is not mapped
	//		is not mapped if prunerIndex0==prunerIndex1
	// - node 1 points to prunerIndex0 or  
	//		is invalid if prunerIndex1 is not mapped
	//		is invalid if prunerIndex0==prunerIndex1
	// - prunerIndex1 is not mapped

	AABBTreeRuntimeNode* nodes = tree.getNodes();

	if(nodeIndex0!=INVALID_NODE_ID)
	{
		PX_ASSERT(nodeIndex0 < tree.getNbNodes());
		PX_ASSERT(nodes[nodeIndex0].isLeaf());
		AABBTreeRuntimeNode* node0 = nodes + nodeIndex0;
		const PxU32 nbPrims = node0->getNbRuntimePrimitives();
		PX_ASSERT(nbPrims <= 16);

		// retrieve the primitives pointer
		PxU32* primitives = node0->getPrimitives(tree.getIndices());
		PX_ASSERT(primitives);

		// PT: look for desired pool index in the leaf
		bool foundIt = false;
		for(PxU32 i=0;i<nbPrims;i++)
		{
			PX_ASSERT(mMapping[primitives[i]] == nodeIndex0); // PT: all primitives should point to the same leaf node

			if(prunerIndex0 == primitives[i])
			{
				foundIt = true;
				const PxU32 last = nbPrims-1;
				node0->setNbRunTimePrimitives(last);
				primitives[i] = INVALID_POOL_ID;			// Mark primitive index as invalid in the node
				mMapping[prunerIndex0] = INVALID_NODE_ID;	// invalidate the node index for pool 0

				// PT: swap within the leaf node. No need to update the mapping since they should all point
				// to the same tree node anyway.
				if(last!=i)
					Ps::swap(primitives[i], primitives[last]);
				break;
			}
		}
		PX_ASSERT(foundIt);
		PX_UNUSED(foundIt);
	}

	if (nodeIndex1!=INVALID_NODE_ID)
	{
		// PT: with multiple primitives per leaf, tree nodes may very well be the same for different pool indices.
		// However the pool indices may be the same when a swap has been skipped in the pruning pool, in which
		// case there is nothing to do.
		if(prunerIndex0!=prunerIndex1)
		{
			PX_ASSERT(nodeIndex1 < tree.getNbNodes());
			PX_ASSERT(nodes[nodeIndex1].isLeaf());
			AABBTreeRuntimeNode* node1 = nodes + nodeIndex1;
			const PxU32 nbPrims = node1->getNbRuntimePrimitives();
			PX_ASSERT(nbPrims <= 16);

			// retrieve the primitives pointer
			PxU32* primitives = node1->getPrimitives(tree.getIndices());
			PX_ASSERT(primitives);

			// PT: look for desired pool index in the leaf
			bool foundIt = false;
			for(PxU32 i=0;i<nbPrims;i++)
			{
				PX_ASSERT(mMapping[primitives[i]] == nodeIndex1); // PT: all primitives should point to the same leaf node

				if(prunerIndex1 == primitives[i])
				{
					foundIt = true;
					primitives[i] = prunerIndex0;				// point node 1 to the pool object moved to ID 0
					mMapping[prunerIndex0] = nodeIndex1;		// pool 0 is pointed at by node 1 now
					mMapping[prunerIndex1] = INVALID_NODE_ID;	// pool 1 is no longer stored in the tree
					break;
				}
			}
			PX_ASSERT(foundIt);
			PX_UNUSED(foundIt);
		}
	}
}

