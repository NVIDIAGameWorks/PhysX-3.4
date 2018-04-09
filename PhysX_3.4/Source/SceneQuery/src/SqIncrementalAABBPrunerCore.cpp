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

#include "SqIncrementalAABBPrunerCore.h"
#include "SqIncrementalAABBTree.h"
#include "SqPruningPool.h"
#include "SqAABBTree.h"
#include "SqAABBTreeQuery.h"
#include "GuSphere.h"
#include "GuBox.h"
#include "GuCapsule.h"
#include "GuBounds.h"

using namespace physx;
using namespace Gu;
using namespace Sq;
using namespace Cm;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define PARANOIA_CHECKS 0

IncrementalAABBPrunerCore::IncrementalAABBPrunerCore(const PruningPool* pool) :
	mCurrentTree		(1),
	mLastTree			(0),
	mPool				(pool)
{
	mAABBTree[0].mapping.reserve(256);
	mAABBTree[1].mapping.reserve(256);
}

IncrementalAABBPrunerCore::~IncrementalAABBPrunerCore()
{
	release();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IncrementalAABBPrunerCore::release() // this can be called from purge()
{
	for(PxU32 i = 0; i < NUM_TREES; i++)
	{
		if(mAABBTree[i].tree)
		{
			PX_DELETE(mAABBTree[i].tree);
			mAABBTree[i].tree = NULL;
		}
		mAABBTree[i].mapping.clear();
		mAABBTree[i].timeStamp = 0;
	}
	mCurrentTree = 1;
	mLastTree = 0;
}

bool IncrementalAABBPrunerCore::addObject(const PoolIndex poolIndex, PxU32 timeStamp)
{
	CoreTree& tree = mAABBTree[mCurrentTree];
	if(!tree.tree || !tree.tree->getNodes())
	{
		if(!tree.tree)
			tree.tree = PX_NEW(IncrementalAABBTree)();
		tree.timeStamp = timeStamp;
	}
	PX_ASSERT(tree.timeStamp == timeStamp);

	bool split = false;
	IncrementalAABBTreeNode* node = tree.tree->insert(poolIndex, mPool->getCurrentWorldBoxes(), split);
	updateMapping(split, tree.mapping, poolIndex, node);

#if PARANOIA_CHECKS
	test();
#endif

	return true;
}

void IncrementalAABBPrunerCore::updateMapping(bool split, IncrementalPrunerMap& mapping, const PoolIndex poolIndex, IncrementalAABBTreeNode* node)
{
	// if a node was split we need to update the node indices and also the sibling indices
	if(split)
	{
		for(PxU32 j = 0; j < node->getNbPrimitives(); j++)
		{
			const PoolIndex index = node->getPrimitives(NULL)[j];
			mapping[index] = node;
		}
		// sibling
		if(node->mParent)
		{
			IncrementalAABBTreeNode* sibling = (node->mParent->mChilds[0] == node) ? node->mParent->mChilds[1] : node->mParent->mChilds[0];
			if(sibling->isLeaf())
			{
				for(PxU32 j = 0; j < sibling->getNbPrimitives(); j++)
				{
					const PoolIndex index = sibling->getPrimitives(NULL)[j];
					mapping[index] = sibling;
				}
			}
		}
	}
	else
	{
		mapping[poolIndex] = node;
	}
}

bool IncrementalAABBPrunerCore::removeObject(const PoolIndex poolIndex, const PoolIndex poolRelocatedLastIndex, PxU32& timeStamp)
{
	// erase the entry and get the data
	IncrementalPrunerMap::Entry entry;
	bool foundEntry = true;
	const PxU32 treeIndex = mAABBTree[mLastTree].mapping.erase(poolIndex, entry) ? mLastTree : mCurrentTree;
	// if it was not found in the last tree look at the current tree
	if(treeIndex == mCurrentTree)
		foundEntry = mAABBTree[mCurrentTree].mapping.erase(poolIndex, entry);

	// exit somethings is wrong here, entry was not found here
	PX_ASSERT(foundEntry);
	if(!foundEntry)
		return false;

	// tree must exist
	PX_ASSERT(mAABBTree[treeIndex].tree);
	CoreTree& tree = mAABBTree[treeIndex];
	timeStamp = tree.timeStamp;

	// remove the poolIndex from the tree, update the tree bounds immediatelly
	IncrementalAABBTreeNode* node = tree.tree->remove(entry.second, poolIndex, mPool->getCurrentWorldBoxes());
	if(node && node->isLeaf())
	{
		for(PxU32 j = 0; j < node->getNbPrimitives(); j++)
		{
			const PoolIndex index = node->getPrimitives(NULL)[j];
			tree.mapping[index] = node;
		}
	}

	// nothing to swap, last object, early exit
	if(poolIndex == poolRelocatedLastIndex)
	{
#if PARANOIA_CHECKS
	test();
#endif
		return true;
	}

	// fix the indices, we need to swap the index with last index
	// erase the relocated index from the tre it is
	IncrementalPrunerMap::Entry relocatedEntry;
	const PxU32 treeRelocatedIndex = mAABBTree[mCurrentTree].mapping.erase(poolRelocatedLastIndex, relocatedEntry) ? mCurrentTree : mLastTree;
	foundEntry = true;
	if(treeRelocatedIndex == mLastTree)
		foundEntry = mAABBTree[mLastTree].mapping.erase(poolRelocatedLastIndex, relocatedEntry);

	if(foundEntry)
	{
		CoreTree& relocatedTree = mAABBTree[treeRelocatedIndex];

		// set the new mapping
		relocatedTree.mapping[poolIndex] = relocatedEntry.second;
		// update the tree indices - swap 
		relocatedTree.tree->fixupTreeIndices(relocatedEntry.second, poolRelocatedLastIndex, poolIndex);
	}

#if PARANOIA_CHECKS
	test();
#endif
	return true;
}

void IncrementalAABBPrunerCore::swapIndex(const PoolIndex poolIndex, const PoolIndex poolRelocatedLastIndex)
{
	// fix the indices, we need to swap the index with last index
	// erase the relocated index from the tre it is
	IncrementalPrunerMap::Entry relocatedEntry;
	const PxU32 treeRelocatedIndex = mAABBTree[mCurrentTree].mapping.erase(poolRelocatedLastIndex, relocatedEntry) ? mCurrentTree : mLastTree;
	bool foundEntry = true;
	if(treeRelocatedIndex == mLastTree)
		foundEntry = mAABBTree[mLastTree].mapping.erase(poolRelocatedLastIndex, relocatedEntry);

	// relocated index is not here
	if(!foundEntry)
		return;

	CoreTree& relocatedTree = mAABBTree[treeRelocatedIndex];

	// set the new mapping
	relocatedTree.mapping[poolIndex] = relocatedEntry.second;
	// update the tree indices - swap 
	relocatedTree.tree->fixupTreeIndices(relocatedEntry.second, poolRelocatedLastIndex, poolIndex);
}

bool IncrementalAABBPrunerCore::updateObject(const PoolIndex poolIndex)
{
	const IncrementalPrunerMap::Entry* entry = mAABBTree[mLastTree].mapping.find(poolIndex);
	const PxU32 treeIndex = entry ? mLastTree : mCurrentTree;
	if(!entry)
		entry = mAABBTree[mCurrentTree].mapping.find(poolIndex);

	// we have not found it
	PX_ASSERT(entry);
	if(!entry)
		return false;

	CoreTree& tree = mAABBTree[treeIndex];
	bool split;
	IncrementalAABBTreeNode* removedNode = NULL;
	IncrementalAABBTreeNode* node = tree.tree->updateFast(entry->second, poolIndex, mPool->getCurrentWorldBoxes(), split, removedNode);
	// we removed node during update, need to update the mapping
	if(removedNode && removedNode->isLeaf())
	{
		for(PxU32 j = 0; j < removedNode->getNbPrimitives(); j++)
		{
			const PoolIndex index = removedNode->getPrimitives(NULL)[j];
			tree.mapping[index] = removedNode;
		}
	}
	if(split || node != entry->second)
		updateMapping(split, tree.mapping, poolIndex, node);

#if PARANOIA_CHECKS
	test();
#endif

	return true;
}

PxU32 IncrementalAABBPrunerCore::removeMarkedObjects(PxU32 timeStamp)
{
	// early exit is no tree exists
	if(!mAABBTree[mLastTree].tree || !mAABBTree[mLastTree].tree->getNodes())
	{
		PX_ASSERT(mAABBTree[mLastTree].mapping.size() == 0);
		PX_ASSERT(!mAABBTree[mCurrentTree].tree || mAABBTree[mCurrentTree].timeStamp != timeStamp);
		return 0;
	}

	PX_UNUSED(timeStamp);
	PX_ASSERT(timeStamp == mAABBTree[mLastTree].timeStamp);

	// release the last tree
	CoreTree& tree = mAABBTree[mLastTree];
	PxU32 nbObjects = tree.mapping.size();
	tree.mapping.clear();
	tree.timeStamp = 0;

	tree.tree->release();

	return nbObjects;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Query Implementation
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PxAgain IncrementalAABBPrunerCore::overlap(const ShapeData& queryVolume, PrunerCallback& pcb) const
{
	PxAgain again = true;

	for(PxU32 i = 0; i < NUM_TREES; i++)
	{
		const CoreTree& tree = mAABBTree[i];
		if(tree.tree && tree.tree->getNodes() && again)
		{
			switch(queryVolume.getType())
			{
			case PxGeometryType::eBOX:
				{
					if(queryVolume.isOBB())
					{	
						const Gu::OBBAABBTest test(queryVolume.getPrunerWorldPos(), queryVolume.getPrunerWorldRot33(), queryVolume.getPrunerBoxGeomExtentsInflated());
						again = AABBTreeOverlap<Gu::OBBAABBTest, IncrementalAABBTree, IncrementalAABBTreeNode>()(mPool->getObjects(), mPool->getCurrentWorldBoxes(), *tree.tree, test, pcb);
					}
					else
					{
						const Gu::AABBAABBTest test(queryVolume.getPrunerInflatedWorldAABB());
						again = AABBTreeOverlap<Gu::AABBAABBTest, IncrementalAABBTree, IncrementalAABBTreeNode>()(mPool->getObjects(), mPool->getCurrentWorldBoxes(), *tree.tree, test, pcb);
					}
				}
				break;
			case PxGeometryType::eCAPSULE:
				{
					const Gu::Capsule& capsule = queryVolume.getGuCapsule();
					const Gu::CapsuleAABBTest test(	capsule.p1, queryVolume.getPrunerWorldRot33().column0,
													queryVolume.getCapsuleHalfHeight()*2.0f, PxVec3(capsule.radius*SQ_PRUNER_INFLATION));
					again = AABBTreeOverlap<Gu::CapsuleAABBTest, IncrementalAABBTree, IncrementalAABBTreeNode>()(mPool->getObjects(), mPool->getCurrentWorldBoxes(), *tree.tree, test, pcb);
				}
				break;
			case PxGeometryType::eSPHERE:
				{
					const Gu::Sphere& sphere = queryVolume.getGuSphere();
					Gu::SphereAABBTest test(sphere.center, sphere.radius);
					again = AABBTreeOverlap<Gu::SphereAABBTest, IncrementalAABBTree, IncrementalAABBTreeNode>()(mPool->getObjects(), mPool->getCurrentWorldBoxes(), *tree.tree, test, pcb);
				}
				break;
			case PxGeometryType::eCONVEXMESH:
				{
					const Gu::OBBAABBTest test(queryVolume.getPrunerWorldPos(), queryVolume.getPrunerWorldRot33(), queryVolume.getPrunerBoxGeomExtentsInflated());
					again = AABBTreeOverlap<Gu::OBBAABBTest, IncrementalAABBTree, IncrementalAABBTreeNode>()(mPool->getObjects(), mPool->getCurrentWorldBoxes(), *tree.tree, test, pcb);			
				}
				break;
			case PxGeometryType::ePLANE:
			case PxGeometryType::eTRIANGLEMESH:
			case PxGeometryType::eHEIGHTFIELD:
			case PxGeometryType::eGEOMETRY_COUNT:
			case PxGeometryType::eINVALID:
				PX_ALWAYS_ASSERT_MESSAGE("unsupported overlap query volume geometry type");
			}
		}
	}

	return again;
}

PxAgain IncrementalAABBPrunerCore::sweep(const ShapeData& queryVolume, const PxVec3& unitDir, PxReal& inOutDistance, PrunerCallback& pcb) const
{
	PxAgain again = true;

	for(PxU32 i = 0; i < NUM_TREES; i++)
	{
		const CoreTree& tree = mAABBTree[i];
		if(tree.tree && tree.tree->getNodes() && again)
		{
			const PxBounds3& aabb = queryVolume.getPrunerInflatedWorldAABB();
			const PxVec3 extents = aabb.getExtents();
			again = AABBTreeRaycast<true, IncrementalAABBTree, IncrementalAABBTreeNode>()(mPool->getObjects(), mPool->getCurrentWorldBoxes(), *tree.tree, aabb.getCenter(), unitDir, inOutDistance, extents, pcb);
		}
	}

	return again;
}

PxAgain IncrementalAABBPrunerCore::raycast(const PxVec3& origin, const PxVec3& unitDir, PxReal& inOutDistance, PrunerCallback& pcb) const
{
	PxAgain again = true;

	for(PxU32 i = 0; i < NUM_TREES; i++)
	{
		const CoreTree& tree = mAABBTree[i];
		if(tree.tree && tree.tree->getNodes() && again)
		{
			again = AABBTreeRaycast<false, IncrementalAABBTree, IncrementalAABBTreeNode>()(mPool->getObjects(), mPool->getCurrentWorldBoxes(), *tree.tree, origin, unitDir, inOutDistance, PxVec3(0.0f), pcb);
		}
	}
	return again;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IncrementalAABBPrunerCore::shiftOrigin(const PxVec3& shift)
{
	for(PxU32 i = 0; i < NUM_TREES; i++)
	{
		if(mAABBTree[i].tree)
		{
			mAABBTree[i].tree->shiftOrigin(shift);
		}
	}
}


#include "CmRenderOutput.h"
void IncrementalAABBPrunerCore::visualize(Cm::RenderOutput& out, PxU32 color) const
{
	for(PxU32 i = 0; i < NUM_TREES; i++)
	{
		if(mAABBTree[i].tree && mAABBTree[i].tree->getNodes())
		{
			struct Local
			{
				static void _Draw(const IncrementalAABBTreeNode* root, const IncrementalAABBTreeNode* node, Cm::RenderOutput& out_)
				{
					PxBounds3 bounds;
					V4StoreU(node->mBVMin, &bounds.minimum.x);
					V4StoreU(node->mBVMax, &bounds.maximum.x);
					out_ << Cm::DebugBox(bounds, true);
					if (node->isLeaf())
						return;
					_Draw(root, node->getPos(root), out_);
					_Draw(root, node->getNeg(root), out_);
				}
			};
			out << PxTransform(PxIdentity);
			out << color;
			Local::_Draw(mAABBTree[i].tree->getNodes(), mAABBTree[i].tree->getNodes(), out);
			

			// Render added objects not yet in the tree
			out << PxTransform(PxIdentity);
			out << PxU32(PxDebugColor::eARGB_WHITE);
		}
	}
}

void IncrementalAABBPrunerCore::test()
{
	for(PxU32 i = 0; i < NUM_TREES; i++)
	{
		if(mAABBTree[i].tree)
		{
			//mAABBTree[i].tree->hierarchyCheck(mPool->getNbActiveObjects(), mPool->getCurrentWorldBoxes());
			for (IncrementalPrunerMap::Iterator iter = mAABBTree[i].mapping.getIterator(); !iter.done(); ++iter)
			{
				mAABBTree[i].tree->checkTreeLeaf(iter->second, iter->first);
			}
		}
	}
}
