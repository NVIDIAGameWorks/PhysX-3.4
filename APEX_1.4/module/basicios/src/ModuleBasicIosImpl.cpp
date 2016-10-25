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
#include "ApexSDKIntl.h"
#include "ModuleBasicIosImpl.h"
#include "ModuleBasicIosRegistration.h"
#include "ModulePerfScope.h"
#include "ApexUsingNamespace.h"
#include "BasicIosSceneCPU.h"
#if APEX_CUDA_SUPPORT
#include "BasicIosSceneGPU.h"
#endif
#if ENABLE_TEST
#include "BasicIosTestSceneCPU.h"
#if APEX_CUDA_SUPPORT
#include "BasicIosTestSceneGPU.h"
#endif
#endif
#include "BasicIosAssetImpl.h"
#include "BasicIosActorImpl.h"

#include "SceneIntl.h"
#include "ModuleIofxIntl.h"
#include "ModuleFieldSamplerIntl.h"
#include "PsMemoryBuffer.h"

#include "Lock.h"

#include "ReadCheck.h"

using namespace basicios;

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

	/* Setup common module global variables */
	gApexSdk = inSdk;

	ModuleBasicIosImpl* impl = PX_NEW(ModuleBasicIos)(inSdk);
	*niRef  = (ModuleIntl*) impl;
	return (ModuleBase*) impl;
}
#else
/* Statically linking entry function */
void instantiateModuleBasicIos()
{
	ApexSDKIntl* sdk = GetInternalApexSDK();
	basicios::ModuleBasicIosImpl* impl = PX_NEW(basicios::ModuleBasicIosImpl)(sdk);
	sdk->registerExternalModule((Module*) impl, (ModuleIntl*) impl);
}
#endif
}

namespace basicios
{
/* =================== ModuleBasicIosImpl =================== */


AuthObjTypeID BasicIosAssetImpl::mAssetTypeID;

#ifdef WITHOUT_APEX_AUTHORING

class BasicIosAssetDummyAuthoring : public AssetAuthoring, public UserAllocated
{
public:
	BasicIosAssetDummyAuthoring(ModuleBasicIosImpl* module, ResourceList& list, NvParameterized::Interface* params, const char* name)
	{
		PX_UNUSED(module);
		PX_UNUSED(list);
		PX_UNUSED(params);
		PX_UNUSED(name);
	}

	BasicIosAssetDummyAuthoring(ModuleBasicIosImpl* module, ResourceList& list, const char* name)
	{
		PX_UNUSED(module);
		PX_UNUSED(list);
		PX_UNUSED(name);
	}

	BasicIosAssetDummyAuthoring(ModuleBasicIosImpl* module, ResourceList& list)
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
	* \brief Returns the name of this APEX authorable object type
	*/
	virtual const char* getObjTypeName() const
	{
		return BasicIosAssetImpl::getClassName();
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

typedef ApexAuthorableObject<ModuleBasicIosImpl, BasicIosAssetImpl, BasicIosAssetDummyAuthoring> BasicIOSAO;

#else
typedef ApexAuthorableObject<ModuleBasicIosImpl, BasicIosAssetImpl, BasicIosAssetAuthoringImpl> BasicIOSAO;
#endif

ModuleBasicIosImpl::ModuleBasicIosImpl(ApexSDKIntl* sdk)
{
	mSdk = sdk;
	mApiProxy = this;
	mName = "BasicIOS";
	mModuleParams = NULL;
	mIofxModule = NULL;
	mFieldSamplerModule = NULL;

	/* Register this module's authorable object types and create their namespaces */
	const char* pName = BasicIOSAssetParam::staticClassName();
	BasicIOSAO* eAO = PX_NEW(BasicIOSAO)(this, mAuthorableObjects,  pName);
	BasicIosAssetImpl::mAssetTypeID = eAO->getResID();

	/* Register the NvParameterized factories */
	NvParameterized::Traits* traits = mSdk->getParameterizedTraits();
	ModuleBasicIosRegistration::invokeRegistration(traits);
}

AuthObjTypeID ModuleBasicIosImpl::getModuleID() const
{
	return BasicIosAssetImpl::mAssetTypeID;
}

ModuleBasicIosImpl::~ModuleBasicIosImpl()
{
}

void ModuleBasicIosImpl::destroy()
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
		ModuleBasicIosRegistration::invokeUnregistration(traits);
	}

	delete this;
}

void ModuleBasicIosImpl::init(const ModuleBasicIosDesc&)
{
}

void ModuleBasicIosImpl::init(NvParameterized::Interface&)
{
	ModuleBasicIosDesc desc;
	init(desc);
}

NvParameterized::Interface* ModuleBasicIosImpl::getDefaultModuleDesc()
{
	NvParameterized::Traits* traits = mSdk->getParameterizedTraits();

	if (!mModuleParams)
	{
		mModuleParams = DYNAMIC_CAST(BasicIosModuleParameters*)
		                (traits->createNvParameterized("BasicIosModuleParameters"));
		PX_ASSERT(mModuleParams);
	}
	else
	{
		mModuleParams->initDefaults();
	}

	return mModuleParams;
}

uint32_t ModuleBasicIosImpl::forceLoadAssets()
{
	return 0;
}

BasicIosScene* ModuleBasicIosImpl::getBasicIosScene(const Scene& apexScene)
{
	const SceneIntl* niScene = DYNAMIC_CAST(const SceneIntl*)(&apexScene);
	for (uint32_t i = 0 ; i < mBasicIosSceneList.getSize() ; i++)
	{
		BasicIosScene* ps = DYNAMIC_CAST(BasicIosScene*)(mBasicIosSceneList.getResource(i));
		if (ps->mApexScene == niScene)
		{
			return ps;
		}
	}

	PX_ASSERT(!"Unable to locate an appropriate BasicIosScene");
	return NULL;
}

const BasicIosScene* ModuleBasicIosImpl::getBasicIosScene(const Scene& apexScene) const
{
	const SceneIntl* niScene = DYNAMIC_CAST(const SceneIntl*)(&apexScene);
	for (uint32_t i = 0 ; i < mBasicIosSceneList.getSize() ; i++)
	{
		BasicIosScene* ps = DYNAMIC_CAST(BasicIosScene*)(mBasicIosSceneList.getResource(i));
		if (ps->mApexScene == niScene)
		{
			return ps;
		}
	}

	PX_ASSERT(!"Unable to locate an appropriate BasicIosScene");
	return NULL;
}

RenderableIterator* ModuleBasicIosImpl::createRenderableIterator(const Scene& apexScene)
{
	BasicIosScene* ps = getBasicIosScene(apexScene);
	if (ps)
	{
		return ps->createRenderableIterator();
	}

	return NULL;
}

ModuleSceneIntl* ModuleBasicIosImpl::createInternalModuleScene(SceneIntl& scene, RenderDebugInterface* renderDebug)
{
	READ_LOCK(scene);
#if APEX_CUDA_SUPPORT
	if (scene.getTaskManager()->getGpuDispatcher() && scene.isUsingCuda())
	{
#if ENABLE_TEST
		return PX_NEW(BasicIosTestSceneGPU)(*this, scene, renderDebug, mBasicIosSceneList);
#else
		return PX_NEW(BasicIosSceneGPU)(*this, scene, renderDebug, mBasicIosSceneList);
#endif
	}
	else
#endif
	{
#if ENABLE_TEST
		return PX_NEW(BasicIosTestSceneCPU)(*this, scene, renderDebug, mBasicIosSceneList);
#else
		return PX_NEW(BasicIosSceneCPU)(*this, scene, renderDebug, mBasicIosSceneList);
#endif
	}
}

void ModuleBasicIosImpl::releaseModuleSceneIntl(ModuleSceneIntl& scene)
{
	BasicIosScene* ps = DYNAMIC_CAST(BasicIosScene*)(&scene);
	ps->destroy();
}

const char* ModuleBasicIosImpl::getBasicIosTypeName()
{
	READ_ZONE();
	return BasicIosAssetImpl::getClassName();
}


ApexActor* ModuleBasicIosImpl::getApexActor(Actor* nxactor, AuthObjTypeID type) const
{
	if (type == BasicIosAssetImpl::mAssetTypeID)
	{
		return static_cast<BasicIosActorImpl*>(nxactor);
	}

	return NULL;
}

ModuleIofxIntl* ModuleBasicIosImpl::getInternalModuleIofx()
{
	if (!mIofxModule )
	{
		ModuleIntl* nim = mSdk->getInternalModuleByName("IOFX");
		if (nim)
		{
			mIofxModule = DYNAMIC_CAST(ModuleIofxIntl*)(nim);
		}
	}
	return mIofxModule;
}

ModuleFieldSamplerIntl* ModuleBasicIosImpl::getInternalModuleFieldSampler()
{
	if (!mFieldSamplerModule)
	{
		ModuleIntl* nim = mSdk->getInternalModuleByName("FieldSampler");
		if (nim)
		{
			mFieldSamplerModule = DYNAMIC_CAST(ModuleFieldSamplerIntl*)(nim);
		}
	}
	return mFieldSamplerModule;
}

const TestBase* ModuleBasicIosImpl::getTestBase(Scene* apexScene) const
{

	const BasicIosScene* scene = getBasicIosScene(*apexScene);

#if APEX_CUDA_SUPPORT && ENABLE_TEST
	if (scene->getApexScene().getTaskManager()->getGpuDispatcher() && scene->getApexScene().isUsingCuda())
	{
		return static_cast<const TestBase*>(DYNAMIC_CAST(const BasicIosTestSceneGPU*)(scene));
	}
#endif

#if ENABLE_TEST
	return static_cast<const TestBase*>(DYNAMIC_CAST(const BasicIosTestSceneCPU*)(scene));
#else
	PX_UNUSED(scene);
	return 0;
#endif

}

}
} // namespace nvidia
