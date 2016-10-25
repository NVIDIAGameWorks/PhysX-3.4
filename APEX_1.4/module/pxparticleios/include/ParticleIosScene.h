/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __PARTICLE_IOS_SCENE_H__
#define __PARTICLE_IOS_SCENE_H__

#include "Apex.h"
#include "ModuleParticleIos.h"
#include "ApexSDKIntl.h"
#include "ModuleIntl.h"
#include "ModuleParticleIosImpl.h"
#include "ApexSharedUtils.h"
#include "ApexSDKHelpers.h"
#include "ApexContext.h"
#include "ApexActor.h"
#include "ModulePerfScope.h"

#include "PsTime.h"

#include "DebugRenderParams.h"
#include "ParticleIosDebugRenderParams.h"

#include "ParticleIosCommon.h"

#include "FieldSamplerQueryIntl.h"

#if APEX_CUDA_SUPPORT
#include "ApexCudaWrapper.h"
#include "ApexCuda.h"
#include "CudaModuleScene.h"

#include "../cuda/include/common.h"

#define SCENE_CUDA_OBJ(scene, name) static_cast<ParticleIosSceneGPU&>(scene).APEX_CUDA_OBJ_NAME(name)

#endif

namespace nvidia
{
namespace apex
{
class RenderDebugInterface;
class FieldSamplerManagerIntl;
}

namespace pxparticleios
{

class ParticleIosInjectorStorage
{
public:
	virtual bool growInjectorStorage(uint32_t newSize) = 0;
};
class ParticleIosInjectorAllocator
{
public:
	ParticleIosInjectorAllocator(ParticleIosInjectorStorage* storage) : mStorage(storage)
	{
		mFreeInjectorListStart = NULL_INJECTOR_INDEX;
		mReleasedInjectorListStart = NULL_INJECTOR_INDEX;
	}

	uint32_t				allocateInjectorID();
	void						releaseInjectorID(uint32_t);
	void						flushReleased();

	static const uint32_t			NULL_INJECTOR_INDEX = 0xFFFFFFFFu;
	static const uint32_t			USED_INJECTOR_INDEX = 0xFFFFFFFEu;

private:
	ParticleIosInjectorStorage*	mStorage;

	physx::Array<uint32_t>			mInjectorList;
	uint32_t						mFreeInjectorListStart;
	uint32_t						mReleasedInjectorListStart;
};


class ParticleIosScene : public ModuleSceneIntl, public ApexContext, public ApexResourceInterface, public ApexResource, protected ParticleIosInjectorStorage
{
public:
	ParticleIosScene(ModuleParticleIosImpl& module, SceneIntl& scene, RenderDebugInterface* renderDebug, ResourceList& list);
	~ParticleIosScene();

	/* ModuleSceneIntl */
	void									release()
	{
		mModule->releaseModuleSceneIntl(*this);
	}

	PxScene*                                getModulePhysXScene() const
	{
		return mPhysXScene;
	}
	void                                    setModulePhysXScene(PxScene*);
	PxScene* 								mPhysXScene;

	void									visualize();

	virtual Module*						getModule()
	{
		return mModule;
	}

	virtual SceneStats* getStats()
	{
		return 0;
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
	uint32_t							getListIndex() const
	{
		return m_listIndex;
	}
	void                                    setListIndex(ResourceList& list, uint32_t index)
	{
		m_listIndex = index;
		m_list = &list;
	}

	virtual ParticleIosActorImpl*				createIosActor(ResourceList& list, ParticleIosAssetImpl& asset, IofxAsset& iofxAsset) = 0;

	void									submitTasks(float elapsedTime, float substepSize, uint32_t numSubSteps);
	void									setTaskDependencies();
	void									fetchResults();

	FieldSamplerManagerIntl* 					getInternalFieldSamplerManager();
	SceneIntl&							getApexScene() const
	{
		return *mApexScene;
	}
	PX_INLINE ParticleIosInjectorAllocator&	getInjectorAllocator()
	{
		return mInjectorAllocator;
	}
	virtual void							fetchInjectorParams(uint32_t injectorID, Px3InjectorParams& injParams) = 0;
	virtual void							updateInjectorParams(uint32_t injectorID, const Px3InjectorParams& injParams) = 0;

protected:
	virtual void onSimulationStart() {}
	virtual void onSimulationFinish()
	{
		mInjectorAllocator.flushReleased();
	}

	void									destroy();
	float							computeAABBDistanceSquared(const PxBounds3& aabb);

	ModuleParticleIosImpl* 						mModule;
	SceneIntl* 							mApexScene;

	RenderDebugInterface* 						mRenderDebug;
	float							mSumBenefit;

	DebugRenderParams* 						mDebugRenderParams;
	ParticleIosDebugRenderParams* 			mParticleIosDebugRenderParams;

	FieldSamplerManagerIntl* 					mFieldSamplerManager;

	ParticleIosInjectorAllocator			mInjectorAllocator;

	friend class ParticleIosActorImpl;
	friend class ParticleIosAssetImpl;
	friend class ModuleParticleIosImpl;
};

class ParticleIosSceneCPU : public ParticleIosScene
{
	class TimerCallback : public FieldSamplerCallbackIntl, public IofxManagerCallbackIntl, public UserAllocated
	{
		shdfnd::Time mTimer;
		float mMinTime, mMaxTime;
	public:
		TimerCallback() {}		
		void operator()(void* stream = NULL);
		void reset();
		float getElapsedTime() const;
	};
public:
	ParticleIosSceneCPU(ModuleParticleIosImpl& module, SceneIntl& scene, RenderDebugInterface* debugRender, ResourceList& list);
	~ParticleIosSceneCPU();

	virtual ParticleIosActorImpl*	createIosActor(ResourceList& list, ParticleIosAssetImpl& asset, IofxAsset& iofxAsset);

	virtual void				fetchInjectorParams(uint32_t injectorID, Px3InjectorParams& injParams)
	{
		PX_ASSERT(injectorID < mInjectorParamsArray.size());
		injParams = mInjectorParamsArray[ injectorID ];
	}
	virtual void				updateInjectorParams(uint32_t injectorID, const Px3InjectorParams& injParams)
	{
		PX_ASSERT(injectorID < mInjectorParamsArray.size());
		mInjectorParamsArray[ injectorID ] = injParams;
	}

	void							fetchResults();

protected:
	virtual bool growInjectorStorage(uint32_t newSize)
	{
		mInjectorParamsArray.resize(newSize);
		return true;
	}

private:
	physx::Array<Px3InjectorParams> mInjectorParamsArray;
	TimerCallback					mTimerCallback;

	friend class ParticleIosActorCPU;
};

#if APEX_CUDA_SUPPORT
class ParticleIosSceneGPU : public ParticleIosScene, public CudaModuleScene
{
	class EventCallback : public FieldSamplerCallbackIntl, public IofxManagerCallbackIntl, public UserAllocated
	{
		void* mEvent;
	public:
		EventCallback();
		void init();
		virtual ~EventCallback();
		void operator()(void* stream);
		PX_INLINE void* getEvent()
		{
			return mEvent;
		}
		bool mIsCalled;
	};
public:
	ParticleIosSceneGPU(ModuleParticleIosImpl& module, SceneIntl& scene, RenderDebugInterface* debugRender, ResourceList& list);
	~ParticleIosSceneGPU();

	virtual ParticleIosActorImpl*		createIosActor(ResourceList& list, ParticleIosAssetImpl& asset, IofxAsset& iofxAsset);

	virtual void					fetchInjectorParams(uint32_t injectorID, Px3InjectorParams& injParams);
	virtual void					updateInjectorParams(uint32_t injectorID, const Px3InjectorParams& injParams);

	void							fetchResults();

	void*							getHeadCudaObj()
	{
		return CudaModuleScene::getHeadCudaObj();
	}
//CUDA module objects
#include "../cuda/include/moduleList.h"

protected:
	virtual bool growInjectorStorage(uint32_t newSize);

	void onSimulationStart();

private:
	ApexCudaConstMemGroup				mInjectorConstMemGroup;
	InplaceHandle<InjectorParamsArray>	mInjectorParamsArrayHandle;

	EventCallback						mOnSimulationStart;
	physx::Array<EventCallback*>		mOnStartCallbacks;
	physx::Array<EventCallback*>		mOnFinishCallbacks;

	friend class ParticleIosActorGPU;
};
#endif

}
} // namespace nvidia

#endif // __PARTICLE_IOS_SCENE_H__
