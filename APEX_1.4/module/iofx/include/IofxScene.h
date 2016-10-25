/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __IOFX_SCENE_H__
#define __IOFX_SCENE_H__

#include "Apex.h"
#include "ModuleIofx.h"

#include "ApexSDKIntl.h"
#include "ModuleIntl.h"
#include "ApexContext.h"
#include "ApexSDKHelpers.h"
#include "RenderVolumeImpl.h"

#include "PxGpuCopyDescQueue.h"

namespace nvidia
{
namespace apex
{
class SceneIntl;
class IofxManagerDescIntl;
class DebugRenderParams;
class IofxRenderCallback;
}
namespace iofx
{

class ModuleIofxImpl;
class IofxDebugRenderParams;
class IofxManager;

class IofxScene;

class IofxRenderableIteratorImpl : public IofxRenderableIterator, public ApexRWLockable, public UserAllocated
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	virtual IofxRenderable* getFirst();
	virtual IofxRenderable* getNext();
	virtual void			  release()
	{
		PX_DELETE(this);
	}

	IofxRenderableIteratorImpl(IofxScene& iofxScene)
		: mIofxScene(iofxScene), mVolumeCount(0), mVolumeIndex(0)
		, mActorCount(0), mActorIndex(0), mActorList(NULL)
	{
	}

private:
	PX_INLINE IofxRenderable* findNextInActorList();
	IofxRenderable* findFirstInNotEmptyVolume();

	IofxScene& mIofxScene;

	uint32_t mVolumeCount;
	uint32_t mVolumeIndex;

	uint32_t mActorCount;
	uint32_t mActorIndex;
	nvidia::apex::IofxActor* const* mActorList;
};

class IofxScene : public ModuleSceneIntl, public ApexContext, public ApexResourceInterface, public ApexResource
{
public:
	enum StatsDataEnum
	{
		SimulatedSpriteParticlesCount,
		SimulatedMeshParticlesCount,
		// insert new items before this line
		NumberOfStats			// The number of stats
	};
public:
	IofxScene(ModuleIofxImpl& module, SceneIntl& scene, RenderDebugInterface* debugRender, ResourceList& list);
	~IofxScene();

	/* ModuleSceneIntl */
	void				visualize();
	void				setModulePhysXScene(PxScene* s);
	PxScene*			getModulePhysXScene() const
	{
		return mPhysXScene;
	}
	PxScene*			mPhysXScene;

	virtual Module*	getModule();

	virtual SceneStats* getStats()
	{
		return &mModuleSceneStats;
	}

	bool							lockRenderResources()
	{
		renderLockAllActors();	// Lock options not implemented yet
		return true;
	}

	bool							unlockRenderResources()
	{
		renderUnLockAllActors();	// Lock options not implemented yet
		return true;
	}

	/* ApexResourceInterface */
	uint32_t		getListIndex() const
	{
		return m_listIndex;
	}
	void				setListIndex(ResourceList& list, uint32_t index)
	{
		m_listIndex = index;
		m_list = &list;
	}
	void				release();

	virtual IofxManager* createIofxManager(const IofxAsset& asset, const IofxManagerDescIntl& desc);
	void				releaseIofxManager(IofxManager* manager);

	void				submitTasks(float elapsedTime, float substepSize, uint32_t numSubSteps);
	void				fetchResults();

	void				fetchResultsPreRenderLock()
	{
		lockLiveRenderVolumes();
	}
	void				fetchResultsPostRenderUnlock()
	{
		unlockLiveRenderVolumes();
	}

	virtual void		prepareRenderables();

	PX_INLINE void		lockLiveRenderVolumes();
	PX_INLINE void		unlockLiveRenderVolumes();

	void				createModuleStats(void);
	void				destroyModuleStats(void);
	void				setStatValue(StatsDataEnum index, StatValue dataVal);

	void					setIofxRenderCallback(IofxRenderCallback* callback)
	{
		mRenderCallback = callback;
	}
	IofxRenderCallback*	getIofxRenderCallback() const
	{
		return mRenderCallback;
	}
	IofxRenderableIterator* createIofxRenderableIterator()
	{
		return PX_NEW(IofxRenderableIteratorImpl)(*this);
	}


	ModuleIofxImpl*		    mModule;
	SceneIntl*		mApexScene;
	RenderDebugInterface*	mDebugRender;

	nvidia::Mutex		mFetchResultsLock;
	shdfnd::ReadWriteLock mManagersLock;

	shdfnd::ReadWriteLock mLiveRenderVolumesLock;
	nvidia::Mutex		mAddedRenderVolumesLock;
	nvidia::Mutex		mDeletedRenderVolumesLock;
	
	ResourceList		mActorManagers;

	physx::Array<RenderVolumeImpl*> mLiveRenderVolumes;
	physx::Array<RenderVolumeImpl*> mAddedRenderVolumes;
	physx::Array<RenderVolumeImpl*> mDeletedRenderVolumes;

	DebugRenderParams*				mDebugRenderParams;
	IofxDebugRenderParams*			mIofxDebugRenderParams;

	SceneStats	mModuleSceneStats;	

	uint32_t		mPrevTotalSimulatedSpriteParticles;
	uint32_t		mPrevTotalSimulatedMeshParticles;

	IofxRenderCallback*	mRenderCallback;

	void                destroy();

	void				processDeferredRenderVolumes();
};

}
} // namespace nvidia

#endif
