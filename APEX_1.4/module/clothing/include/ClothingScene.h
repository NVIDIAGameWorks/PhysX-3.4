/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef CLOTHING_SCENE_H
#define CLOTHING_SCENE_H

#include "ApexContext.h"
#include "ModuleIntl.h"
#include "ApexResource.h"

#include "ClothStructs.h"

#include "PsSync.h"
#include "PxTask.h"
#include "PsTime.h"

#if APEX_UE4
#define CLOTHING_BEFORE_TICK_START_TASK_NAME "ClothingScene::ClothingBeforeTickStartTask"
#endif

#if APEX_CUDA_SUPPORT
namespace nvidia
{
class PhysXGpuIndicator;
}
#endif

namespace nvidia
{

namespace cloth
{
class Cloth;
class Factory;
class Solver;
}


namespace apex
{
class DebugRenderParams;
class RenderMeshAssetIntl;
class ApexSimpleString;
}

namespace clothing
{
class ModuleClothingImpl;
class ClothingAssetImpl;
class ClothingCookingTask;
class ClothingDebugRenderParams;

class ClothingRenderProxyImpl;


class ClothingScene : public ModuleSceneIntl, public ApexContext, public ApexResourceInterface, public ApexResource
{
public:
	ClothingScene(ModuleClothingImpl& module, SceneIntl& scene, RenderDebugInterface* renderDebug, ResourceList& list);
	~ClothingScene();

	/* ModuleSceneIntl */
	virtual void			simulate(float elapsedTime);
	virtual bool			needsManualSubstepping() const;
	virtual void			interStep(uint32_t substepNumber, uint32_t maxSubSteps);
	virtual void			submitTasks(float elapsedTime, float substepSize, uint32_t numSubSteps);
	virtual void			setTaskDependencies();
	virtual void			fetchResults();

#if PX_PHYSICS_VERSION_MAJOR == 3
	void					setModulePhysXScene(PxScene*);
	PxScene*				getModulePhysXScene() const
	{
		return mPhysXScene;
	}
#endif

	void					release();
	void					visualize();

	virtual Module*		getModule();

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
	uint32_t					getListIndex() const
	{
		return m_listIndex;
	}
	void					setListIndex(ResourceList& list, uint32_t index)
	{
		m_listIndex = index;
		m_list = &list;
	}

	bool					isSimulating() const;

	void					registerAsset(ClothingAssetImpl* asset);
	void					unregisterAsset(ClothingAssetImpl* asset);
	void					removeRenderProxies(ClothingAssetImpl* asset);
	
	uint32_t					submitCookingTask(ClothingCookingTask* newTask);

	float					getAverageSimulationFrequency()
	{
		return mAverageSimulationFrequency;
	}

	SceneIntl*			getApexScene()
	{
		return mApexScene;
	}

	const ClothingDebugRenderParams* getDebugRenderParams() const
	{
		return mClothingDebugRenderParams;
	}

	ClothFactory			getClothFactory(bool& useCuda);
	cloth::Solver*			getClothSolver(bool useCuda);

	void					lockScene();
	void					unlockScene();

	void					setSceneRunning(bool on);

	void					embeddedPostSim();

	// render proxy pool
	ClothingRenderProxyImpl*	getRenderProxy(RenderMeshAssetIntl* rma, bool useFallbackSkinning, bool useCustomVertexBuffer, const HashMap<uint32_t, ApexSimpleString>& overrideMaterials, const PxVec3* morphTargetNewPositions, const uint32_t* morphTargetVertexOffsets);
	void					tickRenderProxies();

protected:

	ModuleClothingImpl* 	mModule;
	SceneIntl* 				mApexScene;
#if PX_PHYSICS_VERSION_MAJOR == 3
	PxScene*				mPhysXScene;
#endif

	Array<ClothingAssetImpl*>	mClothingAssets;
	nvidia::Mutex			mClothingAssetsMutex;

	float					mSumBenefit;

	void					destroy();

	class ClothingBeforeTickStartTask : public PxTask
	{
	public:
		ClothingBeforeTickStartTask() : m_pScene(NULL)
		{
		}

		void setScene(ClothingScene* pScene)
		{
			m_pScene = pScene;
		}

		virtual void run();
		virtual const char* getName() const;

		ClothingScene* m_pScene;
	} mClothingBeforeTickStartTask;

private:

	class WaitForSolverTask*				mWaitForSolverTask;
	class ClothingSceneSimulateTask*		mSimulationTask;
	nvidia::Mutex							mSceneLock;
	int32_t									mSceneRunning;

	RenderDebugInterface*					mRenderDebug;

	DebugRenderParams*						mDebugRenderParams;
	ClothingDebugRenderParams*				mClothingDebugRenderParams;

	ClothingCookingTask*					mCurrentCookingTask;
	nvidia::Mutex							mCookingTaskMutex;

	Array<float>							mLastSimulationDeltas;
	uint32_t								mCurrentSimulationDelta;
	float									mAverageSimulationFrequency;

#ifndef _DEBUG
	// For statistics
	uint32_t								mFramesCount;
	float									mSimulatedTime;
	float									mTimestep;
#endif

	nvidia::Time							mClothingSimulationTime;

	ClothFactory							mCpuFactory;
#if APEX_CUDA_SUPPORT
	ClothFactory							mGpuFactory;
	PhysXGpuIndicator*						mPhysXGpuIndicator;
#endif

	Mutex									mRenderProxiesLock;
	nvidia::HashMap<RenderMeshAssetIntl*, nvidia::Array<ClothingRenderProxyImpl*> > mRenderProxies;

	friend class ModuleClothingImpl;
	friend class ClothingActorImpl;
};

}
} // namespace nvidia

#endif // CLOTHING_SCENE_H
