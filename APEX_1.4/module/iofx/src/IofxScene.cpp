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
#include "ApexUsingNamespace.h"
#include "ModuleIofxImpl.h"
#include "IofxSceneCPU.h"
#include "IofxSceneGPU.h"
#include "IofxAssetImpl.h"
#include "IofxActorImpl.h"
#include "IofxActorCPU.h"
#include "IofxDebugRenderParams.h"
#include "DebugRenderParams.h"
#include "RenderVolumeImpl.h"
#include "SceneIntl.h"
#include "ModulePerfScope.h"
#include "RenderDebugInterface.h"

#if APEX_CUDA_SUPPORT
#include <cuda.h>
#include "ApexCutil.h"
#include "IofxActorGPU.h"

#include "ApexCudaSource.h"
#endif

#include "Lock.h"

namespace nvidia
{
namespace iofx
{

IofxRenderable* IofxRenderableIteratorImpl::findNextInActorList()
{
	for (; mActorIndex < mActorCount; ++mActorIndex)
	{
		IofxRenderable* result = mActorList[mActorIndex]->acquireRenderableReference();
		if (result != NULL)
		{
			return result;
		}
	}
	mIofxScene.mLiveRenderVolumes[mVolumeIndex]->unlockIofxActorList();
	return NULL;
}

IofxRenderable* IofxRenderableIteratorImpl::findFirstInNotEmptyVolume()
{
	for (; mVolumeIndex < mVolumeCount; ++mVolumeIndex)
	{
		mActorList = mIofxScene.mLiveRenderVolumes[mVolumeIndex]->lockIofxActorList(mActorCount);
		mActorIndex = 0;
		IofxRenderable* result = findNextInActorList();
		if (result != NULL)
		{
			return result;
		}
	}
	mIofxScene.mLiveRenderVolumesLock.unlockReader();
	return NULL;
}

IofxRenderable* IofxRenderableIteratorImpl::getFirst()
{
	mIofxScene.mLiveRenderVolumesLock.lockReader();
	mVolumeCount = mIofxScene.mLiveRenderVolumes.size();
	mVolumeIndex = 0;
	return findFirstInNotEmptyVolume();
}

IofxRenderable* IofxRenderableIteratorImpl::getNext()
{
	if (mVolumeIndex < mVolumeCount)
	{
		PX_ASSERT(mActorIndex < mActorCount);
		++mActorIndex;
		IofxRenderable* result = findNextInActorList();
		if (result != NULL)
		{
			return result;
		}
		++mVolumeIndex;
		return findFirstInNotEmptyVolume();
	}
	return NULL;
}


static StatsInfo IOFXStatsData[] =
{
	{"IOFX: SimulatedSpriteParticlesCount",	StatDataType::INT,   {{0}} },
	{"IOFX: SimulatedMeshParticlesCount",	StatDataType::INT,   {{0}} }
};

IofxScene::IofxScene(ModuleIofxImpl& module, SceneIntl& scene, RenderDebugInterface* debugRender, ResourceList& list)
	: mModule(&module)
	, mApexScene(&scene)
	, mDebugRender(debugRender)
	, mPrevTotalSimulatedSpriteParticles(0)
	, mPrevTotalSimulatedMeshParticles(0)
	, mRenderCallback(NULL)
{
	list.add(*this);		// Add self to module's list of IofxScenes

	/* Initialize reference to IofxDebugRenderParams */
	{
		WRITE_LOCK(*mApexScene);
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
			memberHandle.initParamRef(IofxDebugRenderParams::staticClassName(), true);
		}
	}

	/* Load reference to IofxDebugRenderParams */
	NvParameterized::Interface* refPtr = NULL;
	memberHandle.getParamRef(refPtr);
	mIofxDebugRenderParams = DYNAMIC_CAST(IofxDebugRenderParams*)(refPtr);
	PX_ASSERT(mIofxDebugRenderParams);

	createModuleStats();
}

IofxScene::~IofxScene()
{
	destroyModuleStats();
}

Module* IofxScene::getModule()
{
	return mModule;
}

void IofxScene::release()
{
	mModule->releaseModuleSceneIntl(*this);
}

IofxManager* IofxScene::createIofxManager(const IofxAsset& asset, const IofxManagerDescIntl& desc)
{
	mManagersLock.lockWriter();

	IofxManager* iofxManager = PX_NEW(IofxManager)(*this, desc, asset.getMeshAssetCount() > 0);

	mManagersLock.unlockWriter();

	return iofxManager;
}

void IofxScene::releaseIofxManager(IofxManager* manager)
{
	mManagersLock.lockWriter();

	manager->destroy();

	mManagersLock.unlockWriter();
}

void IofxScene::createModuleStats(void)
{
	mModuleSceneStats.numApexStats		= NumberOfStats;
	mModuleSceneStats.ApexStatsInfoPtr	= (StatsInfo*)PX_ALLOC(sizeof(StatsInfo) * NumberOfStats, PX_DEBUG_EXP("StatsInfo"));

	for (uint32_t i = 0; i < NumberOfStats; i++)
	{
		mModuleSceneStats.ApexStatsInfoPtr[i] = IOFXStatsData[i];
	}
}

void IofxScene::destroyModuleStats(void)
{
	mModuleSceneStats.numApexStats = 0;
	if (mModuleSceneStats.ApexStatsInfoPtr)
	{
		PX_FREE_AND_RESET(mModuleSceneStats.ApexStatsInfoPtr);
	}
}

void IofxScene::setStatValue(StatsDataEnum index, StatValue dataVal)
{
	if (mModuleSceneStats.ApexStatsInfoPtr)
	{
		mModuleSceneStats.ApexStatsInfoPtr[index].StatCurrentValue = dataVal;
	}
}

void IofxScene::visualize()
{
#ifndef WITHOUT_DEBUG_VISUALIZE
	if (!mIofxDebugRenderParams->VISUALIZE_IOFX_ACTOR)
	{
		return;
	}

	PxMat44 cameraFacingPose((mApexScene->getViewMatrix(0)).inverseRT());

	const physx::PxMat44& savedPose = *RENDER_DEBUG_IFACE(mDebugRender)->getPoseTyped();
	RENDER_DEBUG_IFACE(mDebugRender)->setIdentityPose();
	RENDER_DEBUG_IFACE(mDebugRender)->pushRenderState();
	RENDER_DEBUG_IFACE(mDebugRender)->setCurrentTextScale(3.0f);

	// iofx current bounding volume
	// there is a bug with setting the color. it can change under certain circumstances. use default color for now
	//RENDER_DEBUG_IFACE(mDebugRender)->setCurrentColor(RENDER_DEBUG_IFACE(mDebugRender)->getDebugColor(DebugColors::LightBlue));
	for (uint32_t i = 0 ; i < mActorArray.size() ; i++)
	{
		IofxActorImpl* actor = DYNAMIC_CAST(IofxActorImpl*)(mActorArray[ i ]);
		if (mIofxDebugRenderParams->VISUALIZE_IOFX_ACTOR_NAME)
		{
			PxVec3 textLocation = actor->getBounds().maximum;
			cameraFacingPose.setPosition(textLocation);
			if(actor->getOwner() != NULL)
			{
				RENDER_DEBUG_IFACE(mDebugRender)->debugText(textLocation, " %s %s", actor->getOwner()->getObjTypeName(), actor->getOwner()->getName());
			}
		}
		if (mIofxDebugRenderParams->VISUALIZE_IOFX_BOUNDING_BOX)
		{
			RENDER_DEBUG_IFACE(mDebugRender)->debugBound(actor->getBounds());
		}
	}

	// iofx max bounding volume
	// there is a bug with setting the color. it can change under certain circumstances. use default color for now
	//RENDER_DEBUG_IFACE(mDebugRender)->setCurrentColor(RENDER_DEBUG_IFACE(mDebugRender)->getDebugColor(DebugColors::Orange));
	mLiveRenderVolumesLock.lockReader();
	for (uint32_t i = 0 ; i < mLiveRenderVolumes.size() ; i++)
	{
		if (mIofxDebugRenderParams->VISUALIZE_IOFX_BOUNDING_BOX)
		{
			PxVec3 textLocation = mLiveRenderVolumes[i]->getOwnershipBounds().maximum;
			cameraFacingPose.setPosition(textLocation);
			RENDER_DEBUG_IFACE(mDebugRender)->debugText(textLocation, " Max Render Volume %d", i);
			RENDER_DEBUG_IFACE(mDebugRender)->debugBound(mLiveRenderVolumes[i]->getOwnershipBounds());
		}
	}
	mLiveRenderVolumesLock.unlockReader();

	RENDER_DEBUG_IFACE(mDebugRender)->setPose(savedPose);
	RENDER_DEBUG_IFACE(mDebugRender)->popRenderState();
#endif
}

void IofxScene::destroy()
{
	removeAllActors();
	mApexScene->moduleReleased(*this);

	{
		mLiveRenderVolumesLock.lockWriter();

		/* Handle deferred insertions/deletions of ApexRenderVolumes */
		processDeferredRenderVolumes();

		/* Delete all Live ApexRenderVolumes */
		for (uint32_t i = 0 ; i < mLiveRenderVolumes.size() ; i++)
		{
			RenderVolumeImpl* arv = mLiveRenderVolumes[ i ];
			PX_DELETE(arv);
		}
		mLiveRenderVolumes.clear();

		mLiveRenderVolumesLock.unlockWriter();
	}
	delete this;
}

void IofxScene::setModulePhysXScene(PxScene* nxScene)
{
	if (nxScene)
	{
		for (uint32_t i = 0 ; i < mActorArray.size() ; i++)
		{
			IofxActorImpl* actor = DYNAMIC_CAST(IofxActorImpl*)(mActorArray[ i ]);
			actor->setPhysXScene(nxScene);
		}
	}
	else
	{
		for (uint32_t i = 0 ; i < mActorArray.size() ; i++)
		{
			IofxActorImpl* actor = DYNAMIC_CAST(IofxActorImpl*)(mActorArray[ i ]);
			actor->setPhysXScene(NULL);
		}
	}

	mPhysXScene = nxScene;
}

void IofxScene::processDeferredRenderVolumes()
{
	mAddedRenderVolumesLock.lock();
	while (mAddedRenderVolumes.size())
	{
		RenderVolumeImpl* arv = mAddedRenderVolumes.popBack();
		mLiveRenderVolumes.pushBack(arv);
	}
	mAddedRenderVolumesLock.unlock();

	mDeletedRenderVolumesLock.lock();
	while (mDeletedRenderVolumes.size())
	{
		RenderVolumeImpl* arv = mDeletedRenderVolumes.popBack();
		mLiveRenderVolumes.findAndReplaceWithLast(arv);
		PX_DELETE(arv);
	}
	mDeletedRenderVolumesLock.unlock();
}

void IofxScene::submitTasks(float /*elapsedTime*/, float /*substepSize*/, uint32_t /*numSubSteps*/)
{
	{
		mLiveRenderVolumesLock.lockWriter();

		/* Handle deferred insertions/deletions of ApexRenderVolumes */
		processDeferredRenderVolumes();

		mLiveRenderVolumesLock.unlockWriter();
	}

	mManagersLock.lockReader();

	//IofxManager::submitTasks reads mLiveRenderVolumes so we lock it here and unlock later
	mLiveRenderVolumesLock.lockReader();
	for (uint32_t i = 0; i < mActorManagers.getSize(); ++i)
	{
		IofxManager* mgr = DYNAMIC_CAST(IofxManager*)(mActorManagers.getResource(i));
		mgr->submitTasks();
	}
	mLiveRenderVolumesLock.unlockReader();

	mManagersLock.unlockReader();
}

void IofxScene::fetchResults()
{
	mManagersLock.lockReader();

	mFetchResultsLock.lock();

	uint32_t totalSimulatedSpriteParticles = 0,
				totalSimulatedMeshParticles = 0;
	for (uint32_t i = 0; i < mActorManagers.getSize(); ++i)
	{
		IofxManager* mgr = DYNAMIC_CAST(IofxManager*)(mActorManagers.getResource(i));
		mgr->fetchResults();

		if(mgr->isMesh())
			totalSimulatedMeshParticles += mgr->getSimulatedParticlesCount();
		else
			totalSimulatedSpriteParticles += mgr->getSimulatedParticlesCount();
	}

	StatValue dataVal;
	dataVal.Int = int32_t(totalSimulatedSpriteParticles - mPrevTotalSimulatedSpriteParticles);
	setStatValue(SimulatedSpriteParticlesCount, dataVal);
	mPrevTotalSimulatedSpriteParticles = totalSimulatedSpriteParticles;
	dataVal.Int = int32_t(totalSimulatedMeshParticles - mPrevTotalSimulatedMeshParticles);
	setStatValue(SimulatedMeshParticlesCount, dataVal);
	mPrevTotalSimulatedMeshParticles = totalSimulatedMeshParticles;

	mFetchResultsLock.unlock();

	mManagersLock.unlockReader();
}

void IofxScene::prepareRenderables()
{
	mManagersLock.lockReader();

	lockLiveRenderVolumes();
	mFetchResultsLock.lock();

	for (uint32_t i = 0; i < mActorManagers.getSize(); ++i)
	{
		IofxManager* mgr = DYNAMIC_CAST(IofxManager*)(mActorManagers.getResource(i));
		mgr->prepareRenderables();
	}

	mFetchResultsLock.unlock();
	unlockLiveRenderVolumes();

	mManagersLock.unlockReader();
}


void IofxScene::lockLiveRenderVolumes()
{
	mLiveRenderVolumesLock.lockReader();
	//for (uint32_t i = 0 ; i < mLiveRenderVolumes.size() ; i++)
	//{
	//	mLiveRenderVolumes[ i ]->lockReader();
	//}
}

void IofxScene::unlockLiveRenderVolumes()
{
	//for (uint32_t i = 0 ; i < mLiveRenderVolumes.size() ; i++)
	//{
	//	mLiveRenderVolumes[ i ]->unlockReader();
	//}
	mLiveRenderVolumesLock.unlockReader();
}

/******************************** CPU Version ********************************/

IofxSceneCPU::IofxSceneCPU(ModuleIofxImpl& module, SceneIntl& scene, RenderDebugInterface* debugRender, ResourceList& list) 
	: IOFX_SCENE(module, scene, debugRender, list)
{
}

/******************************** GPU Version ********************************/

#if APEX_CUDA_SUPPORT
IofxSceneGPU::IofxSceneGPU(ModuleIofxImpl& module, SceneIntl& scene, RenderDebugInterface* debugRender, ResourceList& list) :
	IOFX_SCENE(module, scene, debugRender, list),
	CudaModuleScene(scene, *mModule, APEX_CUDA_TO_STR(APEX_CUDA_MODULE_PREFIX)),
	mContextManager(scene.getTaskManager()->getGpuDispatcher()->getCudaContextManager())
{
	PxGpuDispatcher* gd = scene.getTaskManager()->getGpuDispatcher();
	PX_ASSERT(gd != NULL);
	PxScopedCudaLock _lock_(*gd->getCudaContextManager());

//CUDA module objects
#include "../cuda/include/moduleList.h"
}

IofxSceneGPU::~IofxSceneGPU()
{
	PxScopedCudaLock s(*mContextManager);
	CudaModuleScene::destroy(*mApexScene);
}

void IofxSceneGPU::submitTasks(float elapsedTime, float substepSize, uint32_t numSubSteps)
{
	IofxScene::submitTasks(elapsedTime, substepSize, numSubSteps);

	{
		PxScopedCudaLock _lock_(*mContextManager);

		APEX_CUDA_OBJ_NAME(migrationStorage).copyToDevice(mContextManager, 0);
		APEX_CUDA_OBJ_NAME(remapStorage).copyToDevice(mContextManager, 0);
		APEX_CUDA_OBJ_NAME(modifierStorage).copyToDevice(mContextManager, 0);
	}
}


void IofxSceneGPU::prepareRenderables()
{
	mManagersLock.lockReader();

	lockLiveRenderVolumes();
	mFetchResultsLock.lock();

	for (uint32_t i = 0; i < mActorManagers.getSize(); ++i)
	{
		IofxManager* mgr = DYNAMIC_CAST(IofxManager*)(mActorManagers.getResource(i));
		mgr->prepareRenderables();
	}

	if (mContextManager->getInteropMode() != PxCudaInteropMode::NO_INTEROP)
	{
		mToMapArray.clear();
		mToUnmapArray.clear();

		for (uint32_t i = 0; i < mActorManagers.getSize(); ++i)
		{
			IofxManager* mgr = DYNAMIC_CAST(IofxManager*)(mActorManagers.getResource(i));
			mgr->fillMapUnmapArraysForInterop(mToMapArray, mToUnmapArray);
		}

		bool mapSuccess = true;
		bool unmapSuccess = true;
		{
			PxScopedCudaLock s(*mContextManager);

			if (!mToMapArray.empty())
			{
				for (uint32_t i = 0; i < mToMapArray.size(); ++i)
				{
					cuGraphicsResourceSetMapFlags( mToMapArray[i], CU_GRAPHICS_MAP_RESOURCE_FLAGS_WRITE_DISCARD );
				}
				CUresult res = cuGraphicsMapResources(mToMapArray.size(), &mToMapArray.front(), 0);
				mapSuccess = (res == CUDA_SUCCESS || res == CUDA_ERROR_ALREADY_MAPPED);
			}
			if (!mToUnmapArray.empty())
			{
				CUresult res = cuGraphicsUnmapResources(mToUnmapArray.size(), &mToUnmapArray.front(), 0);
				unmapSuccess = (res == CUDA_SUCCESS || res == CUDA_ERROR_NOT_MAPPED);
			}
		}

		for (uint32_t i = 0; i < mActorManagers.getSize(); ++i)
		{
			IofxManager* mgr = DYNAMIC_CAST(IofxManager*)(mActorManagers.getResource(i));
			mgr->mapBufferResultsForInterop(mapSuccess, unmapSuccess);
		}
	}

	mFetchResultsLock.unlock();
	unlockLiveRenderVolumes();

	mManagersLock.unlockReader();
}

#endif

}
} // namespace nvidia
