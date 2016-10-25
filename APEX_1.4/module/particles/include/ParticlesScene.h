/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __PARTICLES_SCENE_H__
#define __PARTICLES_SCENE_H__

#include "Apex.h"
#include "PairFilter.h"
#include "ModuleParticlesImpl.h"

#include "ApexResource.h"
#include "ApexContext.h"
#include "ApexSDKHelpers.h"

#include "RenderDebugInterface.h"
#include "ApexSDKIntl.h"
#include "ModuleIntl.h"

#include "DebugRenderParams.h"
#include "ParticlesDebugRenderParams.h"
#include "PxTask.h"

namespace nvidia
{

namespace apex
{
class SceneIntl;
}
namespace emitter
{
class EmitterActor;
class EmitterAsset;
}

namespace particles
{

class ModuleParticlesImpl;

#define EMITTER_FORGET_TIME 30

class EmitterPool
{
public:
	EmitterPool()
	{
		mEmitter = NULL;
		mEmitterTime = 0;
	}

	EmitterPool(EmitterActor* emitterActor, float simTime);

	~EmitterPool()
	{
	}

	bool process(float simTime);
	void releaseEmitter();

	EmitterActor* mEmitter;
	float				mEmitterTime;
};

typedef Array< EmitterPool > EmitterPoolVector;

class ParticlesScene : public ModuleSceneIntl, public ApexContext, public ApexResourceInterface, public ApexResource
{
public:


	ParticlesScene(ModuleParticlesImpl& module, SceneIntl& scene, RenderDebugInterface* renderDebug, ResourceList& list);
	~ParticlesScene();



	/* ModuleSceneIntl */
	void						updateActors(float deltaTime);
	void						submitTasks(float elapsedTime, float substepSize, uint32_t numSubSteps);
	virtual	void				updateFromSimulate(float dt);

	virtual void				visualize();
	virtual void				fetchResults();
	virtual void				fetchResultsPostRenderUnlock();

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
	uint32_t						getListIndex() const
	{
		return m_listIndex;
	}
	void						setListIndex(ResourceList& list, uint32_t index)
	{
		m_listIndex = index;
		m_list = &list;
	}
	virtual void				release()
	{
		mModule->releaseModuleSceneIntl(*this);
	}

	virtual SceneStats* 	getStats()
	{
		return NULL;
	}


	SceneIntl*                mApexScene;

	ModuleParticlesImpl* getModuleParticles() const
	{
		return mModule;
	}

	void addToEmitterPool(EmitterActor* emitterActor);
	EmitterActor* getEmitterFromPool(EmitterAsset* assert);

	void resetEmitterPool();

private:
	void						destroy();

	ModuleParticlesImpl* 				mModule;

	PxScene*                    mPhysXScene;
	RenderDebugInterface* 			mRenderDebug;

	DebugRenderParams*			mDebugRenderParams;
	ParticlesDebugRenderParams*	mParticlesDebugRenderParams;

	class TaskUpdate : public PxTask
	{
	public:
		TaskUpdate(ParticlesScene& owner) : mOwner(owner) {}
		const char* getName() const
		{
			return "ParticlesScene::Update";
		}
		void run();

	protected:
		ParticlesScene& mOwner;

	private:
		TaskUpdate& operator=(const TaskUpdate&);
	};

	TaskUpdate						mUpdateTask;
	friend class ModuleParticlesImpl;
	friend class ParticlesActor;
	friend class TaskUpdate;


	float								mCheckTime;
	float								mSimTime;
	EmitterPoolVector					mEmitterPool;
};

}
} // end namespace nvidia

#endif
