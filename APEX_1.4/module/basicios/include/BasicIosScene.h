/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __BASIC_IOS_SCENE_H__
#define __BASIC_IOS_SCENE_H__

#include "Apex.h"
#include "ModuleBasicIos.h"
#include "ApexSDKIntl.h"
#include "ModuleIntl.h"
#include "ModuleBasicIosImpl.h"
#include "ApexSharedUtils.h"
#include "ApexSDKHelpers.h"
#include "ApexContext.h"
#include "ApexActor.h"
#include "ModulePerfScope.h"

#include "PsTime.h"

#include "DebugRenderParams.h"
#include "BasicIosDebugRenderParams.h"

#include "BasicIosCommon.h"
#include "BasicIosCommonSrc.h"

#include "FieldSamplerQueryIntl.h"

#if APEX_CUDA_SUPPORT
#include "../cuda/include/common.h"

#include "ApexCudaWrapper.h"
#include "CudaModuleScene.h"

#define SCENE_CUDA_OBJ(scene, name) static_cast<BasicIosSceneGPU&>(scene).APEX_CUDA_OBJ_NAME(name)

#endif

namespace nvidia
{
namespace apex
{
class RenderDebugInterface;
class FieldSamplerManagerIntl;
}
namespace basicios
{

class BasicIosInjectorStorage
{
public:
	virtual bool growInjectorStorage(uint32_t newSize) = 0;
};
class BasicIosInjectorAllocator
{
public:
	BasicIosInjectorAllocator(BasicIosInjectorStorage* storage) : mStorage(storage)
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
	BasicIosInjectorStorage*	mStorage;

	physx::Array<uint32_t>			mInjectorList;
	uint32_t						mFreeInjectorListStart;
	uint32_t						mReleasedInjectorListStart;
};


class BasicIosScene : public ModuleSceneIntl, public ApexContext, public ApexResourceInterface, public ApexResource, protected BasicIosInjectorStorage
{
public:
	BasicIosScene(ModuleBasicIosImpl& module, SceneIntl& scene, RenderDebugInterface* renderDebug, ResourceList& list);
	~BasicIosScene();

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

	virtual BasicIosActorImpl*					createIosActor(ResourceList& list, BasicIosAssetImpl& asset, nvidia::apex::IofxAsset& iofxAsset) = 0;

	virtual void							submitTasks(float elapsedTime, float substepSize, uint32_t numSubSteps);
	virtual void							setTaskDependencies();
	virtual void							fetchResults();

	FieldSamplerManagerIntl* 					getInternalFieldSamplerManager();

	SceneIntl&							getApexScene() const
	{
		return *mApexScene;
	}

	PX_INLINE BasicIosInjectorAllocator&	getInjectorAllocator()
	{
		return mInjectorAllocator;
	}
	virtual void							fetchInjectorParams(uint32_t injectorID, InjectorParams& injParams) = 0;
	virtual void							updateInjectorParams(uint32_t injectorID, const InjectorParams& injParams) = 0;

protected:
	virtual void onSimulationStart() {}
	virtual void onSimulationFinish()
	{
		mInjectorAllocator.flushReleased();
	}


	ModuleBasicIosImpl* 						mModule;
	SceneIntl* 							mApexScene;

	void									destroy();

	float							computeAABBDistanceSquared(const PxBounds3& aabb);

	RenderDebugInterface* 						mDebugRender;
	float							mSumBenefit;

	DebugRenderParams*						mDebugRenderParams;
	BasicIosDebugRenderParams*				mBasicIosDebugRenderParams;

	FieldSamplerManagerIntl* 					mFieldSamplerManager;

	BasicIosInjectorAllocator				mInjectorAllocator;

	friend class BasicIosActorImpl;
	friend class BasicIosAssetImpl;
	friend class ModuleBasicIosImpl;
};


}
} // namespace nvidia

#endif // __BASIC_IOS_SCENE_H__
