/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "Apex.h"

#include "BasicIosScene.h"
#include "ModuleBasicIosImpl.h"
#include "BasicIosActorImpl.h"
#include "BasicIosActorCPU.h"
#include "ApexUsingNamespace.h"
#include "SceneIntl.h"
#include "ModulePerfScope.h"
#include "ModuleFieldSamplerIntl.h"
#include "RenderDebugInterface.h"

#if APEX_CUDA_SUPPORT
#include <cuda.h>
#include "ApexCutil.h"
#include "BasicIosActorGPU.h"

#include "ApexCudaSource.h"
#endif

#include <PxScene.h>

namespace nvidia
{
namespace basicios
{

#pragma warning(push)
#pragma warning(disable:4355)

BasicIosScene::BasicIosScene(ModuleBasicIosImpl& _module, SceneIntl& scene, RenderDebugInterface* debugRender, ResourceList& list)
	: mPhysXScene(NULL)
	, mModule(&_module)
	, mApexScene(&scene)
	, mDebugRender(debugRender)
	, mSumBenefit(0.0f)
	, mFieldSamplerManager(NULL)
	, mInjectorAllocator(this)
{
	list.add(*this);

	/* Initialize reference to BasicIosDebugRenderParams */
	mDebugRenderParams = DYNAMIC_CAST(DebugRenderParams*)(mApexScene->getDebugRenderParams());
	PX_ASSERT(mDebugRenderParams);
	NvParameterized::Handle handle(*mDebugRenderParams), memberHandle(*mDebugRenderParams);
	int size;

	if (mDebugRenderParams->getParameterHandle("moduleName", handle) == NvParameterized::ERROR_NONE)
	{
		handle.getArraySize(size, 0);
		handle.resizeArray(size + 1);
		if (handle.getChildHandle(size, memberHandle) == NvParameterized::ERROR_NONE)
		{
			memberHandle.initParamRef(BasicIosDebugRenderParams::staticClassName(), true);
		}
	}

	/* Load reference to BasicIosDebugRenderParams */
	NvParameterized::Interface* refPtr = NULL;
	memberHandle.getParamRef(refPtr);
	mBasicIosDebugRenderParams = DYNAMIC_CAST(BasicIosDebugRenderParams*)(refPtr);
	PX_ASSERT(mBasicIosDebugRenderParams);
}

#pragma warning(pop)

BasicIosScene::~BasicIosScene()
{
}

void BasicIosScene::destroy()
{
	removeAllActors();
	mApexScene->moduleReleased(*this);
	delete this;
}

void BasicIosScene::setModulePhysXScene(PxScene* s)
{
	if (mPhysXScene == s)
	{
		return;
	}

	mPhysXScene = s;
	for (uint32_t i = 0; i < mActorArray.size(); ++i)
	{
		BasicIosActorImpl* actor = DYNAMIC_CAST(BasicIosActorImpl*)(mActorArray[i]);
		actor->setPhysXScene(mPhysXScene);
	}
}

void BasicIosScene::visualize()
{
#ifndef WITHOUT_DEBUG_VISUALIZE
	if (!mBasicIosDebugRenderParams->VISUALIZE_BASIC_IOS_ACTOR)
	{
		return;
	}

	RENDER_DEBUG_IFACE(mDebugRender)->pushRenderState();
	for (uint32_t i = 0 ; i < mActorArray.size() ; i++)
	{
		BasicIosActorImpl* testActor = DYNAMIC_CAST(BasicIosActorImpl*)(mActorArray[ i ]);
		testActor->visualize();
	}
	RENDER_DEBUG_IFACE(mDebugRender)->popRenderState();
#endif
}

void BasicIosScene::submitTasks(float /*elapsedTime*/, float /*substepSize*/, uint32_t /*numSubSteps*/)
{
	for (uint32_t i = 0; i < mActorArray.size(); ++i)
	{
		BasicIosActorImpl* actor = DYNAMIC_CAST(BasicIosActorImpl*)(mActorArray[i]);
		if (mPhysXScene)
		{
			PxVec3 gravity = mApexScene->getGravity();

			if (actor->getGravity() != gravity)
			{
				actor->setGravity(gravity);
			}
		}
		actor->submitTasks();
	}
}

void BasicIosScene::setTaskDependencies()
{
	for (uint32_t i = 0; i < mActorArray.size(); ++i)
	{
		BasicIosActorImpl* actor = DYNAMIC_CAST(BasicIosActorImpl*)(mActorArray[i]);
		actor->setTaskDependencies();
	}

	onSimulationStart();
}

void BasicIosScene::fetchResults()
{
	onSimulationFinish();

	for (uint32_t i = 0; i < mActorArray.size(); ++i)
	{
		BasicIosActorImpl* actor = DYNAMIC_CAST(BasicIosActorImpl*)(mActorArray[i]);
		actor->fetchResults();
	}
}

FieldSamplerManagerIntl* BasicIosScene::getInternalFieldSamplerManager()
{
	if (mFieldSamplerManager == NULL)
	{
		ModuleFieldSamplerIntl* moduleFieldSampler = mModule->getInternalModuleFieldSampler();
		if (moduleFieldSampler != NULL)
		{
			mFieldSamplerManager = moduleFieldSampler->getInternalFieldSamplerManager(*mApexScene);
			PX_ASSERT(mFieldSamplerManager != NULL);
		}
	}
	return mFieldSamplerManager;
}

/******************************** CPU Version ********************************/

void BasicIosSceneCPU::TimerCallback::operator()(void* stream)
{
	PX_UNUSED(stream);

	float elapsed = (float)mTimer.peekElapsedSeconds();
	mMinTime = PxMin(elapsed, mMinTime);
	mMaxTime = PxMax(elapsed, mMaxTime);
}

void BasicIosSceneCPU::TimerCallback::reset()
{
	mTimer.getElapsedSeconds();
	mMinTime = 1e20;
	mMaxTime = 0.f;
}

float BasicIosSceneCPU::TimerCallback::getElapsedTime() const
{
	return (mMaxTime - mMinTime) * 1000.f;
}

BasicIosSceneCPU::BasicIosSceneCPU(ModuleBasicIosImpl& module, SceneIntl& scene, RenderDebugInterface* debugRender, ResourceList& list) 
	: BASIC_IOS_SCENE(module, scene, debugRender, list)
{
}

BasicIosSceneCPU::~BasicIosSceneCPU()
{
}


void BasicIosSceneCPU::setCallbacks(BasicIosActorCPU* actor)
{
	actor->setOnStartFSCallback(&mTimerCallback);
	actor->setOnFinishIOFXCallback(&mTimerCallback);
}


BasicIosActorImpl* BasicIosSceneCPU::createIosActor(ResourceList& list, BasicIosAssetImpl& asset, nvidia::apex::IofxAsset& iofxAsset)
{
	BasicIosActorCPU* actor = PX_NEW(BasicIosActorCPU)(list, asset, *this, iofxAsset);
	setCallbacks(actor);
	return actor;
}

void BasicIosSceneCPU::fetchResults()
{
	BasicIosScene::fetchResults();

	nvidia::apex::StatValue val;
	val.Float = mTimerCallback.getElapsedTime();
	mTimerCallback.reset();
	if (val.Float > 0.f)
	{
		mApexScene->setApexStatValue(SceneIntl::ParticleSimulationTime, val);
	}
}

/******************************** GPU Version ********************************/

#if APEX_CUDA_SUPPORT

BasicIosSceneGPU::EventCallback::EventCallback() : mIsCalled(false), mEvent(NULL)
{
}
void BasicIosSceneGPU::EventCallback::init()
{
	if (mEvent == NULL)
	{
		CUT_SAFE_CALL(cuEventCreate((CUevent*)(&mEvent), CU_EVENT_DEFAULT));
	}
}

BasicIosSceneGPU::EventCallback::~EventCallback()
{
	if (mEvent != NULL)
	{
		CUT_SAFE_CALL(cuEventDestroy((CUevent)mEvent));
	}
}

void BasicIosSceneGPU::EventCallback::operator()(void* stream)
{
	if (mEvent != NULL)
	{
		CUT_SAFE_CALL(cuEventRecord((CUevent)mEvent, (CUstream)stream));
		mIsCalled = true;
	}
}

BasicIosSceneGPU::BasicIosSceneGPU(ModuleBasicIosImpl& module, SceneIntl& scene, RenderDebugInterface* debugRender, ResourceList& list)
	: BASIC_IOS_SCENE(module, scene, debugRender, list)
	, CudaModuleScene(scene, *mModule, APEX_CUDA_TO_STR(APEX_CUDA_MODULE_PREFIX))
	, mInjectorConstMemGroup(APEX_CUDA_OBJ_NAME(simulateStorage))
{
	{
		PxGpuDispatcher* gd = mApexScene->getTaskManager()->getGpuDispatcher();
		PX_ASSERT(gd != NULL);
		PxScopedCudaLock _lock_(*gd->getCudaContextManager());

		mOnSimulationStart.init();

//CUDA module objects
#include "../cuda/include/moduleList.h"
	}

	{
		mInjectorConstMemGroup.begin();

		mInjectorParamsArrayHandle.alloc(mInjectorConstMemGroup.getStorage());

		mInjectorConstMemGroup.end();
	}

}

BasicIosSceneGPU::~BasicIosSceneGPU()
{
	for (uint32_t i = 0; i < mOnStartCallbacks.size(); i++)
	{
		PX_DELETE(mOnStartCallbacks[i]);
	}
	for (uint32_t i = 0; i < mOnFinishCallbacks.size(); i++)
	{
		PX_DELETE(mOnFinishCallbacks[i]);
	}
	CudaModuleScene::destroy(*mApexScene);
}


void BasicIosSceneGPU::setCallbacks(BasicIosActorGPU* actor)
{
	mOnStartCallbacks.pushBack(PX_NEW(EventCallback)());
	mOnFinishCallbacks.pushBack(PX_NEW(EventCallback)());
	{
		PxGpuDispatcher* gd = mApexScene->getTaskManager()->getGpuDispatcher();
		PX_ASSERT(gd != NULL);
		PxScopedCudaLock _lock_(*gd->getCudaContextManager());

		mOnStartCallbacks.back()->init();
		mOnFinishCallbacks.back()->init();
	}
	actor->setOnStartFSCallback(mOnStartCallbacks.back());
	actor->setOnFinishIOFXCallback(mOnFinishCallbacks.back());
}


BasicIosActorImpl* BasicIosSceneGPU::createIosActor(ResourceList& list, BasicIosAssetImpl& asset, nvidia::apex::IofxAsset& iofxAsset)
{
	BasicIosActorGPU* actor = PX_NEW(BasicIosActorGPU)(list, asset, *this, iofxAsset);
	setCallbacks(actor);
	return actor;
}

void BasicIosSceneGPU::fetchInjectorParams(uint32_t injectorID, InjectorParams& injParams)
{
	APEX_CUDA_CONST_MEM_GROUP_SCOPE(mInjectorConstMemGroup);

	InjectorParamsArray injectorParamsArray;
	mInjectorParamsArrayHandle.fetch(_storage_, injectorParamsArray);
	PX_ASSERT(injectorID < injectorParamsArray.getSize());
	injectorParamsArray.fetchElem(_storage_, injParams, injectorID);
}
void BasicIosSceneGPU::updateInjectorParams(uint32_t injectorID, const InjectorParams& injParams)
{
	APEX_CUDA_CONST_MEM_GROUP_SCOPE(mInjectorConstMemGroup);

	InjectorParamsArray injectorParamsArray;
	mInjectorParamsArrayHandle.fetch(_storage_, injectorParamsArray);
	PX_ASSERT(injectorID < injectorParamsArray.getSize());
	injectorParamsArray.updateElem(_storage_, injParams, injectorID);
}

bool BasicIosSceneGPU::growInjectorStorage(uint32_t newSize)
{
	APEX_CUDA_CONST_MEM_GROUP_SCOPE(mInjectorConstMemGroup);

	if (mApexScene->isSimulating())
	{
		APEX_INTERNAL_ERROR("BasicIosSceneGPU::growInjectorStorage - is called while ApexScene in simulating!");
		PX_ASSERT(0);
	}

	InjectorParamsArray injectorParamsArray;
	mInjectorParamsArrayHandle.fetch(_storage_, injectorParamsArray);
	if (injectorParamsArray.resize(_storage_, newSize))
	{
		mInjectorParamsArrayHandle.update(_storage_, injectorParamsArray);
		return true;
	}
	return false;
}


void BasicIosSceneGPU::onSimulationStart()
{
	BasicIosScene::onSimulationStart();

	PxGpuDispatcher* gd = mApexScene->getTaskManager()->getGpuDispatcher();
	PX_ASSERT(gd != NULL);
	PxScopedCudaLock _lock_(*gd->getCudaContextManager());

	//we pass default 0 stream so that this copy happens before any kernel launches
	APEX_CUDA_OBJ_NAME(simulateStorage).copyToDevice(gd->getCudaContextManager(), 0);

	mOnSimulationStart(NULL);
}

void BasicIosSceneGPU::fetchResults()
{
	BasicIosScene::fetchResults();

	PxGpuDispatcher* gd = mApexScene->getTaskManager()->getGpuDispatcher();
	PX_ASSERT(gd != NULL);
	PxScopedCudaLock _lock_(*gd->getCudaContextManager());

	nvidia::apex::StatValue val;	
	val.Float = 0.f;
	float minTime = 1e30;
	
	for (uint32_t i = 0 ; i < this->mOnStartCallbacks.size(); i++)
	{
		if (mOnStartCallbacks[i]->mIsCalled && mOnFinishCallbacks[i]->mIsCalled)
		{
			mOnStartCallbacks[i]->mIsCalled = false;
			mOnFinishCallbacks[i]->mIsCalled = false;
			CUT_SAFE_CALL(cuEventSynchronize((CUevent)mOnStartCallbacks[i]->getEvent()));
			CUT_SAFE_CALL(cuEventSynchronize((CUevent)mOnFinishCallbacks[i]->getEvent()));
			float tmp;
			CUT_SAFE_CALL(cuEventElapsedTime(&tmp, (CUevent)mOnSimulationStart.getEvent(), (CUevent)mOnStartCallbacks[i]->getEvent()));
			minTime = PxMin(tmp, minTime);
			CUT_SAFE_CALL(cuEventElapsedTime(&tmp, (CUevent)mOnSimulationStart.getEvent(), (CUevent)mOnFinishCallbacks[i]->getEvent()));
			val.Float = PxMax(tmp, val.Float);
		}
	}
	val.Float -= PxMin(minTime, val.Float);	
	
	if (val.Float > 0.f)
	{
		mApexScene->setApexStatValue(SceneIntl::ParticleSimulationTime, val);
	}
}
#endif

// BasicIosInjectorAllocator
uint32_t BasicIosInjectorAllocator::allocateInjectorID()
{
	uint32_t size = mInjectorList.size();
	if (mFreeInjectorListStart == NULL_INJECTOR_INDEX)
	{
		//try to get new injectors
		if (mStorage->growInjectorStorage(size + 1) == false)
		{
			return NULL_INJECTOR_INDEX;
		}

		mFreeInjectorListStart = size;

		mInjectorList.resize(size + 1);
		mInjectorList.back() = NULL_INJECTOR_INDEX;
	}
	uint32_t injectorID = mFreeInjectorListStart;
	mFreeInjectorListStart = mInjectorList[injectorID];
	mInjectorList[injectorID] = USED_INJECTOR_INDEX;
	return injectorID;
}

void BasicIosInjectorAllocator::releaseInjectorID(uint32_t injectorID)
{
	//add to released injector list
	PX_ASSERT(mInjectorList[injectorID] == USED_INJECTOR_INDEX);
	mInjectorList[injectorID] = mReleasedInjectorListStart;
	mReleasedInjectorListStart = injectorID;
}

void BasicIosInjectorAllocator::flushReleased()
{
	//add all released injectors to free injector list
	while (mReleasedInjectorListStart != NULL_INJECTOR_INDEX)
	{
		uint32_t injectorID = mInjectorList[mReleasedInjectorListStart];

		//add to free injector list
		mInjectorList[mReleasedInjectorListStart] = mFreeInjectorListStart;
		mFreeInjectorListStart = mReleasedInjectorListStart;

		mReleasedInjectorListStart = injectorID;
	}
}

}
} // namespace nvidia
