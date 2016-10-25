/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __BASIC_FS_SCENE_H__
#define __BASIC_FS_SCENE_H__

#include "Apex.h"

#include "ModuleBasicFSImpl.h"
#include "ApexSDKIntl.h"
#include "ModuleIntl.h"
#include "ApexResource.h"
#include "ApexContext.h"
#include "ApexSDKHelpers.h"

#include "DebugRenderParams.h"
#include "BasicFSDebugRenderParams.h"

#include "PxTask.h"

#include "FieldSamplerSceneIntl.h"

#if APEX_CUDA_SUPPORT
#include "ApexCudaWrapper.h"
#include "ApexCuda.h"
#include "CudaModuleScene.h"

#include "../cuda/include/common.h"

#define SCENE_CUDA_OBJ(scene, name) static_cast<BasicFSSceneGPU*>(scene)->APEX_CUDA_OBJ_NAME(name)
#define CUDA_OBJ(name) SCENE_CUDA_OBJ(mScene, name)
#endif


namespace nvidia
{
namespace apex
{
class SceneIntl;
class FieldSamplerManagerIntl;
}
namespace basicfs
{

class ModuleBasicFSImpl;

class BasicFSAssetImpl;
class BasicFSActor;

class JetFSAsset;
class JetFSActorImpl;

class AttractorFSAsset;
class AttractorFSActorImpl;

class VortexFSAsset;
class VortexFSActorImpl;

class NoiseFSAsset;
class NoiseFSActorImpl;

class WindFSAsset;
class WindFSActorImpl;


class BasicFSScene : public FieldSamplerSceneIntl, public ApexContext, public ApexResourceInterface, public ApexResource
{
public:
	BasicFSScene(ModuleBasicFSImpl& module, SceneIntl& scene, RenderDebugInterface* debugRender, ResourceList& list);
	~BasicFSScene();

	/* ModuleSceneIntl */
	void						visualize();
	void						setModulePhysXScene(PxScene* s);
	PxScene*					getModulePhysXScene() const
	{
		return mPhysXScene;
	}

	void						submitTasks(float elapsedTime, float substepSize, uint32_t numSubSteps);
	void						fetchResults();

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
	uint32_t						getListIndex() const
	{
		return m_listIndex;
	}
	void						setListIndex(ResourceList& list, uint32_t index)
	{
		m_listIndex = index;
		m_list = &list;
	}
	void						release()
	{
		mModule->releaseModuleSceneIntl(*this);
	}

	virtual JetFSActorImpl*			createJetFSActor(const JetFSActorParams&, JetFSAsset&, ResourceList&) = 0;
	virtual AttractorFSActorImpl*	createAttractorFSActor(const AttractorFSActorParams&, AttractorFSAsset&, ResourceList&) = 0;
	virtual VortexFSActorImpl*		createVortexFSActor(const VortexFSActorParams&, VortexFSAsset&, ResourceList&) = 0;
	virtual NoiseFSActorImpl*		createNoiseFSActor(const NoiseFSActorParams&, NoiseFSAsset&, ResourceList&) = 0;
	virtual WindFSActorImpl*		createWindFSActor(const WindFSActorParams&, WindFSAsset&, ResourceList&) = 0;

	SceneIntl&				getApexScene() const
	{
		return *mApexScene;
	}

	FieldSamplerManagerIntl* 		getInternalFieldSamplerManager();

	/* FieldSamplerSceneIntl */
	virtual void				getFieldSamplerSceneDesc(FieldSamplerSceneDescIntl& desc) const
	{
		PX_UNUSED(desc);
	}

protected:
	void						destroy();

	ModuleBasicFSImpl*				mModule;
	SceneIntl*				mApexScene;
	PxScene*					mPhysXScene;
	RenderDebugInterface*			mDebugRender;

	DebugRenderParams*			mDebugRenderParams;
	BasicFSDebugRenderParams*	mBasicFSDebugRenderParams;

	FieldSamplerManagerIntl* 		mFieldSamplerManager;

	friend class ModuleBasicFSImpl;
	friend class JetFSActorImpl;
	friend class AttractorFSActorImpl;
	friend class VortexFSActorImpl;
	friend class NoiseFSActorImpl;
	friend class WindFSActorImpl;
};

class BasicFSSceneCPU : public BasicFSScene
{
public:
	BasicFSSceneCPU(ModuleBasicFSImpl& module, SceneIntl& scene, RenderDebugInterface* debugRender, ResourceList& list);
	~BasicFSSceneCPU();

	JetFSActorImpl*					createJetFSActor(const JetFSActorParams&, JetFSAsset&, ResourceList&);
	AttractorFSActorImpl*			createAttractorFSActor(const AttractorFSActorParams&, AttractorFSAsset&, ResourceList&);
	VortexFSActorImpl*				createVortexFSActor(const VortexFSActorParams&, VortexFSAsset&, ResourceList&);
	NoiseFSActorImpl*				createNoiseFSActor(const NoiseFSActorParams&, NoiseFSAsset&, ResourceList&);
	WindFSActorImpl*				createWindFSActor(const WindFSActorParams&, WindFSAsset&, ResourceList&);

	/* FieldSamplerSceneIntl */

protected:
};

#if APEX_CUDA_SUPPORT
class BasicFSSceneGPU : public BasicFSScene, public CudaModuleScene
{
public:
	BasicFSSceneGPU(ModuleBasicFSImpl& module, SceneIntl& scene, RenderDebugInterface* debugRender, ResourceList& list);
	~BasicFSSceneGPU();

	JetFSActorImpl*					createJetFSActor(const JetFSActorParams&, JetFSAsset&, ResourceList&);
	AttractorFSActorImpl*			createAttractorFSActor(const AttractorFSActorParams&, AttractorFSAsset&, ResourceList&);
	VortexFSActorImpl*				createVortexFSActor(const VortexFSActorParams&, VortexFSAsset&, ResourceList&);
	NoiseFSActorImpl*				createNoiseFSActor(const NoiseFSActorParams&, NoiseFSAsset&, ResourceList&);
	WindFSActorImpl*				createWindFSActor(const WindFSActorParams&, WindFSAsset&, ResourceList&);

	void*						getHeadCudaObj()
	{
		return CudaModuleScene::getHeadCudaObj();
	}

//CUDA module objects
#include "../cuda/include/basicfs.h"

	/* FieldSamplerSceneIntl */
	virtual ApexCudaConstStorage*	getFieldSamplerCudaConstStorage();
	virtual bool					launchFieldSamplerCudaKernel(const fieldsampler::FieldSamplerKernelLaunchDataIntl&);

protected:
	/* keep a convenience pointer to the cuda context manager */
	PxCudaContextManager* mCtxMgr;
};
#endif

}
} // end namespace nvidia::apex

#endif
