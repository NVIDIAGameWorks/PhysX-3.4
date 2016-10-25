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
#include "SceneIntl.h"
#include "ApexSDKIntl.h"
#include "ParticleIosActor.h"
#include "ParticleIosActorImpl.h"
#include "ParticleIosAssetImpl.h"

#include "iofx/IofxAsset.h"
#include "iofx/IofxActor.h"

#include "ModuleParticleIosImpl.h"
#include "ParticleIosScene.h"
#include "RenderDebugInterface.h"
#include "AuthorableObjectIntl.h"
#include "ModuleIofxIntl.h"
#include "FieldSamplerManagerIntl.h"
#include "FieldSamplerQueryIntl.h"
#include "FieldSamplerSceneIntl.h"
#include "ApexResourceHelper.h"
#include "ApexMirroredArray.h"

#include <PxPhysics.h>
#include <PsAsciiConversion.h>
#include "ReadCheck.h"
#include "WriteCheck.h"

#include "PsMathUtils.h"

namespace nvidia
{
namespace pxparticleios
{

using namespace physx;

void ParticleIosActorImpl::initStorageGroups(InplaceStorage& storage)
{
	mSimulationStorageGroup.init(storage);
}

#pragma warning(disable: 4355) // 'this' : used in base member initializer list

ParticleIosActorImpl::ParticleIosActorImpl(
    ResourceList& list,
    ParticleIosAssetImpl& asset,
    ParticleIosScene& scene,
    IofxAsset& iofxAsset,
    bool isDataOnDevice)
	: mAsset(&asset)
	, mParticleIosScene(&scene)
	, mIsParticleSystem(false)
	, mParticleActor(NULL)
	, mIofxMgr(NULL)
	, mTotalElapsedTime(0.0f)
	, mParticleCount(0)
	, mParticleBudget(0)
	, mInjectedCount(0)
	, mLastActiveCount(0)
	, mLastBenefitSum(0)
	, mLastBenefitMin(+FLT_MAX)
	, mLastBenefitMax(-FLT_MAX)
	, mLifeSpan(scene.getApexScene(), PX_ALLOC_INFO("mLifeSpan", PARTICLES))
	, mLifeTime(scene.getApexScene(), PX_ALLOC_INFO("mLifeTime", PARTICLES))
	, mInjector(scene.getApexScene(), PX_ALLOC_INFO("mInjector", PARTICLES))
	, mBenefit(scene.getApexScene(), PX_ALLOC_INFO("mBenefit", PARTICLES))
	, mInjectorsCounters(scene.getApexScene(), PX_ALLOC_INFO("mInjectorsCounters", PARTICLES))
	, mInputIdToParticleIndex(scene.getApexScene(), PX_ALLOC_INFO("mInputIdToParticleIndex", PARTICLES))
	, mGridDensityGrid(scene.getApexScene(), PX_ALLOC_INFO("mGridDensityGrid", PARTICLES))
	, mGridDensityGridLowPass(scene.getApexScene(), PX_ALLOC_INFO("mGridDensityGridLowPass", PARTICLES))
	, mFieldSamplerQuery(NULL)
	, mField(scene.getApexScene(), PX_ALLOC_INFO("mField", PARTICLES))
	, mInjectTask(*this)
	, mDensityOrigin(0.f,0.f,0.f)
	, mOnStartCallback(NULL)
	, mOnFinishCallback(NULL)
{
	list.add(*this);

	if (mAsset->getParticleDesc()->Enable == false)
	{
		APEX_DEBUG_WARNING("ParticleIos Asset '%s' has Enable=false, particle simulation is disabled.", mAsset->getName());
	}

	mMaxParticleCount = mAsset->mParams->maxParticleCount;
	float maxInjectCount = mAsset->mParams->maxInjectedParticleCount;
	mMaxTotalParticleCount = mMaxParticleCount + uint32_t(maxInjectCount <= 1.0f ? mMaxParticleCount * maxInjectCount : maxInjectCount);

	IofxManagerDescIntl desc;
	desc.iosAssetName         = mAsset->getName();
	desc.iosSupportsDensity   = mAsset->getSupportsDensity();
	desc.iosSupportsCollision = true;
	desc.iosSupportsUserData  = true;
	desc.iosOutputsOnDevice   = isDataOnDevice;
	desc.maxObjectCount       = mMaxParticleCount;
	desc.maxInputCount        = mMaxTotalParticleCount;
	desc.maxInStateCount      = mMaxTotalParticleCount;

	ModuleIofxIntl* moduleIofx = mAsset->mModule->getInternalModuleIofx();
	if (moduleIofx)
	{
		mIofxMgr = moduleIofx->createActorManager(*mParticleIosScene->mApexScene, iofxAsset, desc);
		mIofxMgr->createSimulationBuffers(mBufDesc);

		for (uint32_t i = 0 ; i < mMaxParticleCount ; i++)
		{
			mBufDesc.pmaInStateToInput->get(i) = IosBufferDescIntl::NOT_A_PARTICLE;
		}
	}

	FieldSamplerManagerIntl* fieldSamplerManager = mParticleIosScene->getInternalFieldSamplerManager();
	if (fieldSamplerManager)
	{
		FieldSamplerQueryDescIntl queryDesc;
		queryDesc.maxCount = mMaxParticleCount;
		queryDesc.samplerFilterData = ApexResourceHelper::resolveCollisionGroup128(mAsset->mParams->fieldSamplerFilterData);

		mFieldSamplerQuery = fieldSamplerManager->createFieldSamplerQuery(queryDesc);
	}

	addSelfToContext(*scene.mApexScene->getApexContext());		// add self to Scene
	addSelfToContext(*DYNAMIC_CAST(ApexContext*)(&scene));		// add self to ParticleIosScene	

	// Pull Grid Density Parameters
	{
		if(mIsParticleSystem && mBufDesc.pmaDensity)
		{
			ParticleIosAssetParam* params = (ParticleIosAssetParam*)(mAsset->getAssetNvParameterized());
			const SimpleParticleSystemParams* gridParams = static_cast<SimpleParticleSystemParams*>(params->particleType);
			mGridDensityParams.Enabled = gridParams->GridDensity.Enabled;
			mGridDensityParams.GridSize = gridParams->GridDensity.GridSize;
			mGridDensityParams.GridMaxCellCount = gridParams->GridDensity.MaxCellCount;
			mGridDensityParams.GridResolution = general_string_parsing2::PxAsc::strToU32(&gridParams->GridDensity.Resolution[4],NULL);
			mGridDensityParams.DensityOrigin = mDensityOrigin;
		}		
		else
		{
			mGridDensityParams.Enabled = false;
			mGridDensityParams.GridSize = 1.f;
			mGridDensityParams.GridMaxCellCount = 1u;
			mGridDensityParams.GridResolution = 8;
			mGridDensityParams.DensityOrigin = mDensityOrigin;
		}
	}
}

ParticleIosActorImpl::~ParticleIosActorImpl()
{
}

void ParticleIosActorImpl::release()
{
	if (mInRelease)
	{
		return;
	}
	mInRelease = true;
	mAsset->releaseIosActor(*this);
}

void ParticleIosActorImpl::destroy()
{
	ApexActor::destroy();

	setPhysXScene(NULL);

	// remove ourself from our asset's resource list, in case releasing our emitters
	// causes our asset's resource count to reach zero and for it to be released.
	ApexResource::removeSelf();

	// Release all injectors, releasing all emitters and their IOFX asset references
	while (mInjectorList.getSize())
	{
		ParticleParticleInjector* inj = DYNAMIC_CAST(ParticleParticleInjector*)(mInjectorList.getResource(mInjectorList.getSize() - 1));
		inj->release();
	}

	if (mIofxMgr)
	{
		mIofxMgr->release();
	}
	if (mFieldSamplerQuery)
	{
		mFieldSamplerQuery->release();
	}

	delete this;
}

void ParticleIosActorImpl::setPhysXScene(PxScene* scene)
{
	if (scene)
	{
		putInScene(scene);
	}
	else
	{
		removeFromScene();
	}
}

PxScene* ParticleIosActorImpl::getPhysXScene() const
{
	if (mParticleActor)
	{
		return mParticleActor->getScene();
	}
	else
	{
		return NULL;
	}
}

void ParticleIosActorImpl::putInScene(PxScene* scene)
{
	SCOPED_PHYSX_LOCK_WRITE(scene);
	mUp = scene->getGravity();

	// apply asset's scene gravity scale and external acceleration
	// mUp *= mAsset->getSceneGravityScale();
	// mUp += mAsset->getExternalAcceleration();

	mGravity = mUp.magnitude();
	if (!PxIsFinite(mGravity))
	{
		// and they could set both to 0,0,0
		mUp = PxVec3(0.0f, -1.0f, 0.0f);
		mGravity = 1.0f;
	}
	mUp *= -1.0f;

	const ParticleIosAssetParam*	desc = mAsset->getParticleDesc();

	if (!isParticleDescValid(desc))
	{
		PX_ASSERT(0);
		return;
	}

	uint32_t	maxParticles = mMaxParticleCount;
	PxParticleBase*		particle	= NULL;
	PxParticleFluid*	fluid		= NULL;

	ApexSimpleString className(mAsset->getParticleTypeClassName());
	if (className == SimpleParticleSystemParams::staticClassName())
	{
		mIsParticleSystem	= true;
		particle			= scene->getPhysics().createParticleSystem(maxParticles, desc->PerParticleRestOffset);
	}
	else
	{
		mIsParticleSystem	= false;
		fluid				= scene->getPhysics().createParticleFluid(maxParticles, desc->PerParticleRestOffset);
		particle			= fluid;
	}

	if (particle)
	{
		particle->setMaxMotionDistance(desc->maxMotionDistance);
		particle->setContactOffset(desc->contactOffset);
		particle->setRestOffset(desc->restOffset);
		particle->setGridSize(desc->gridSize);
		particle->setDamping(desc->damping);
		particle->setExternalAcceleration(desc->externalAcceleration);
		particle->setProjectionPlane(desc->projectionPlaneNormal, desc->projectionPlaneDistance);
		particle->setParticleMass(desc->particleMass);
		particle->setRestitution(desc->restitution);
		particle->setDynamicFriction(desc->dynamicFriction);
		particle->setStaticFriction(desc->staticFriction);
		if (desc->fieldSamplerFilterData && desc->fieldSamplerFilterData[0])
		{
			ResourceProviderIntl* nrp = mAsset->mModule->mSdk->getInternalResourceProvider();

			ResID cgmns = mAsset->mModule->mSdk->getCollisionGroup128NameSpace();
			ResID cgmresid = nrp->createResource(cgmns, desc->fieldSamplerFilterData);
			void* tmpCGM = nrp->getResource(cgmresid);
			if (tmpCGM)
			{
				particle->setSimulationFilterData(*(static_cast<PxFilterData*>(tmpCGM)));
			}
			//nrp->releaseResource( cgresid );
		}
		particle->setParticleBaseFlag(PxParticleBaseFlag::eCOLLISION_TWOWAY, desc->CollisionTwoway);
		particle->setParticleBaseFlag(PxParticleBaseFlag::eCOLLISION_WITH_DYNAMIC_ACTORS, desc->CollisionWithDynamicActors);
		particle->setParticleBaseFlag(PxParticleBaseFlag::eENABLED, desc->Enable);
		particle->setParticleBaseFlag(PxParticleBaseFlag::ePROJECT_TO_PLANE, desc->ProjectToPlane);
		// PxParticleBaseFlag::ePER_PARTICLE_REST_OFFSET is set in create() function
		particle->setParticleBaseFlag(PxParticleBaseFlag::ePER_PARTICLE_COLLISION_CACHE_HINT, desc->PerParticleCollisionCacheHint);
		// set hardware flag only if hardware is available
		particle->setParticleBaseFlag(PxParticleBaseFlag::eGPU, NULL != scene->getTaskManager()->getGpuDispatcher());
		
		particle->setParticleReadDataFlag(PxParticleReadDataFlag::ePOSITION_BUFFER, true);
		particle->setParticleReadDataFlag(PxParticleReadDataFlag::eVELOCITY_BUFFER, true);
		particle->setParticleReadDataFlag(PxParticleReadDataFlag::eREST_OFFSET_BUFFER, desc->PerParticleRestOffset);
		particle->setParticleReadDataFlag(PxParticleReadDataFlag::eFLAGS_BUFFER, true);
		particle->setParticleReadDataFlag(PxParticleReadDataFlag::eCOLLISION_NORMAL_BUFFER, true);
		if (fluid)
		{
			particle->setParticleReadDataFlag(PxParticleReadDataFlag::eDENSITY_BUFFER, desc->DensityBuffer);

			const FluidParticleSystemParams*	fluidDesc	= (FluidParticleSystemParams*)desc->particleType;
			fluid->setRestParticleDistance(fluidDesc->restParticleDistance);
			fluid->setStiffness(fluidDesc->stiffness);
			fluid->setViscosity(fluidDesc->viscosity);
		}
	}
	mParticleActor = particle;

	PX_ASSERT(mParticleActor);

	scene->addActor(*mParticleActor);
	if (mParticleIosScene->getInternalFieldSamplerManager())
	{
		mParticleIosScene->getInternalFieldSamplerManager()->registerUnhandledParticleSystem(mParticleActor);
	}
	PX_ASSERT(mParticleActor->getScene());

	mIofxMgr->setSimulationParameters(desc->restOffset, mUp, mGravity, 1 / desc->restOffset);
}

void ParticleIosActorImpl::removeFromScene()
{
	if (mParticleActor)
	{
		if (mParticleIosScene->getInternalFieldSamplerManager())
		{
			mParticleIosScene->getInternalFieldSamplerManager()->unregisterUnhandledParticleSystem(mParticleActor);
		}
		SCOPED_PHYSX_LOCK_WRITE(mParticleActor->getScene());
		mParticleActor->getScene()->removeActor(*mParticleActor);
		mParticleActor->release();
	}
	mParticleActor = NULL;
	mParticleCount = 0;
}



void ParticleIosActorImpl::getLodRange(float& min, float& max, bool& intOnly) const
{
	READ_ZONE();
	PX_UNUSED(min);
	PX_UNUSED(max);
	PX_UNUSED(intOnly);
	APEX_INVALID_OPERATION("not implemented");
}


float ParticleIosActorImpl::getActiveLod() const
{
	READ_ZONE();
	APEX_INVALID_OPERATION("ParticleIosActor does not support this operation");
	return -1.0f;
}


void ParticleIosActorImpl::forceLod(float lod)
{
	WRITE_ZONE();
	PX_UNUSED(lod);
	APEX_INVALID_OPERATION("not implemented");
}



const PxVec3* ParticleIosActorImpl::getRecentPositions(uint32_t& count, uint32_t& stride) const
{
	count = mParticleCount;
	stride = sizeof(PxVec4);
	return (const PxVec3*) mBufDesc.pmaPositionMass->getPtr();
}

IosInjectorIntl* ParticleIosActorImpl::allocateInjector(IofxAsset* iofxAsset)
{
	ParticleParticleInjector* inj = 0;
	//createInjector
	{
		uint32_t injectorID = mParticleIosScene->getInjectorAllocator().allocateInjectorID();
		if (injectorID != ParticleIosInjectorAllocator::NULL_INJECTOR_INDEX)
		{
			inj = PX_NEW(ParticleParticleInjector)(mInjectorList, *this, injectorID);
		}
	}
	if (inj == 0)
	{
		APEX_INTERNAL_ERROR("Failed to create new ParticleIos injector.");
		return NULL;
	}

	inj->init(iofxAsset);
	return inj;
}

void ParticleIosActorImpl::releaseInjector(IosInjectorIntl& injector)
{
	ParticleParticleInjector* inj = DYNAMIC_CAST(ParticleParticleInjector*)(&injector);
	//destroyInjector
	{
		//set mLODBias to FLT_MAX to mark released injector
		//all particles from released injectors will be removed in simulation
		Px3InjectorParams injParams;
		mParticleIosScene->fetchInjectorParams(inj->mInjectorID, injParams);
		injParams.mLODBias = FLT_MAX;
		mParticleIosScene->updateInjectorParams(inj->mInjectorID, injParams);

		mParticleIosScene->getInjectorAllocator().releaseInjectorID(inj->mInjectorID);
		inj->destroy();
	}

    if(mInjectorList.getSize() == 0)
    {
        //if we have no injectors - release self
        release();
    }
}

void ParticleIosActorImpl::visualize()
{
#ifndef WITHOUT_DEBUG_VISUALIZE
	if ( !mEnableDebugVisualization ) return;
	RenderDebugInterface* renderer = mParticleIosScene->mRenderDebug;
	const physx::PxMat44& savedPose = *RENDER_DEBUG_IFACE(renderer)->getPoseTyped();
	RENDER_DEBUG_IFACE(renderer)->setIdentityPose();
	if(mParticleIosScene->mParticleIosDebugRenderParams->VISUALIZE_PARTICLE_IOS_GRID_DENSITY)
	{
		if(mGridDensityParams.Enabled)
		{					
			RENDER_DEBUG_IFACE(renderer)->setCurrentColor(0x0000ff);
			uint32_t onScreenRes = mGridDensityParams.GridResolution - 4;
			for (uint32_t i = 0 ; i <= onScreenRes; i++)
			{
				float u = 2.f*((float)i/(onScreenRes))-1.f;
				PxVec4 a = mDensityDebugMatInv.transform(PxVec4(u,-1.f,0.1f,1.f));
				PxVec4 b = mDensityDebugMatInv.transform(PxVec4(u, 1.f,0.1f,1.f));
				PxVec4 c = mDensityDebugMatInv.transform(PxVec4(-1.f,u,0.1f,1.f));
				PxVec4 d = mDensityDebugMatInv.transform(PxVec4( 1.f,u,0.1f,1.f));
				RENDER_DEBUG_IFACE(renderer)->debugLine(PxVec3(a.getXYZ()/a.w),PxVec3(b.getXYZ()/b.w));
				RENDER_DEBUG_IFACE(renderer)->debugLine(PxVec3(c.getXYZ()/c.w),PxVec3(d.getXYZ()/d.w));
			}
		}
	}
	RENDER_DEBUG_IFACE(renderer)->setPose(savedPose);
#endif
}


PxTaskID ParticleIosActorImpl::submitTasks(PxTaskManager* tm)
{
	return tm->submitUnnamedTask(mInjectTask);
}

void ParticleIosActorImpl::setTaskDependencies(PxTaskID taskStartAfterID, PxTaskID taskFinishBeforeID, PxTask* iosTask, bool isDataOnDevice)
{
	PxTaskManager* tm = mParticleIosScene->getApexScene().getTaskManager();

	PxTaskID simTaskID = tm->getNamedTask(AST_PHYSX_SIMULATE);
	mInjectTask.finishBefore(simTaskID);
	if (iosTask == 0)
	{
		return;
	}
	iosTask->startAfter(mInjectTask.getTaskID());

	if (taskStartAfterID != (PxTaskID)0xFFFFFFFF)
	{
		iosTask->startAfter(taskStartAfterID);
	}
	if (taskFinishBeforeID != (PxTaskID)0xFFFFFFFF)
	{
		iosTask->finishBefore(taskFinishBeforeID);
	}

	if (mFieldSamplerQuery != NULL)
	{
		float deltaTime = mParticleIosScene->getApexScene().getPhysXSimulateTime();

		FieldSamplerQueryDataIntl queryData;
		queryData.timeStep = deltaTime;
		queryData.count = mParticleCount;
		queryData.isDataOnDevice = isDataOnDevice;
		queryData.positionStrideBytes = sizeof(PxVec4);
		queryData.velocityStrideBytes = sizeof(PxVec4);
		queryData.massStrideBytes = sizeof(PxVec4);
		queryData.pmaInIndices = 0;
		if (isDataOnDevice)
		{
#if APEX_CUDA_SUPPORT
			queryData.pmaInPosition = (float*)mBufDesc.pmaPositionMass->getGpuPtr();
			queryData.pmaInVelocity = (float*)mBufDesc.pmaVelocityLife->getGpuPtr();
			queryData.pmaInMass = &mBufDesc.pmaPositionMass->getGpuPtr()->w;
			queryData.pmaOutField = mField.getGpuPtr();
#endif
		}
		else
		{
			queryData.pmaInPosition = (float*)mBufDesc.pmaPositionMass->getPtr();
			queryData.pmaInVelocity = (float*)mBufDesc.pmaVelocityLife->getPtr();
			queryData.pmaInMass = &mBufDesc.pmaPositionMass->getPtr()->w;
			queryData.pmaOutField = mField.getPtr();
		}

		mFieldSamplerQuery->submitFieldSamplerQuery(queryData, iosTask->getTaskID());
	}

	const PxTaskID postIofxTaskID = tm->getNamedTask(AST_PHYSX_FETCH_RESULTS);
	PxTaskID iofxTaskID = mIofxMgr->getUpdateEffectsTaskID(postIofxTaskID);
	if (iofxTaskID == (PxTaskID)0xFFFFFFFF)
	{
		iofxTaskID = postIofxTaskID;
	}
	iosTask->finishBefore(iofxTaskID);
}

void ParticleIosActorImpl::fetchResults()
{
	for(uint32_t i = 0; i < mInjectorList.getSize(); ++i)
	{
		ParticleParticleInjector* inj = DYNAMIC_CAST(ParticleParticleInjector*)(mInjectorList.getResource(i));
		inj->assignSimParticlesCount(mInjectorsCounters.get(i));
	}
}

void ParticleIosActorImpl::injectNewParticles()
{
	mInjectedBenefitSum = 0;
	mInjectedBenefitMin = +FLT_MAX;
	mInjectedBenefitMax = -FLT_MAX;

	uint32_t maxInjectCount = (mMaxTotalParticleCount - mParticleCount);

	uint32_t injectCount = 0;
	uint32_t lastInjectCount = 0;
	do
	{
		lastInjectCount = injectCount;
		for (uint32_t i = 0; i < mInjectorList.getSize(); i++)
		{
			ParticleParticleInjector* inj = DYNAMIC_CAST(ParticleParticleInjector*)(mInjectorList.getResource(i));
			if (inj->mInjectedParticles.size() == 0)
			{
				continue;
			}

			if (injectCount < maxInjectCount)
			{
				IosNewObject obj;
				if (inj->mInjectedParticles.popFront(obj))
				{
					PX_ASSERT(obj.lifetime > 0.0f);
					PX_ASSERT(PxIsFinite(obj.lodBenefit));

					uint32_t injectIndex = mParticleCount + injectCount;

					float particleMass = mAsset->getParticleMass();
					mBufDesc.pmaPositionMass->get(injectIndex) = PxVec4(obj.initialPosition.x, obj.initialPosition.y, obj.initialPosition.z, particleMass);
					mBufDesc.pmaVelocityLife->get(injectIndex) = PxVec4(obj.initialVelocity.x, obj.initialVelocity.y, obj.initialVelocity.z, 1.0f);
					mBufDesc.pmaCollisionNormalFlags->get(injectIndex).setZero();
					mBufDesc.pmaActorIdentifiers->get(injectIndex) = obj.iofxActorID;

					mBufDesc.pmaUserData->get(injectIndex) = obj.userData;

					mLifeSpan[injectIndex] = obj.lifetime;
					mInjector[injectIndex] = inj->mInjectorID;
					mBenefit[injectIndex] = obj.lodBenefit;

					mInjectedBenefitSum += obj.lodBenefit;
					mInjectedBenefitMin = PxMin(mInjectedBenefitMin, obj.lodBenefit);
					mInjectedBenefitMax = PxMax(mInjectedBenefitMax, obj.lodBenefit);

					++injectCount;
				}
			}
		}
	}
	while (injectCount > lastInjectCount);

	mInjectedCount = injectCount;

	//clear injectors FIFO
	for (uint32_t i = 0; i < mInjectorList.getSize(); i++)
	{
		ParticleParticleInjector* inj = DYNAMIC_CAST(ParticleParticleInjector*)(mInjectorList.getResource(i));

		IosNewObject obj;
		while (inj->mInjectedParticles.popFront(obj))
		{
			;
		}
	}
}

bool ParticleIosActorImpl::isParticleDescValid( const ParticleIosAssetParam* desc) const
{
	if (desc->gridSize <= 0.0f) return false;
	if (desc->maxMotionDistance <= 0.0f) return false;
	if (desc->maxMotionDistance + desc->contactOffset > desc->gridSize) return false;
	if (desc->contactOffset < 0.0f) return false;
	if (desc->contactOffset < desc->restOffset) return false;
	if (desc->particleMass < 0.0f) return false;
	if (desc->damping < 0.0f) return false;
	if (desc->projectionPlaneNormal.isZero()) return false;
	if (desc->restitution < 0.0f || desc->restitution > 1.0f) return false;
	if (desc->dynamicFriction < 0.0f || desc->dynamicFriction > 1.0f) return false;
	if (desc->staticFriction < 0.0f) return false;
	if (desc->maxParticleCount < 1) return false;

	ApexSimpleString className(mAsset->getParticleTypeClassName());
	if (className == SimpleParticleSystemParams::staticClassName())
	{
		return true;
	}
	else
	if (className == FluidParticleSystemParams::staticClassName())
	{
		const FluidParticleSystemParams*	fluidDesc	= (FluidParticleSystemParams*)desc->particleType;
		if (fluidDesc->restParticleDistance <= 0.0f) return false;

		if (fluidDesc->stiffness <= 0.0f) return false;
		if (fluidDesc->viscosity <= 0.0f) return false;

		return true;
	}
	else
	{
		return false;
	}
}
////////////////////////////////////////////////////////////////////////////////

ParticleParticleInjector::ParticleParticleInjector(ResourceList& list, ParticleIosActorImpl& actor, uint32_t injectorID)
	: mIosActor(&actor)
	, mIofxClient(NULL)
	, mVolume(NULL)
	, mLastRandomID(0)
	, mVolumeID(IofxActorIDIntl::NO_VOLUME)
	, mInjectorID(injectorID)
	, mSimulatedParticlesCount(0)
{
	list.add(*this);
	
	setLODWeights(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);

	mInjectedParticles.reserve(actor.mMaxTotalParticleCount);
}

void ParticleParticleInjector::setListIndex(ResourceList& list, uint32_t index)
{
	m_listIndex = index;
	m_list = &list;

	Px3InjectorParams injParams;
	mIosActor->mParticleIosScene->fetchInjectorParams(mInjectorID, injParams);
	injParams.mLocalIndex = index;
	mIosActor->mParticleIosScene->updateInjectorParams(mInjectorID, injParams);
}

/* Emitter calls this function to adjust their particle weights with respect to other emitters */
void ParticleParticleInjector::setLODWeights(float maxDistance, float distanceWeight, float speedWeight, float lifeWeight, float separationWeight, float bias)
{
	PX_UNUSED(separationWeight);

	Px3InjectorParams injParams;
	mIosActor->mParticleIosScene->fetchInjectorParams(mInjectorID, injParams);

	injParams.mLODMaxDistance = maxDistance;
	injParams.mLODDistanceWeight = distanceWeight;
	injParams.mLODSpeedWeight = speedWeight;
	injParams.mLODLifeWeight = lifeWeight;
	injParams.mLODBias = bias;

	mIosActor->mParticleIosScene->updateInjectorParams(mInjectorID, injParams);
}

PxTaskID ParticleParticleInjector::getCompletionTaskID() const
{
	return mIosActor->mInjectTask.getTaskID();
}

void ParticleParticleInjector::setObjectScale(float objectScale)
{
	PX_ASSERT(mIofxClient);
	IofxManagerClientIntl::Params params;
	mIofxClient->getParams(params);
	params.objectScale = objectScale;
	mIofxClient->setParams(params);
}

void ParticleParticleInjector::init(IofxAsset* iofxAsset)
{
	mIofxClient = mIosActor->mIofxMgr->createClient(iofxAsset, IofxManagerClientIntl::Params());

	/* add this injector to the IOFX asset's context (so when the IOFX goes away our ::release() is called) */
	iofxAsset->addDependentActor(this);

	mRandomActorClassIDs.clear();
	if (iofxAsset->getMeshAssetCount() < 2)
	{
		mRandomActorClassIDs.pushBack(mIosActor->mIofxMgr->getActorClassID(mIofxClient, 0));
		return;
	}

	/* Cache actorClassIDs for this asset */
	physx::Array<uint16_t> temp;
	for (uint32_t i = 0 ; i < iofxAsset->getMeshAssetCount() ; i++)
	{
		uint32_t w = iofxAsset->getMeshAssetWeight(i);
		uint16_t acid = mIosActor->mIofxMgr->getActorClassID(mIofxClient, (uint16_t) i);
		for (uint32_t j = 0 ; j < w ; j++)
		{
			temp.pushBack(acid);
		}
	}

	mRandomActorClassIDs.reserve(temp.size());
	while (temp.size())
	{
		uint32_t index = (uint32_t)physx::shdfnd::rand(0, (int32_t)temp.size() - 1);
		mRandomActorClassIDs.pushBack(temp[ index ]);
		temp.replaceWithLast(index);
	}
}


void ParticleParticleInjector::release()
{
	if (mInRelease)
	{
		return;
	}
	mInRelease = true;
	mIosActor->releaseInjector(*this);
}

void ParticleParticleInjector::destroy()
{
	ApexActor::destroy();

	mIosActor->mIofxMgr->releaseClient(mIofxClient);

	delete this;
}

void ParticleParticleInjector::setPreferredRenderVolume(RenderVolume* volume)
{
	mVolume = volume;
	mVolumeID = mVolume ? mIosActor->mIofxMgr->getVolumeID(mVolume) : IofxActorIDIntl::NO_VOLUME;
}

/* Emitter calls this virtual injector API to insert new particles.  It is safe for an emitter to
 * call this function at any time except for during the IOS::fetchResults().  Since
 * ParticleScene::fetchResults() is single threaded, it should be safe to call from
 * emitter::fetchResults() (destruction may want to do this because of contact reporting)
 */
void ParticleParticleInjector::createObjects(uint32_t count, const IosNewObject* createList)
{
	PX_PROFILE_ZONE("ParticleIosCreateObjects", GetInternalApexSDK()->getContextId());

	if (mRandomActorClassIDs.size() == 0)
	{
		return;
	}

	const PxVec3& eyePos = mIosActor->mParticleIosScene->getApexScene().getEyePosition();
	Px3InjectorParams injParams;
	mIosActor->mParticleIosScene->fetchInjectorParams(mInjectorID, injParams);

	// Append new objects to our FIFO.  We do copies because we must perform buffering for the
	// emitters.  We have to hold these new objects until there is room in the TurbulenceFS and the
	// injector's virtID range to emit them.
	for (uint32_t i = 0 ; i < count ; i++)
	{
		if (mInjectedParticles.size() == mInjectedParticles.capacity())
		{
			break;
		}

		IosNewObject obj = *createList++;

		obj.lodBenefit = calcParticleBenefit(injParams, eyePos, obj.initialPosition, obj.initialVelocity, 1.0f);
		obj.iofxActorID.set(mVolumeID, mRandomActorClassIDs[ mLastRandomID++ ]);
		mLastRandomID = mLastRandomID == mRandomActorClassIDs.size() ? 0 : mLastRandomID;
		//mInjectedParticleBenefit += obj.lodBenefit;
		mInjectedParticles.pushBack(obj);
	}
}

#if APEX_CUDA_SUPPORT
void ParticleParticleInjector::createObjects(ApexMirroredArray<const IosNewObject>& createArray)
{
	PX_UNUSED(createArray);

	// An emitter will call this API when it has filled a host or device buffer.  The injector
	// should trigger a copy to the location it would like to see the resulting data when the
	// IOS is finally ticked.

	PX_ALWAYS_ASSERT(); /* Not yet supported */
}
#endif

uint32_t ParticleParticleInjector::getActivePaticleCount() const
{
	return mSimulatedParticlesCount;
}

}
} // end namespace nvidia

