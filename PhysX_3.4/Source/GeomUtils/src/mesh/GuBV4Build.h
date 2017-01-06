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

#ifndef GU_BV4_BUILD_H
#define GU_BV4_BUILD_H

#include "foundation/PxSimpleTypes.h"
#include "GuBV4.h"

namespace physx
{
namespace Gu
{
	class BV4Tree;
	class SourceMesh;

	//! Contains AABB-tree build statistics
	// PT: TODO: this is a duplicate of the SQ structure (TA34704)
	struct BuildStats
	{
								BuildStats() : mCount(0), mTotalPrims(0)	{}

						PxU32	mCount;			//!< Number of nodes created
						PxU32	mTotalPrims;	//!< Total accumulated number of primitives. Should be much higher than the source
												//!< number of prims, since it accumulates all prims covered by each node (i.e. internal
												//!< nodes too, not just leaf ones)

		PX_FORCE_INLINE	void	reset()							{ mCount = mTotalPrims = 0;	}

		PX_FORCE_INLINE	void	setCount(PxU32 nb)				{ mCount=nb;				}
		PX_FORCE_INLINE	void	increaseCount(PxU32 nb)			{ mCount+=nb;				}
		PX_FORCE_INLINE	PxU32	getCount()				const	{ return mCount;			}
	};

	// PT: TODO: refactor with SQ version (TA34704)
	class AABBTreeNode : public physx::shdfnd::UserAllocated
	{
		public:
		PX_FORCE_INLINE							AABBTreeNode() : mPos(0), mNodePrimitives(NULL), mNbPrimitives(0)
												{
												}
		PX_FORCE_INLINE							~AABBTreeNode()
												{
													mPos = 0;
													mNodePrimitives	= NULL;	// This was just a shortcut to the global list => no release
													mNbPrimitives	= 0;
												}
		// Data access
		PX_FORCE_INLINE	const PxBounds3&		getAABB()		const	{ return mBV;							}

		PX_FORCE_INLINE	const AABBTreeNode*		getPos()		const	{ return reinterpret_cast<const AABBTreeNode*>(mPos);		}
		PX_FORCE_INLINE	const AABBTreeNode*		getNeg()		const	{ const AABBTreeNode* P = getPos(); return P ? P+1 : NULL;	}

		PX_FORCE_INLINE	bool					isLeaf()		const	{ return !getPos();						}

						PxBounds3				mBV;		// Global bounding-volume enclosing all the node-related primitives
						size_t					mPos;		// "Positive" & "Negative" children

		// Data access
		PX_FORCE_INLINE	const PxU32*			getPrimitives()		const	{ return mNodePrimitives;	}
		PX_FORCE_INLINE	PxU32					getNbPrimitives()	const	{ return mNbPrimitives;		}

						PxU32*					mNodePrimitives;	//!< Node-related primitives (shortcut to a position in mIndices below)
						PxU32					mNbPrimitives;		//!< Number of primitives for this node
	};

	typedef		bool	(*WalkingCallback)	(const AABBTreeNode* current, PxU32 depth, void* userData);

	// PT: TODO: refactor with SQ version (TA34704)
	class AABBTree : public physx::shdfnd::UserAllocated
	{
		public:
											AABBTree();
											~AABBTree();

						bool				buildFromMesh(SourceMesh& mesh, PxU32 limit);
						void				release();

		PX_FORCE_INLINE	const PxU32*		getIndices()		const	{ return mIndices;		}	//!< Catch the indices
		PX_FORCE_INLINE	PxU32				getNbNodes()		const	{ return mTotalNbNodes;	}	//!< Catch the number of nodes

		PX_FORCE_INLINE	const PxU32*		getPrimitives()		const	{ return mPool->mNodePrimitives;	}
		PX_FORCE_INLINE	PxU32				getNbPrimitives()	const	{ return mPool->mNbPrimitives;		}
		PX_FORCE_INLINE	const AABBTreeNode*	getNodes()			const	{ return mPool;						}
		PX_FORCE_INLINE	const PxBounds3&	getBV()				const	{ return mPool->mBV;				}

						PxU32				walk(WalkingCallback callback, void* userData) const;
		private:
						PxU32*				mIndices;			//!< Indices in the app list. Indices are reorganized during build (permutation).
						AABBTreeNode*		mPool;				//!< Linear pool of nodes for complete trees. Null otherwise. [Opcode 1.3]
						PxU32				mTotalNbNodes;		//!< Number of nodes in the tree.
	};

	PX_PHYSX_COMMON_API bool BuildBV4Ex(BV4Tree& tree, SourceMesh& mesh, float epsilon, PxU32 nbTrisPerLeaf);

} // namespace Gu
}

#endif // GU_BV4_BUILD_H
