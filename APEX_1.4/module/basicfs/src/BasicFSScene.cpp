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
#include "BasicFSScene.h"
#include "JetFSActorImpl.h"
#include "AttractorFSActorImpl.h"
#include "VortexFSActorImpl.h"
#include "NoiseFSActorImpl.h"
#include "WindFSActorImpl.h"

#include "SceneIntl.h"
#include "RenderDebugInterface.h"
#include "ModulePerfScope.h"
#include "ModuleFieldSamplerIntl.h"

#if APEX_CUDA_SUPPORT
#include "ApexCudaSource.h"
#endif


namespace nvidia
{
namespace basicfs
{

BasicFSScene::BasicFSScene(ModuleBasicFSImpl& module, SceneIntl& scene, RenderDebugInterface* debugRender, ResourceList& list)
	: mModule(&module)
	, mApexScene(&scene)
	, mDebugRender(debugRender)
	, mFieldSamplerManager(0)
{
	list.add(*this);		// Add self to module's list of BasicFSScenes

	/* Initialize reference to JetFSDebugRenderParams */
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
			memberHandle.initParamRef(BasicFSDebugRenderParams::staticClassName(), true);
		}
	}

	/* Load reference to JetFSDebugRenderParams */
	NvParameterized::Interface* refPtr = NULL;
	memberHandle.getParamRef(refPtr);
	mBasicFSDebugRenderParams = DYNAMIC_CAST(BasicFSDebugRenderParams*)(refPtr);
	PX_ASSERT(mBasicFSDebugRenderParams);
}

BasicFSScene::~BasicFSScene()
{
}

void BasicFSScene::visualize() // Fix!
{
#ifndef WITHOUT_DEBUG_VISUALIZE
	if (!mBasicFSDebugRenderParams->VISUALIZE_JET_FS_ACTOR && !mBasicFSDebugRenderParams->VISUALIZE_ATTRACTOR_FS_ACTOR && !mBasicFSDebugRenderParams->VISUALIZE_VORTEX_FS_ACTOR && !mBasicFSDebugRenderParams->VISUALIZE_NOISE_FS_ACTOR)
	{
		return;
	}

	RENDER_DEBUG_IFACE(mDebugRender)->pushRenderState();
	// This is using the new debug rendering
	for (uint32_t i = 0 ; i < mActorArray.size() ; i++)
	{
		BasicFSActor* actor = DYNAMIC_CAST(BasicFSActor*)(mActorArray[i]); // Fix!
		actor->visualize();
	}
	RENDER_DEBUG_IFACE(mDebugRender)->popRenderState();
#endif
}

void BasicFSScene::destroy()
{
	removeAllActors();
	mApexScene->moduleReleased(*this);
	delete this;
}

void BasicFSScene::setModulePhysXScene(PxScene* nxScene)
{
	if (nxScene)
	{
		for (uint32_t i = 0 ; i < mActorArray.size() ; i++)
		{
			BasicFSActor* actor = DYNAMIC_CAST(BasicFSActor*)(mActorArray[i]);
			actor->setPhysXScene(nxScene);
		}
	}
	else
	{
		for (uint32_t i = 0 ; i < mActorArray.size() ; i++)
		{
			BasicFSActor* actor = DYNAMIC_CAST(BasicFSActor*)(mActorArray[i]);
			actor->setPhysXScene(NULL);
		}
	}

	mPhysXScene = nxScene;
}

void BasicFSScene::submitTasks(float elapsedTime, float /*substepSize*/, uint32_t /*numSubSteps*/)
{
	for (uint32_t i = 0; i < mActorArray.size(); ++i)
	{
		BasicFSActor* actor = DYNAMIC_CAST(BasicFSActor*)(mActorArray[i]);
		actor->simulate(elapsedTime);
	}
}


// Called by ApexScene::fetchResults() with all actors render data locked.
void BasicFSScene::fetchResults()
{
	PX_PROFILE_ZONE("BasicFSSceneFetchResults", GetInternalApexSDK()->getContextId());
}

FieldSamplerManagerIntl* BasicFSScene::getInternalFieldSamplerManager()
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


BasicFSSceneCPU::BasicFSSceneCPU(ModuleBasicFSImpl& module, SceneIntl& scene, RenderDebugInterface* debugRender, ResourceList& list) :
	BasicFSScene(module, scene, debugRender, list)
{
}

BasicFSSceneCPU::~BasicFSSceneCPU()
{
}

JetFSActorImpl*	BasicFSSceneCPU::createJetFSActor(const JetFSActorParams& params, JetFSAsset& asset, ResourceList& list)
{
	return PX_NEW(JetFSActorCPU)(params, asset, list, *this);
}

AttractorFSActorImpl* BasicFSSceneCPU::createAttractorFSActor(const AttractorFSActorParams& params, AttractorFSAsset& asset, ResourceList& list)
{
	return PX_NEW(AttractorFSActorCPU)(params, asset, list, *this);
}

VortexFSActorImpl*	BasicFSSceneCPU::createVortexFSActor(const VortexFSActorParams& params, VortexFSAsset& asset, ResourceList& list)
{
	return PX_NEW(VortexFSActorCPU)(params, asset, list, *this);
}

NoiseFSActorImpl* BasicFSSceneCPU::createNoiseFSActor(const NoiseFSActorParams& params, NoiseFSAsset& asset, ResourceList& list)
{
	return PX_NEW(NoiseFSActorCPU)(params, asset, list, *this);
}

WindFSActorImpl* BasicFSSceneCPU::createWindFSActor(const WindFSActorParams& params, WindFSAsset& asset, ResourceList& list)
{
	return PX_NEW(WindFSActorCPU)(params, asset, list, *this);
}

/******************************** GPU Version ********************************/

#if APEX_CUDA_SUPPORT


BasicFSSceneGPU::BasicFSSceneGPU(ModuleBasicFSImpl& module, SceneIntl& scene, RenderDebugInterface* debugRender, ResourceList& list)
	: BasicFSScene(module, scene, debugRender, list)
	, CudaModuleScene(scene, *mModule, APEX_CUDA_TO_STR(APEX_CUDA_MODULE_PREFIX))
{
	{
		PxGpuDispatcher* gd = mApexScene->getTaskManager()->getGpuDispatcher();
		PX_ASSERT(gd != NULL);
		mCtxMgr = gd->getCudaContextManager();
		PxScopedCudaLock _lock_(*mCtxMgr);

//CUDA module objects
#include "../cuda/include/basicfs.h"
	}
}

BasicFSSceneGPU::~BasicFSSceneGPU()
{
	CudaModuleScene::destroy(*mApexScene);
}

JetFSActorImpl*	BasicFSSceneGPU::createJetFSActor(const JetFSActorParams& params, JetFSAsset& asset, ResourceList& list)
{
	return PX_NEW(JetFSActorGPU)(params, asset, list, *this);
}

AttractorFSActorImpl* BasicFSSceneGPU::createAttractorFSActor(const AttractorFSActorParams& params, AttractorFSAsset& asset, ResourceList& list)
{
	return PX_NEW(AttractorFSActorGPU)(params, asset, list, *this);
}

VortexFSActorImpl*	BasicFSSceneGPU::createVortexFSActor(const VortexFSActorParams& params, VortexFSAsset& asset, ResourceList& list)
{
	return PX_NEW(VortexFSActorGPU)(params, asset, list, *this);
}

NoiseFSActorImpl* BasicFSSceneGPU::createNoiseFSActor(const NoiseFSActorParams& params, NoiseFSAsset& asset, ResourceList& list)
{
	return PX_NEW(NoiseFSActorGPU)(params, asset, list, *this);
}

WindFSActorImpl* BasicFSSceneGPU::createWindFSActor(const WindFSActorParams& params, WindFSAsset& asset, ResourceList& list)
{
	return PX_NEW(WindFSActorGPU)(params, asset, list, *this);
}

ApexCudaConstStorage* BasicFSSceneGPU::getFieldSamplerCudaConstStorage()
{
	return &APEX_CUDA_OBJ_NAME(fieldSamplerStorage);
}

bool BasicFSSceneGPU::launchFieldSamplerCudaKernel(const fieldsampler::FieldSamplerKernelLaunchDataIntl& launchData)
{
	LAUNCH_FIELD_SAMPLER_KERNEL(launchData);
}


#endif

}
} // end namespace nvidia::apex

