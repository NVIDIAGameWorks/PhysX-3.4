/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __FIELD_SAMPLER_SCENE_H__
#define __FIELD_SAMPLER_SCENE_H__

#include "Apex.h"

#include "ModuleFieldSamplerImpl.h"
#include "FieldSamplerSceneIntl.h"
#include "FieldSamplerQueryIntl.h"
#include "ApexSDKIntl.h"
#include "ModuleIntl.h"
#include "ApexContext.h"
#include "ApexSDKHelpers.h"
#include "PsArray.h"

#include "PxTask.h"

#if APEX_CUDA_SUPPORT
#include "ApexCudaWrapper.h"
#include "ApexCuda.h"
#include "CudaModuleScene.h"

#include "../cuda/include/common.h"

#define SCENE_CUDA_OBJ(scene, name) static_cast<FieldSamplerSceneGPU*>(scene)->APEX_CUDA_OBJ_NAME(name)
#endif


namespace nvidia
{
namespace apex
{
class SceneIntl;
}
namespace fieldsampler
{

class ModuleFieldSamplerImpl;
class FieldSamplerPhysXMonitor;
class FieldSamplerManager;
class FieldSamplerQuery;

class FieldSamplerScene : public ModuleSceneIntl, public ApexContext, public ApexResourceInterface, public ApexResource
{
public:
	FieldSamplerScene(ModuleFieldSamplerImpl& module, SceneIntl& scene, RenderDebugInterface* debugRender, ResourceList& list);
	~FieldSamplerScene();

	/* ModuleSceneIntl */
	void				visualize();

	PxScene*			getModulePhysXScene() const
	{
		return mPhysXScene;
	}
	void				setModulePhysXScene(PxScene*);
	PxScene* 			mPhysXScene;

	void				submitTasks(float elapsedTime, float substepSize, uint32_t numSubSteps);
	void				setTaskDependencies();
	void				fetchResults();

	virtual Module*	getModule()
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
	uint32_t				getListIndex() const
	{
		return m_listIndex;
	}
	void				setListIndex(ResourceList& list, uint32_t index)
	{
		m_listIndex = index;
		m_list = &list;
	}
	void				release()
	{
		mModule->releaseModuleSceneIntl(*this);
	}
	SceneIntl& getApexScene() const
	{
		return *mApexScene;
	}

	FieldSamplerManagerIntl* getManager();

	uint32_t createForceSampleBatch(uint32_t maxCount, const physx::PxFilterData filterData);
	void releaseForceSampleBatch(uint32_t batchId);
	void submitForceSampleBatch(	uint32_t batchId, PxVec4* forces, const uint32_t forcesStride,
									const PxVec3* positions, const uint32_t positionsStride,
									const PxVec3* velocities, const uint32_t velocitiesStride,
									const float* mass, const uint32_t massStride,
									const uint32_t* indices, const uint32_t numIndices);

	/* Toggle PhysX Monitor on/off */
	void enablePhysXMonitor(bool enable);

	void setPhysXFilterData(physx::PxFilterData filterData);

protected:
	void                destroy();

	virtual FieldSamplerManager* createManager() = 0;

	class TaskPhysXMonitorLoad : public PxTask
	{
	public:
		TaskPhysXMonitorLoad() {}
		const char* getName() const
		{
			return FSST_PHYSX_MONITOR_LOAD;
		}		
		void run() {/* void task */};
	};
	TaskPhysXMonitorLoad	mPhysXMonitorLoadTask;
	class TaskPhysXMonitorFetch : public PxTask
	{
	public:
		TaskPhysXMonitorFetch() {}
		const char* getName() const
		{
			return FSST_PHYSX_MONITOR_FETCH;
		}		
		void run() {/* void task */};
	};
	TaskPhysXMonitorFetch	mPhysXMonitorFetchTask;

	ModuleFieldSamplerImpl*		mModule;
	SceneIntl*			mApexScene;
	RenderDebugInterface*		mDebugRender;
	FieldSamplerPhysXMonitor* mPhysXMonitor;

	FieldSamplerManager*	mManager;

	uint32_t							mForceSampleBatchBufferSize;
	uint32_t							mForceSampleBatchBufferPos;
	Array <FieldSamplerQuery*>		mForceSampleBatchQuery;
	Array <FieldSamplerQueryDataIntl> mForceSampleBatchQueryData;
	Array <PxVec4>					mForceSampleBatchPosition;
	Array <PxVec4>					mForceSampleBatchVelocity;
	Array <float>					mForceSampleBatchMass;

	friend class ModuleFieldSamplerImpl;
	friend class FieldSamplerManager;
};

class FieldSamplerSceneCPU : public FieldSamplerScene
{
public:
	FieldSamplerSceneCPU(ModuleFieldSamplerImpl& module, SceneIntl& scene, RenderDebugInterface* debugRender, ResourceList& list);
	~FieldSamplerSceneCPU();

protected:
	virtual FieldSamplerManager* createManager();

};

#if APEX_CUDA_SUPPORT
class FieldSamplerSceneGPU : public FieldSamplerScene, public CudaModuleScene
{
public:
	FieldSamplerSceneGPU(ModuleFieldSamplerImpl& module, SceneIntl& scene, RenderDebugInterface* debugRender, ResourceList& list);
	~FieldSamplerSceneGPU();

	void* getHeadCudaObj()
	{
		return CudaModuleScene::getHeadCudaObj();
	}

//CUDA module objects
#include "../cuda/include/fieldsampler.h"

	PxCudaContextManager* getCudaContext() const
	{
		return mCtxMgr;
	}

protected:
	virtual FieldSamplerManager* createManager();

	/* keep a convenience pointer to the cuda context manager */
	PxCudaContextManager* mCtxMgr;
};
#endif

}
} // end namespace nvidia::apex

#endif
