/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "ApexDefs.h"
#include "Apex.h"
#include "ParticleIosScene.h"
#include "ModuleParticleIosImpl.h"
#include "ParticleIosActorImpl.h"
#include "ParticleIosActorCPU.h"
#include "SceneIntl.h"
#include "ModuleFieldSamplerIntl.h"
#include "ModulePerfScope.h"
#include "ApexUsingNamespace.h"
#include "RenderDebugInterface.h"


#if APEX_CUDA_SUPPORT
#include <cuda.h>
#include "ApexCutil.h"
#include "ParticleIosActorGPU.h"
#include "ApexCudaSource.h"
#endif

#include "Lock.h"

#define CUDA_OBJ(name) SCENE_CUDA_OBJ(*this, name)

namespace nvidia
{
namespace pxparticleios
{


#pragma warning(push)
#pragma warning(disable:4355)

ParticleIosScene::ParticleIosScene(ModuleParticleIosImpl& _module, SceneIntl& scene, RenderDebugInterface* renderDebug, ResourceList& list)
	: mPhysXScene(NULL)
	, mModule(&_module)
	, mApexScene(&scene)
	, mRenderDebug(renderDebug)
	, mSumBenefit(0.0f)
	, mFieldSamplerManager(NULL)
	, mInjectorAllocator(this)
{
	list.add(*this);

	/* Initialize reference to ParticleIosDebugRenderParams */
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
			memberHandle.initParamRef(ParticleIosDebugRenderParams::staticClassName(), true);
		}
	}

	/* Load reference to ParticleIosDebugRenderParams */
	NvParameterized::Interface* refPtr = NULL;
	memberHandle.getParamRef(refPtr);
	mParticleIosDebugRenderParams = DYNAMIC_CAST(ParticleIosDebugRenderParams*)(refPtr);
	PX_ASSERT(mParticleIosDebugRenderParams);
}
#pragma warning(pop)

ParticleIosScene::~ParticleIosScene()
{
}

void ParticleIosScene::destroy()
{
	removeAllActors();
	mApexScene->moduleReleased(*this);
	delete this;
}

void ParticleIosScene::setModulePhysXScene(PxScene* s)
{
	if (mPhysXScene == s)
	{
		return;
	}

	mPhysXScene = s;
	for (uint32_t i = 0; i < mActorArray.size(); ++i)
	{
		ParticleIosActorImpl* actor = DYNAMIC_CAST(ParticleIosActorImpl*)(mActorArray[i]);
		actor->setPhysXScene(mPhysXScene);
	}
}

void ParticleIosScene::visualize()
{
#ifndef WITHOUT_DEBUG_VISUALIZE
	if (!mParticleIosDebugRenderParams->VISUALIZE_PARTICLE_IOS_ACTOR)
	{
		return;
	}

	RENDER_DEBUG_IFACE(mRenderDebug)->pushRenderState();
	for (uint32_t i = 0 ; i < mActorArray.size() ; i++)
	{
		ParticleIosActorImpl* testActor = DYNAMIC_CAST(ParticleIosActorImpl*)(mActorArray[ i ]);
		testActor->visualize();
	}
	RENDER_DEBUG_IFACE(mRenderDebug)->popRenderState();
#endif
}

void ParticleIosScene::submitTasks(float /*elapsedTime*/, float /*substepSize*/, uint32_t /*numSubSteps*/)
{
	PxTaskManager* tm;
	{
		READ_LOCK(*mApexScene);
		tm = mApexScene->getTaskManager();
	}

	for (uint32_t i = 0; i < mActorArray.size(); ++i)
	{
		ParticleIosActorImpl* actor = DYNAMIC_CAST(ParticleIosActorImpl*)(mActorArray[i]);
		actor->submitTasks(tm);
	}
}

void ParticleIosScene::setTaskDependencies()
{
	PxTaskManager* tm;
	{
		READ_LOCK(*mApexScene);
		tm	= mApexScene->getTaskManager();
	}
#if 0
	//run IOS after PhysX
	PxTaskID		taskStartAfterID	= tm->getNamedTask(AST_PHYSX_CHECK_RESULTS);
	PxTaskID		taskFinishBeforeID	= (PxTaskID)0xFFFFFFFF;
#else
	//run IOS before PhysX
	PxTaskID		taskStartAfterID	= (PxTaskID)0xFFFFFFFF;
	PxTaskID		taskFinishBeforeID	= tm->getNamedTask(AST_PHYSX_SIMULATE);
#endif

	for (uint32_t i = 0; i < mActorArray.size(); ++i)
	{
		ParticleIosActorImpl* actor = DYNAMIC_CAST(ParticleIosActorImpl*)(mActorArray[i]);
		actor->setTaskDependencies(taskStartAfterID, taskFinishBeforeID);
	}

	onSimulationStart();
}

void ParticleIosScene::fetchResults()
{
	onSimulationFinish();

	for (uint32_t i = 0; i < mActorArray.size(); ++i)
	{
		ParticleIosActorImpl* actor = DYNAMIC_CAST(ParticleIosActorImpl*)(mActorArray[i]);
		actor->fetchResults();
	}
}

FieldSamplerManagerIntl* ParticleIosScene::getInternalFieldSamplerManager()
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

void ParticleIosSceneCPU::TimerCallback::operator()(void* stream)
{
	PX_UNUSED(stream);

	float elapsed = (float)mTimer.peekElapsedSeconds();
	mMinTime = PxMin(elapsed, mMinTime);
	mMaxTime = PxMax(elapsed, mMaxTime);
}

void ParticleIosSceneCPU::TimerCallback::reset()
{
	mTimer.getElapsedSeconds();
	mMinTime = 1e20;
	mMaxTime = 0.f;
}

float ParticleIosSceneCPU::TimerCallback::getElapsedTime() const
{
	return (mMaxTime - mMinTime) * 1000.f;
}

ParticleIosSceneCPU::ParticleIosSceneCPU(ModuleParticleIosImpl& module, SceneIntl& scene, RenderDebugInterface* debugRender, ResourceList& list) :
	ParticleIosScene(module, scene, debugRender, list)
{
}

ParticleIosSceneCPU::~ParticleIosSceneCPU()
{
}

ParticleIosActorImpl* ParticleIosSceneCPU::createIosActor(ResourceList& list, ParticleIosAssetImpl& asset, IofxAsset& iofxAsset)
{
	ParticleIosActorCPU* actor = PX_NEW(ParticleIosActorCPU)(list, asset, *this, iofxAsset);

	actor->setOnStartFSCallback(&mTimerCallback);
	actor->setOnFinishIOFXCallback(&mTimerCallback);
	return actor;
}

void ParticleIosSceneCPU::fetchResults()
{
	ParticleIosScene::fetchResults();

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

ParticleIosSceneGPU::EventCallback::EventCallback() : mIsCalled(false), mEvent(NULL)
{
}
void ParticleIosSceneGPU::EventCallback::init()
{
	if (mEvent == NULL)
	{
		CUT_SAFE_CALL(cuEventCreate((CUevent*)(&mEvent), CU_EVENT_DEFAULT));
	}
}

ParticleIosSceneGPU::EventCallback::~EventCallback()
{
	if (mEvent != NULL)
	{
		CUT_SAFE_CALL(cuEventDestroy((CUevent)mEvent));
	}
}

void ParticleIosSceneGPU::EventCallback::operator()(void* stream)
{
	if (mEvent != NULL)
	{
		CUT_SAFE_CALL(cuEventRecord((CUevent)mEvent, (CUstream)stream));
		mIsCalled = true;
	}
}

ParticleIosSceneGPU::ParticleIosSceneGPU(ModuleParticleIosImpl& module, SceneIntl& scene, RenderDebugInterface* debugRender, ResourceList& list)
	: ParticleIosScene(module, scene, debugRender, list)
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
		//injectorParamsArray.resize( mInjectorConstMemGroup.getStorage(), MAX_INJECTOR_COUNT );
		mInjectorConstMemGroup.end();
	}

}

ParticleIosSceneGPU::~ParticleIosSceneGPU()
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

ParticleIosActorImpl* ParticleIosSceneGPU::createIosActor(ResourceList& list, ParticleIosAssetImpl& asset, IofxAsset& iofxAsset)
{
	ParticleIosActorGPU* actor = PX_NEW(ParticleIosActorGPU)(list, asset, *this, iofxAsset);
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
	return actor;
}

void ParticleIosSceneGPU::fetchInjectorParams(uint32_t injectorID, Px3InjectorParams& injParams)
{
	APEX_CUDA_CONST_MEM_GROUP_SCOPE(mInjectorConstMemGroup);

	InjectorParamsArray injectorParamsArray;
	mInjectorParamsArrayHandle.fetch(_storage_, injectorParamsArray);
	PX_ASSERT(injectorID < injectorParamsArray.getSize());
	injectorParamsArray.fetchElem(_storage_, injParams, injectorID);
}
void ParticleIosSceneGPU::updateInjectorParams(uint32_t injectorID, const Px3InjectorParams& injParams)
{
	APEX_CUDA_CONST_MEM_GROUP_SCOPE(mInjectorConstMemGroup);

	InjectorParamsArray injectorParamsArray;
	mInjectorParamsArrayHandle.fetch(_storage_, injectorParamsArray);
	PX_ASSERT(injectorID < injectorParamsArray.getSize());
	injectorParamsArray.updateElem(_storage_, injParams, injectorID);
}

void ParticleIosSceneGPU::fetchResults()
{
	ParticleIosScene::fetchResults();

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

bool ParticleIosSceneGPU::growInjectorStorage(uint32_t newSize)
{
	APEX_CUDA_CONST_MEM_GROUP_SCOPE(mInjectorConstMemGroup);

	InjectorParamsArray injectorParamsArray;
	mInjectorParamsArrayHandle.fetch(_storage_, injectorParamsArray);
	if (injectorParamsArray.resize(_storage_, newSize))
	{
		mInjectorParamsArrayHandle.update(_storage_, injectorParamsArray);
		return true;
	}
	return false;
}


void ParticleIosSceneGPU::onSimulationStart()
{
	ParticleIosScene::onSimulationStart();

	PxGpuDispatcher* gd = mApexScene->getTaskManager()->getGpuDispatcher();
	PX_ASSERT(gd != NULL);
	PxScopedCudaLock _lock_(*gd->getCudaContextManager());

	//we pass default 0 stream so that this copy happens before any kernel launches
	APEX_CUDA_OBJ_NAME(simulateStorage).copyToDevice(gd->getCudaContextManager(), 0);

	mOnSimulationStart(NULL);
}

#endif

// ParticleIosInjectorAllocator
uint32_t ParticleIosInjectorAllocator::allocateInjectorID()
{
	if (mFreeInjectorListStart == NULL_INJECTOR_INDEX)
	{
		//try to get new injectors
		uint32_t size = mInjectorList.size();
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

void ParticleIosInjectorAllocator::releaseInjectorID(uint32_t injectorID)
{
	//add to released injector list
	PX_ASSERT(mInjectorList[injectorID] == USED_INJECTOR_INDEX);
	mInjectorList[injectorID] = mReleasedInjectorListStart;
	mReleasedInjectorListStart = injectorID;
}

void ParticleIosInjectorAllocator::flushReleased()
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

