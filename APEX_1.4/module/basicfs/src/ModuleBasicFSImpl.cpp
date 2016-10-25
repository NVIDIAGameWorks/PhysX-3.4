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

/* === ModuleBasicFSImpl DLL Setup === */

#include "ModuleBasicFSImpl.h"
#include "ModuleBasicFSRegistration.h"
#include "ModulePerfScope.h"
//#include "BasicFSAsset.h"
#include "JetFSAsset.h"
#include "AttractorFSAsset.h"
#include "VortexFSAsset.h"
#include "NoiseFSAsset.h"
#include "WindFSAsset.h"
#include "BasicFSScene.h"
#include "SceneIntl.h"
#include "PsMemoryBuffer.h"
//#include "BasicFSActor.h"
#include "JetFSActorImpl.h"
#include "AttractorFSActorImpl.h"
#include "VortexFSActorImpl.h"
#include "NoiseFSActorImpl.h"
#include "WindFSActorImpl.h"
#include "ModuleFieldSamplerIntl.h"

using namespace basicfs;

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
	ModuleBasicFSImpl* impl = PX_NEW(ModuleBasicFS)(inSdk);
	*niRef  = (ModuleIntl*) impl;
	return (Module*) impl;
}

#else
/* Statically linking entry function */
void instantiateModuleBasicFS()
{
	ApexSDKIntl* sdk = GetInternalApexSDK();
	nvidia::basicfs::ModuleBasicFSImpl* impl = PX_NEW(nvidia::basicfs::ModuleBasicFSImpl)(sdk);
	sdk->registerExternalModule((Module*) impl, (ModuleIntl*) impl);
}
#endif // `defined(_USRDLL)
}

namespace basicfs
{
/* === ModuleBasicFSImpl Implementation === */

#ifdef WITHOUT_APEX_AUTHORING

class BasicFSAssetDummyAuthoring : public AssetAuthoring, public UserAllocated
{
public:
	BasicFSAssetDummyAuthoring(ModuleBasicFSImpl* module, ResourceList& list, NvParameterized::Interface* params, const char* name)
	{
		PX_UNUSED(module);
		PX_UNUSED(list);
		PX_UNUSED(params);
		PX_UNUSED(name);
	}

	BasicFSAssetDummyAuthoring(ModuleBasicFSImpl* module, ResourceList& list, const char* name)
	{
		PX_UNUSED(module);
		PX_UNUSED(list);
		PX_UNUSED(name);
	}

	BasicFSAssetDummyAuthoring(ModuleBasicFSImpl* module, ResourceList& list)
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
		return JetFSAsset::getClassName(); // Fix
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

typedef ApexAuthorableObject<ModuleBasicFSImpl, JetFSAsset, JetFSAssetAuthoring> JetFSAO;
typedef ApexAuthorableObject<ModuleBasicFSImpl, AttractorFSAsset, AttractorFSAssetAuthoring> AttractorFSAO;
typedef ApexAuthorableObject<ModuleBasicFSImpl, VortexFSAsset, VortexFSAssetAuthoring> VortexFSAO;
typedef ApexAuthorableObject<ModuleBasicFSImpl, NoiseFSAsset, NoiseFSAssetAuthoring> NoiseFSAO;
typedef ApexAuthorableObject<ModuleBasicFSImpl, WindFSAsset, WindFSAssetAuthoring> WindFSAO;

#else
typedef ApexAuthorableObject<ModuleBasicFSImpl, JetFSAsset, JetFSAssetAuthoring> JetFSAO;
typedef ApexAuthorableObject<ModuleBasicFSImpl, AttractorFSAsset, AttractorFSAssetAuthoring> AttractorFSAO;
typedef ApexAuthorableObject<ModuleBasicFSImpl, VortexFSAsset, VortexFSAssetAuthoring> VortexFSAO;
typedef ApexAuthorableObject<ModuleBasicFSImpl, NoiseFSAsset, NoiseFSAssetAuthoring> NoiseFSAO;
typedef ApexAuthorableObject<ModuleBasicFSImpl, WindFSAsset, WindFSAssetAuthoring> WindFSAO;
#endif

ModuleBasicFSImpl::ModuleBasicFSImpl(ApexSDKIntl* sdk)
{
	mName = "BasicFS";
	mSdk = sdk;
	mApiProxy = this;
	mModuleParams = NULL;
	mFieldSamplerModule = NULL;

	/* Register asset type and create a namespace for its assets */
	const char* pName1 = JetFSAssetParams::staticClassName();					
	JetFSAO* eAO1 = PX_NEW(JetFSAO)(this, mAuthorableObjects, pName1);
	JetFSAsset::mAssetTypeID = eAO1->getResID();

	const char* pName2 = AttractorFSAssetParams::staticClassName();					
	AttractorFSAO* eAO2 = PX_NEW(AttractorFSAO)(this, mAuthorableObjects, pName2);
	AttractorFSAsset::mAssetTypeID = eAO2->getResID();

	const char* pName3 = NoiseFSAssetParams::staticClassName();					
	NoiseFSAO* eAO3 = PX_NEW(NoiseFSAO)(this, mAuthorableObjects, pName3);
	NoiseFSAsset::mAssetTypeID = eAO3->getResID();

	const char* pName4 = VortexFSAssetParams::staticClassName();					
	VortexFSAO* eAO4 = PX_NEW(VortexFSAO)(this, mAuthorableObjects, pName4);
	VortexFSAsset::mAssetTypeID = eAO4->getResID();

	const char* pName5 = WindFSAssetParams::staticClassName();					
	WindFSAO* eAO5 = PX_NEW(WindFSAO)(this, mAuthorableObjects, pName5);
	WindFSAsset::mAssetTypeID = eAO5->getResID();

	/* Register the NvParameterized factories */
	NvParameterized::Traits* traits = mSdk->getParameterizedTraits();
	ModuleBasicFSRegistration::invokeRegistration(traits);
}

ModuleBasicFSImpl::~ModuleBasicFSImpl()
{
}

void ModuleBasicFSImpl::destroy()
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
		ModuleBasicFSRegistration::invokeUnregistration(traits);
	}
	delete this;
}


void ModuleBasicFSImpl::init(NvParameterized::Interface&)
{
}

NvParameterized::Interface* ModuleBasicFSImpl::getDefaultModuleDesc()
{
	WRITE_ZONE();
	NvParameterized::Traits* traits = mSdk->getParameterizedTraits();

	if (!mModuleParams)
	{
		mModuleParams = DYNAMIC_CAST(BasicFSModuleParameters*)
		                (traits->createNvParameterized("BasicFSModuleParameters"));
		PX_ASSERT(mModuleParams);
	}
	else
	{
		mModuleParams->initDefaults();
	}

	return mModuleParams;
}

AuthObjTypeID ModuleBasicFSImpl::getJetFSAssetTypeID() const
{
	return JetFSAsset::mAssetTypeID;
}

AuthObjTypeID ModuleBasicFSImpl::getAttractorFSAssetTypeID() const
{
	return AttractorFSAsset::mAssetTypeID;
}

AuthObjTypeID ModuleBasicFSImpl::getVortexFSAssetTypeID() const
{
	return VortexFSAsset::mAssetTypeID;
}

AuthObjTypeID ModuleBasicFSImpl::getNoiseFSAssetTypeID() const
{
	return NoiseFSAsset::mAssetTypeID;
}

AuthObjTypeID ModuleBasicFSImpl::getWindFSAssetTypeID() const
{
	return WindFSAsset::mAssetTypeID;
}

AuthObjTypeID ModuleBasicFSImpl::getModuleID() const
{
	return JetFSAsset::mAssetTypeID; // What should return?
}

ApexActor* ModuleBasicFSImpl::getApexActor(Actor* nxactor, AuthObjTypeID type) const
{
	if (type == JetFSAsset::mAssetTypeID)
	{
		return static_cast<JetFSActorImpl*>(nxactor);
	}
	else if (type == AttractorFSAsset::mAssetTypeID)
	{
		return static_cast<AttractorFSActorImpl*>(nxactor);
	}
	else if (type == VortexFSAsset::mAssetTypeID)
	{
		return static_cast<VortexFSActorImpl*>(nxactor);
	}
	else if (type == NoiseFSAsset::mAssetTypeID)
	{
		return static_cast<NoiseFSActorImpl*>(nxactor);
	}
	else if (type == WindFSAsset::mAssetTypeID)
	{
		return static_cast<WindFSActorImpl*>(nxactor);
	}

	return NULL;
}

/* == Example Scene methods == */
ModuleSceneIntl* ModuleBasicFSImpl::createInternalModuleScene(SceneIntl& scene, RenderDebugInterface* debugRender)
{
#if APEX_CUDA_SUPPORT
	READ_LOCK(scene);
	if (scene.getTaskManager()->getGpuDispatcher())
	{
		return PX_NEW(BasicFSSceneGPU)(*this, scene, debugRender, mBasicFSScenes);
	}
	else
#endif
		return PX_NEW(BasicFSSceneCPU)(*this, scene, debugRender, mBasicFSScenes);
}

void ModuleBasicFSImpl::releaseModuleSceneIntl(ModuleSceneIntl& scene)
{
	BasicFSScene* es = DYNAMIC_CAST(BasicFSScene*)(&scene);
	es->destroy();
}

BasicFSScene* ModuleBasicFSImpl::getBasicFSScene(const Scene& apexScene)
{
	for (uint32_t i = 0 ; i < mBasicFSScenes.getSize() ; i++)
	{
		BasicFSScene* es = DYNAMIC_CAST(BasicFSScene*)(mBasicFSScenes.getResource(i));
		if (es->mApexScene == &apexScene)
		{
			return es;
		}
	}

	PX_ASSERT(!"Unable to locate an appropriate BasicFSScene");
	return NULL;
}

RenderableIterator* ModuleBasicFSImpl::createRenderableIterator(const Scene& apexScene)
{
	WRITE_ZONE();
	BasicFSScene* es = getBasicFSScene(apexScene);
	if (es)
	{
		return es->createRenderableIterator();
	}

	return NULL;
}

ModuleFieldSamplerIntl* ModuleBasicFSImpl::getInternalModuleFieldSampler()
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
} // end namespace nvidia::apex


