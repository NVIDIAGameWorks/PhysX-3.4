/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef SCENE_INTL_H
#define SCENE_INTL_H

#define APEX_CHECK_STAT_TIMER(name)// { PX_PROFILE_ZONE(name, GetInternalApexSDK()->getContextId()); }

#include "Scene.h"
#include "ApexUsingNamespace.h"

#if PX_PHYSICS_VERSION_MAJOR == 3
// PH prevent PxScene.h from including PxPhysX.h, it will include sooo many files that it will break the clothing embedded branch
#define PX_PHYSICS_NX_PHYSICS
#include "PxScene.h"
#undef PX_PHYSICS_NX_PHYSICS
#endif

namespace nvidia
{
namespace apex
{

class ModuleSceneIntl;
class ApexContext;
class RenderDebugInterface;
class PhysX3Interface;
class ApexCudaTestManager;

/**
 * Framework interface to ApexScenes for use by modules
 */
class SceneIntl : public Scene
{
public:
	/**
	When a module has been released by the end-user, the module must release
	its ModuleScenesIntl and notify those Scenes that their module
	scenes no longer exist
	*/
	virtual void moduleReleased(ModuleSceneIntl& moduleScene) = 0;

#if PX_PHYSICS_VERSION_MAJOR == 3
	virtual void lockRead(const char *fileName,uint32_t lineo) = 0;
	virtual void lockWrite(const char *fileName,uint32_t lineno) = 0;
	virtual void unlockRead() = 0;
	virtual void unlockWrite() = 0;

	virtual void addModuleUserNotifier(physx::PxSimulationEventCallback& notify) = 0;
	virtual void removeModuleUserNotifier(physx::PxSimulationEventCallback& notify) = 0;
	virtual void addModuleUserContactModify(physx::PxContactModifyCallback& notify) = 0;
	virtual void removeModuleUserContactModify(physx::PxContactModifyCallback& notify) = 0;
	virtual PhysX3Interface* getApexPhysX3Interface()	const = 0;
#endif

	virtual ApexContext* getApexContext() = 0;
	virtual float getElapsedTime() const = 0;

	/* Get total elapsed simulation time, in integer milliseconds */
	virtual uint32_t getTotalElapsedMS() const = 0;

	virtual bool isSimulating() const = 0;
	virtual bool physXElapsedTime(float& dt) const = 0;

	virtual float getPhysXSimulateTime() const = 0;

	virtual bool isFinalStep() const = 0;

	virtual uint32_t getSeed() = 0; // Not necessarily const

	enum ApexStatsDataEnum
	{
		NumberOfActors,
		NumberOfShapes,
		NumberOfAwakeShapes,
		NumberOfCpuShapePairs,
		ApexBeforeTickTime,
		ApexDuringTickTime,
		ApexPostTickTime,
		PhysXSimulationTime,
		ClothingSimulationTime,
		ParticleSimulationTime,
		TurbulenceSimulationTime,
		PhysXFetchResultTime,
		UserDelayedFetchTime,
		RbThroughput,
		SimulatedSpriteParticlesCount,
		SimulatedMeshParticlesCount,
		VisibleDestructibleChunkCount,
		DynamicDestructibleChunkIslandCount,

		// insert new items before this line
		NumberOfApexStats	// The number of stats
	};

	virtual void setApexStatValue(int32_t index, StatValue dataVal) = 0;

#if APEX_CUDA_SUPPORT
	virtual ApexCudaTestManager& getApexCudaTestManager() = 0;
	virtual bool isUsingCuda() const = 0;
#endif
	virtual ModuleSceneIntl* getInternalModuleScene(const char* moduleName) = 0;
};

/* ApexScene task names */
#define APEX_DURING_TICK_TIMING_FIX 1

#define AST_PHYSX_SIMULATE				"ApexScene::PhysXSimulate"
#define AST_PHYSX_BETWEEN_STEPS			"ApexScene::PhysXBetweenSteps"

#if APEX_DURING_TICK_TIMING_FIX
#	define AST_DURING_TICK_COMPLETE		"ApexScene::DuringTickComplete"
#endif

#define AST_PHYSX_CHECK_RESULTS			"ApexScene::CheckResults"
#define AST_PHYSX_FETCH_RESULTS			"ApexScene::FetchResults"




}
} // end namespace nvidia::apex


#endif // SCENE_INTL_H
