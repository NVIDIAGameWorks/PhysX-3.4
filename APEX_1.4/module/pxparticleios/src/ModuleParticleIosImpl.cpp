/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "Apex.h"

#include "ModuleParticleIosImpl.h"
#include "ModuleParticleIosRegistration.h"
#include "ParticleIosScene.h"
#include "ParticleIosAssetImpl.h"
#include "ParticleIosActorImpl.h"
#include "SceneIntl.h"
#include "ModuleIofxIntl.h"
#include "ModuleFieldSamplerIntl.h"
#include "PsMemoryBuffer.h"
#include "ModulePerfScope.h"
using namespace pxparticleios;

#include "ReadCheck.h"
#include "WriteCheck.h"
#include "ApexSDKIntl.h"
#include "ApexUsingNamespace.h"

#include "Lock.h"

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
	ModuleParticleIosImpl* impl = PX_NEW(ModuleParticleIos)(inSdk);
	*niRef  = (ModuleIntl*) impl;
	return (Module*) impl;
}
#else
/* Statically linking entry function */
void instantiateModuleParticleIos()
{
	ApexSDKIntl* sdk = GetInternalApexSDK();
	pxparticleios::ModuleParticleIosImpl* impl = PX_NEW(pxparticleios::ModuleParticleIosImpl)(sdk);
	sdk->registerExternalModule((Module*) impl, (ModuleIntl*) impl);
}
#endif
}
namespace pxparticleios
{

/* =================== ModuleParticleIosImpl =================== */


AuthObjTypeID ParticleIosAssetImpl::mAssetTypeID;

#ifdef WITHOUT_APEX_AUTHORING

class ParticleIosAssetDummyAuthoring : public AssetAuthoring, public UserAllocated
{
public:
	ParticleIosAssetDummyAuthoring(ModuleParticleIosImpl* module, ResourceList& list, NvParameterized::Interface* params, const char* name)
	{
		PX_UNUSED(module);
		PX_UNUSED(list);
		PX_UNUSED(params);
		PX_UNUSED(name);
	}

	ParticleIosAssetDummyAuthoring(ModuleParticleIosImpl* module, ResourceList& list, const char* name)
	{
		PX_UNUSED(module);
		PX_UNUSED(list);
		PX_UNUSED(name);
	}

	ParticleIosAssetDummyAuthoring(ModuleParticleIosImpl* module, ResourceList& list)
	{
		PX_UNUSED(module);
		PX_UNUSED(list);
	}

	virtual void setToolString(const char* /*toolName*/, const char* /*toolVersion*/, uint32_t /*toolChangelist*/)
	{

	}


	virtual void release()
	{
		destroy();
	}

	// internal
	void destroy()
	{
		delete this;
	}

	/**
	* \brief Save asset configuration to a stream
	*/
	virtual PxFileBuf& serialize(PxFileBuf& stream) const
	{
		PX_ASSERT(0);
		return stream;
	}

	/**
	* \brief Load asset configuration from a stream
	*/
	virtual PxFileBuf& deserialize(PxFileBuf& stream)
	{
		PX_ASSERT(0);
		return stream;
	}

	/**
	* \brief Returns the name of this APEX authorable object type
	*/
	virtual const char* getObjTypeName() const
	{
		return ParticleIosAssetImpl::getClassName();
	}

	/**
	 * \brief Prepares a fully authored Asset Authoring object for a specified platform
	 */
	virtual bool prepareForPlatform(nvidia::apex::PlatformTag)
	{
		PX_ASSERT(0);
		return false;
	}

	const char* getName(void) const
	{
		return NULL;
	}

	/**
	* \brief Save asset's NvParameterized interface, may return NULL
	*/
	virtual NvParameterized::Interface* getNvParameterized() const
	{
		PX_ASSERT(0);
		return NULL;
	}

	virtual NvParameterized::Interface* releaseAndReturnNvParameterizedInterface(void)
	{
		PX_ALWAYS_ASSERT();
		return NULL;
	}
};

typedef ApexAuthorableObject<ModuleParticleIosImpl, ParticleIosAssetImpl, ParticleIosAssetDummyAuthoring> ParticleIosAO;

#else
typedef ApexAuthorableObject<ModuleParticleIosImpl, ParticleIosAssetImpl, ParticleIosAssetAuthoringImpl> ParticleIosAO;
#endif

ModuleParticleIosImpl::ModuleParticleIosImpl(ApexSDKIntl* sdk)
{
	mSdk = sdk;
	mApiProxy = this;
	mName = "ParticleIOS";
	mModuleParams = NULL;
	mIofxModule = NULL;
	mFieldSamplerModule = NULL;

	/* Register this module's authorable object types and create their namespaces */
	const char* pName = ParticleIosAssetParam::staticClassName();
	ParticleIosAO* eAO = PX_NEW(ParticleIosAO)(this, mAuthorableObjects,  pName);
	ParticleIosAssetImpl::mAssetTypeID = eAO->getResID();

	/* Register the NvParameterized factories */
	NvParameterized::Traits* traits = mSdk->getParameterizedTraits();
	ModuleParticleIosRegistration::invokeRegistration(traits);
}

AuthObjTypeID ModuleParticleIosImpl::getModuleID() const
{
	return ParticleIosAssetImpl::mAssetTypeID;
}

ModuleParticleIosImpl::~ModuleParticleIosImpl()
{
}

void ModuleParticleIosImpl::destroy()
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
		ModuleParticleIosRegistration::invokeUnregistration(traits);
	}
	delete this;
}


void ModuleParticleIosImpl::init(const ModuleParticleIosDesc&)
{
	WRITE_ZONE();
}

void ModuleParticleIosImpl::init(NvParameterized::Interface&)
{
	WRITE_ZONE();
	ModuleParticleIosDesc desc;
	init(desc);
}

NvParameterized::Interface* ModuleParticleIosImpl::getDefaultModuleDesc()
{
	WRITE_ZONE();
	NvParameterized::Traits* traits = mSdk->getParameterizedTraits();

	if (!mModuleParams)
	{
		mModuleParams = DYNAMIC_CAST(ParticleIosModuleParameters*)
		                (traits->createNvParameterized("ParticleIosModuleParameters"));
		PX_ASSERT(mModuleParams);
	}
	else
	{
		mModuleParams->initDefaults();
	}

	return mModuleParams;
}

uint32_t ModuleParticleIosImpl::forceLoadAssets()
{
	return 0;
}

ParticleIosScene* ModuleParticleIosImpl::getParticleIosScene(const Scene& apexScene)
{
	const SceneIntl* niScene = DYNAMIC_CAST(const SceneIntl*)(&apexScene);
	for (uint32_t i = 0 ; i < mParticleIosSceneList.getSize() ; i++)
	{
		ParticleIosScene* ps = DYNAMIC_CAST(ParticleIosScene*)(mParticleIosSceneList.getResource(i));
		if (ps->mApexScene == niScene)
		{
			return ps;
		}
	}

	PX_ASSERT(!"Unable to locate an appropriate ParticleIosScene");
	return NULL;
}

const ParticleIosScene* ModuleParticleIosImpl::getParticleIosScene(const Scene& apexScene) const
{
	const SceneIntl* niScene = DYNAMIC_CAST(const SceneIntl*)(&apexScene);
	for (uint32_t i = 0 ; i < mParticleIosSceneList.getSize() ; i++)
	{
		ParticleIosScene* ps = DYNAMIC_CAST(ParticleIosScene*)(mParticleIosSceneList.getResource(i));
		if (ps->mApexScene == niScene)
		{
			return ps;
		}
	}

	PX_ASSERT(!"Unable to locate an appropriate ParticleIosScene");
	return NULL;
}

RenderableIterator* ModuleParticleIosImpl::createRenderableIterator(const Scene& apexScene)
{
	WRITE_ZONE();
	ParticleIosScene* ps = getParticleIosScene(apexScene);
	if (ps)
	{
		return ps->createRenderableIterator();
	}

	return NULL;
}

ModuleSceneIntl* ModuleParticleIosImpl::createInternalModuleScene(SceneIntl& scene, RenderDebugInterface* renderDebug)
{
#if APEX_CUDA_SUPPORT
	READ_LOCK(scene);
	if (scene.getTaskManager()->getGpuDispatcher() && scene.isUsingCuda())
	{
		return PX_NEW(ParticleIosSceneGPU)(*this, scene, renderDebug, mParticleIosSceneList);
	}
	else
#endif
	{
		return PX_NEW(ParticleIosSceneCPU)(*this, scene, renderDebug, mParticleIosSceneList);
	}
}

void ModuleParticleIosImpl::releaseModuleSceneIntl(ModuleSceneIntl& scene)
{
	ParticleIosScene* ps = DYNAMIC_CAST(ParticleIosScene*)(&scene);
	ps->destroy();
}

const char* ModuleParticleIosImpl::getParticleIosTypeName()
{
	READ_ZONE();
	return ParticleIosAssetImpl::getClassName();
}


ApexActor* ModuleParticleIosImpl::getApexActor(Actor* nxactor, AuthObjTypeID type) const
{
	if (type == ParticleIosAssetImpl::mAssetTypeID)
	{
		return (ParticleIosActorImpl*) nxactor;
	}

	return NULL;
}

ModuleIofxIntl* ModuleParticleIosImpl::getInternalModuleIofx()
{
	if (!mIofxModule)
	{
		ModuleIntl* nim = mSdk->getInternalModuleByName("IOFX");
		if (nim)
		{
			mIofxModule = DYNAMIC_CAST(ModuleIofxIntl*)(nim);
		}
	}
	return mIofxModule;
}

ModuleFieldSamplerIntl* ModuleParticleIosImpl::getInternalModuleFieldSampler()
{
	if (!mFieldSamplerModule )
	{
		ModuleIntl* nim = mSdk->getInternalModuleByName("FieldSampler");
		if (nim)
		{
			mFieldSamplerModule = DYNAMIC_CAST(ModuleFieldSamplerIntl*)(nim);
		}
	}
	return mFieldSamplerModule;
}

}
} // namespace nvidia
