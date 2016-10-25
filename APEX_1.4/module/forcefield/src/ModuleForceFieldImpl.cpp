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
#include "ModuleForceFieldImpl.h"
#include "ModuleForceFieldRegistration.h"
#include "ForceFieldScene.h"
#include "ForceFieldAssetImpl.h"
#include "SceneIntl.h"
#include "PsMemoryBuffer.h"
#include "ModulePerfScope.h"
#include "ModuleFieldSamplerIntl.h"
using namespace forcefield;

#include "Apex.h"
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
	ModuleForceFieldImpl* impl = PX_NEW(ModuleForceField)(inSdk);
	*niRef  = (ModuleIntl*) impl;
	return (Module*) impl;
}

#else
/* Statically linking entry function */
void instantiateModuleForceField()
{
#if SDK_VERSION_NUMBER >= MIN_PHYSX_SDK_VERSION_REQUIRED
	ApexSDKIntl* sdk = GetInternalApexSDK();
	nvidia::forcefield::ModuleForceFieldImpl* impl = PX_NEW(nvidia::forcefield::ModuleForceFieldImpl)(sdk);
	sdk->registerExternalModule((Module*) impl, (ModuleIntl*) impl);
#endif
}
#endif // `defined(_USRDLL)
}

namespace forcefield
{
/* === ModuleForceFieldImpl Implementation === */

AuthObjTypeID ForceFieldAssetImpl::mAssetTypeID;  // Static class member of ForceFieldAssetImpl
#ifdef WITHOUT_APEX_AUTHORING

class ForceFieldAssetDummyAuthoring : public AssetAuthoring, public UserAllocated
{
public:
	ForceFieldAssetDummyAuthoring(ModuleForceFieldImpl* module, ResourceList& list, NvParameterized::Interface* params, const char* name)
	{
		PX_UNUSED(module);
		PX_UNUSED(list);
		PX_UNUSED(params);
		PX_UNUSED(name);
	}

	ForceFieldAssetDummyAuthoring(ModuleForceFieldImpl* module, ResourceList& list, const char* name)
	{
		PX_UNUSED(module);
		PX_UNUSED(list);
		PX_UNUSED(name);
	}

	ForceFieldAssetDummyAuthoring(ModuleForceFieldImpl* module, ResourceList& list)
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

	const char* getName(void) const
	{
		return NULL;
	}

	/**
	* \brief Returns the name of this APEX authorable object type
	*/
	virtual const char* getObjTypeName() const
	{
		return ForceFieldAssetImpl::getClassName();
	}

	/**
	 * \brief Prepares a fully authored Asset Authoring object for a specified platform
	*/
	virtual bool prepareForPlatform(nvidia::apex::PlatformTag)
	{
		PX_ASSERT(0);
		return false;
	}

	/**
	* \brief Save asset's NvParameterized interface, may return NULL
	*/
	virtual NvParameterized::Interface* getNvParameterized()
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

typedef ApexAuthorableObject<ModuleForceFieldImpl, ForceFieldAssetImpl, ForceFieldAssetDummyAuthoring> ForceFieldAO;

#else
typedef ApexAuthorableObject<ModuleForceFieldImpl, ForceFieldAssetImpl, ForceFieldAssetAuthoringImpl> ForceFieldAO;
#endif

ModuleForceFieldImpl::ModuleForceFieldImpl(ApexSDKIntl* inSdk)
{
	mName = "ForceField";
	mSdk = inSdk;
	mApiProxy = this;
	mModuleParams = NULL;
	mFieldSamplerModule = NULL;

	/* Register asset type and create a namespace for its assets */
	const char* pName = ForceFieldAssetParams::staticClassName();
	ForceFieldAO* eAO = PX_NEW(ForceFieldAO)(this, mAuthorableObjects, pName);
	ForceFieldAssetImpl::mAssetTypeID = eAO->getResID();

	/* Register the NvParameterized factories */
	NvParameterized::Traits* traits = mSdk->getParameterizedTraits();
	ModuleForceFieldRegistration::invokeRegistration(traits);
}

ModuleForceFieldImpl::~ModuleForceFieldImpl()
{
}

void ModuleForceFieldImpl::destroy()
{
	// release the NvParameterized factory
	NvParameterized::Traits* traits = mSdk->getParameterizedTraits();

	if (mModuleParams)
	{
		mModuleParams->destroy();
		mModuleParams = NULL;
	}

	ModuleBase::destroy();

	if (traits)
	{
		ModuleForceFieldRegistration::invokeUnregistration(traits);
	}
	delete this;
}

NvParameterized::Interface* ModuleForceFieldImpl::getDefaultModuleDesc()
{
	NvParameterized::Traits* traits = mSdk->getParameterizedTraits();

	if (!mModuleParams)
	{
		mModuleParams = DYNAMIC_CAST(ForceFieldModuleParams*)
		                (traits->createNvParameterized("ForceFieldModuleParams"));
		PX_ASSERT(mModuleParams);
	}
	else
	{
		mModuleParams->initDefaults();
	}

	return mModuleParams;
}

void ModuleForceFieldImpl::init(const ModuleForceFieldDesc& expDesc)
{
	PX_PROFILE_ZONE("ForceFieldModuleInit", GetInternalApexSDK()->getContextId());  // profile this function
	mModuleValue = expDesc.moduleValue;
}

AuthObjTypeID ModuleForceFieldImpl::getForceFieldAssetTypeID() const
{
	return ForceFieldAssetImpl::mAssetTypeID;
}
AuthObjTypeID ModuleForceFieldImpl::getModuleID() const
{
	return ForceFieldAssetImpl::mAssetTypeID;
}


/* == ForceField Scene methods == */
ModuleSceneIntl* ModuleForceFieldImpl::createInternalModuleScene(SceneIntl& scene, RenderDebugInterface* renderDebug)
{
#if APEX_CUDA_SUPPORT
	READ_LOCK(scene);
	if (scene.getTaskManager()->getGpuDispatcher())
	{
		return PX_NEW(ForceFieldSceneGPU)(*this, scene, renderDebug, mForceFieldScenes);
	}
	else
#endif
		return PX_NEW(ForceFieldSceneCPU)(*this, scene, renderDebug, mForceFieldScenes);
}

void ModuleForceFieldImpl::releaseModuleSceneIntl(ModuleSceneIntl& scene)
{
	ForceFieldScene* es = DYNAMIC_CAST(ForceFieldScene*)(&scene);
	es->destroy();
}

uint32_t ModuleForceFieldImpl::forceLoadAssets()
{
	uint32_t loadedAssetCount = 0;
	for (uint32_t i = 0; i < mAuthorableObjects.getSize(); i++)
	{
		AuthorableObjectIntl* ao = static_cast<AuthorableObjectIntl*>(mAuthorableObjects.getResource(i));
		loadedAssetCount += ao->forceLoadAssets();
	}
	return loadedAssetCount;
}

ForceFieldScene* ModuleForceFieldImpl::getForceFieldScene(const Scene& apexScene)
{
	for (uint32_t i = 0 ; i < mForceFieldScenes.getSize() ; i++)
	{
		ForceFieldScene* es = DYNAMIC_CAST(ForceFieldScene*)(mForceFieldScenes.getResource(i));
		if (es->mApexScene == &apexScene)
		{
			return es;
		}
	}

	PX_ASSERT(!"Unable to locate an appropriate ForceFieldScene");
	return NULL;
}

RenderableIterator* ModuleForceFieldImpl::createRenderableIterator(const Scene& apexScene)
{
	ForceFieldScene* es = getForceFieldScene(apexScene);
	if (es)
	{
		return es->createRenderableIterator();
	}

	return NULL;
}

ModuleFieldSamplerIntl* ModuleForceFieldImpl::getInternalModuleFieldSampler()
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

}
} // end namespace nvidia
