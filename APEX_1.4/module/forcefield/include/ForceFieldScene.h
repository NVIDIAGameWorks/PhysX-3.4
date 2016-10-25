/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __FORCEFIELD_SCENE_H__
#define __FORCEFIELD_SCENE_H__

#include "Apex.h"

#include "ModuleForceFieldImpl.h"

#include "ApexResource.h"
#include "ApexContext.h"
#include "ApexSDKHelpers.h"

#include "RenderDebugInterface.h"
#include "ApexSDKIntl.h"
#include "ModuleIntl.h"

#include "DebugRenderParams.h"
#include "ForceFieldDebugRenderParams.h"

#include "PxTask.h"

#include "FieldSamplerSceneIntl.h"

#if APEX_CUDA_SUPPORT
#include "ApexCudaWrapper.h"
#include "ApexCuda.h"
#include "CudaModuleScene.h"

#include "../cuda/include/common.h"

#define SCENE_CUDA_OBJ(scene, name) static_cast<ForceFieldSceneGPU*>(scene)->APEX_CUDA_OBJ_NAME(name)
#define CUDA_OBJ(name) SCENE_CUDA_OBJ(mForceFieldScene, name)
#endif

namespace nvidia
{
namespace apex
{
class SceneIntl;
class DebugRenderParams;
class FieldSamplerManagerIntl;
}
namespace forcefield
{
class ModuleForceFieldImpl;
class ForceFieldActorImpl;
class ForceFieldActorDesc;


class ForceFieldScene : public FieldSamplerSceneIntl, public ApexContext, public ApexResourceInterface, public ApexResource
{
public:
	ForceFieldScene(ModuleForceFieldImpl& module, SceneIntl& scene, RenderDebugInterface* renderDebug, ResourceList& list);
	~ForceFieldScene();

	/* ModuleSceneIntl */
	void						updateActors(float deltaTime);
	void						submitTasks(float elapsedTime, float substepSize, uint32_t numSubSteps);
	void						setTaskDependencies();

	virtual void				visualize(void);
	virtual void				visualizeForceFieldForceFields(void);
	virtual void				visualizeForceFieldForces(void);
	virtual void				fetchResults(void);

	virtual void				setModulePhysXScene(PxScene* s);
	virtual PxScene*			getModulePhysXScene() const
	{
		return mPhysXScene;
	}

	virtual Module*			getModule()
	{
		return mModule;
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
	uint32_t						getListIndex(void) const
	{
		return m_listIndex;
	}
	void						setListIndex(ResourceList& list, uint32_t index)
	{
		m_listIndex = index;
		m_list = &list;
	}
	virtual void				release(void)
	{
		mModule->releaseModuleSceneIntl(*this);
	}

	virtual ForceFieldActorImpl*	createForceFieldActor(const ForceFieldActorDesc& desc, ForceFieldAssetImpl& asset, ResourceList& list) = 0;

	SceneIntl& getApexScene() const
	{
		return *mApexScene;
	}

	FieldSamplerManagerIntl*	getInternalFieldSamplerManager();

	/* FieldSamplerSceneIntl */
	virtual void getFieldSamplerSceneDesc(FieldSamplerSceneDescIntl& ) const
	{
	}

protected:
	void						destroy();

	ModuleForceFieldImpl* 			mModule;
	SceneIntl*                mApexScene;
	PxScene*                    mPhysXScene;

	RenderDebugInterface* 			mRenderDebug;

	DebugRenderParams*				mDebugRenderParams;
	ForceFieldDebugRenderParams*	mForceFieldDebugRenderParams;

	FieldSamplerManagerIntl*		mFieldSamplerManager;

private:
	class TaskUpdate : public PxTask
	{
	public:
		TaskUpdate(ForceFieldScene& owner) : mOwner(owner) {}
		const char* getName() const
		{
			return "ForceFieldScene::Update";
		}
		void run();
	protected:
		ForceFieldScene& mOwner;
	private:
		TaskUpdate& operator=(const  TaskUpdate&);
	};

	TaskUpdate					mUpdateTask;

	friend class ModuleForceFieldImpl;
	friend class ForceFieldActorImpl;
	friend class TaskUpdate;
};

class ForceFieldSceneCPU : public ForceFieldScene
{
public:
	ForceFieldSceneCPU(ModuleForceFieldImpl& module, SceneIntl& scene, RenderDebugInterface* renderDebug, ResourceList& list);
	~ForceFieldSceneCPU();

	ForceFieldActorImpl*	createForceFieldActor(const ForceFieldActorDesc& desc, ForceFieldAssetImpl& asset, ResourceList& list);

	/* FieldSamplerSceneIntl */

protected:
};

#if APEX_CUDA_SUPPORT
class ForceFieldSceneGPU : public ForceFieldScene, public CudaModuleScene
{
public:
	ForceFieldSceneGPU(ModuleForceFieldImpl& module, SceneIntl& scene, RenderDebugInterface* renderDebug, ResourceList& list);
	~ForceFieldSceneGPU();

	ForceFieldActorImpl*	createForceFieldActor(const ForceFieldActorDesc& desc, ForceFieldAssetImpl& asset, ResourceList& list);

//CUDA module objects
#include "../cuda/include/ForceField.h"

	/* FieldSamplerSceneIntl */
	virtual ApexCudaConstStorage*	getFieldSamplerCudaConstStorage();
	virtual bool					launchFieldSamplerCudaKernel(const nvidia::fieldsampler::FieldSamplerKernelLaunchDataIntl&);

protected:
	/* keep a convenience pointer to the cuda context manager */
	PxCudaContextManager* mCtxMgr;
};
#endif

}
} // end namespace nvidia

#endif
