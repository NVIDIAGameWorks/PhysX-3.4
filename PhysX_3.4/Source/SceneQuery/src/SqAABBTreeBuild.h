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

#ifndef SQ_AABBTREE_BUILD_H
#define SQ_AABBTREE_BUILD_H

#include "foundation/PxMemory.h"
#include "foundation/PxBounds3.h"
#include "PsUserAllocated.h"
#include "PsVecMath.h"
#include "SqTypedef.h"
#include "PsArray.h"

namespace physx
{

	using namespace shdfnd::aos;

	namespace Sq
	{
		//! Contains AABB-tree build statistics
		struct BuildStats
		{
			BuildStats() : mCount(0), mTotalPrims(0) {}

			PxU32	mCount;			//!< Number of nodes created
			PxU32	mTotalPrims;	//!< Total accumulated number of primitives. Should be much higher than the source
									//!< number of prims, since it accumulates all prims covered by each node (i.e. internal
									//!< nodes too, not just leaf ones)

			PX_FORCE_INLINE	void	reset() { mCount = mTotalPrims = 0; }

			PX_FORCE_INLINE	void	setCount(PxU32 nb) { mCount = nb; }
			PX_FORCE_INLINE	void	increaseCount(PxU32 nb) { mCount += nb; }
			PX_FORCE_INLINE	PxU32	getCount()				const { return mCount; }
		};

		//! Contains AABB-tree build parameters
		class AABBTreeBuildParams : public Ps::UserAllocated
		{
		public:
			AABBTreeBuildParams(PxU32 limit = 1, PxU32 nb_prims = 0, const PxBounds3* boxes = NULL) :
				mLimit(limit), mNbPrimitives(nb_prims), mAABBArray(boxes), mCache(NULL) {}
			~AABBTreeBuildParams()
			{
				reset();
			}

			PX_FORCE_INLINE	void	reset()
			{
				mLimit = mNbPrimitives = 0;
				mAABBArray = NULL;
				PX_FREE_AND_RESET(mCache);
			}

			PxU32			mLimit;			//!< Limit number of primitives / node. If limit is 1, build a complete tree (2*N-1 nodes)
			PxU32			mNbPrimitives;	//!< Number of (source) primitives.
			const	PxBounds3*		mAABBArray;		//!< Shortcut to an app-controlled array of AABBs.
			PxVec3*			mCache;			//!< Cache for AABB centers - managed by build code.
		};

		class NodeAllocator;

		//! AABB tree node used for building
		class AABBTreeBuildNode : public Ps::UserAllocated
		{
		public:
			PX_FORCE_INLINE								AABBTreeBuildNode() {}
			PX_FORCE_INLINE								~AABBTreeBuildNode() {}

			PX_FORCE_INLINE	const PxBounds3&			getAABB()							const { return mBV; }
			PX_FORCE_INLINE	const AABBTreeBuildNode*	getPos()							const { return mPos; }
			PX_FORCE_INLINE	const AABBTreeBuildNode*	getNeg()							const { const AABBTreeBuildNode* P = mPos; return P ? P + 1 : NULL; }

			PX_FORCE_INLINE	bool						isLeaf()							const { return !getPos(); }

			PxBounds3					mBV;	//!< Global bounding-volume enclosing all the node-related primitives
			const AABBTreeBuildNode*	mPos;	//!< "Positive" & "Negative" children

			PxU32						mNodeIndex;			//!< Index of node-related primitives (in the tree's mIndices array)
			PxU32						mNbPrimitives;		//!< Number of primitives for this node

															// Data access
			PX_FORCE_INLINE	PxU32						getNbPrimitives()					const { return mNbPrimitives; }

			PX_FORCE_INLINE	PxU32						getNbRuntimePrimitives()			const { return mNbPrimitives; }
			PX_FORCE_INLINE void						setNbRunTimePrimitives(PxU32 val) { mNbPrimitives = val; }
			PX_FORCE_INLINE	const PxU32*				getPrimitives(const PxU32* base)	const { return base + mNodeIndex; }
			PX_FORCE_INLINE	PxU32*						getPrimitives(PxU32* base) { return base + mNodeIndex; }

			// Internal methods
			void						subdivide(const AABBTreeBuildParams& params, BuildStats& stats, NodeAllocator& allocator, PxU32* const indices);
			void						_buildHierarchy(AABBTreeBuildParams& params, BuildStats& stats, NodeAllocator& allocator, PxU32* const indices);
		};

		// Progressive building
		class FIFOStack;
		//~Progressive building

		//! For complete trees we can predict the final number of nodes and preallocate them. For incomplete trees we can't.
		//! But we don't want to allocate nodes one by one (which would be quite slow), so we use this helper class to
		//! allocate N nodes at once, while minimizing the amount of nodes allocated for nothing. An initial amount of
		//! nodes is estimated using the max number for a complete tree, and the user-defined number of primitives per leaf.
		//! In ideal cases this estimated number will be quite close to the final number of nodes. When that number is not
		//! enough though, slabs of N=1024 extra nodes are allocated until the build is complete.
		class NodeAllocator : public Ps::UserAllocated
		{
		public:
			NodeAllocator();
			~NodeAllocator();

			void						release();
			void						init(PxU32 nbPrimitives, PxU32 limit);
			AABBTreeBuildNode*			getBiNode();

			AABBTreeBuildNode*			mPool;

			struct Slab
			{
				PX_FORCE_INLINE	Slab() {}
				PX_FORCE_INLINE	Slab(AABBTreeBuildNode* pool, PxU32 nbUsedNodes, PxU32 maxNbNodes) : mPool(pool), mNbUsedNodes(nbUsedNodes), mMaxNbNodes(maxNbNodes) {}
				AABBTreeBuildNode*		mPool;
				PxU32					mNbUsedNodes;
				PxU32					mMaxNbNodes;
			};
			Ps::Array<Slab>				mSlabs;
			PxU32						mCurrentSlabIndex;
			PxU32						mTotalNbNodes;
		};


	} // namespace Sq

}

#endif // SQ_AABBTREE_H
