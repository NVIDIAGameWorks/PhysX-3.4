/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __EMITTER_SCENE_H__
#define __EMITTER_SCENE_H__

#include "Apex.h"

#include "ModuleEmitterImpl.h"
#include "ApexSDKIntl.h"
#include "ModuleIntl.h"
#include "ApexContext.h"
#include "ApexSDKHelpers.h"
#include "ApexActor.h"

#include "DebugRenderParams.h"
#include "EmitterDebugRenderParams.h"

#include "PxTask.h"

namespace nvidia
{
namespace apex
{
class SceneIntl;
}
namespace emitter
{

class ModuleEmitterImpl;


/* Each Emitter Actor should derive this class, so the scene can deal with it */
class EmitterActorBase : public ApexActor
{
public:
	virtual bool		isValid()
	{
		return mValid;
	}
	virtual void		tick() = 0;
	virtual void					visualize(RenderDebugInterface& renderDebug) = 0;

	virtual void		submitTasks() = 0;
	virtual void		setTaskDependencies() = 0;
	virtual void		fetchResults() = 0;

protected:
	EmitterActorBase() : mValid(false) {}

	bool mValid;
};

class EmitterScene : public ModuleSceneIntl, public ApexContext, public ApexResourceInterface, public ApexResource
{
public:
	EmitterScene(ModuleEmitterImpl& module, SceneIntl& scene, RenderDebugInterface* debugRender, ResourceList& list);
	~EmitterScene();

	/* ModuleSceneIntl */
	void				visualize();
	void				setModulePhysXScene(PxScene* s);
	PxScene*			getModulePhysXScene() const
	{
		return mPhysXScene;
	}
	PxScene*			mPhysXScene;

	Module*			getModule()
	{
		return mModule;
	}

	void				fetchResults();

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
	uint32_t		getListIndex() const
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

	void				submitTasks(float elapsedTime, float substepSize, uint32_t numSubSteps);
	void				setTaskDependencies();

protected:
	void                destroy();

	ModuleEmitterImpl*		mModule;
	SceneIntl*		mApexScene;

	float		mSumBenefit;
private:
	RenderDebugInterface* mDebugRender;

	DebugRenderParams*					mDebugRenderParams;
	EmitterDebugRenderParams*			mEmitterDebugRenderParams;

	friend class ModuleEmitterImpl;
	friend class EmitterActorImpl;
	friend class GroundEmitterActorImpl;
	friend class ImpactEmitterActorImpl;
};

}
} // end namespace nvidia

#endif
