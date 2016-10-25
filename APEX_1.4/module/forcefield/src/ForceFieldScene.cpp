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
#include "ApexUsingNamespace.h"
#include "ScopedPhysXLock.h"

#include "ForceFieldScene.h"
#include "ForceFieldActorImpl.h"
#include "SceneIntl.h"
#include "ModulePerfScope.h"
#include "ModuleFieldSamplerIntl.h"

#if APEX_CUDA_SUPPORT
#include "ApexCudaSource.h"
#endif

#pragma warning(disable: 4355) // 'this' : used in base member initializer list

#include "Lock.h"

namespace nvidia
{
namespace forcefield
{

ForceFieldScene::ForceFieldScene(ModuleForceFieldImpl& module, SceneIntl& scene, RenderDebugInterface* renderDebug, ResourceList& list)
	: mRenderDebug(renderDebug)
	, mFieldSamplerManager(NULL)
	, mUpdateTask(*this)
{
	mModule = &module;
	mApexScene = &scene;
	mPhysXScene = NULL;
	list.add(*this);		// Add self to module's list of ForceFieldScenes

	/* Initialize reference to ForceFieldDebugRenderParams */
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
			memberHandle.initParamRef(ForceFieldDebugRenderParams::staticClassName(), true);
		}
	}

	/* Load reference to ForceFieldDebugRenderParams */
	NvParameterized::Interface* refPtr = NULL;
	memberHandle.getParamRef(refPtr);
	mForceFieldDebugRenderParams = DYNAMIC_CAST(ForceFieldDebugRenderParams*)(refPtr);
	PX_ASSERT(mForceFieldDebugRenderParams);
}

ForceFieldScene::~ForceFieldScene()
{
}

// Called by scene task graph between LOD and PhysX::simulate()
void ForceFieldScene::TaskUpdate::run()
{
	setProfileStat((uint16_t) mOwner.mActorArray.size());
	float dt = mOwner.mApexScene->getElapsedTime();
	mOwner.updateActors(dt);
}

// Called by updateTask between LOD and PhysX simulate.  Any writes
// to render data must be protected by acquiring the actor's render data lock
void ForceFieldScene::updateActors(float dt)
{
	SCOPED_PHYSX_LOCK_WRITE(mApexScene);
	for (uint32_t i = 0 ; i < mActorArray.size() ; i++)
	{
		ForceFieldActorImpl* actor = DYNAMIC_CAST(ForceFieldActorImpl*)(mActorArray[i]);
		actor->updateForceField(dt);
	}
}

// submit the task that updates the explosion actors
// called from ApexScene::simulate()
void ForceFieldScene::submitTasks(float /*elapsedTime*/, float /*substepSize*/, uint32_t /*numSubSteps*/)
{
	PxTaskManager* tm;
	{
		READ_LOCK(*mApexScene);
		tm = mApexScene->getTaskManager();
	}
	tm->submitUnnamedTask(mUpdateTask);
	mUpdateTask.startAfter(tm->getNamedTask(FSST_PHYSX_MONITOR_LOAD));
	mUpdateTask.finishBefore(tm->getNamedTask(FSST_PHYSX_MONITOR_FETCH));
}

void ForceFieldScene::setTaskDependencies()
{
}

// Called by ApexScene::fetchResults() with all actors render data locked.
void ForceFieldScene::fetchResults()
{
	PX_PROFILE_ZONE("ForceFieldSceneFetchResults", GetInternalApexSDK()->getContextId());

	for (uint32_t i = 0 ; i < mActorArray.size() ; i++)
	{
		ForceFieldActorImpl* actor = DYNAMIC_CAST(ForceFieldActorImpl*)(mActorArray[i]);
		actor->updatePoseAndBounds();
	}
}

void ForceFieldScene::visualize()
{
#ifndef WITHOUT_DEBUG_VISUALIZE
	if (!mForceFieldDebugRenderParams->VISUALIZE_FORCEFIELD_ACTOR)
	{
		return;
	}

	RENDER_DEBUG_IFACE(mRenderDebug)->pushRenderState();
	for (uint32_t i = 0 ; i < mActorArray.size() ; i++)
	{
		ForceFieldActorImpl* actor = DYNAMIC_CAST(ForceFieldActorImpl*)(mActorArray[i]);
		actor->visualize();
	}
	RENDER_DEBUG_IFACE(mRenderDebug)->popRenderState();
#endif
}

void ForceFieldScene::visualizeForceFieldForces()
{
}

void ForceFieldScene::visualizeForceFieldForceFields()
{
}

void ForceFieldScene::destroy()
{
	removeAllActors();
	mApexScene->moduleReleased(*this);
	delete this;
}

void ForceFieldScene::setModulePhysXScene(PxScene* pxScene)
{
	if (pxScene)
	{
		for (uint32_t i = 0 ; i < mActorArray.size() ; i++)
		{
			ForceFieldActorImpl* actor = DYNAMIC_CAST(ForceFieldActorImpl*)(mActorArray[i]);
			actor->setPhysXScene(pxScene);
		}
	}
	else
	{
		for (uint32_t i = 0 ; i < mActorArray.size() ; i++)
		{
			ForceFieldActorImpl* actor = DYNAMIC_CAST(ForceFieldActorImpl*)(mActorArray[i]);
			actor->setPhysXScene(NULL);
		}
	}

	mPhysXScene = pxScene;
}

FieldSamplerManagerIntl* ForceFieldScene::getInternalFieldSamplerManager()
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

ForceFieldSceneCPU::ForceFieldSceneCPU(ModuleForceFieldImpl& module, SceneIntl& scene, RenderDebugInterface* renderDebug, ResourceList& list) :
	ForceFieldScene(module, scene, renderDebug, list)
{
}

ForceFieldSceneCPU::~ForceFieldSceneCPU()
{
}

ForceFieldActorImpl* ForceFieldSceneCPU::createForceFieldActor(const ForceFieldActorDesc& desc, ForceFieldAssetImpl& asset, ResourceList& list)
{
	return PX_NEW(ForceFieldActorCPU)(desc, asset, list, *this);
}

/******************************** GPU Version ********************************/

#if APEX_CUDA_SUPPORT

ForceFieldSceneGPU::ForceFieldSceneGPU(ModuleForceFieldImpl& module, SceneIntl& scene, RenderDebugInterface* renderDebug, ResourceList& list)
	: ForceFieldScene(module, scene, renderDebug, list)
	, CudaModuleScene(scene, *mModule, APEX_CUDA_TO_STR(APEX_CUDA_MODULE_PREFIX))
{
	{
		PxGpuDispatcher* gd = mApexScene->getTaskManager()->getGpuDispatcher();
		PX_ASSERT(gd != NULL);
		mCtxMgr = gd->getCudaContextManager();
		PxScopedCudaLock _lock_(*mCtxMgr);

//CUDA module objects
#include "../cuda/include/ForceField.h"
	}
}

ForceFieldSceneGPU::~ForceFieldSceneGPU()
{
	CudaModuleScene::destroy(*mApexScene);
}

ForceFieldActorImpl* ForceFieldSceneGPU::createForceFieldActor(const ForceFieldActorDesc& desc, ForceFieldAssetImpl& asset, ResourceList& list)
{
	return PX_NEW(ForceFieldActorGPU)(desc, asset, list, *this);
}

ApexCudaConstStorage* ForceFieldSceneGPU::getFieldSamplerCudaConstStorage()
{
	return &APEX_CUDA_OBJ_NAME(fieldSamplerStorage);
}

bool ForceFieldSceneGPU::launchFieldSamplerCudaKernel(const nvidia::fieldsampler::FieldSamplerKernelLaunchDataIntl& launchData)
{
	LAUNCH_FIELD_SAMPLER_KERNEL(launchData);
}

#endif

}
} // end namespace nvidia
