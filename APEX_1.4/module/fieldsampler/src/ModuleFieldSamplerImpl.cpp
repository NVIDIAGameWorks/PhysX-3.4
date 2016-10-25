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
#include "ModuleFieldSamplerImpl.h"
#include "ModuleFieldSamplerRegistration.h"
#include "FieldSamplerScene.h"
#include "FieldSamplerManager.h"
#include "SceneIntl.h"
#include "PsMemoryBuffer.h"
#include "ModulePerfScope.h"
using namespace fieldsampler;

#include "ApexSDKIntl.h"
#include "ApexUsingNamespace.h"

#include "Lock.h"

#include "ReadCheck.h"
#include "WriteCheck.h"

namespace nvidia
{
namespace apex
{

#if defined(_USRDLL)

/* Modules don't have to link against the framework, they keep their own */
ApexSDKIntl* gApexSdk = 0;
ApexSDK* GetApexSDK()
{
	return gApexSdk;
}
ApexSDKIntl* GetInternalApexSDK()
{
	return gApexSdk;
}

APEX_API Module*  CALL_CONV createModule(
    ApexSDKIntl* inSdk,
    ModuleIntl** niRef,
    uint32_t APEXsdkVersion,
    uint32_t PhysXsdkVersion,
    ApexCreateError* errorCode)
{
	if (APEXsdkVersion != APEX_SDK_VERSION)
	{
		if (errorCode)
		{
			*errorCode = APEX_CE_WRONG_VERSION;
		}
		return NULL;
	}

	if (PhysXsdkVersion != PX_PHYSICS_VERSION)
	{
		if (errorCode)
		{
			*errorCode = APEX_CE_WRONG_VERSION;
		}
		return NULL;
	}

	gApexSdk = inSdk;
	ModuleFieldSamplerImpl* impl = PX_NEW(ModuleFieldSampler)(inSdk);
	*niRef  = (ModuleIntl*) impl;
	return (Module*) impl;
}

#else
/* Statically linking entry function */
void instantiateModuleFieldSampler()
{
	ApexSDKIntl* sdk = GetInternalApexSDK();
	nvidia::fieldsampler::ModuleFieldSamplerImpl* impl = PX_NEW(fieldsampler::ModuleFieldSamplerImpl)(sdk);
	sdk->registerExternalModule((Module*) impl, (ModuleIntl*) impl);
}
#endif // `defined(_USRDLL)
}

namespace fieldsampler
{
/* === ModuleFieldSamplerImpl Implementation === */
ModuleFieldSamplerImpl::ModuleFieldSamplerImpl(ApexSDKIntl* sdk)
{
	mName = "FieldSampler";
	mSdk = sdk;
	mApiProxy = this;
	mModuleParams = NULL;

	/* Register the NvParameterized factories */
	NvParameterized::Traits* traits = mSdk->getParameterizedTraits();
	ModuleFieldSamplerRegistration::invokeRegistration(traits);
}

ModuleFieldSamplerImpl::~ModuleFieldSamplerImpl()
{
}

void ModuleFieldSamplerImpl::destroy()
{
	NvParameterized::Traits* traits = mSdk->getParameterizedTraits();

	if (mModuleParams)
	{
		mModuleParams->destroy();
		mModuleParams = NULL;
	}

	ModuleBase::destroy();

	if (traits)
	{
		/* Remove the NvParameterized factories */
		ModuleFieldSamplerRegistration::invokeUnregistration(traits);
	}

	delete this;
}

void ModuleFieldSamplerImpl::init(NvParameterized::Interface&)
{
}

NvParameterized::Interface* ModuleFieldSamplerImpl::getDefaultModuleDesc()
{
	NvParameterized::Traits* traits = mSdk->getParameterizedTraits();

	if (!mModuleParams)
	{
		mModuleParams = DYNAMIC_CAST(FieldSamplerModuleParameters*)
		                (traits->createNvParameterized("FieldSamplerModuleParameters"));
		PX_ASSERT(mModuleParams);
	}
	else
	{
		mModuleParams->initDefaults();
	}

	return mModuleParams;
}

FieldSamplerManagerIntl* ModuleFieldSamplerImpl::getInternalFieldSamplerManager(const Scene& apexScene)
{
	FieldSamplerScene* scene = ModuleFieldSamplerImpl::getFieldSamplerScene(apexScene);
	return scene->getManager();
}


AuthObjTypeID ModuleFieldSamplerImpl::getModuleID() const
{
	return 0;
}


/* == Example Scene methods == */
ModuleSceneIntl* ModuleFieldSamplerImpl::createInternalModuleScene(SceneIntl& scene, RenderDebugInterface* debugRender)
{
#if APEX_CUDA_SUPPORT
	READ_LOCK(scene);
	if (scene.getTaskManager()->getGpuDispatcher() && scene.isUsingCuda())
	{
		return PX_NEW(FieldSamplerSceneGPU)(*this, scene, debugRender, mFieldSamplerScenes);
	}
	else
#endif
		return PX_NEW(FieldSamplerSceneCPU)(*this, scene, debugRender, mFieldSamplerScenes);
}

void ModuleFieldSamplerImpl::releaseModuleSceneIntl(ModuleSceneIntl& scene)
{
	FieldSamplerScene* es = DYNAMIC_CAST(FieldSamplerScene*)(&scene);
	es->destroy();
}

nvidia::fieldsampler::FieldSamplerScene* ModuleFieldSamplerImpl::getFieldSamplerScene(const Scene& apexScene) const
{
	for (uint32_t i = 0 ; i < mFieldSamplerScenes.getSize() ; i++)
	{
		FieldSamplerScene* es = DYNAMIC_CAST(FieldSamplerScene*)(mFieldSamplerScenes.getResource(i));
		if (es->mApexScene == &apexScene)
		{
			return es;
		}
	}

	PX_ASSERT(!"Unable to locate an appropriate FieldSamplerScene");
	return NULL;
}

RenderableIterator* ModuleFieldSamplerImpl::createRenderableIterator(const Scene& apexScene)
{
	FieldSamplerScene* es = getFieldSamplerScene(apexScene);
	if (es)
	{
		return es->createRenderableIterator();
	}

	return NULL;
}



bool ModuleFieldSamplerImpl::setFieldSamplerWeightedCollisionFilterCallback(const Scene& apexScene,FieldSamplerWeightedCollisionFilterCallback *callback)
{
	WRITE_ZONE();
	FieldSamplerScene* scene = getFieldSamplerScene(apexScene);
	if (scene != NULL)
	{
		DYNAMIC_CAST(FieldSamplerManager*)(scene->getManager())->setFieldSamplerWeightedCollisionFilterCallback(callback);
		return true;
	}
	return false;

}

#if PX_PHYSICS_VERSION_MAJOR == 3
void ModuleFieldSamplerImpl::enablePhysXMonitor(const Scene& apexScene, bool enable)
{
	WRITE_ZONE();
	getFieldSamplerScene(apexScene)->enablePhysXMonitor(enable);	
}

void ModuleFieldSamplerImpl::setPhysXMonitorFilterData(const Scene& apexScene, physx::PxFilterData filterData)
{
	WRITE_ZONE();
	getFieldSamplerScene(apexScene)->setPhysXFilterData(filterData);
}
#endif

uint32_t ModuleFieldSamplerImpl::createForceSampleBatch(const Scene& apexScene, uint32_t maxCount, const physx::PxFilterData filterData)
{
	WRITE_ZONE();
	FieldSamplerScene* fsScene = getFieldSamplerScene(apexScene);
	if (fsScene)
	{
		return fsScene->createForceSampleBatch(maxCount, filterData);
	}
	return (uint32_t)~0;
}


void ModuleFieldSamplerImpl::releaseForceSampleBatch(const Scene& apexScene, uint32_t batchId)
{
	WRITE_ZONE();
	FieldSamplerScene* fsScene = getFieldSamplerScene(apexScene);
	if (fsScene)
	{
		fsScene->releaseForceSampleBatch(batchId);
	}
}


void ModuleFieldSamplerImpl::submitForceSampleBatch(const Scene& apexScene, uint32_t batchId,
												PxVec4* forces, const uint32_t forcesStride,
												const PxVec3* positions, const uint32_t positionsStride, 
												const PxVec3* velocities, const uint32_t velocitiesStride,
												const float* mass, const uint32_t massStride,
												const uint32_t* indices, const uint32_t numIndices)
{
	WRITE_ZONE();
	FieldSamplerScene* fsScene = getFieldSamplerScene(apexScene);
	if (fsScene)
	{
		fsScene->submitForceSampleBatch(batchId, forces, forcesStride, positions, positionsStride, velocities, velocitiesStride, mass, massStride, indices, numIndices);
	}
}


}
} // end namespace nvidia::apex
