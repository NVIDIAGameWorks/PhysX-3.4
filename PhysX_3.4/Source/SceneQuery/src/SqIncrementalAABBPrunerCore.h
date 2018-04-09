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

#ifndef SQ_INCREMENTAL_AABB_PRUNER_CORE_H
#define SQ_INCREMENTAL_AABB_PRUNER_CORE_H

#include "SqPruner.h"
#include "SqPruningPool.h"
#include "SqIncrementalAABBTree.h"
#include "SqAABBTreeUpdateMap.h"
#include "PsHashMap.h"

namespace physx
{

namespace Sq
{
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	typedef Ps::HashMap<PoolIndex, IncrementalAABBTreeNode*>			IncrementalPrunerMap;

	struct CoreTree
	{
		CoreTree():
			timeStamp(0),
			tree(NULL)
		{
		}

		PxU32					timeStamp;
		IncrementalAABBTree*	tree;
		IncrementalPrunerMap	mapping;
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	class IncrementalAABBPrunerCore : public Ps::UserAllocated
	{
	public:
		IncrementalAABBPrunerCore(const PruningPool* pool);
		~IncrementalAABBPrunerCore();

		void				release();

		bool				addObject(const PoolIndex poolIndex, PxU32 timeStamp);
		bool				removeObject(const PoolIndex poolIndex, const PoolIndex poolRelocatedLastIndex, PxU32& timeStamp);

		// if we swap object from bucket pruner index with an index in the regular AABB pruner
		void				swapIndex(const PoolIndex poolIndex, const PoolIndex poolRelocatedLastIndex);

		bool				updateObject(const PoolIndex poolIndex);

		PxU32				removeMarkedObjects(PxU32 timeStamp);

		PxAgain				raycast(const PxVec3& origin, const PxVec3& unitDir, PxReal& inOutDistance, PrunerCallback&) const;
		PxAgain				overlap(const Gu::ShapeData& queryVolume, PrunerCallback&) const;
		PxAgain				sweep(const Gu::ShapeData& queryVolume, const PxVec3& unitDir, PxReal& inOutDistance, PrunerCallback&) const;

		void				shiftOrigin(const PxVec3& shift);

		void				visualize(Cm::RenderOutput& out, PxU32 color) const;

		PX_FORCE_INLINE void				timeStampChange()
		{
			// swap current and last tree
			mLastTree = (mLastTree + 1) % 2;
			mCurrentTree = (mCurrentTree + 1) % 2;
		}

		void				build() {}

		PX_FORCE_INLINE	PxU32				getNbObjects()	const { return mAABBTree[0].mapping.size() + mAABBTree[1].mapping.size(); }

	private:
		void				updateMapping(bool split, IncrementalPrunerMap& mapping, const PoolIndex poolIndex, IncrementalAABBTreeNode* node);
		void				test();			

	private:
		static const PxU32		NUM_TREES = 2;

		PxU32					mCurrentTree;
		PxU32					mLastTree;
		CoreTree				mAABBTree[NUM_TREES];
		const PruningPool*		mPool;					// Pruning pool from AABB pruner
	};

}}

#endif

