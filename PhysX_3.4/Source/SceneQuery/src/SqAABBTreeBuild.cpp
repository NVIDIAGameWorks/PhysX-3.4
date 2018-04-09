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

#include "SqAABBTreeBuild.h"
#include "SqBounds.h"

#include "PsMathUtils.h"
#include "PsFoundation.h"
#include "GuInternal.h"

using namespace physx;
using namespace Sq;

NodeAllocator::NodeAllocator() : mPool(NULL), mCurrentSlabIndex(0), mTotalNbNodes(0)
{
}

NodeAllocator::~NodeAllocator()
{
	release();
}

void NodeAllocator::release()
{
	const PxU32 nbSlabs = mSlabs.size();
	for (PxU32 i = 0; i<nbSlabs; i++)
	{
		Slab& s = mSlabs[i];
		PX_DELETE_ARRAY(s.mPool);
	}

	mSlabs.reset();
	mCurrentSlabIndex = 0;
	mTotalNbNodes = 0;
}

void NodeAllocator::init(PxU32 nbPrimitives, PxU32 limit)
{
	const PxU32 maxSize = nbPrimitives * 2 - 1;	// PT: max possible #nodes for a complete tree
	const PxU32 estimatedFinalSize = maxSize <= 1024 ? maxSize : maxSize / limit;
	mPool = PX_NEW(AABBTreeBuildNode)[estimatedFinalSize];
	PxMemZero(mPool, sizeof(AABBTreeBuildNode)*estimatedFinalSize);

	// Setup initial node. Here we have a complete permutation of the app's primitives.
	mPool->mNodeIndex = 0;
	mPool->mNbPrimitives = nbPrimitives;

	mSlabs.pushBack(Slab(mPool, 1, estimatedFinalSize));
	mCurrentSlabIndex = 0;
	mTotalNbNodes = 1;
}

// PT: TODO: inline this?
AABBTreeBuildNode* NodeAllocator::getBiNode()
{
	mTotalNbNodes += 2;
	Slab& currentSlab = mSlabs[mCurrentSlabIndex];
	if (currentSlab.mNbUsedNodes + 2 <= currentSlab.mMaxNbNodes)
	{
		AABBTreeBuildNode* biNode = currentSlab.mPool + currentSlab.mNbUsedNodes;
		currentSlab.mNbUsedNodes += 2;
		return biNode;
	}
	else
	{
		// Allocate new slab
		const PxU32 size = 1024;
		AABBTreeBuildNode* pool = PX_NEW(AABBTreeBuildNode)[size];
		PxMemZero(pool, sizeof(AABBTreeBuildNode)*size);

		mSlabs.pushBack(Slab(pool, 2, size));
		mCurrentSlabIndex++;
		return pool;
	}
}

static PX_FORCE_INLINE float getSplittingValue(const PxBounds3& global_box, PxU32 axis)
{
	// Default split value = middle of the axis (using only the box)
	return global_box.getCenter(axis);
}

static PxU32 split(const PxBounds3& box, PxU32 nb, PxU32* const PX_RESTRICT prims, PxU32 axis, const AABBTreeBuildParams& params)
{
	// Get node split value
	const float splitValue = getSplittingValue(box, axis);

	PxU32 nbPos = 0;
	// Loop through all node-related primitives. Their indices range from "mNodePrimitives[0]" to "mNodePrimitives[mNbPrimitives-1]",
	// with mNodePrimitives = mIndices + mNodeIndex (i.e. those indices map the global list in the tree params).

	// PT: to avoid calling the unsafe [] operator
	const size_t ptrValue = size_t(params.mCache) + axis * sizeof(float);
	const PxVec3* /*PX_RESTRICT*/ cache = reinterpret_cast<const PxVec3*>(ptrValue);

	for (PxU32 i = 0; i<nb; i++)
	{
		// Get index in global list
		const PxU32 index = prims[i];

		// Test against the splitting value. The primitive value is tested against the enclosing-box center.
		// [We only need an approximate partition of the enclosing box here.]
		const float primitiveValue = cache[index].x;
		PX_ASSERT(primitiveValue == params.mCache[index][axis]);

		// Reorganize the list of indices in this order: positive - negative.
		if (primitiveValue > splitValue)
		{
			// Swap entries
			prims[i] = prims[nbPos];
			prims[nbPos] = index;
			// Count primitives assigned to positive space
			nbPos++;
		}
	}
	return nbPos;
}

void AABBTreeBuildNode::subdivide(const AABBTreeBuildParams& params, BuildStats& stats, NodeAllocator& allocator, PxU32* const indices)
{
	PxU32* const PX_RESTRICT primitives = indices + mNodeIndex;
	const PxU32 nbPrims = mNbPrimitives;

	// Compute global box & means for current node. The box is stored in mBV.
	Vec4V meansV;
	{
		const PxBounds3* PX_RESTRICT boxes = params.mAABBArray;
		PX_ASSERT(boxes);
		PX_ASSERT(primitives);
		PX_ASSERT(nbPrims);

		Vec4V minV = V4LoadU(&boxes[primitives[0]].minimum.x);
		Vec4V maxV = V4LoadU(&boxes[primitives[0]].maximum.x);

		meansV = V4LoadU(&params.mCache[primitives[0]].x);

		for (PxU32 i = 1; i<nbPrims; i++)
		{
			const PxU32 index = primitives[i];
			const Vec4V curMinV = V4LoadU(&boxes[index].minimum.x);
			const Vec4V curMaxV = V4LoadU(&boxes[index].maximum.x);
			meansV = V4Add(meansV, V4LoadU(&params.mCache[index].x));
			minV = V4Min(minV, curMinV);
			maxV = V4Max(maxV, curMaxV);
		}

		StoreBounds(mBV, minV, maxV);

		const float coeff = 1.0f / float(nbPrims);
		meansV = V4Scale(meansV, FLoad(coeff));
	}

	// Check the user-defined limit. Also ensures we stop subdividing if we reach a leaf node.
	if (nbPrims <= params.mLimit)
		return;

	bool validSplit = true;
	PxU32 nbPos;
	{
		// Compute variances
		Vec4V varsV = V4Zero();
		for (PxU32 i = 0; i<nbPrims; i++)
		{
			const PxU32 index = primitives[i];
			Vec4V centerV = V4LoadU(&params.mCache[index].x);
			centerV = V4Sub(centerV, meansV);
			centerV = V4Mul(centerV, centerV);
			varsV = V4Add(varsV, centerV);
		}
		const float coeffNb1 = 1.0f / float(nbPrims - 1);
		varsV = V4Scale(varsV, FLoad(coeffNb1));
		PX_ALIGN(16, PxVec4) vars;
		V4StoreA(varsV, &vars.x);

		// Choose axis with greatest variance
		const PxU32 axis = Ps::largestAxis(PxVec3(vars.x, vars.y, vars.z));

		// Split along the axis
		nbPos = split(mBV, nbPrims, primitives, axis, params);

		// Check split validity
		if (!nbPos || nbPos == nbPrims)
			validSplit = false;
	}

	// Check the subdivision has been successful
	if (!validSplit)
	{
		// Here, all boxes lie in the same sub-space. Two strategies:
		// - if we are over the split limit, make an arbitrary 50-50 split
		// - else stop subdividing
		if (nbPrims>params.mLimit)
		{
			nbPos = nbPrims >> 1;
		}
		else return;
	}

	// Now create children and assign their pointers.
	mPos = allocator.getBiNode();

	stats.increaseCount(2);

	// Assign children
	PX_ASSERT(!isLeaf());
	AABBTreeBuildNode* Pos = const_cast<AABBTreeBuildNode*>(mPos);
	AABBTreeBuildNode* Neg = Pos + 1;
	Pos->mNodeIndex = mNodeIndex;
	Pos->mNbPrimitives = nbPos;
	Neg->mNodeIndex = mNodeIndex + nbPos;
	Neg->mNbPrimitives = mNbPrimitives - nbPos;
}

void AABBTreeBuildNode::_buildHierarchy(AABBTreeBuildParams& params, BuildStats& stats, NodeAllocator& nodeBase, PxU32* const indices)
{
	// Subdivide current node
	subdivide(params, stats, nodeBase, indices);

	// Recurse
	if (!isLeaf())
	{
		AABBTreeBuildNode* Pos = const_cast<AABBTreeBuildNode*>(getPos());
		PX_ASSERT(Pos);
		AABBTreeBuildNode* Neg = Pos + 1;
		Pos->_buildHierarchy(params, stats, nodeBase, indices);
		Neg->_buildHierarchy(params, stats, nodeBase, indices);
	}

	stats.mTotalPrims += mNbPrimitives;
}
