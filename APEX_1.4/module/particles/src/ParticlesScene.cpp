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
#include "ApexUsingNamespace.h"

#include "Apex.h"
#include "ParticlesScene.h"
#include "SceneIntl.h"
#include "ModulePerfScope.h"
#include "ModuleParticlesImpl.h"
#include "PxScene.h"
#include "EffectPackageActorImpl.h"
#include "ApexEmitterAssetParameters.h"
#include "EmitterAsset.h"
#include "EmitterActor.h"
#include "PxProfiler.h"

#include "ScopedPhysXLock.h"
#include "Lock.h"

#pragma warning(disable: 4355) // 'this' : used in base member initializer list

namespace nvidia
{

namespace particles
{

ParticlesScene::ParticlesScene(ModuleParticlesImpl& module, SceneIntl& scene, RenderDebugInterface* renderDebug, ResourceList& list)
	: mRenderDebug(renderDebug)
	, mUpdateTask(*this)
	, mSimTime(0)
	, mCheckTime(0)
{
	mModule = &module;
	mApexScene = &scene;
	mPhysXScene = NULL;

	list.add(*this);		// Add self to module's list of ParticlesScenes

	/* Initialize reference to ParticlesDebugRenderParams */
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
			memberHandle.initParamRef(ParticlesDebugRenderParams::staticClassName(), true);
		}
	}

	/* Load reference to ParticlesDebugRenderParams */
	NvParameterized::Interface* refPtr = NULL;
	memberHandle.getParamRef(refPtr);
	mParticlesDebugRenderParams = DYNAMIC_CAST(ParticlesDebugRenderParams*)(refPtr);
	PX_ASSERT(mParticlesDebugRenderParams);
}

ParticlesScene::~ParticlesScene()
{
}

// Called by scene task graph between LOD and PhysX::simulate()
void ParticlesScene::TaskUpdate::run()
{
#if 1
	setProfileStat((uint16_t) mOwner.mActorArray.size());
	float dt = mOwner.mApexScene->getElapsedTime();
	mOwner.updateActors(dt);

#endif
}

void ParticlesScene::updateFromSimulate(float dt)
{
	PX_UNUSED(dt);
}

// Called by updateTask between LOD and PhysX simulate.  Any writes
// to render data must be protected by acquiring the actor's render data lock
void ParticlesScene::updateActors(float dt)
{
	PX_PROFILE_ZONE("ParticlesScene::updateActors", GetInternalApexSDK()->getContextId());

	//_ASSERTE (dt <= 1.0f /60.0f);
	SCOPED_PHYSX_LOCK_WRITE(mApexScene);
	for (uint32_t i = 0 ; i < mActorArray.size() ; i++)
	{
		ParticlesBase* db = static_cast< ParticlesBase*>(mActorArray[i]);
		switch (db->getParticlesType())
		{
			case ParticlesBase::DST_EFFECT_PACKAGE_ACTOR:
				{
					EffectPackageActorImpl* actor = static_cast<EffectPackageActorImpl*>(db);
					actor->updateParticles(dt);
				}
				break;
			default:
				PX_ASSERT(0);
		}
	}

}

// submit the task that updates the dynamicsystem actors
// called from ApexScene::simulate()
void ParticlesScene::submitTasks(float elapsedTime, float /*substepSize*/, uint32_t /*numSubSteps*/)
{
	PX_UNUSED(elapsedTime);
#if 1
	mSimTime += elapsedTime;
	mCheckTime += elapsedTime;
	PxTaskManager* tm;
	{
		READ_LOCK(*mApexScene);
		tm = mApexScene->getTaskManager();
	}
	tm->submitUnnamedTask(mUpdateTask);
	mUpdateTask.finishBefore(tm->getNamedTask(AST_PHYSX_SIMULATE));
#else
	updateActors(elapsedTime);
#endif
}

void ParticlesScene::fetchResults()
{

}

// Called by ApexScene::fetchResults() with all actors render data locked.
void ParticlesScene::fetchResultsPostRenderUnlock()
{
	PX_PROFILE_ZONE("ParticlesSceneFetchResults", GetInternalApexSDK()->getContextId());

	bool screenCulling = mModule->getEnableScreenCulling();
	bool znegative = mModule->getZnegative();

	for (uint32_t i = 0 ; i < mActorArray.size() ; i++)
	{
		ParticlesBase* db = static_cast< ParticlesBase*>(mActorArray[i]);

		switch (db->getParticlesType())
		{
			case ParticlesBase::DST_EFFECT_PACKAGE_ACTOR:
				{
					EffectPackageActorImpl* actor = static_cast< EffectPackageActorImpl*>(db);
					actor->updatePoseAndBounds(screenCulling, znegative);
				}
				break;
			default:
				PX_ASSERT(0);
		}
	}

	//

	if (!mEmitterPool.empty())
	{
		if (mCheckTime > 1)   // only check once every second
		{
			mCheckTime = 0;
			EmitterPool* source = &mEmitterPool[0];
			EmitterPool* dest = source;
			uint32_t incount = mEmitterPool.size();
			uint32_t outcount = 0;
			for (uint32_t i = 0; i < incount; i++)
			{
				bool alive = source->process(mSimTime);
				if (!alive)   // there must be at least one emitter sharing the same asset!
				{
					alive = true; // by default we cannot delete it unless there is at least once emitter in the pool sharing the same IOS
					EmitterPool* scan = &mEmitterPool[0];
					for (uint32_t i = 0; i < outcount; i++)
					{
						if (scan != source)
						{
							if (source->mEmitter->getEmitterAsset() == scan->mEmitter->getEmitterAsset())
							{
								alive = false;
								break;
							}
						}
						scan++;
					}
				}
				if (alive)
				{
					*dest = *source;
					dest++;
					outcount++;
				}
				else
				{
					source->releaseEmitter();
				}
				source++;
			}
			if (outcount != incount)
			{
				mEmitterPool.resize(outcount);
			}
		}
	}
	//
}

void ParticlesScene::visualize()
{
#ifndef WITHOUT_DEBUG_VISUALIZE
	if (!mParticlesDebugRenderParams->VISUALIZE_HEAT_SOURCE_ACTOR &&  !mParticlesDebugRenderParams->VISUALIZE_EFFECT_PACKAGE_ACTOR)
	{
		return;
	}
	const physx::PxMat44& savedPose = *RENDER_DEBUG_IFACE(mRenderDebug)->getPoseTyped();
	RENDER_DEBUG_IFACE(mRenderDebug)->setIdentityPose();
	RENDER_DEBUG_IFACE(mRenderDebug)->pushRenderState();
	for (uint32_t i = 0 ; i < mActorArray.size() ; i++)
	{
		ParticlesBase* db = static_cast< ParticlesBase*>(mActorArray[i]);
		switch (db->getParticlesType())
		{
		case ParticlesBase::DST_EFFECT_PACKAGE_ACTOR:
			if (mParticlesDebugRenderParams->VISUALIZE_EFFECT_PACKAGE_ACTOR)
			{
				EffectPackageActorImpl* hsa = static_cast< EffectPackageActorImpl*>(db);
				hsa->visualize(mRenderDebug, false);
			}
			break;
		default:
			PX_ASSERT(0);
		}
	}
	RENDER_DEBUG_IFACE(mRenderDebug)->setPose(savedPose);
	RENDER_DEBUG_IFACE(mRenderDebug)->popRenderState();
#endif
}

void ParticlesScene::destroy()
{
	removeAllActors();
	resetEmitterPool();
	mApexScene->moduleReleased(*this);
	delete this;
}

void ParticlesScene::setModulePhysXScene(PxScene* nxScene)
{
	mPhysXScene = nxScene;
}

void ParticlesScene::resetEmitterPool()
{
	for (uint32_t i = 0; i < mEmitterPool.size(); i++)
	{
		mEmitterPool[i].releaseEmitter();
	}
	mEmitterPool.clear();
}

void ParticlesScene::addToEmitterPool(EmitterActor* emitterActor)
{
	EmitterPool ep(emitterActor, mSimTime);
	emitterActor->stopEmit();
	mEmitterPool.pushBack(ep);
}

EmitterActor* ParticlesScene::getEmitterFromPool(EmitterAsset* asset)
{
	EmitterActor* ret = NULL;

	uint32_t ecount = mEmitterPool.size();
	if (ecount)
	{
		for (uint32_t i = 0; i < ecount; i++)
		{
			if (mEmitterPool[i].mEmitter->getEmitterAsset() == asset)
			{
				ret = mEmitterPool[i].mEmitter;
				for (uint32_t j = i + 1; j < ecount; j++)
				{
					mEmitterPool[j - 1] = mEmitterPool[j];
				}
				mEmitterPool.resize(ecount - 1);
				break;
			}
		}
	}
	return ret;
}

bool EmitterPool::process(float simTime)
{
	bool ret = true;
	if (simTime > mEmitterTime)
	{
		mEmitterTime = simTime + EMITTER_FORGET_TIME;
		ret = false;
	}
	return ret;
}

void EmitterPool::releaseEmitter()
{
	PX_ASSERT(mEmitter);
	if (mEmitter)
	{
		mEmitter->release();
		mEmitter = NULL;
	}
}

EmitterPool::EmitterPool(EmitterActor* emitterActor, float simTime)
{
	mEmitter = emitterActor;
	mEmitterTime = simTime + EMITTER_FORGET_TIME;
}

}
} // end namespace nvidia


