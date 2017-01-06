/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "Apex.h"
#include "ApexUsingNamespace.h"
#include "EmitterScene.h"
#include "SceneIntl.h"
#include "ModulePerfScope.h"

#include "Lock.h"

namespace nvidia
{
namespace emitter
{

EmitterScene::EmitterScene(ModuleEmitterImpl& module, SceneIntl& scene, RenderDebugInterface* debugRender, ResourceList& list) :
	mSumBenefit(0.0f),
	mDebugRender(debugRender)
{
	mModule = &module;
	mApexScene = &scene;
	list.add(*this);		// Add self to module's list of EmitterScenes

	/* Initialize reference to EmitterDebugRenderParams */
	{
		READ_LOCK(*mApexScene);
		mDebugRenderParams = DYNAMIC_CAST(DebugRenderParams*)(mApexScene->getDebugRenderParams());
	}
	PX_ASSERT(mDebugRenderParams);
	NvParameterized::Handle handle(*mDebugRenderParams), memberHandle(*mDebugRenderParams);
	int size;

	if (mDebugRenderParams->getParameterHandle("moduleName", handle) == NvParameterized::ERROR_NONE)
	{
		handle.getArraySize(size, 0);
		handle.resizeArray(size + 1);
		if (handle.getChildHandle(size, memberHandle) == NvParameterized::ERROR_NONE)
		{
			memberHandle.initParamRef(EmitterDebugRenderParams::staticClassName(), true);
		}
	}

	/* Load reference to EmitterDebugRenderParams */
	NvParameterized::Interface* refPtr = NULL;
	memberHandle.getParamRef(refPtr);
	mEmitterDebugRenderParams = DYNAMIC_CAST(EmitterDebugRenderParams*)(refPtr);
	PX_ASSERT(mEmitterDebugRenderParams);
}

EmitterScene::~EmitterScene()
{
}

void EmitterScene::visualize()
{
#ifndef WITHOUT_DEBUG_VISUALIZE
	for (uint32_t i = 0 ; i < mActorArray.size() ; i++)
	{
		EmitterActorBase* actor = DYNAMIC_CAST(EmitterActorBase*)(mActorArray[ i ]);
		actor->visualize(*mDebugRender);
	}
#endif
}

void EmitterScene::destroy()
{
	removeAllActors();
	mApexScene->moduleReleased(*this);
	delete this;
}

void EmitterScene::setModulePhysXScene(PxScene* scene)
{
	if (scene)
	{
		for (uint32_t i = 0 ; i < mActorArray.size() ; i++)
		{
			EmitterActorBase* actor = DYNAMIC_CAST(EmitterActorBase*)(mActorArray[ i ]);
			actor->setPhysXScene(scene);
		}
	}
	else
	{
		for (uint32_t i = 0 ; i < mActorArray.size() ; i++)
		{
			EmitterActorBase* actor = DYNAMIC_CAST(EmitterActorBase*)(mActorArray[ i ]);
			actor->setPhysXScene(NULL);
		}
	}

	mPhysXScene = scene;
}

void EmitterScene::submitTasks(float /*elapsedTime*/, float /*substepSize*/, uint32_t /*numSubSteps*/)
{
	for (uint32_t i = 0 ; i < mActorArray.size() ; i++)
	{
		EmitterActorBase* actor = DYNAMIC_CAST(EmitterActorBase*)(mActorArray[ i ]);
		actor->submitTasks();
	}
}

void EmitterScene::setTaskDependencies()
{
	for (uint32_t i = 0 ; i < mActorArray.size() ; i++)
	{
		EmitterActorBase* actor = DYNAMIC_CAST(EmitterActorBase*)(mActorArray[ i ]);
		actor->setTaskDependencies();
	}
}

// Called by ApexScene simulation thread after PhysX scene is stepped. All
// actors in the scene are render-locked.
void EmitterScene::fetchResults()
{
	PX_PROFILE_ZONE("EmitterSceneFetchResults", GetInternalApexSDK()->getContextId());

	for (uint32_t i = 0 ; i < mActorArray.size() ; i++)
	{
		EmitterActorBase* actor = DYNAMIC_CAST(EmitterActorBase*)(mActorArray[ i ]);
		actor->fetchResults();
	}
}

}
} // namespace nvidia::apex
