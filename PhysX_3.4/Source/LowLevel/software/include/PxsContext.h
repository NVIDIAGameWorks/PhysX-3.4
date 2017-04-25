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


#ifndef PXS_CONTEXT_H
#define PXS_CONTEXT_H

#include "PxVisualizationParameter.h"
#include "PxSceneDesc.h"

#include "CmPool.h"

#include "PxvContext.h"
#include "PxvNphaseImplementationContext.h"
#include "PxsContactManager.h"
#include "PxcNpBatch.h"
#include "PxcConstraintBlockStream.h"
#include "PxcNpCacheStreamPair.h"
#include "PxcNpMemBlockPool.h"
#include "CmRenderOutput.h"
#include "CmUtils.h"
#include "CmTask.h"

#include "PxContactModifyCallback.h"

#include "PxsTransformCache.h"
#include "GuPersistentContactManifold.h"
#include "DyArticulation.h"

#if PX_SUPPORT_GPU_PHYSX
namespace physx
{
	class PxCudaContextManager;
}
#endif

namespace physx
{

class PxsRigidBody;
struct PxcConstraintBlock;
class PxsMaterialManager;
class PxsCCDContext;
struct PxsContactManagerOutput;
	
namespace Cm
{
	class FlushPool;
}

namespace Bp
{
	class AABBManagerImpl;
}

namespace IG
{
	class SimpleIslandManager;
	typedef PxU32 EdgeIndex;
}

enum PxsTouchEventCount
{
	PXS_LOST_TOUCH_COUNT			= 0,
	PXS_NEW_TOUCH_COUNT				= 1,
	PXS_CCD_RETOUCH_COUNT			= 2, 	// pairs that are touching at a CCD pass and were touching at discrete collision or at a previous CCD pass already
											// (but they could have lost touch in between)
	PXS_PATCH_FOUND_COUNT			= 3,
	PXS_PATCH_LOST_COUNT			= 4,
	PXS_TOUCH_EVENT_COUNT			= 5
};


class PxsContext : public Ps::UserAllocated, public PxcNpContext
{
												PX_NOCOPY(PxsContext)
public:
												PxsContext(	const PxSceneDesc& desc, PxTaskManager*, Cm::FlushPool&, PxU64 contextID);
												~PxsContext();

					void						removeRigidBody(PxsRigidBody&);

					Dy::Articulation*			createArticulation();
					void						destroyArticulation(Dy::Articulation&);

						void					createTransformCache(Ps::VirtualAllocatorCallback& allocatorCallback);

						PxsContactManager*		createContactManager(PxsContactManager* contactManager, const bool useCCD);
						void					createCache(Gu::Cache& cache, PxsContactManager* cm, PxU8 geomType0, PxU8 geomType1);
						void					destroyCache(Gu::Cache& cache);
						void					destroyContactManager(PxsContactManager* cm);

						
	PX_FORCE_INLINE		PxU64						getContextId() const { return mContextID; }

	// Collision properties
	PX_FORCE_INLINE		PxContactModifyCallback*	getContactModifyCallback()						const		{ return mContactModifyCallback;	}
	PX_FORCE_INLINE		void						setContactModifyCallback(PxContactModifyCallback* c)		{ mContactModifyCallback = c; mNpImplementationContext->setContactModifyCallback(c);}


    // resource-related
					void						setScratchBlock(void* addr, PxU32 size);

					void						setContactDistance(Ps::Array<PxReal, Ps::VirtualAllocator>* contactDistance);

	// Task-related
					void						updateContactManager(PxReal dt, bool hasBoundsArrayChanged, bool hasContactDistanceChanged, PxBaseTask* continuation, PxBaseTask* firstPassContinuation);
					void						secondPassUpdateContactManager(PxReal dt, PxBaseTask* continuation);
					void						fetchUpdateContactManager();
					void						swapStreams();
						
					void						resetThreadContexts();

	// Manager status change
					bool						getManagerTouchEventCount(int* newTouch, int* lostTouch, int* ccdTouch) const;
					bool						fillManagerTouchEvents(
													PxvContactManagerTouchEvent* newTouch, PxI32& newTouchCount,
													PxvContactManagerTouchEvent* lostTouch, PxI32& lostTouchCount,
													PxvContactManagerTouchEvent* ccdTouch, PxI32& ccdTouchCount);

	PX_FORCE_INLINE		void					getManagerPatchEventCount(PxU32& foundPatch, PxU32& lostPatch) const { foundPatch = mCMTouchEventCount[PXS_PATCH_FOUND_COUNT]; lostPatch = mCMTouchEventCount[PXS_PATCH_LOST_COUNT]; }
						bool					fillManagerPatchChangedEvents(
													PxsContactManager** foundPatch, PxU32& foundPatchCount,
													PxsContactManager** lostPatch, PxU32& lostPatchCount);

						void					beginUpdate();

	// PX_ENABLE_SIM_STATS
	PX_FORCE_INLINE	PxvSimStats&				getSimStats()						{ return mSimStats;													}
	PX_FORCE_INLINE	const PxvSimStats&			getSimStats()				const	{ return mSimStats;													}

	PX_FORCE_INLINE	Cm::FlushPool&				getTaskPool()				const	{ return mTaskPool;													}
	PX_FORCE_INLINE	Cm::RenderBuffer&			getRenderBuffer()					{ return mRenderBuffer;												}

					PxReal						getVisualizationParameter(PxVisualizationParameter::Enum param) const;
					void						setVisualizationParameter(PxVisualizationParameter::Enum param, PxReal value);

	PX_FORCE_INLINE	void						setVisualizationCullingBox(const PxBounds3& box)	{ mVisualizationCullingBox = box;					}
	PX_FORCE_INLINE	const PxBounds3&			getVisualizationCullingBox()const	{ return mVisualizationCullingBox;									}

	PX_FORCE_INLINE	PxReal						getRenderScale()			const	{ return mVisualizationParams[PxVisualizationParameter::eSCALE];	}
					Cm::RenderOutput			getRenderOutput()					{ return Cm::RenderOutput(mRenderBuffer);							}
	PX_FORCE_INLINE	bool						getPCM()					const	{ return mPCM;														}
	PX_FORCE_INLINE	bool						getContactCacheFlag()		const	{ return mContactCache;												}
	PX_FORCE_INLINE	bool						getCreateAveragePoint()		const	{ return mCreateAveragePoint;										}

	// general stuff
					void						shiftOrigin(const PxVec3& shift);

					void						setCreateContactStream(bool to);
	PX_FORCE_INLINE	void						setPCM(bool enabled)					{ mPCM = enabled;				}
	PX_FORCE_INLINE	void						setContactCache(bool enabled)			{ mContactCache = enabled;		}

	PX_FORCE_INLINE	PxcScratchAllocator&		getScratchAllocator()					{ return mScratchAllocator;		}
	PX_FORCE_INLINE PxsTransformCache&			getTransformCache()						{ return *mTransformCache;		}
	PX_FORCE_INLINE PxReal*						getContactDistance()					{ return mContactDistance->begin(); }

	PX_FORCE_INLINE		PxvNphaseImplementationContext*		getNphaseImplementationContext() const
	{
		return mNpImplementationContext;
	}

	PX_FORCE_INLINE		void	setNphaseImplementationContext(PxvNphaseImplementationContext* ctx)
	{
		mNpImplementationContext = ctx;
	}

	PX_FORCE_INLINE		PxvNphaseImplementationContext*		getNphaseFallbackImplementationContext() const
	{
		return mNpFallbackImplementationContext;
	}

	PX_FORCE_INLINE		void	setNphaseFallbackImplementationContext(PxvNphaseImplementationContext* ctx)
	{
		mNpFallbackImplementationContext = ctx;
	}

	PxU32										getTotalCompressedContactSize() const	{ return mTotalCompressedCacheSize; }
	PxU32										getMaxPatchCount() const				{ return mMaxPatches; }

	PX_FORCE_INLINE		PxcThreadCoherentCache<PxcNpThreadContext, PxcNpContext>&		getNpThreadContextPool()
	{
		return mNpThreadContextPool;
	}

	PX_FORCE_INLINE		PxcNpThreadContext*		getNpThreadContext()
	{
		// We may want to conditional compile to exclude this on single threaded implementations
		// if it is determined to be a performance hit.
		return mNpThreadContextPool.get();
	}

	PX_FORCE_INLINE	void						putNpThreadContext(PxcNpThreadContext* threadContext)
																						{ mNpThreadContextPool.put(threadContext);	}
	PX_FORCE_INLINE Ps::Mutex&					getLock()								{ return mLock;					}

	PX_FORCE_INLINE	PxTaskManager&				getTaskManager() 
												{ 
													PX_ASSERT(mTaskManager);
													return *mTaskManager; 
												}

	PX_FORCE_INLINE	void						clearManagerTouchEvents();

	PX_FORCE_INLINE Cm::PoolList<PxsContactManager, PxsContext>& getContactManagerPool()
	{
		return this->mContactManagerPool;
	}

	PX_FORCE_INLINE void setActiveContactManager(const PxsContactManager* manager)
	{
		const PxU32 index = manager->getIndex();
		if (index >= mActiveContactManager.size())
		{
			PxU32 newSize = (2 * index + 256)&~255;
			mActiveContactManager.resize(newSize);
		}
		mActiveContactManager.set(index);

		//Record any pairs that have CCD enabled!
		if (manager->getCCD())
		{
			if (index >= mActiveContactManagersWithCCD.size())
			{
				PxU32 newSize = (2 * index + 256)&~255;
				mActiveContactManagersWithCCD.resize(newSize);
			}
			mActiveContactManagersWithCCD.set(index);
		}
	}


private:
						void					mergeCMDiscreteUpdateResults(PxBaseTask* continuation);
							
						PxU32					mIndex;

	// Threading
	PxcThreadCoherentCache<PxcNpThreadContext, PxcNpContext>
												mNpThreadContextPool;

	// Contact managers
	Cm::PoolList<PxsContactManager, PxsContext>		mContactManagerPool;
	Ps::Pool<Gu::LargePersistentContactManifold>	mManifoldPool;
	Ps::Pool<Gu::SpherePersistentContactManifold>	mSphereManifoldPool;
	
	Cm::BitMap				mActiveContactManager;
	Cm::BitMap				mActiveContactManagersWithCCD; //KS - adding to filter any pairs that had a touch
	Cm::BitMap				mContactManagersWithCCDTouch; //KS - adding to filter any pairs that had a touch
	Cm::BitMap				mContactManagerTouchEvent;
	Cm::BitMap				mContactManagerPatchChangeEvent;
	PxU32					mCMTouchEventCount[PXS_TOUCH_EVENT_COUNT];

	Ps::Mutex									mLock;



	PxContactModifyCallback*					mContactModifyCallback;

	// narrowphase platform-dependent implementations support
	PxvNphaseImplementationContext*				mNpImplementationContext;
	PxvNphaseImplementationContext*				mNpFallbackImplementationContext;
	
		
	// debug rendering (CS TODO: MS would like to have these wrapped into a class)
					PxReal						mVisualizationParams[PxVisualizationParameter::eNUM_VALUES];

					PxBounds3					mVisualizationCullingBox;

					PxTaskManager*				mTaskManager;
					Cm::FlushPool&				mTaskPool;


					//	PxU32					mTouchesLost;
					//	PxU32					mTouchesFound;

						// PX_ENABLE_SIM_STATS
					PxvSimStats									mSimStats;
					bool										mPCM;
					bool										mContactCache;
					bool										mCreateAveragePoint;

					PxsTransformCache*							mTransformCache;
					Ps::Array<PxReal, Ps::VirtualAllocator>*	mContactDistance;


					PxU32										mMaxPatches;
					PxU32										mTotalCompressedCacheSize;

					PxU64										mContextID;

					friend class PxsCCDContext;
					friend class PxsNphaseImplementationContext;
					friend class PxgNphaseImplementationContext; //FDTODO ideally it shouldn't be here..
};


PX_FORCE_INLINE void PxsContext::clearManagerTouchEvents()
{
	mContactManagerTouchEvent.clear();
	mContactManagerPatchChangeEvent.clear();
	for(PxU32 i = 0; i < PXS_TOUCH_EVENT_COUNT; ++i)
	{
		mCMTouchEventCount[i] = 0;
	}
}


}

#endif
