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

#include "FieldSamplerScene.h"
#include "FieldSamplerManager.h"
#include "FieldSamplerQuery.h"
#include "FieldSamplerPhysXMonitor.h"
#include "SceneIntl.h"
#include "RenderDebugInterface.h"
#include "ModulePerfScope.h"

#if APEX_CUDA_SUPPORT
#include "PxGpuTask.h"
#include "ApexCudaSource.h"
#endif

#include "Lock.h"

namespace nvidia
{
namespace fieldsampler
{

FieldSamplerScene::FieldSamplerScene(ModuleFieldSamplerImpl& module, SceneIntl& scene, RenderDebugInterface* debugRender, ResourceList& list)
	: mModule(&module)
	, mApexScene(&scene)
	, mDebugRender(debugRender)
	, mManager(NULL)
	, mPhysXScene(NULL)
	, mForceSampleBatchBufferPos(0)
	, mForceSampleBatchBufferSize(0)
{
	list.add(*this);		// Add self to module's list of FieldSamplerScenes
}

FieldSamplerScene::~FieldSamplerScene()
{
}

void FieldSamplerScene::visualize()
{
#ifndef WITHOUT_DEBUG_VISUALIZE
#endif
}

void FieldSamplerScene::destroy()
{
#if PX_PHYSICS_VERSION_MAJOR == 3
	PX_DELETE(mPhysXMonitor);
#endif
	PX_DELETE(mManager);

	removeAllActors();
	mApexScene->moduleReleased(*this);
	delete this;
}

FieldSamplerManagerIntl* FieldSamplerScene::getManager()
{
	if (mManager == NULL)
	{
		mManager = createManager();
		PX_ASSERT(mManager != NULL);
	}
	return mManager;
}



void FieldSamplerScene::setModulePhysXScene(PxScene* s)
{
	if (s)
	{
		mPhysXMonitor->setPhysXScene(s);
	}
	mPhysXScene = s;
}


void FieldSamplerScene::submitTasks(float /*elapsedTime*/, float /*substepSize*/, uint32_t /*numSubSteps*/)
{
#if PX_PHYSICS_VERSION_MAJOR == 3
	PxTaskManager* tm;
	{
		READ_LOCK(*mApexScene);
		tm = mApexScene->getTaskManager();
	}
	tm->submitNamedTask(&mPhysXMonitorFetchTask, FSST_PHYSX_MONITOR_FETCH);
	tm->submitNamedTask(&mPhysXMonitorLoadTask, FSST_PHYSX_MONITOR_LOAD);
#endif
	if (mManager != NULL)
	{
		mManager->submitTasks();
	}
}

void FieldSamplerScene::setTaskDependencies()
{
#if PX_PHYSICS_VERSION_MAJOR == 3
	for (uint32_t i = 0; i < mForceSampleBatchQuery.size(); i++)
	{
		if (mForceSampleBatchQuery[i] && mForceSampleBatchQueryData[i].count > 0)
		{
			static_cast<FieldSamplerQueryIntl*>(mForceSampleBatchQuery[i])->submitFieldSamplerQuery(mForceSampleBatchQueryData[i], mApexScene->getTaskManager()->getNamedTask(FSST_PHYSX_MONITOR_LOAD));
			mForceSampleBatchQueryData[i].count = 0;
		}
		mForceSampleBatchBufferPos = 0;
	}
	if (mPhysXMonitor->isEnable())
	{
		mPhysXMonitor->update();
	}
#endif
	if (mManager != NULL)
	{
		mManager->setTaskDependencies();
	}

#if PX_PHYSICS_VERSION_MAJOR == 3
	// Just in case one of the scene conditions doesn't set a bounding dependency, let's not let these dangle
	PxTaskManager* tm;
	{
		READ_LOCK(*mApexScene);
		tm = mApexScene->getTaskManager();
	}
	mPhysXMonitorFetchTask.finishBefore(tm->getNamedTask(AST_PHYSX_FETCH_RESULTS));
	mPhysXMonitorLoadTask.finishBefore(tm->getNamedTask(AST_PHYSX_FETCH_RESULTS));
#endif
}

void FieldSamplerScene::fetchResults()
{
	if (mManager != NULL)
	{
		mManager->fetchResults();
	}
}

#if PX_PHYSICS_VERSION_MAJOR == 3
void FieldSamplerScene::enablePhysXMonitor(bool enable)
{
	PX_UNUSED(enable);
	mPhysXMonitor->enablePhysXMonitor(enable);
}

void FieldSamplerScene::setPhysXFilterData(physx::PxFilterData filterData)
{
	mPhysXMonitor->setPhysXFilterData(filterData);
}


uint32_t FieldSamplerScene::createForceSampleBatch(uint32_t maxCount, const physx::PxFilterData filterData)
{
	mForceSampleBatchBufferSize += maxCount;
	mForceSampleBatchPosition.resize(mForceSampleBatchBufferSize);
	mForceSampleBatchVelocity.resize(mForceSampleBatchBufferSize);
	mForceSampleBatchMass.resize(mForceSampleBatchBufferSize);

	FieldSamplerQueryDescIntl desc;
	desc.maxCount = maxCount;
	desc.samplerFilterData = filterData;
	//SceneInfo* sceneInfo = DYNAMIC_CAST(SceneInfo*)(mSceneList.getResource(i));
	//InternalFieldSamplerScene* niFieldSamplerScene = sceneInfo->getSceneWrapper()->getInternalFieldSamplerScene();
	//desc.ownerFieldSamplerScene = this;
	

	uint32_t id = 0;
	while (id < mForceSampleBatchQuery.size() && mForceSampleBatchQuery[id]) 
	{
		id++;
	}
	if (id == mForceSampleBatchQuery.size())
	{
		mForceSampleBatchQuery.pushBack(0);
		FieldSamplerQueryDataIntl data;
		data.count = 0;
		mForceSampleBatchQueryData.pushBack(data);
	}
	mForceSampleBatchQuery[id] = static_cast<FieldSamplerQuery*>(mManager->createFieldSamplerQuery(desc));
	return id;
}


void FieldSamplerScene::releaseForceSampleBatch(uint32_t batchId)
{
	if (batchId < mForceSampleBatchQuery.size() && mForceSampleBatchQuery[batchId])
	{
		mForceSampleBatchBufferSize -= mForceSampleBatchQuery[batchId]->getQueryDesc().maxCount;
		mForceSampleBatchPosition.resize(mForceSampleBatchBufferSize);
		mForceSampleBatchVelocity.resize(mForceSampleBatchBufferSize);
		mForceSampleBatchMass.resize(mForceSampleBatchBufferSize);

		mForceSampleBatchQuery[batchId]->release();
		mForceSampleBatchQuery[batchId] = 0;
	}
}


void FieldSamplerScene::submitForceSampleBatch(	uint32_t batchId, PxVec4* forces, const uint32_t forcesStride,
								const PxVec3* positions, const uint32_t positionsStride,
								const PxVec3* velocities, const uint32_t velocitiesStride,
								const float* mass, const uint32_t massStride,
								const uint32_t* indices, const uint32_t numIndices)
{
	PX_UNUSED(forcesStride);
	PX_ASSERT(forcesStride == sizeof(PxVec4));
	PX_ASSERT(indices);
	if (batchId >= mForceSampleBatchQuery.size() || mForceSampleBatchQuery[batchId] == 0) return;

	uint32_t maxIndices = indices[numIndices - 1] + 1; //supposed that indices are sorted
	for (uint32_t i = 0; i < maxIndices; i++)
	{
		mForceSampleBatchPosition[mForceSampleBatchBufferPos + i] = *(PxVec4*)((uint8_t*)positions + i * positionsStride);
		mForceSampleBatchVelocity[mForceSampleBatchBufferPos + i] = *(PxVec4*)((uint8_t*)velocities + i * velocitiesStride);
		mForceSampleBatchMass[mForceSampleBatchBufferPos + i] = *(float*)((uint8_t*)mass + i * massStride);
	}

	FieldSamplerQueryDataIntl& data = mForceSampleBatchQueryData[batchId];
	data.count = numIndices;
	data.isDataOnDevice = false;
	data.positionStrideBytes = sizeof(PxVec4);
	data.velocityStrideBytes = sizeof(PxVec4);
	data.massStrideBytes = massStride ? sizeof(float) : 0;
	data.pmaInMass = (float*)&mForceSampleBatchMass[mForceSampleBatchBufferPos];
	data.pmaInPosition = (float*)&mForceSampleBatchPosition[mForceSampleBatchBufferPos];
	data.pmaInVelocity = (float*)&mForceSampleBatchVelocity[mForceSampleBatchBufferPos];
	data.pmaInIndices = (uint32_t*)indices;
	data.pmaOutField = forces;
	data.timeStep = getApexScene().getPhysXSimulateTime();

	mForceSampleBatchBufferPos += maxIndices;
}

#endif

/******************************** CPU Version ********************************/


FieldSamplerSceneCPU::FieldSamplerSceneCPU(ModuleFieldSamplerImpl& module, SceneIntl& scene, RenderDebugInterface* debugRender, ResourceList& list) :
	FieldSamplerScene(module, scene, debugRender, list)
{
#if PX_PHYSICS_VERSION_MAJOR == 3
	mPhysXMonitor = PX_NEW(FieldSamplerPhysXMonitor)(*this);
#endif
}

FieldSamplerSceneCPU::~FieldSamplerSceneCPU()
{
}

FieldSamplerManager* FieldSamplerSceneCPU::createManager()
{
	return PX_NEW(FieldSamplerManagerCPU)(this);
}

/******************************** GPU Version ********************************/

#if APEX_CUDA_SUPPORT

FieldSamplerSceneGPU::FieldSamplerSceneGPU(ModuleFieldSamplerImpl& module, SceneIntl& scene, RenderDebugInterface* debugRender, ResourceList& list)
	: FieldSamplerScene(module, scene, debugRender, list)
	, CudaModuleScene(scene, *mModule, APEX_CUDA_TO_STR(APEX_CUDA_MODULE_PREFIX))
{
#if PX_PHYSICS_VERSION_MAJOR == 3
	mPhysXMonitor = PX_NEW(FieldSamplerPhysXMonitor)(*this);
#endif
	{
		PxGpuDispatcher* gd = mApexScene->getTaskManager()->getGpuDispatcher();
		PX_ASSERT(gd != NULL);
		mCtxMgr = gd->getCudaContextManager();
		PxScopedCudaLock _lock_(*mCtxMgr);

//CUDA module objects
#include "../cuda/include/fieldsampler.h"
	}
}

FieldSamplerSceneGPU::~FieldSamplerSceneGPU()
{
	CudaModuleScene::destroy(*mApexScene);
}

FieldSamplerManager* FieldSamplerSceneGPU::createManager()
{
	return PX_NEW(FieldSamplerManagerGPU)(this);
}

#endif

}
} // end namespace nvidia::apex


