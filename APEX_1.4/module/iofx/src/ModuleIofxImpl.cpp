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
#include "ApexUsingNamespace.h"
#include "ApexSDKIntl.h"
#include "ModuleIofxImpl.h"
#include "ModuleIofxRegistration.h"
#include "IofxAssetImpl.h"
#include "SceneIntl.h"
#include "PsMemoryBuffer.h"
#include "IofxSceneCPU.h"
#include "IofxSceneGPU.h"
#include "IofxActorImpl.h"
#include "RenderVolumeImpl.h"

#include "ModulePerfScope.h"
using namespace iofx;

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

	/* Setup common module global variables */
	gApexSdk = inSdk;
	ModuleIofxImpl* impl = PX_NEW(ModuleIofx)(inSdk);
	*niRef  = (ModuleIntl*) impl;
	return (Module*) impl;
}
#else
/* Statically linking entry function */
void instantiateModuleIofx()
{
	ApexSDKIntl* sdk = GetInternalApexSDK();
	iofx::ModuleIofxImpl* impl = PX_NEW(iofx::ModuleIofxImpl)(sdk);
	sdk->registerExternalModule((Module*) impl, (ModuleIntl*) impl);
}
#endif
}

namespace iofx
{
/* =================== ModuleIofxImpl =================== */


AuthObjTypeID IofxAssetImpl::mAssetTypeID;
#ifdef WITHOUT_APEX_AUTHORING

class IofxAssetDummyAuthoring : public AssetAuthoring, public UserAllocated
{
public:
	IofxAssetDummyAuthoring(ModuleIofxImpl* module, ResourceList& list, NvParameterized::Interface* params, const char* name)
	{
		PX_UNUSED(module);
		PX_UNUSED(list);
		PX_UNUSED(params);
		PX_UNUSED(name);
	}

	IofxAssetDummyAuthoring(ModuleIofxImpl* module, ResourceList& list, const char* name)
	{
		PX_UNUSED(module);
		PX_UNUSED(list);
		PX_UNUSED(name);
	}

	IofxAssetDummyAuthoring(ModuleIofxImpl* module, ResourceList& list)
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
		return IOFX_AUTHORING_TYPE_NAME;
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

typedef ApexAuthorableObject<ModuleIofxImpl, IofxAssetImpl, IofxAssetDummyAuthoring> IofxAO;

#else
typedef ApexAuthorableObject<ModuleIofxImpl, IofxAssetImpl, IofxAssetAuthoringImpl> IofxAO;
#endif

ModuleIofxImpl::ModuleIofxImpl(ApexSDKIntl* sdk)
{
	mSdk = sdk;
	mApiProxy = this;
	mName = "IOFX";
	mModuleParams = NULL;
	mInteropDisabled = false;
	mCudaDisabled = false;
	mDeferredDisabled = false;

	/* Register this module's authorable object types and create their namespaces */
	const char* pName = IofxAssetParameters::staticClassName();
	IofxAO* AO = PX_NEW(IofxAO)(this, mAuthorableObjects, pName);
	IofxAssetImpl::mAssetTypeID = AO->getResID();

	/* Register the NvParameterized factories */
	NvParameterized::Traits* traits = mSdk->getParameterizedTraits();
	ModuleIofxRegistration::invokeRegistration(traits);
}

AuthObjTypeID ModuleIofxImpl::getModuleID() const
{
	READ_ZONE();
	return IofxAssetImpl::mAssetTypeID;
}

ModuleIofxImpl::~ModuleIofxImpl()
{
}

void ModuleIofxImpl::destroy()
{
	/* Remove the NvParameterized factories */
	NvParameterized::Traits* traits = mSdk->getParameterizedTraits();

	if (mModuleParams)
	{
		mModuleParams->destroy();
		mModuleParams = NULL;
	}

	ModuleBase::destroy();

	if (traits)
	{
		ModuleIofxRegistration::invokeUnregistration(traits);
	}
	delete this;
}

NvParameterized::Interface* ModuleIofxImpl::getDefaultModuleDesc()
{
	READ_ZONE();
	NvParameterized::Traits* traits = mSdk->getParameterizedTraits();

	if (!mModuleParams)
	{
		mModuleParams = DYNAMIC_CAST(IofxModuleParameters*)
		                (traits->createNvParameterized("IofxModuleParameters"));
		PX_ASSERT(mModuleParams);
	}
	else
	{
		mModuleParams->initDefaults();
	}

	return mModuleParams;
}

void ModuleIofxImpl::init(const ModuleIofxDesc&)
{
	WRITE_ZONE();
}

uint32_t ModuleIofxImpl::forceLoadAssets()
{
	uint32_t loadedAssetCount = 0;

	for (uint32_t i = 0; i < mAuthorableObjects.getSize(); i++)
	{
		AuthorableObjectIntl* ao = static_cast<AuthorableObjectIntl*>(mAuthorableObjects.getResource(i));
		loadedAssetCount += ao->forceLoadAssets();
	}
	return loadedAssetCount;
}

ModuleSceneIntl* ModuleIofxImpl::createInternalModuleScene(SceneIntl& scene, RenderDebugInterface* debugRender)
{
	IofxScene* res = NULL;
#if APEX_CUDA_SUPPORT
	PxGpuDispatcher* gd;
	{
		READ_LOCK(scene);
		gd = scene.getTaskManager()->getGpuDispatcher();
	}

	if (gd && gd->getCudaContextManager()->contextIsValid())
	{
		res = PX_NEW(IofxSceneGPU)(*this, scene, debugRender, mIofxScenes);
	}
	else
#endif
	{
		res = PX_NEW(IofxSceneCPU)(*this, scene, debugRender, mIofxScenes);
	}
	return res;
}

void ModuleIofxImpl::releaseModuleSceneIntl(ModuleSceneIntl& scene)
{
	IofxScene* is = DYNAMIC_CAST(IofxScene*)(&scene);
	is->destroy();
}

IofxScene* ModuleIofxImpl::getIofxScene(const Scene& apexScene)
{
	for (uint32_t i = 0 ; i < mIofxScenes.getSize() ; i++)
	{
		IofxScene* is = DYNAMIC_CAST(IofxScene*)(mIofxScenes.getResource(i));
		if (is->mApexScene == &apexScene)
		{
			return is;
		}
	}

	PX_ASSERT(!"Unable to locate an appropriate IofxScene");
	return NULL;
}

const IofxScene* ModuleIofxImpl::getIofxScene(const Scene& apexScene) const
{
	for (uint32_t i = 0 ; i < mIofxScenes.getSize() ; i++)
	{
		IofxScene* is = DYNAMIC_CAST(IofxScene*)(mIofxScenes.getResource(i));
		if (is->mApexScene == &apexScene)
		{
			return is;
		}
	}

	PX_ASSERT(!"Unable to locate an appropriate IofxScene");
	return NULL;
}

RenderVolume* ModuleIofxImpl::createRenderVolume(const Scene& apexScene, const PxBounds3& b, uint32_t priority, bool allIofx)
{
	WRITE_ZONE();
	IofxScene* is = getIofxScene(apexScene);
	if (is)
	{
		return PX_NEW(RenderVolumeImpl)(*is, b, priority, allIofx);
	}

	return NULL;
}

void ModuleIofxImpl::releaseRenderVolume(RenderVolume& volume)
{
	WRITE_ZONE();
	RenderVolumeImpl* arv = DYNAMIC_CAST(RenderVolumeImpl*)(&volume);
	arv->destroy();
}


IofxManagerIntl* ModuleIofxImpl::createActorManager(const Scene& scene, const IofxAsset& asset, const IofxManagerDescIntl& desc)
{
	WRITE_ZONE();
	IofxScene* is = getIofxScene(scene);
	return is ? is->createIofxManager(asset, desc) : NULL;
}

const TestBase* ModuleIofxImpl::getTestBase(Scene* apexScene) const
{
#if ENABLE_TEST
	const IofxScene* scene = getIofxScene(*apexScene);
	return static_cast<const TestBase*>(DYNAMIC_CAST(const IofxTestScene*)(scene));
#else
	PX_UNUSED(apexScene);
	return 0;
#endif
}

bool ModuleIofxImpl::setIofxRenderCallback(const Scene& apexScene, IofxRenderCallback* callback)
{
	WRITE_ZONE();
	IofxScene* is = getIofxScene(apexScene);
	if (is)
	{
		is->setIofxRenderCallback(callback);
		return true;
	}
	return false;
}

IofxRenderCallback* ModuleIofxImpl::getIofxRenderCallback(const Scene& apexScene) const
{
	READ_ZONE();
	const IofxScene* is = getIofxScene(apexScene);
	if (is)
	{
		return is->getIofxRenderCallback();
	}
	return NULL;
}

IofxRenderableIterator* ModuleIofxImpl::createIofxRenderableIterator(const Scene& apexScene)
{
	WRITE_ZONE();
	IofxScene* is = getIofxScene(apexScene);
	if (is)
	{
		return is->createIofxRenderableIterator();
	}

	return NULL;
}

void ModuleIofxImpl::prepareRenderables(const Scene& apexScene)
{
	WRITE_ZONE();
	URR_SCOPE;
	IofxScene* is = getIofxScene(apexScene);
	if (is)
	{
		is->prepareRenderables();
	}
}

}
} // namespace nvidia
