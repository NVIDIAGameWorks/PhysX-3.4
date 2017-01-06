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

#ifndef SQ_AABBTREE_H
#define SQ_AABBTREE_H

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
	class AABBTreeUpdateMap;

	typedef Ps::Pair<PxU32, PxU32>		TreeMergePair;
	typedef Ps::Array<TreeMergePair >	TreeMergeMap;

	class BitArray
	{
		public:
										BitArray() : mBits(NULL), mSize(0) {}
										BitArray(PxU32 nb_bits) { init(nb_bits); }
										~BitArray() { PX_FREE_AND_RESET(mBits); mBits = NULL; }

						bool			init(PxU32 nb_bits);

		// Data management
		PX_FORCE_INLINE	void			setBit(PxU32 bit_number)
										{
											mBits[bit_number>>5] |= 1<<(bit_number&31);
										}
		PX_FORCE_INLINE	void			clearBit(PxU32 bit_number)
										{
											mBits[bit_number>>5] &= ~(1<<(bit_number&31));
										}
		PX_FORCE_INLINE	void			toggleBit(PxU32 bit_number)
										{
											mBits[bit_number>>5] ^= 1<<(bit_number&31);
										}

		PX_FORCE_INLINE	void			clearAll()			{ PxMemZero(mBits, mSize*4);		}
		PX_FORCE_INLINE	void			setAll()			{ PxMemSet(mBits, 0xff, mSize*4);	}

						void			resize(PxU32 maxBitNumber);

		// Data access
		PX_FORCE_INLINE	Ps::IntBool		isSet(PxU32 bit_number)	const
										{
											return Ps::IntBool(mBits[bit_number>>5] & (1<<(bit_number&31)));
										}

		PX_FORCE_INLINE	const PxU32*	getBits()	const	{ return mBits;		}
		PX_FORCE_INLINE	PxU32			getSize()	const	{ return mSize;		}

		protected:
						PxU32*			mBits;		//!< Array of bits
						PxU32			mSize;		//!< Size of the array in dwords
	};

	//! Contains AABB-tree build statistics
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

	//! Contains AABB-tree build parameters
	class AABBTreeBuildParams : public Ps::UserAllocated
	{
		public:
								AABBTreeBuildParams(PxU32 limit=1, PxU32 nb_prims=0, const PxBounds3* boxes=NULL) :
									mLimit(limit), mNbPrimitives(nb_prims), mAABBArray(boxes), mCache(NULL)	{}
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
		PX_FORCE_INLINE								AABBTreeBuildNode()			{}
		PX_FORCE_INLINE								~AABBTreeBuildNode()		{}

		PX_FORCE_INLINE	const PxBounds3&			getAABB()							const	{ return mBV;												}
		PX_FORCE_INLINE	const AABBTreeBuildNode*	getPos()							const	{ return mPos;												}
		PX_FORCE_INLINE	const AABBTreeBuildNode*	getNeg()							const	{ const AABBTreeBuildNode* P = mPos; return P ? P+1 : NULL;	}

		PX_FORCE_INLINE	bool						isLeaf()							const	{ return !getPos();			}

						PxBounds3					mBV;	//!< Global bounding-volume enclosing all the node-related primitives
						const AABBTreeBuildNode*	mPos;	//!< "Positive" & "Negative" children

						PxU32						mNodeIndex;			//!< Index of node-related primitives (in the tree's mIndices array)
						PxU32						mNbPrimitives;		//!< Number of primitives for this node

		// Data access
		PX_FORCE_INLINE	PxU32						getNbPrimitives()					const	{ return mNbPrimitives;		}

		PX_FORCE_INLINE	PxU32						getNbRuntimePrimitives()			const	{ return mNbPrimitives;		}
		PX_FORCE_INLINE void						setNbRunTimePrimitives(PxU32 val)			{ mNbPrimitives = val;		}
		PX_FORCE_INLINE	const PxU32*				getPrimitives(const PxU32* base)	const	{ return base+mNodeIndex;	}
		PX_FORCE_INLINE	PxU32*						getPrimitives(PxU32* base)					{ return base+mNodeIndex;	}

		// Internal methods
						void						subdivide(const AABBTreeBuildParams& params, BuildStats& stats, NodeAllocator& allocator, PxU32* const indices);
						void						_buildHierarchy(AABBTreeBuildParams& params, BuildStats& stats, NodeAllocator& allocator, PxU32* const indices);
	};

	//! AABB tree node used for runtime (smaller than for build)
	class AABBTreeRuntimeNode : public Ps::UserAllocated
	{
		public:
		PX_FORCE_INLINE								AABBTreeRuntimeNode()		{}
		PX_FORCE_INLINE								~AABBTreeRuntimeNode()		{}

		PX_FORCE_INLINE	PxU32						isLeaf()								const	{ return mData&1;			}

		PX_FORCE_INLINE	const PxU32*				getPrimitives(const PxU32* base)		const	{ return base + (mData>>5);	}
		PX_FORCE_INLINE	PxU32*						getPrimitives(PxU32* base)						{ return base + (mData>>5);	}
		PX_FORCE_INLINE	PxU32						getNbPrimitives()						const	{ return (mData>>1)&15;		}

		PX_FORCE_INLINE	PxU32						getPosIndex()							const	{ return mData>>1;			}
		PX_FORCE_INLINE	PxU32						getNegIndex()							const	{ return (mData>>1) + 1;			}
		PX_FORCE_INLINE	const AABBTreeRuntimeNode*	getPos(const AABBTreeRuntimeNode* base)	const	{ return base + (mData>>1);	}
		PX_FORCE_INLINE	const AABBTreeRuntimeNode*	getNeg(const AABBTreeRuntimeNode* base)	const	{ const AABBTreeRuntimeNode* P = getPos(base); return P ? P+1 : NULL;}

		PX_FORCE_INLINE	AABBTreeRuntimeNode*		getPos(AABBTreeRuntimeNode* base)				{ return base + (mData >> 1); }
		PX_FORCE_INLINE	AABBTreeRuntimeNode*		getNeg(AABBTreeRuntimeNode* base)				{ AABBTreeRuntimeNode* P = getPos(base); return P ? P + 1 : NULL; }

		PX_FORCE_INLINE	PxU32						getNbRuntimePrimitives()				const	{ return (mData>>1)&15;		}
		PX_FORCE_INLINE void						setNbRunTimePrimitives(PxU32 val)
													{
														PX_ASSERT(val<16);
														PxU32 data = mData & ~(15<<1);
														data |= val<<1;
														mData = data;
													}

		PX_FORCE_INLINE	void						getAABBCenterExtentsV(Vec3V* center, Vec3V* extents) const
													{
														const Vec4V minV = V4LoadU(&mBV.minimum.x);
														const Vec4V maxV = V4LoadU(&mBV.maximum.x);

														const float half = 0.5f;
														const FloatV halfV = FLoad(half);

														*extents = Vec3V_From_Vec4V(V4Scale(V4Sub(maxV, minV), halfV));
														*center = Vec3V_From_Vec4V(V4Scale(V4Add(maxV, minV), halfV));
													}

		PX_FORCE_INLINE	void						getAABBCenterExtentsV2(Vec3V* center, Vec3V* extents) const
													{
														const Vec4V minV = V4LoadU(&mBV.minimum.x);
														const Vec4V maxV = V4LoadU(&mBV.maximum.x);

														*extents = Vec3V_From_Vec4V(V4Sub(maxV, minV));
														*center = Vec3V_From_Vec4V(V4Add(maxV, minV));
													}

		PX_FORCE_INLINE	void						getAABBMinMaxV(Vec4V* minV, Vec4V* maxV) const
													{
														*minV = V4LoadU(&mBV.minimum.x);
														*maxV = V4LoadU(&mBV.maximum.x);
													}

						PxBounds3					mBV;	// Global bounding-volume enclosing all the node-related primitives
						PxU32						mData;	// 27 bits node or prim index|4 bits #prims|1 bit leaf
	};

	//! Contains AABB-tree merge parameters
	class AABBTreeMergeData
	{
	public:
		AABBTreeMergeData(PxU32 nbNodes, const AABBTreeRuntimeNode* nodes, PxU32 nbIndices, const PxU32* indices, PxU32 indicesOffset) :
			mNbNodes(nbNodes), mNodes(nodes), mNbIndices(nbIndices), mIndices(indices), mIndicesOffset(indicesOffset)
		{
		}

		~AABBTreeMergeData()		{}

		PX_FORCE_INLINE const AABBTreeRuntimeNode& getRootNode() const { return mNodes[0]; }

	public:
		PxU32			mNbNodes;		//!< Number of nodes of AABB tree merge
		const AABBTreeRuntimeNode*	mNodes;	//!< Nodes of AABB tree merge

		PxU32			mNbIndices;		//!< Number of indices of AABB tree merge
		const PxU32*	mIndices;		//!< Indices of AABB tree merge

		PxU32			mIndicesOffset;	//!< Indices offset from pruning pool
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
						void						flatten(AABBTreeRuntimeNode* dest);
						AABBTreeBuildNode*			getBiNode();

						AABBTreeBuildNode*			mPool;

						struct Slab
						{
							PX_FORCE_INLINE	Slab()	{}
							PX_FORCE_INLINE	Slab(AABBTreeBuildNode* pool, PxU32 nbUsedNodes, PxU32 maxNbNodes) : mPool(pool), mNbUsedNodes(nbUsedNodes), mMaxNbNodes(maxNbNodes)	{}
							AABBTreeBuildNode*		mPool;
							PxU32					mNbUsedNodes;
							PxU32					mMaxNbNodes;
						};
						Ps::Array<Slab>				mSlabs;
						PxU32						mCurrentSlabIndex;
						PxU32						mTotalNbNodes;
	};

	//! AABB-tree, N primitives/leaf
	class AABBTree : public Ps::UserAllocated
	{
		public:
													AABBTree();													
													~AABBTree();
		// Build
						bool						build(AABBTreeBuildParams& params);
		// Progressive building
						PxU32						progressiveBuild(AABBTreeBuildParams& params, BuildStats& stats, PxU32 progress, PxU32 limit);
		//~Progressive building
						void						release(bool clearRefitMap=true);

		// Merge tree with another one
						void						mergeTree(const AABBTreeMergeData& tree);
		// Initialize tree from given merge data
						void						initTree(const AABBTreeMergeData& tree);

		// Data access
		PX_FORCE_INLINE	const PxU32*				getIndices()		const	{ return mIndices;		}
		PX_FORCE_INLINE	PxU32*						getIndices()				{ return mIndices;		}
		PX_FORCE_INLINE	void						setIndices(PxU32* indices)	{ mIndices = indices;	}
		PX_FORCE_INLINE	PxU32						getNbNodes()		const	{ return mTotalNbNodes;	}
		PX_FORCE_INLINE	const AABBTreeRuntimeNode*	getNodes()			const	{ return mRuntimePool;	}
		PX_FORCE_INLINE	AABBTreeRuntimeNode*		getNodes()					{ return mRuntimePool;	}		
		PX_FORCE_INLINE	void						setNodes(AABBTreeRuntimeNode* nodes) { mRuntimePool = nodes;	}		
		PX_FORCE_INLINE	PxU32						getTotalPrims()		const	{ return mTotalPrims;	}

#if PX_DEBUG 
						void						validate()			const;
#endif
						void						shiftOrigin(const PxVec3& shift);

		// Shift indices of the tree by offset. Used for merged trees, when initial indices needs to be shifted to match indices in current pruning pool
						void						shiftIndices(PxU32 offset);
				
		private:
						PxU32*						mIndices;			//!< Indices in the app list. Indices are reorganized during build (permutation).
						PxU32						mNbIndices;			//!< Nb indices
						AABBTreeRuntimeNode*		mRuntimePool;		//!< Linear pool of nodes.
						NodeAllocator				mNodeAllocator;
						PxU32*						mParentIndices;		//!< PT: hot/cold split, keep parent data in separate array
		// Stats
						PxU32						mTotalNbNodes;		//!< Number of nodes in the tree.
						PxU32						mTotalPrims;		//!< Copy of final BuildStats::mTotalPrims

	// Progressive building
						FIFOStack*					mStack;
	//~Progressive building
						bool						buildInit(AABBTreeBuildParams& params, BuildStats& stats);
						void						buildEnd(AABBTreeBuildParams& params, BuildStats& stats);

		// tree merge							
						void						mergeRuntimeNode(AABBTreeRuntimeNode& targetNode, const AABBTreeMergeData& tree, PxU32 targetNodeIndex);
						void						mergeRuntimeLeaf(AABBTreeRuntimeNode& targetNode, const AABBTreeMergeData& tree, PxU32 targetNodeIndex);
						void						addRuntimeChilds(PxU32& nodeIndex, const AABBTreeMergeData& tree);
						void						traverseRuntimeNode(AABBTreeRuntimeNode& targetNode, const AABBTreeMergeData& tree, PxU32 nodeIndex);
		// REFIT
		public:
						void						fullRefit(const PxBounds3* boxes);

		// adds node[index] to a list of nodes to refit when refitMarkedNodes is called
		// Note that this includes updating the hierarchy up the chain
						void						markNodeForRefit(TreeNodeIndex nodeIndex);
						void						refitMarkedNodes(const PxBounds3* boxes);
		private:
						BitArray					mRefitBitmask; //!< bit is set for each node index in markForRefit
						PxU32						mRefitHighestSetWord;
		//~REFIT
	};

} // namespace Sq

}

#endif // SQ_AABBTREE_H
