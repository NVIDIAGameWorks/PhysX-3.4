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
// Copyright (c) 2008-2018 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  


#ifndef PXC_NPTHREADCONTEXT_H
#define PXC_NPTHREADCONTEXT_H

#include "PxvConfig.h"
#include "CmScaling.h"
#include "CmRenderOutput.h"
#include "PxcNpCacheStreamPair.h"
#include "PxcConstraintBlockStream.h"
#include "GuContactBuffer.h"
#include "PxvContext.h"
#include "PxcThreadCoherentCache.h"
#include "CmBitMap.h"
#include "../pcm/GuPersistentContactManifold.h"

namespace physx
{

class PxsTransformCache;
class PxsMaterialManager;

namespace Sc
{
	class BodySim;
}
   
/*!
Per-thread context used by contact generation routines.
*/

struct PxcDataStreamPool
{
	PxU8* mDataStream;
	PxI32 mSharedDataIndex;
	PxU32 mDataStreamSize;
	PxU32 mSharedDataIndexGPU;

	bool isOverflown()	const
	{
		//FD: my expectaton is that reading those variables is atomic, shared indices are non-decreasing, 
		//so we can only get a false overflow alert because of concurrency issues, which is not a big deal as it means 
		//it did overflow a bit later  
		return mSharedDataIndex + mSharedDataIndexGPU >= mDataStreamSize;
	}
};

struct PxcNpContext
{
	private:
												PX_NOCOPY(PxcNpContext)
	public:

												PxcNpContext() :
													mNpMemBlockPool			(mScratchAllocator),
													mMeshContactMargin		(0.0f),
													mToleranceLength		(0.0f),
													mCreateContactStream	(false),
													mContactStreamPool		(NULL),
													mPatchStreamPool		(NULL),
													mForceAndIndiceStreamPool(NULL),
													mMaterialManager		(NULL)
												{
												}

					PxcScratchAllocator			mScratchAllocator;
					PxcNpMemBlockPool			mNpMemBlockPool;
					PxReal						mMeshContactMargin;
					PxReal						mToleranceLength;
					Cm::RenderBuffer			mRenderBuffer;
					bool						mCreateContactStream; // flag to enforce that contacts are stored persistently per workunit. Used for PVD.
					PxcDataStreamPool*			mContactStreamPool;
					PxcDataStreamPool*			mPatchStreamPool;
					PxcDataStreamPool*			mForceAndIndiceStreamPool;
					PxcDataStreamPool*			mConstraintWriteBackStreamPool;
					PxsMaterialManager*			mMaterialManager;

	PX_FORCE_INLINE	PxReal						getToleranceLength()		const	{ return mToleranceLength;					}
	PX_FORCE_INLINE	void						setToleranceLength(PxReal x)		{ mToleranceLength = x;						}
	PX_FORCE_INLINE	PxReal						getMeshContactMargin()		const	{ return mMeshContactMargin;				}
	PX_FORCE_INLINE	void						setMeshContactMargin(PxReal x)		{ mMeshContactMargin = x;					}
	PX_FORCE_INLINE	bool						getCreateContactStream()			{ return mCreateContactStream;				}

	PX_FORCE_INLINE	PxcNpMemBlockPool&			getNpMemBlockPool()					{ return mNpMemBlockPool;					}
	PX_FORCE_INLINE	const PxcNpMemBlockPool&	getNpMemBlockPool()			const	{ return mNpMemBlockPool;					}
	PX_FORCE_INLINE void						setMaterialManager(PxsMaterialManager* m){ mMaterialManager = m;				}
	PX_FORCE_INLINE PxsMaterialManager*			getMaterialManager() const			{ return mMaterialManager;					}

					Cm::RenderOutput			getRenderOutput()					{ return Cm::RenderOutput(mRenderBuffer);	}
};

class PxcNpThreadContext : public PxcThreadCoherentCache<PxcNpThreadContext, PxcNpContext>::EntryBase
{
												PX_NOCOPY(PxcNpThreadContext)
public:
												PxcNpThreadContext(PxcNpContext* params);
												~PxcNpThreadContext();

#if PX_ENABLE_SIM_STATS
					void						clearStats();
#endif

	PX_FORCE_INLINE void						setCreateContactStream(bool to)					{ mCreateContactStream = to;				}

	PX_FORCE_INLINE void						addLocalNewTouchCount(PxU32 newTouchCMCount)	{ mLocalNewTouchCount += newTouchCMCount;	}
	PX_FORCE_INLINE void						addLocalLostTouchCount(PxU32 lostTouchCMCount)	{ mLocalLostTouchCount += lostTouchCMCount;	}
	PX_FORCE_INLINE PxU32						getLocalNewTouchCount()					const	{ return mLocalNewTouchCount;				}
	PX_FORCE_INLINE PxU32						getLocalLostTouchCount()				const	{ return mLocalLostTouchCount;				}

	PX_FORCE_INLINE void						addLocalFoundPatchCount(PxU32 foundPatchCount)	{ mLocalFoundPatchCount += foundPatchCount;	}
	PX_FORCE_INLINE void						addLocalLostPatchCount(PxU32 lostPatchCount)	{ mLocalLostPatchCount += lostPatchCount;	}
	PX_FORCE_INLINE PxU32						getLocalFoundPatchCount()				const	{ return mLocalFoundPatchCount;				}
	PX_FORCE_INLINE PxU32						getLocalLostPatchCount()				const	{ return mLocalLostPatchCount;				}

	PX_FORCE_INLINE Cm::BitMap&					getLocalChangeTouch()							{ return mLocalChangeTouch;					}

	PX_FORCE_INLINE Cm::BitMap&					getLocalPatchChangeMap()						{ return mLocalPatchCountChange;			}

	void										reset(PxU32 cmCount);
	// debugging
					Cm::RenderOutput 			mRenderOutput;

	// dsequeira: Need to think about this block pool allocation a bit more. Ideally we'd be 
	// taking blocks from a single pool, except that we want to be able to selectively reclaim
	// blocks if the user needs to defragment, depending on which artifacts they're willing
	// to tolerate, such that the blocks we don't reclaim are contiguous.
#if PX_ENABLE_SIM_STATS
					PxU32						mDiscreteContactPairs	[PxGeometryType::eGEOMETRY_COUNT][PxGeometryType::eGEOMETRY_COUNT];
					PxU32						mModifiedContactPairs	[PxGeometryType::eGEOMETRY_COUNT][PxGeometryType::eGEOMETRY_COUNT];
#endif
					PxcContactBlockStream 		mContactBlockStream;		// constraint block pool
					PxcNpCacheStreamPair		mNpCacheStreamPair;			// narrow phase pairwise data cache

	// Everything below here is scratch state. Most of it can even overlap.

	// temporary contact buffer
					Gu::ContactBuffer			mContactBuffer;    

	PX_ALIGN(16, Gu::MultiplePersistentContactManifold		mTempManifold); 

					Gu::NarrowPhaseParams		mNarrowPhaseParams;

	// DS: this stuff got moved here from the PxcNpPairContext. As Pierre says:
	////////// PT: those members shouldn't be there in the end, it's not necessary
					Ps::Array<Sc::BodySim*>		mBodySimPool;
					PxsTransformCache*			mTransformCache;
					PxReal*						mContactDistance;
					bool						mPCM;
					bool						mContactCache;
					bool						mCreateContactStream;	// flag to enforce that contacts are stored persistently per workunit. Used for PVD.
					bool						mCreateAveragePoint;	// flag to enforce whether we create average points
#if PX_ENABLE_SIM_STATS
					PxU32						mCompressedCacheSize;
					PxU32						mNbDiscreteContactPairsWithCacheHits;
					PxU32						mNbDiscreteContactPairsWithContacts;
#endif
					PxReal						mDt; // AP: still needed for ccd
					PxU32						mCCDPass;
					PxU32						mCCDFaceIndex;

					PxU32						mMaxPatches;
					//PxU32						mTotalContactCount;
					PxU32						mTotalCompressedCacheSize;
					//PxU32						mTotalPatchCount;

					PxcDataStreamPool*			mContactStreamPool;
					PxcDataStreamPool*			mPatchStreamPool;
					PxcDataStreamPool*			mForceAndIndiceStreamPool; //this stream is used to store the force buffer and triangle index if we are performing mesh/heightfield contact gen
					PxcDataStreamPool*			mConstraintWriteBackStreamPool;
					PxsMaterialManager*			mMaterialManager;
private:
		// change touch handling.
					Cm::BitMap					mLocalChangeTouch;
					Cm::BitMap					mLocalPatchCountChange;
					PxU32						mLocalNewTouchCount;
					PxU32						mLocalLostTouchCount;
					PxU32						mLocalFoundPatchCount;
					PxU32						mLocalLostPatchCount;
};

}

#endif
