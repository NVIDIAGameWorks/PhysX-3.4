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

#include "foundation/PxMemory.h"
#include "SqIncrementalAABBTree.h"
#include "SqAABBTree.h"
#include "SqAABBTreeUpdateMap.h"
#include "SqBounds.h"
#include "PsVecMath.h"
#include "PsFPU.h"

using namespace physx;
using namespace Sq;
using namespace shdfnd::aos;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

IncrementalAABBTree::IncrementalAABBTree():
	mIndicesPool("AABBTreeIndicesPool", 256),
	mNodesPool("AABBTreeNodesPool", 256	),
	mRoot(NULL)
{
	
}

IncrementalAABBTree::~IncrementalAABBTree()
{
	release();
}

void IncrementalAABBTree::release()
{
	if(mRoot)
	{
		releaseNode(mRoot);
		mRoot = NULL;
	}
}

void IncrementalAABBTree::releaseNode(IncrementalAABBTreeNode* node)
{
	PX_ASSERT(node);
	if(node->isLeaf())
	{
		mIndicesPool.deallocate(node->mIndices);
	}
	else
	{
		releaseNode(node->mChilds[0]);
		releaseNode(node->mChilds[1]);
	}
	if(!node->mParent)
	{
		mNodesPool.deallocate(reinterpret_cast<IncrementalAABBTreeNodePair*>(node));
		return;
	}
	if(node->mParent->mChilds[1] == node)
	{
		mNodesPool.deallocate(reinterpret_cast<IncrementalAABBTreeNodePair*>(node->mParent->mChilds[0]));
	}
}


// check if node is inside the given bounds
PX_FORCE_INLINE static bool nodeInsideBounds(const Vec4V& nodeMin, const Vec4V& nodeMax, const Vec4V& parentMin, const Vec4V& parentMax)
{
	return !(Ps::IntBool(V4AnyGrtr3(parentMin, nodeMin)) || Ps::IntBool(V4AnyGrtr3(nodeMax, parentMax)));
}

// update the node parent hierarchy, when insert happen, we can early exit when the node is inside its parent
// no further update is needed
PX_FORCE_INLINE static void updateHierarchyAfterInsert(IncrementalAABBTreeNode* node)
{
	IncrementalAABBTreeNode* parent = node->mParent;
	IncrementalAABBTreeNode* testNode = node;
	while(parent)
	{
		// check if we can early exit
		if(!nodeInsideBounds(testNode->mBVMin, testNode->mBVMax, parent->mBVMin, parent->mBVMax))
		{
			parent->mBVMin = V4Min(parent->mChilds[0]->mBVMin, parent->mChilds[1]->mBVMin);
			parent->mBVMax = V4Max(parent->mChilds[0]->mBVMax, parent->mChilds[1]->mBVMax);
		}
		else
			break;
		testNode = parent;
		parent = parent->mParent;
	}
}

// add an index into the leaf indices list and update the node bounds
PX_FORCE_INLINE static void addPrimitiveIntoNode(IncrementalAABBTreeNode* node, const PoolIndex index,  const Vec4V& minV,  const Vec4V& maxV)
{
	PX_ASSERT(node->isLeaf());
	AABBTreeIndices& nodeIndices = *node->mIndices;
	PX_ASSERT(nodeIndices.nbIndices < NB_OBJECTS_PER_NODE);

	// store the new handle
	nodeIndices.indices[nodeIndices.nbIndices++] = index;

	// increase the node bounds
	node->mBVMin = V4Min(node->mBVMin, minV);
	node->mBVMax = V4Max(node->mBVMax, maxV);

	updateHierarchyAfterInsert(node);
}

// check if node does intersect with given bounds
PX_FORCE_INLINE static bool nodeIntersection(IncrementalAABBTreeNode& node, const Vec4V& minV,  const Vec4V& maxV)
{
	return !(Ps::IntBool(V4AnyGrtr3(node.mBVMin, maxV)) || Ps::IntBool(V4AnyGrtr3(minV, node.mBVMax)));
}

// traversal strategy
PX_FORCE_INLINE static PxU32 traversalDirection(IncrementalAABBTreeNode& child0, IncrementalAABBTreeNode& child1, const Vec4V& testCenterV)
{
	// traverse in the direction of a node which is closer
	// we compare the node and object centers
	const Vec4V centerCh0V = V4Add(child0.mBVMax, child0.mBVMin);
	const Vec4V centerCh1V = V4Add(child1.mBVMax, child1.mBVMin);

	const Vec4V ch0D = V4Sub(testCenterV, centerCh0V);
	const Vec4V ch1D = V4Sub(testCenterV, centerCh1V);

	const BoolV con = FIsGrtr(V4Dot3(ch0D, ch0D), V4Dot3(ch1D, ch1D));
	return (BAllEqTTTT(con) == 1) ? PxU32(1) : PxU32(0);
}

// remove an index from the leaf
PX_FORCE_INLINE static void removePrimitiveFromNode(IncrementalAABBTreeNode* node, const PoolIndex index)
{
	AABBTreeIndices& indices = *node->mIndices;
	PX_ASSERT(indices.nbIndices > 1);

	for (PxU32 i = indices.nbIndices; i--; )
	{
		if(node->mIndices->indices[i] == index)
		{
			node->mIndices->indices[i] = node->mIndices->indices[--indices.nbIndices];
			return;
		}
	}
	// if handle was not found something is wrong here
	PX_ASSERT(0);
}

// check if bounds are equal with given node min/max
PX_FORCE_INLINE static bool boundsEqual(const Vec4V& testMin, const Vec4V& testMax, const Vec4V& nodeMin, const Vec4V& nodeMax)
{
	return (Ps::IntBool(V4AllEq(nodeMin, testMin)) && Ps::IntBool(V4AllEq(testMax, nodeMax)));
}

// update the node hierarchy bounds when remove happen, we can early exit if the bounds are equal and no bounds update
// did happen
PX_FORCE_INLINE static void updateHierarchyAfterRemove(IncrementalAABBTreeNode* node, const PxBounds3* bounds)
{	
	if(node->isLeaf())
	{		
		const AABBTreeIndices& indices = *node->mIndices;
		PX_ASSERT(indices.nbIndices > 0);

		Vec4V bvMin = V4LoadU(&bounds[indices.indices[0]].minimum.x);
		Vec4V bvMax = V4LoadU(&bounds[indices.indices[0]].maximum.x);
		for(PxU32 i = 1; i < indices.nbIndices; i++)
		{
			const Vec4V minV = V4LoadU(&bounds[indices.indices[i]].minimum.x);
			const Vec4V maxV = V4LoadU(&bounds[indices.indices[i]].maximum.x);

			bvMin = V4Min(bvMin, minV);
			bvMax = V4Max(bvMax, maxV);
		}

		node->mBVMin = V4ClearW(bvMin);
		node->mBVMax = V4ClearW(bvMax);
	}
	else
	{
		node->mBVMin = V4Min(node->mChilds[0]->mBVMin, node->mChilds[1]->mBVMin);
		node->mBVMax = V4Max(node->mChilds[0]->mBVMax, node->mChilds[1]->mBVMax);
	}

	IncrementalAABBTreeNode* parent = node->mParent;
	while(parent)
	{
		const Vec4V newMinV = V4Min(parent->mChilds[0]->mBVMin, parent->mChilds[1]->mBVMin);
		const Vec4V newMaxV = V4Max(parent->mChilds[0]->mBVMax, parent->mChilds[1]->mBVMax);

		const bool earlyExit = boundsEqual(newMinV, newMaxV, parent->mBVMin, parent->mBVMax);
		if(earlyExit)
			break;

		parent->mBVMin = newMinV;
		parent->mBVMax = newMaxV;

		parent = parent->mParent;
	}
}

// split the leaf node along the most significant axis
IncrementalAABBTreeNode* IncrementalAABBTree::splitLeafNode(IncrementalAABBTreeNode* node, const PoolIndex index,  const Vec4V& minV,  const Vec4V& maxV, const PxBounds3* bounds)
{
	PX_ASSERT(node->isLeaf());

	IncrementalAABBTreeNode* returnNode = NULL;

	// create new pairs of nodes, parent will remain the node (the one we split_
	IncrementalAABBTreeNode* child0 = reinterpret_cast<IncrementalAABBTreeNode*>(mNodesPool.allocate());
	IncrementalAABBTreeNode* child1 = child0 + 1;	
	AABBTreeIndices* newIndices = mIndicesPool.allocate();

	// get the split axis
	PX_ALIGN(16, PxVec4) vars;
	PX_ALIGN(16, PxVec4) center;
	const float half = 0.5f;
	const FloatV halfV = FLoad(half);
	const Vec4V newMinV = V4Min(node->mBVMin, minV);
	const Vec4V newMaxV = V4Max(node->mBVMax, maxV);
	const Vec4V centerV = V4Scale(V4Add(newMaxV, newMinV), halfV);
	const Vec4V varsV = V4Sub(newMaxV, newMinV);
	V4StoreA(varsV, &vars.x);
	V4StoreA(centerV, &center.x);
	const PxU32 axis = Ps::largestAxis(PxVec3(vars.x, vars.y, vars.z));

	// setup parent
	child0->mParent = node;
	child1->mParent = node;
	child0->mIndices = node->mIndices;
	child0->mChilds[1] = NULL;
	child1->mIndices = newIndices;
	child1->mChilds[1] = NULL;

	AABBTreeIndices& child0Indices = *child0->mIndices;	// the original node indices
	AABBTreeIndices& child1Indices = *child1->mIndices; // new empty indices
	child1Indices.nbIndices = 0;

	// split the node
	for(PxU32 i = child0Indices.nbIndices; i--;)
	{
		const PxBounds3& primitiveBounds = bounds[child0Indices.indices[i]];
		const float pCenter = primitiveBounds.getCenter(axis);
		if(center[axis] > pCenter)
		{
			// move to new node
			child1Indices.indices[child1Indices.nbIndices++] = child0Indices.indices[i];						
			child0Indices.nbIndices--;
			child0Indices.indices[i] = child0Indices.indices[child0Indices.nbIndices];
		}
	}

	// check where to put the new node, if there is still a free space
	if(child0Indices.nbIndices == 0 || child1Indices.nbIndices == NB_OBJECTS_PER_NODE)
	{
		child0Indices.nbIndices = 1;
		child0Indices.indices[0] = index;
		returnNode = child0;
	}
	else
	{
		if(child0Indices.nbIndices == NB_OBJECTS_PER_NODE)
		{
			child1Indices.nbIndices = 1;
			child1Indices.indices[0] = index;
			returnNode = child1;
		}
		else
		{
			const PxBounds3& primitiveBounds = bounds[index];
			const float pCenter = primitiveBounds.getCenter(axis);
			if(center[axis] > pCenter)
			{
				// move to new node
				child1Indices.indices[child1Indices.nbIndices++] = index;
				returnNode = child1;
			}
			else
			{
				// move to old node
				child0Indices.indices[child0Indices.nbIndices++] = index;	
				returnNode = child0;
			}
		}
	}

	// update bounds for the new nodes
	Vec4V bvMin = V4LoadU(&bounds[child0Indices.indices[0]].minimum.x);
	Vec4V bvMax = V4LoadU(&bounds[child0Indices.indices[0]].maximum.x);	
	for(PxU32 i = 1; i < child0Indices.nbIndices; i++)
	{
		const Vec4V nodeMinV = V4LoadU(&bounds[child0Indices.indices[i]].minimum.x);
		const Vec4V nodeMaxV = V4LoadU(&bounds[child0Indices.indices[i]].maximum.x);	

		bvMin = V4Min(bvMin, nodeMinV);
		bvMax = V4Max(bvMax, nodeMaxV);
	}
	child0->mBVMin = V4ClearW(bvMin);
	child0->mBVMax = V4ClearW(bvMax);	

	bvMin = V4LoadU(&bounds[child1Indices.indices[0]].minimum.x);
	bvMax = V4LoadU(&bounds[child1Indices.indices[0]].maximum.x);	
	for(PxU32 i = 1; i < child1Indices.nbIndices; i++)
	{
		const Vec4V nodeMinV = V4LoadU(&bounds[child1Indices.indices[i]].minimum.x);
		const Vec4V nodeMaxV = V4LoadU(&bounds[child1Indices.indices[i]].maximum.x);	

		bvMin = V4Min(bvMin, nodeMinV);
		bvMax = V4Max(bvMax, nodeMaxV);
	}
	child1->mBVMin = V4ClearW(bvMin);
	child1->mBVMax = V4ClearW(bvMax);

	// node parent is the same, setup the new childs
	node->mChilds[0] = child0;
	node->mChilds[1] = child1;
	node->mBVMin = newMinV;
	node->mBVMax = newMaxV;

	updateHierarchyAfterInsert(node);

	PX_ASSERT(returnNode);
	return returnNode;
}


// insert new bounds into tree
IncrementalAABBTreeNode* IncrementalAABBTree::insert(const PoolIndex index, const PxBounds3* bounds, bool& split)
{	
	PX_SIMD_GUARD;

	// get the bounds, reset the W value
	const Vec4V minV = V4ClearW(V4LoadU(&bounds[index].minimum.x));
	const Vec4V maxV = V4ClearW(V4LoadU(&bounds[index].maximum.x));	

	split = false;

	// check if tree is empty
	if(!mRoot)
	{
		// make it a leaf
		AABBTreeIndices* indices = mIndicesPool.construct(index);
		mRoot = reinterpret_cast<IncrementalAABBTreeNode*> (mNodesPool.allocate());
		mRoot->mBVMin = minV;
		mRoot->mBVMax = maxV;
		mRoot->mIndices = indices;
		mRoot->mChilds[1] = NULL;
		mRoot->mParent = NULL;
		
		return mRoot;
	}
	else
	{
		// check if root is a leaf
		if(mRoot->isLeaf())
		{
			// if we still can insert the primitive into the leaf, or we need to split			
			if(mRoot->getNbPrimitives() < NB_OBJECTS_PER_NODE)
			{
				// simply add the primitive into the current leaf
				addPrimitiveIntoNode(mRoot, index, minV, maxV);
				return mRoot;
			}
			else
			{
				// need to split the node
				IncrementalAABBTreeNode* retNode = splitLeafNode(mRoot, index, minV, maxV, bounds);
				mRoot = retNode->mParent;
				split = true;
				return retNode;
			}
		}
		else
		{
			const Vec4V testCenterV = V4Add(maxV, minV);
			// we dont need to modify root, lets traverse the tree to find the right spot
			PxU32 traversalIndex = traversalDirection(*mRoot->mChilds[0], *mRoot->mChilds[1], testCenterV);
			IncrementalAABBTreeNode* baseNode = mRoot->mChilds[traversalIndex];
			while(!baseNode->isLeaf())
			{
				Ps::prefetchLine(baseNode->mChilds[0]->mChilds[0]);
				Ps::prefetchLine(baseNode->mChilds[1]->mChilds[0]);

				traversalIndex = traversalDirection(*baseNode->mChilds[0], *baseNode->mChilds[1], testCenterV);
				baseNode = baseNode->mChilds[traversalIndex];
			}
			
			// if we still can insert the primitive into the leaf, or we need to split			
			if(baseNode->getNbPrimitives() < NB_OBJECTS_PER_NODE)
			{
				// simply add the primitive into the current leaf
				addPrimitiveIntoNode(baseNode, index, minV, maxV);
				return baseNode;
			}
			else
			{
				// split
				IncrementalAABBTreeNode* retNode = splitLeafNode(baseNode, index, minV, maxV, bounds);				
				split = true;
				return retNode;
			}
		}
	}
}

// update the index, do a full remove/insert update
IncrementalAABBTreeNode* IncrementalAABBTree::update(IncrementalAABBTreeNode* node, const PoolIndex index, const PxBounds3* bounds, bool& split, IncrementalAABBTreeNode*& removedNode)
{
	PX_SIMD_GUARD;

	removedNode = remove(node, index, bounds);
	return insert(index, bounds, split);	
}

// update the index, faster version with a lazy update of objects that moved just a bit
IncrementalAABBTreeNode* IncrementalAABBTree::updateFast(IncrementalAABBTreeNode* node, const PoolIndex index, const PxBounds3* bounds, bool& split, IncrementalAABBTreeNode*& removedNode)
{
	PX_SIMD_GUARD;

	const Vec4V minV = V4ClearW(V4LoadU(&bounds[index].minimum.x));
	const Vec4V maxV = V4ClearW(V4LoadU(&bounds[index].maximum.x));

	// for update fast, we dont care if the tree gets slowly unbalanced, we are building a new tree already
	if(nodeIntersection(*node, minV, maxV))
	{
		updateHierarchyAfterRemove(node, bounds);
		return node;
	}
	else
	{
		removedNode = remove(node, index, bounds);	
		return insert(index, bounds, split);	
	}
}

// remove primitive from the tree, return a node if it moved to its parent
IncrementalAABBTreeNode* IncrementalAABBTree::remove(IncrementalAABBTreeNode* node, const PoolIndex index, const PxBounds3* bounds)
{
	PX_SIMD_GUARD;
	PX_ASSERT(node->isLeaf());
	// if we just remove the primitive from the list
	if(node->getNbPrimitives() > 1)
	{
		removePrimitiveFromNode(node, index);

		// update the hierarchy		
		updateHierarchyAfterRemove(node, bounds);
		return NULL;
	}
	else
	{
		// if root node and the last primitive remove root
		if(node == mRoot)
		{
			mNodesPool.deallocate(reinterpret_cast<IncrementalAABBTreeNodePair*>(node));
			mRoot = NULL;
			return NULL;
		}
		else
		{
			// create new parent and remove the current leaf
			IncrementalAABBTreeNode* parent = node->mParent;
			IncrementalAABBTreeNodePair* removedPair = reinterpret_cast<IncrementalAABBTreeNodePair*>(parent->mChilds[0]);
			PX_ASSERT(!parent->isLeaf());

			// copy the remaining child into parent
			IncrementalAABBTreeNode* remainingChild = (parent->mChilds[0] == node) ? parent->mChilds[1] : parent->mChilds[0];
			parent->mBVMax = remainingChild->mBVMax;
			parent->mBVMin = remainingChild->mBVMin;
			if(remainingChild->isLeaf())
			{
				parent->mIndices = remainingChild->mIndices;
				parent->mChilds[1] = NULL;
			}
			else
			{
				parent->mChilds[0] = remainingChild->mChilds[0];
				parent->mChilds[0]->mParent = parent;
				parent->mChilds[1] = remainingChild->mChilds[1];
				parent->mChilds[1]->mParent = parent;
			}

			if(parent->mParent)
			{
				updateHierarchyAfterRemove(parent->mParent, bounds);
			}

			mIndicesPool.deallocate(node->mIndices);
			mNodesPool.deallocate(removedPair);	
			return parent;
		}
	}
}

// fixup the indices
void IncrementalAABBTree::fixupTreeIndices(IncrementalAABBTreeNode* node, const PoolIndex index, const PoolIndex newIndex)
{
	PX_ASSERT(node->isLeaf());

	AABBTreeIndices& indices = *node->mIndices;
	for(PxU32 i = 0; i < indices.nbIndices; i++)
	{
		if(indices.indices[i] == index)			
		{
			indices.indices[i] = newIndex;
			return;
		}
	}	
	PX_ASSERT(0);
}

// shift node
static void shiftNode(IncrementalAABBTreeNode* node, const Vec4V& shiftV)
{
	node->mBVMax = V4Sub(node->mBVMax, shiftV);
	node->mBVMin = V4Sub(node->mBVMin, shiftV);

	if(!node->isLeaf())
	{
		shiftNode(node->mChilds[0], shiftV);
		shiftNode(node->mChilds[1], shiftV);
	}
}

// shift origin
void IncrementalAABBTree::shiftOrigin(const PxVec3& shift)
{
	if(mRoot)
	{
		const Vec4V shiftV = V4ClearW(V4LoadU(&shift.x));

		shiftNode(mRoot, shiftV);
	}
}

static void checkNode(IncrementalAABBTreeNode* node, IncrementalAABBTreeNode* parent, const PxBounds3* bounds, PoolIndex maxIndex, PxU32& numIndices)
{
	PX_ASSERT(node->mParent == parent);
	PX_ASSERT(!parent->isLeaf());
	PX_ASSERT(parent->mChilds[0] == node || parent->mChilds[1] == node);

	//ASSERT_ISVALIDVEC3V(node->mBVMin);
	//ASSERT_ISVALIDVEC3V(node->mBVMax);

	if(!node->isLeaf())
	{
		PX_ASSERT(nodeInsideBounds(node->mChilds[0]->mBVMin, node->mChilds[0]->mBVMax, node->mBVMin, node->mBVMax));
		PX_ASSERT(nodeInsideBounds(node->mChilds[1]->mBVMin, node->mChilds[1]->mBVMax, node->mBVMin, node->mBVMax));

		const Vec4V testMinV = V4Min(parent->mChilds[0]->mBVMin, parent->mChilds[1]->mBVMin);
		const Vec4V testMaxV = V4Max(parent->mChilds[0]->mBVMax, parent->mChilds[1]->mBVMax);

		PX_UNUSED(testMinV);
		PX_UNUSED(testMaxV);
		PX_ASSERT(boundsEqual(testMinV, testMaxV, node->mBVMin, node->mBVMax));

		checkNode(node->mChilds[0], node, bounds, maxIndex, numIndices);
		checkNode(node->mChilds[1], node, bounds, maxIndex, numIndices);
	}
	else
	{
		const AABBTreeIndices& indices = *node->mIndices;
		PX_ASSERT(indices.nbIndices);
		Vec4V testMinV = V4ClearW(V4LoadU(&bounds[indices.indices[0]].minimum.x));
		Vec4V testMaxV = V4ClearW(V4LoadU(&bounds[indices.indices[0]].maximum.x));
		for(PxU32 i = 0; i < indices.nbIndices; i++)
		{
			PX_ASSERT(indices.indices[i] < maxIndex);
			numIndices++;

			const Vec4V minV = V4ClearW(V4LoadU(&bounds[indices.indices[i]].minimum.x));
			const Vec4V maxV = V4ClearW(V4LoadU(&bounds[indices.indices[i]].maximum.x));

			testMinV = V4Min(testMinV, minV);
			testMaxV = V4Max(testMaxV, maxV);

			PX_ASSERT(nodeInsideBounds(minV, maxV, node->mBVMin, node->mBVMax));
		}

		PX_ASSERT(boundsEqual(testMinV, testMaxV, node->mBVMin, node->mBVMax));
	}
}

void IncrementalAABBTree::hierarchyCheck(PoolIndex maxIndex, const PxBounds3* bounds)
{
	PxU32 numHandles = 0;
	if(mRoot && !mRoot->isLeaf())
	{
		checkNode(mRoot->mChilds[0], mRoot, bounds, maxIndex, numHandles);
		checkNode(mRoot->mChilds[1], mRoot, bounds, maxIndex, numHandles);

		PX_ASSERT(numHandles == maxIndex);
	}	
}

void IncrementalAABBTree::checkTreeLeaf(IncrementalAABBTreeNode* leaf, PoolIndex h)
{
	PX_ASSERT(leaf->isLeaf());

	const AABBTreeIndices& indices = *leaf->mIndices;
	bool found = false;
	for(PxU32 i = 0; i < indices.nbIndices; i++)
	{
		if(indices.indices[i] == h)
		{
			found = true;
			break;
		}
	}
	PX_UNUSED(found);
	PX_ASSERT(found);
}

// build the tree from given bounds
bool IncrementalAABBTree::build(AABBTreeBuildParams& params, Ps::Array<IncrementalAABBTreeNode*>& mapping)
{
	// Init stats
	BuildStats stats;
	const PxU32 nbPrimitives = params.mNbPrimitives;
	if (!nbPrimitives)
		return false;

	// Init stats
	stats.setCount(1);

	// Initialize indices. This list will be modified during build.
	PxU32* indices = reinterpret_cast<PxU32*>(PX_ALLOC(sizeof(PxU32)*nbPrimitives, "AABB tree indices"));
	// Identity permutation
	for (PxU32 i = 0; i<nbPrimitives; i++)
		indices[i] = i;

	// Allocate a pool of nodes
	mNodeAllocator.init(nbPrimitives, params.mLimit);

	// Compute box centers only once and cache them
	params.mCache = reinterpret_cast<PxVec3*>(PX_ALLOC(sizeof(PxVec3)*(nbPrimitives + 1), "cache"));
	const float half = 0.5f;
	const FloatV halfV = FLoad(half);
	for (PxU32 i = 0; i<nbPrimitives; i++)
	{
		const Vec4V curMinV = V4LoadU(&params.mAABBArray[i].minimum.x);
		const Vec4V curMaxV = V4LoadU(&params.mAABBArray[i].maximum.x);
		const Vec4V centerV = V4Scale(V4Add(curMaxV, curMinV), halfV);
		V4StoreU(centerV, &params.mCache[i].x);
	}

	// Build the hierarchy
	mNodeAllocator.mPool->_buildHierarchy(params, stats, mNodeAllocator, indices);

	PX_FREE_AND_RESET(params.mCache);

	IncrementalAABBTreeNode** treeNodes = reinterpret_cast<IncrementalAABBTreeNode**>(PX_ALLOC(sizeof(IncrementalAABBTreeNode*)*(stats.getCount()), "temp node helper array"));
	PxMemSet(treeNodes, 0, sizeof(IncrementalAABBTreeNode*)*(stats.getCount()));

	clone(mapping, indices, treeNodes);
	mRoot = treeNodes[0];
	mRoot->mParent = NULL;
	
	PX_FREE_AND_RESET(indices);
	PX_FREE_AND_RESET(treeNodes);

	mNodeAllocator.release();
	return true;
}

// clone the tree, the tree is computed in the NodeAllocator, similar to AABBTree flatten
void IncrementalAABBTree::clone(Ps::Array<IncrementalAABBTreeNode*>& mapping, const PxU32* _indices, IncrementalAABBTreeNode** treeNodes)
{		
	PxU32 offset = 0;
	const PxU32 nbSlabs = mNodeAllocator.mSlabs.size();
	for (PxU32 s = 0; s<nbSlabs; s++)
	{
		const NodeAllocator::Slab& currentSlab = mNodeAllocator.mSlabs[s];

		AABBTreeBuildNode* pool = currentSlab.mPool;
		for (PxU32 i = 0; i < currentSlab.mNbUsedNodes; i++)
		{
			IncrementalAABBTreeNode* destNode = treeNodes[offset];
			if(!destNode)
			{
				destNode = reinterpret_cast<IncrementalAABBTreeNode*>(mNodesPool.allocate());
				treeNodes[offset] = destNode;
			}

			destNode->mBVMin = V4ClearW(V4LoadU(&pool[i].mBV.minimum.x));
			destNode->mBVMax = V4ClearW(V4LoadU(&pool[i].mBV.maximum.x));

			if (pool[i].isLeaf())
			{
				AABBTreeIndices* indices = mIndicesPool.allocate();
				destNode->mIndices = indices;
				destNode->mChilds[1] = NULL;
				indices->nbIndices = pool[i].getNbPrimitives();
				PX_ASSERT(indices->nbIndices <= 16);
				const PxU32* sourceIndices = _indices + pool[i].mNodeIndex;
				for (PxU32 iIndices = 0; iIndices < indices->nbIndices; iIndices++)
				{
					const PxU32 sourceIndex = sourceIndices[iIndices];
					indices->indices[iIndices] = sourceIndex;
					PX_ASSERT(sourceIndex < mapping.size());
					mapping[sourceIndex] = destNode;
				}
			}
			else
			{
				PX_ASSERT(pool[i].mPos);
				PxU32 localNodeIndex = 0xffffffff;
				PxU32 nodeBase = 0;
				for (PxU32 j = 0; j<nbSlabs; j++)
				{
					if (pool[i].mPos >= mNodeAllocator.mSlabs[j].mPool && pool[i].mPos < mNodeAllocator.mSlabs[j].mPool + mNodeAllocator.mSlabs[j].mNbUsedNodes)
					{
						localNodeIndex = PxU32(pool[i].mPos - mNodeAllocator.mSlabs[j].mPool);
						break;
					}
					nodeBase += mNodeAllocator.mSlabs[j].mNbUsedNodes;
				}
				const PxU32 nodeIndex = nodeBase + localNodeIndex;

				IncrementalAABBTreeNode* child0 = treeNodes[nodeIndex];
				IncrementalAABBTreeNode* child1 = treeNodes[nodeIndex + 1];
				if(!child0)
				{
					PX_ASSERT(!child1);
					child0 = reinterpret_cast<IncrementalAABBTreeNode*>(mNodesPool.allocate());
					child1 = child0 + 1;
					treeNodes[nodeIndex] = child0;
					treeNodes[nodeIndex + 1] = child1;
				}

				destNode->mChilds[0] = child0;
				destNode->mChilds[1] = child1;
				child0->mParent = destNode;
				child1->mParent = destNode;
			}
			offset++;
		}
	}
}


