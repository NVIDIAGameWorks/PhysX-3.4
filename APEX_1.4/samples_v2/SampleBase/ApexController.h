/*
* Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#ifndef APEX_CONTROLLER_H
#define APEX_CONTROLLER_H

#include "SampleManager.h"
#include <DirectXMath.h>

#include "ApexCudaContextManager.h"
#include "ModuleClothing.h"
#include "ClothingActor.h"
#include "ClothingAsset.h"
#include "ClothingRenderProxy.h"
#include "DestructibleAsset.h"
#include "DestructibleActor.h"
#include "DestructibleRenderable.h"
#include "ModuleParticles.h"
#include "ModuleIofx.h"
#include "IofxActor.h"
#include "RenderVolume.h"
#include "EffectPackageActor.h"
#include "EffectPackageAsset.h"
#include "nvparameterized/NvParameterized.h"
#include "nvparameterized/NvParamUtils.h"
#include "PxPhysicsAPI.h"
#include "Apex.h"
#include "PxMat44.h"

#pragma warning(push)
#pragma warning(disable : 4350)
#include <set>
#include <map>
#pragma warning(pop)

using namespace physx;
using namespace nvidia;
using namespace nvidia::apex;

class ApexRenderResourceManager;
class ApexResourceCallback;
class CFirstPersonCamera;
class PhysXPrimitive;
class ApexRenderer;

class ApexController : public ISampleController
{
  public:
	ApexController(PxSimulationFilterShader filterShader, CFirstPersonCamera* camera);
	virtual ~ApexController();

	virtual void onInitialize();
	virtual void onTerminate();

	virtual void Animate(double dt);

	void renderOpaque(ApexRenderer* renderer);
	void renderTransparency(ApexRenderer* renderer);

	void getEyePoseAndPickDir(float mouseX, float mouseY, PxVec3& eyePos, PxVec3& pickDir);

	DestructibleActor* spawnDestructibleActor(const char* assetPath);
	void removeActor(DestructibleActor* actor);

	ClothingActor* spawnClothingActor(const char* assetPath);
	void removeActor(ClothingActor* actor);

	EffectPackageActor* spawnEffectPackageActor(const char* assetPath);
	void removeActor(EffectPackageActor* actor);

	PhysXPrimitive* spawnPhysXPrimitiveBox(const nvidia::PxTransform& position, float density = 2000.0f);
	PhysXPrimitive* spawnPhysXPrimitivePlane(const nvidia::PxPlane& plane);
	void removePhysXPrimitive(PhysXPrimitive*);


	void setRenderDebugInterface(RENDER_DEBUG::RenderDebugInterface* iFace)
	{
		mRenderDebugInterface = iFace;
	}


	ApexResourceCallback* getResourceCallback()
	{
		return mApexResourceCallback;
	};

	ResourceProvider* getResourceProvider()
	{
		return mApexSDK->getNamedResourceProvider();
	}

	ModuleDestructible* getModuleDestructible()
	{
		return mModuleDestructible;
	}

	ModuleParticles* getModuleParticles()
	{
		return mModuleParticles;
	}

	ApexSDK* getApexSDK()
	{
		return mApexSDK;
	}

	Scene* getApexScene()
	{
		return mApexScene;
	}

	float getLastSimulationTime()
	{
		return mLastSimulationTime;
	}

	void togglePlayPause()
	{
		mPaused = !mPaused;
	}

	void toggleFixedTimestep()
	{
		mUseFixedTimestep = !mUseFixedTimestep;
		if (mUseFixedTimestep)
		{
			mTimeRemainder = 0.0;
		}
	}

	bool usingFixedTimestep() const
	{
		return mUseFixedTimestep;
	}

	void setFixedTimestep(double fixedTimestep)
	{
		if (fixedTimestep > 0.0)
		{
			mFixedTimestep = fixedTimestep;
		}
	}

	double getFixedTimestep()
	{
		return mFixedTimestep;
	}

private:
	void initPhysX();
	void releasePhysX();

	void initApex();
	void releaseApex();

	void initPhysXPrimitives();
	void releasePhysXPrimitives();

	void renderParticles(ApexRenderer* renderer, IofxRenderable::Type type);

	PhysXPrimitive* spawnPhysXPrimitive(PxRigidActor* position, PxVec3 scale);

	PxSimulationFilterShader mFilterShader;
#if APEX_CUDA_SUPPORT
	PxCudaContextManager* mCudaContext;
#endif
	ApexSDK* mApexSDK;
	ApexRenderResourceManager* mApexRenderResourceManager;
	ApexResourceCallback* mApexResourceCallback;

	ModuleDestructible* mModuleDestructible;
	ModuleParticles* mModuleParticles;
	ModuleIofx* mModuleIofx;
	Module* mModuleTurbulenceFS;
	Module* mModuleLegacy;
	ModuleClothing* mModuleClothing;
	RenderVolume* mRenderVolume;

	Scene* mApexScene;

	PxDefaultAllocator mAllocator;
	PxDefaultErrorCallback mErrorCallback;

	PxFoundation* mFoundation;
	PxPhysics* mPhysics;
	PxCooking* mCooking;

	PxDefaultCpuDispatcher* mDispatcher;
	PxScene* mPhysicsScene;

	PxMaterial* mDefaultMaterial;

	PxPvd* mPvd;

	RenderDebugInterface* mApexRenderDebug;
	RENDER_DEBUG::RenderDebugInterface* mRenderDebugInterface;

	std::set<Asset*> mAssets;
	std::map<DestructibleActor*, DestructibleRenderable*> mDestructibleActors;
	std::set<ClothingActor*> mClothingActors;
	std::set<EffectPackageActor*> mEffectPackageActors;
	std::set<PhysXPrimitive*> mPrimitives;

	CFirstPersonCamera* mCamera;

	float mLastSimulationTime;
	LARGE_INTEGER mPerformanceFreq;

	bool mPaused;

	bool mUseFixedTimestep;
	double mFixedTimestep;
	double mTimeRemainder;
};

#endif
