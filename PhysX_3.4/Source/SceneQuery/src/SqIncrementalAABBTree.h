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

#ifndef SQ_INCREMENTAL_AABB_TREE_H
#define SQ_INCREMENTAL_AABB_TREE_H

#include "foundation/PxBounds3.h"
#include "PsUserAllocated.h"
#include "PsVecMath.h"
#include "SqPruner.h"
#include "SqAABBTreeBuild.h"
#include "PsPool.h"

namespace physx
{
	using namespace shdfnd::aos;

	namespace Sq
	{
		class AABBTree;

		#define NB_OBJECTS_PER_NODE	4

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// tree indices, can change in runtime
		struct AABBTreeIndices
		{
			PX_FORCE_INLINE AABBTreeIndices(PoolIndex index) :
				nbIndices(1)
			{
				indices[0] = index;
				for(PxU32 i = 1; i < NB_OBJECTS_PER_NODE; i++)
				{
					indices[i] = 0;
				}
			}

			PxU32			nbIndices;
			PoolIndex		indices[NB_OBJECTS_PER_NODE];
		};

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// tree node, has parent information
		class IncrementalAABBTreeNode : public Ps::UserAllocated
		{
		public:
			PX_FORCE_INLINE								IncrementalAABBTreeNode():
				mParent(NULL)
			{
				mChilds[0] = NULL;
				mChilds[1] = NULL;
			}
			PX_FORCE_INLINE								IncrementalAABBTreeNode(AABBTreeIndices* indices):
				mParent(NULL)
			{
				mIndices = indices;
				mChilds[1] = NULL;
			}
			PX_FORCE_INLINE								~IncrementalAABBTreeNode() {}

			PX_FORCE_INLINE	PxU32						isLeaf()								const { return PxU32(mChilds[1]==0); }

			PX_FORCE_INLINE	const PxU32*				getPrimitives(const PxU32* )			const { return &mIndices->indices[0]; }
			PX_FORCE_INLINE	PxU32*						getPrimitives(PxU32* )					{ return &mIndices->indices[0]; }
			PX_FORCE_INLINE	PxU32						getNbPrimitives()						const { return mIndices->nbIndices; }

			PX_FORCE_INLINE	const IncrementalAABBTreeNode*	getPos(const IncrementalAABBTreeNode* )	const { return mChilds[0]; }
			PX_FORCE_INLINE	const IncrementalAABBTreeNode*	getNeg(const IncrementalAABBTreeNode* )	const { return mChilds[1]; }

			PX_FORCE_INLINE	IncrementalAABBTreeNode*		getPos(IncrementalAABBTreeNode* ) { return mChilds[0]; }
			PX_FORCE_INLINE	IncrementalAABBTreeNode*		getNeg(IncrementalAABBTreeNode* ) { return mChilds[1]; }

			PX_FORCE_INLINE	void						getAABBCenterExtentsV(physx::shdfnd::aos::Vec3V* center, physx::shdfnd::aos::Vec3V* extents) const
			{
				const float half = 0.5f;
				const FloatV halfV = FLoad(half);

				*extents = Vec3V_From_Vec4V((V4Scale(V4Sub(mBVMax, mBVMin), halfV)));
				*center = Vec3V_From_Vec4V((V4Scale(V4Add(mBVMax, mBVMin), halfV)));
			}

			PX_FORCE_INLINE	void						getAABBCenterExtentsV2(physx::shdfnd::aos::Vec3V* center, physx::shdfnd::aos::Vec3V* extents) const
			{
				*extents = Vec3V_From_Vec4V((V4Sub(mBVMax, mBVMin)));
				*center = Vec3V_From_Vec4V((V4Add(mBVMax, mBVMin)));
			}

			PX_FORCE_INLINE	void						getAABBMinMaxV(physx::shdfnd::aos::Vec4V* minV, physx::shdfnd::aos::Vec4V* maxV) const
			{
				*minV = mBVMin;
				*maxV = mBVMax;
			}

			Vec4V						mBVMin;		// Global bounding-volume min enclosing all the node-related primitives
			Vec4V						mBVMax;		// Global bounding-volume max enclosing all the node-related primitives			
			IncrementalAABBTreeNode*	mParent;	// node parent
			union
			{
				IncrementalAABBTreeNode*	mChilds[2];		// childs of node if not a leaf
				AABBTreeIndices*			mIndices;		// if leaf, indices information 
			};			
		};

		struct IncrementalAABBTreeNodePair
		{
			IncrementalAABBTreeNode	mNode0;
			IncrementalAABBTreeNode	mNode1;
		};

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// incremental AABB tree, all changes are immediatelly reflected to the tree
		class IncrementalAABBTree : public Ps::UserAllocated
		{
		public:
			IncrementalAABBTree();
			~IncrementalAABBTree();

			// Build the tree for the first time 
			bool						build(AABBTreeBuildParams& params, Ps::Array<IncrementalAABBTreeNode*>& mapping);

			// insert a new index into the tre
			IncrementalAABBTreeNode*	insert(const PoolIndex index,  const PxBounds3* bounds, bool& split);

			// update the object in the tree - full update insert/remove
			IncrementalAABBTreeNode*	update(IncrementalAABBTreeNode* node, const PoolIndex index, const PxBounds3* bounds, bool& split, IncrementalAABBTreeNode*& removedNode);
			// update the object in the tree, faster method, that may unballance the tree
			IncrementalAABBTreeNode*	updateFast(IncrementalAABBTreeNode* node, const PoolIndex index, const PxBounds3* bounds, bool& split, IncrementalAABBTreeNode*& removedNode);

			// remove object from the tree
			IncrementalAABBTreeNode*	remove(IncrementalAABBTreeNode* node, const PoolIndex index, const PxBounds3* bounds);

			// fixup the tree indices, if we swapped the objects in the pruning pool
			void						fixupTreeIndices(IncrementalAABBTreeNode* node, const PoolIndex index, const PoolIndex newIndex);

			// origin shift
			void						shiftOrigin(const PxVec3& shift);
			
			// get the tree root node
			const IncrementalAABBTreeNode*	getNodes() const { return mRoot; }

			// define this function so we can share the scene query code with regular AABBTree
			const PxU32*				getIndices() const { return NULL; }

			// paranoia checks
			void						hierarchyCheck(PoolIndex maxIndex, const PxBounds3* bounds);
			void						checkTreeLeaf(IncrementalAABBTreeNode* leaf, PoolIndex h);

			void						release();
			
		private:

			// clone the tree from the generic AABB tree that was built
			void						clone(Ps::Array<IncrementalAABBTreeNode*>& mapping, const PxU32* indices, IncrementalAABBTreeNode** treeNodes);

			// split leaf node, the newly added object does not fit in
			IncrementalAABBTreeNode*	splitLeafNode(IncrementalAABBTreeNode* node,  const PoolIndex index,  const Vec4V& minV,  const Vec4V& maxV, const PxBounds3* bounds);

			void						releaseNode(IncrementalAABBTreeNode* node);
		private:
			Ps::Pool<AABBTreeIndices>				mIndicesPool;
			Ps::Pool<IncrementalAABBTreeNodePair>	mNodesPool;
			IncrementalAABBTreeNode*			mRoot;

			NodeAllocator						mNodeAllocator;
		};
	}
}

#endif
