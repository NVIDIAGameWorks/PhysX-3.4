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

#define SAFE_MODULE_RELEASE(x) if ( x ) { ModuleIntl *m = mSdk->getInternalModule(x); PX_ASSERT(m); m->setParent(NULL); x->release(); x = NULL; }

/* === ModuleParticlesImpl DLL Setup === */

#pragma warning(disable:4505)

#include "ModuleParticlesImpl.h"
#include "ModuleParticlesRegistration.h"
#include "ModulePerfScope.h"
#include "ParticlesScene.h"
#include "SceneIntl.h"
#include "PxMaterial.h"
#include "ModuleTurbulenceFS.h"
#include "PsMemoryBuffer.h"

#include "ApexSDKIntl.h"
#include "ApexUsingNamespace.h"
#include "Apex.h"
#include "nvparameterized/NvParamUtils.h"
#include "ApexEmitterAssetParameters.h"
#include "EmitterGeomSphereParams.h"
#include "IofxAssetParameters.h"
#include "SpriteIofxParameters.h"
#include "ViewDirectionSortingModifierParams.h"
#include "SimpleScaleModifierParams.h"
#include "InitialColorModifierParams.h"
#include "TurbulenceFSAssetParams.h"
#include "BasicIOSAssetParam.h"
#include "EmitterAsset.h"
#include "GroundEmitterAsset.h"
#include "ImpactEmitterAsset.h"
#include "IofxAsset.h"
#include "BasicIosAsset.h"
#include "ParticleIosAsset.h"
#include "TurbulenceFSAsset.h"
#include "EmitterGeomBoxParams.h"
#include "RandomScaleModifierParams.h"
#include "RandomRotationModifierParams.h"
#include "ColorVsLifeCompositeModifierParams.h"
#include "ScaleVsLife2DModifierParams.h"
#include "FloatMath.h"
#include "JetFSActor.h"
#include "WindFSActor.h"
#include "AttractorFSActor.h"
#include "BasicFSAsset.h"
#include "JetFSAssetParams.h"
#include "WindFSAssetParams.h"
#include "NoiseFSAssetParams.h"
#include "VortexFSAssetParams.h"
#include "OrientScaleAlongScreenVelocityModifierParams.h"
#include "RandomRotationModifierParams.h"
#include "RotationRateModifierParams.h"
#include "RotationRateVsLifeModifierParams.h"
#include "EffectPackageActorImpl.h"
#include "AttractorFSAssetParams.h"
#include "FluidParticleSystemParams.h"
#include "ModuleParticleIosRegistration.h"
#include "MeshIofxParameters.h"
#include "SimpleScaleModifierParams.h"
#include "RotationModifierParams.h"
#include "ScaleVsLife3DModifierParams.h"
#include "ForceFieldAssetParams.h"
#include "RadialForceFieldKernelParams.h"
#include "ForceFieldFalloffParams.h"
#include "ForceFieldNoiseParams.h"
#include "HeatSourceAsset.h"
#include "SubstanceSourceAsset.h"
#include "VelocitySourceAsset.h"
#include "ForceFieldAsset.h"
#include "EffectPackageAssetParams.h"
#include "FlameEmitterAsset.h"
#include "FlameEmitterAssetParams.h"

#include "ViewDirectionSortingModifierParams.h"
#include "RandomSubtextureModifierParams.h"
#include "UserOpaqueMesh.h"
#include "HeatSourceAssetParams.h"
#include "HeatSourceGeomSphereParams.h"
#include "SubstanceSourceAssetParams.h"
#include "VelocitySourceAssetParams.h"
#include "PxPhysics.h"
#include "EffectPackageData.h"

#define SAFE_DESTROY(x) if ( x ) { x->destroy(); x = NULL; }


#if PX_X86

#define LINK_KERNEL(name) "/include:_"#name
#define LINK_S2_KERNEL(name) "/include:_"#name"_templ$0 /include:_"#name"_templ$1"

#elif PX_X64

#define LINK_KERNEL(name) "/include:"#name
#define LINK_S2_KERNEL(name) "/include:"#name"_templ$0 /include:"#name"_templ$1"

#endif

#if APEX_CUDA_SUPPORT

#pragma comment(linker, LINK_S2_KERNEL(BasicFS_fieldSamplerGridKernel))
#pragma comment(linker, LINK_S2_KERNEL(BasicFS_fieldSamplerPointsKernel))

#pragma comment(linker, LINK_KERNEL(BasicIOS_compactKernel))
#pragma comment(linker, LINK_KERNEL(BasicIOS_histogramKernel))
#pragma comment(linker, LINK_KERNEL(BasicIOS_mergeHistogramKernel))
#pragma comment(linker, LINK_KERNEL(BasicIOS_reduceKernel))
#pragma comment(linker, LINK_KERNEL(BasicIOS_scanKernel))
#pragma comment(linker, LINK_S2_KERNEL(BasicIOS_simulateApplyFieldKernel))
#pragma comment(linker, LINK_S2_KERNEL(BasicIOS_simulateKernel))
#pragma comment(linker, LINK_KERNEL(BasicIOS_stateKernel))

#pragma comment(linker, LINK_KERNEL(FieldSampler_applyParticlesKernel))
#pragma comment(linker, LINK_KERNEL(FieldSampler_clearGridKernel))
#pragma comment(linker, LINK_KERNEL(FieldSampler_clearKernel))
#pragma comment(linker, LINK_KERNEL(FieldSampler_composeKernel))

#pragma comment(linker, LINK_S2_KERNEL(ForceField_fieldSamplerGridKernel))
#pragma comment(linker, LINK_S2_KERNEL(ForceField_fieldSamplerPointsKernel))

#pragma comment(linker, LINK_KERNEL(IOFX_actorRangeKernel))
#pragma comment(linker, LINK_KERNEL(IOFX_bboxKernel))
#pragma comment(linker, LINK_S2_KERNEL(IOFX_meshModifiersKernel))
#pragma comment(linker, LINK_KERNEL(IOFX_newRadixSortBlockKernel))
#pragma comment(linker, LINK_KERNEL(IOFX_newRadixSortStepKernel))
#pragma comment(linker, LINK_KERNEL(IOFX_radixSortStepKernel))
#pragma comment(linker, LINK_S2_KERNEL(IOFX_makeSortKeys))
#pragma comment(linker, LINK_S2_KERNEL(IOFX_remapKernel))
#pragma comment(linker, LINK_S2_KERNEL(IOFX_spriteModifiersKernel))
#pragma comment(linker, LINK_S2_KERNEL(IOFX_spriteTextureModifiersKernel))
#pragma comment(linker, LINK_S2_KERNEL(IOFX_volumeMigrationKernel))

#pragma comment(linker, LINK_KERNEL(ParticleIOS_compactKernel))
#pragma comment(linker, LINK_KERNEL(ParticleIOS_histogramKernel))
#pragma comment(linker, LINK_KERNEL(ParticleIOS_mergeHistogramKernel))
#pragma comment(linker, LINK_KERNEL(ParticleIOS_reduceKernel))
#pragma comment(linker, LINK_KERNEL(ParticleIOS_scanKernel))
#pragma comment(linker, LINK_S2_KERNEL(ParticleIOS_simulateApplyFieldKernel))
#pragma comment(linker, LINK_S2_KERNEL(ParticleIOS_simulateKernel))
#pragma comment(linker, LINK_KERNEL(ParticleIOS_stateKernel))

#endif

namespace nvidia
{
namespace apex
{


void instantiateModuleBasicIos();
void instantiateModuleEmitter();
void instantiateModuleIofx();
void instantiateModuleFieldSampler();
void instantiateModuleParticleIos();
void instantiateModuleForceField();
void instantiateModuleBasicFS();

static const char* getAuthoringTypeName(const char* className)
{
	const char* ret = NULL;

	if (nvidia::strcmp(className, "BasicIOSAssetParam") == 0)
	{
		ret = BASIC_IOS_AUTHORING_TYPE_NAME;
	}
	else if (nvidia::strcmp(className, "IofxAssetParameters") == 0)
	{
		ret = IOFX_AUTHORING_TYPE_NAME;
	}
	else if (nvidia::strcmp(className, "ParticleIosAssetParam") == 0)
	{
		ret = PARTICLE_IOS_AUTHORING_TYPE_NAME;
	}


	PX_ASSERT(ret);
	return ret;
}

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

static PxTransform getPose(float x, float y, float z, float rotX, float rotY, float rotZ)
{
	PxTransform ret;
	ret.p = PxVec3(x, y, z);
	fm_eulerToQuat(rotX * FM_DEG_TO_RAD, rotY * FM_DEG_TO_RAD, rotZ * FM_DEG_TO_RAD, &ret.q.x);
	return ret;
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
	particles::ModuleParticlesImpl* impl = PX_NEW(particles::ModuleParticlesImpl)(inSdk);
	*niRef  = (ModuleIntl*) impl;
	return (Module*) impl;
}

#else
/* Statically linking entry function */
void instantiateModuleParticles()
{
	ApexSDKIntl* sdk = GetInternalApexSDK();
	particles::ModuleParticlesImpl* impl = PX_NEW(particles::ModuleParticlesImpl)(sdk);
	sdk->registerExternalModule((Module*) impl, (ModuleIntl*) impl);
}
#endif // `defined(_USRDLL)

}

namespace particles
{

/* === ModuleParticlesImpl Implementation === */

#ifdef WITHOUT_APEX_AUTHORING

class ParticlesAssetDummyAuthoring : public AssetAuthoring, public UserAllocated
{
public:
	ParticlesAssetDummyAuthoring(ModuleParticlesImpl* module, ResourceList& list, NvParameterized::Interface* params, const char* name)
	{
		PX_UNUSED(module);
		PX_UNUSED(list);
		PX_UNUSED(params);
		PX_UNUSED(name);
	}

	ParticlesAssetDummyAuthoring(ModuleParticlesImpl* module, ResourceList& list, const char* name)
	{
		PX_UNUSED(module);
		PX_UNUSED(list);
		PX_UNUSED(name);
	}

	ParticlesAssetDummyAuthoring(ModuleParticlesImpl* module, ResourceList& list)
	{
		PX_UNUSED(module);
		PX_UNUSED(list);
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

	virtual void setToolString(const char* toolName, const char* toolVersion, uint32_t toolChangelist)
	{
		PX_ALWAYS_ASSERT();
		PX_UNUSED(toolName);
		PX_UNUSED(toolVersion);
		PX_UNUSED(toolChangelist);
	}
};

typedef ApexAuthorableObject<ModuleParticlesImpl, ParticlesAsset, ParticlesAssetDummyAuthoring> ParticlesAO;

#else
typedef ApexAuthorableObject<ModuleParticlesImpl, ParticlesAsset, ParticlesAssetAuthoring> ParticlesAO;
#endif


AuthObjTypeID EffectPackageAssetImpl::mAssetTypeID;  // Static class member of ParticlesAsset

#ifdef WITHOUT_APEX_AUTHORING

class EffectPackageAssetDummyAuthoring : public AssetAuthoring, public UserAllocated
{
public:
	EffectPackageAssetDummyAuthoring(ModuleParticlesImpl* module, ResourceList& list, NvParameterized::Interface* params, const char* name)
	{
		PX_UNUSED(module);
		PX_UNUSED(list);
		PX_UNUSED(params);
		PX_UNUSED(name);
	}

	EffectPackageAssetDummyAuthoring(ModuleParticlesImpl* module, ResourceList& list, const char* name)
	{
		PX_UNUSED(module);
		PX_UNUSED(list);
		PX_UNUSED(name);
	}

	EffectPackageAssetDummyAuthoring(ModuleParticlesImpl* module, ResourceList& list)
	{
		PX_UNUSED(module);
		PX_UNUSED(list);
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
		return EffectPackageAssetImpl::getClassName();
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

	virtual void setToolString(const char* toolName, const char* toolVersion, uint32_t toolChangelist)
	{
		PX_ALWAYS_ASSERT();
		PX_UNUSED(toolName);
		PX_UNUSED(toolVersion);
		PX_UNUSED(toolChangelist);
	}
};

typedef ApexAuthorableObject<ModuleParticlesImpl, EffectPackageAssetImpl, EffectPackageAssetDummyAuthoring> EffectPackageAO;

#else
typedef ApexAuthorableObject<ModuleParticlesImpl, EffectPackageAssetImpl, EffectPackageAssetAuthoringImpl> EffectPackageAO;
#endif


//********************************************************************

#define MODULE_PARENT(x) if ( x ) { ModuleIntl *m = mSdk->getInternalModule(x); PX_ASSERT(m); m->setParent(this); m->setCreateOk(false); }

ModuleParticlesImpl::ModuleParticlesImpl(ApexSDKIntl* inSdk)
{
	mSdk = inSdk;

	PxPhysics *sdk = mSdk->getPhysXSDK();
	mDefaultMaterial = sdk->createMaterial(1, 1, 1);

	instantiateModuleBasicIos();			// Instantiate the BasicIOS module statically
	mModuleBasicIos = mSdk->createModule("BasicIOS");
	PX_ASSERT(mModuleBasicIos);
	MODULE_PARENT(mModuleBasicIos);

	instantiateModuleEmitter();				// Instantiate the Emitter module statically
	mModuleEmitter = mSdk->createModule("Emitter");
	PX_ASSERT(mModuleEmitter);
	MODULE_PARENT(mModuleEmitter);

	instantiateModuleIofx();				// Instantiate the IOFX module statically
	mModuleIofx = mSdk->createModule("IOFX");
	PX_ASSERT(mModuleIofx);
	MODULE_PARENT(mModuleIofx);

	instantiateModuleFieldSampler();		// Instantiate the field sampler module statically
	mModuleFieldSampler = mSdk->createModule("FieldSampler");
	PX_ASSERT(mModuleFieldSampler);
	MODULE_PARENT(mModuleFieldSampler);

	instantiateModuleBasicFS();				// Instantiate the BasicFS module statically
	mModuleBasicFS = mSdk->createModule("BasicFS");
	PX_ASSERT(mModuleBasicFS);
	MODULE_PARENT(mModuleBasicFS);

	instantiateModuleParticleIos();			// PhysX 3.x only : Instantiate the ParticleIOS module
	mModuleParticleIos = mSdk->createModule("ParticleIOS");
	PX_ASSERT(mModuleParticleIos);
	MODULE_PARENT(mModuleParticleIos);

	instantiateModuleForceField();			// PhysX 3.x only : Instantiate the ForceField module
	mModuleForceField = mSdk->createModule("ForceField");
	PX_ASSERT(mModuleForceField);
	MODULE_PARENT(mModuleForceField);


	mName = "Particles";
	mApiProxy = this;
	mModuleParams = NULL;
	mTurbulenceModule = NULL;
	mGraphicsMaterialsDatabase = NULL;
	mEnableScreenCulling = false;
	mZnegative = false;
	mUseEmitterPool = false;

	mEffectPackageIOSDatabaseParams = NULL;
	mEffectPackageIOFXDatabaseParams = NULL;
	mEffectPackageEmitterDatabaseParams = NULL;
	mEffectPackageDatabaseParams = NULL;
	mEffectPackageFieldSamplerDatabaseParams = NULL;

	{
		/* Register asset type and create a namespace for it's assets */
		const char* pName = EffectPackageAssetParams::staticClassName();
		EffectPackageAO* eAO = PX_NEW(EffectPackageAO)(this, mEffectPackageAuthorableObjects, pName);
		EffectPackageAssetImpl::mAssetTypeID = eAO->getResID();
	}

	/* Register the NvParameterized factories */
	NvParameterized::Traits* traits = mSdk->getParameterizedTraits();
	ModuleParticlesRegistration::invokeRegistration(traits);

	{
		uint32_t count = mSdk->getNbModules();
		Module** modules = mSdk->getModules();
		for (uint32_t i = 0; i < count; i++)
		{
			Module* m = modules[i];
			const char* name = m->getName();
			if (nvidia::strcmp(name, "TurbulenceFS") == 0)
			{
				mTurbulenceModule = static_cast< ModuleTurbulenceFS*>(m);
				break;
			}
		}
	}
}

ModuleParticlesImpl::~ModuleParticlesImpl()
{
	if ( mDefaultMaterial )
	{
		mDefaultMaterial->release();
	}
}

void ModuleParticlesImpl::destroy()
{
	// release the NvParameterized factory
	NvParameterized::Traits* traits = mSdk->getParameterizedTraits();

	if (mModuleParams)
	{
		mModuleParams->destroy();
		mModuleParams = NULL;
	}

	SAFE_DESTROY(mEffectPackageIOSDatabaseParams);
	SAFE_DESTROY(mEffectPackageIOFXDatabaseParams);
	SAFE_DESTROY(mEffectPackageEmitterDatabaseParams);
	SAFE_DESTROY(mEffectPackageDatabaseParams);
	SAFE_DESTROY(mEffectPackageFieldSamplerDatabaseParams);
	SAFE_DESTROY(mGraphicsMaterialsDatabase);

	ModuleBase::destroy();

	if (traits)
	{
		/* Remove the NvParameterized factories */
		ModuleParticlesRegistration::invokeUnregistration(traits);
	}

	SAFE_MODULE_RELEASE(mModuleBasicIos);
	SAFE_MODULE_RELEASE(mModuleEmitter);
	SAFE_MODULE_RELEASE(mModuleIofx);
	SAFE_MODULE_RELEASE(mModuleFieldSampler);
	SAFE_MODULE_RELEASE(mModuleBasicFS);
	SAFE_MODULE_RELEASE(mModuleParticleIos);			// PhysX 3.x only : Instantiate the ParticleIOS module
	SAFE_MODULE_RELEASE(mModuleForceField);			// PhysX 3.x only : Instantiate the ForceField module

	// clear before deletion, so that we don't call back into releaseModuleSceneIntl during "delete this"
	mParticlesScenes.clear();

	delete this;


}

NvParameterized::Interface* ModuleParticlesImpl::getDefaultModuleDesc()
{
	WRITE_ZONE();
	NvParameterized::Traits* traits = mSdk->getParameterizedTraits();

	if (!mModuleParams)
	{
		mModuleParams = DYNAMIC_CAST(ParticlesModuleParameters*)
		                (traits->createNvParameterized("ParticlesModuleParameters"));
		PX_ASSERT(mModuleParams);
	}
	else
	{
		mModuleParams->initDefaults();
	}

	return mModuleParams;
}

void ModuleParticlesImpl::init(const ModuleParticlesDesc& expDesc)
{
	WRITE_ZONE();
	mModuleValue = expDesc.moduleValue;
}

AuthObjTypeID ModuleParticlesImpl::getParticlesAssetTypeID() const
{
// TODO	return ParticlesAsset::mAssetTypeID;
	return 0;
}
AuthObjTypeID ModuleParticlesImpl::getModuleID() const
{
	READ_ZONE();
//	return ParticlesAsset::mAssetTypeID;
// TODO
	return 0;
}


/* == Particles Scene methods == */
ModuleSceneIntl* ModuleParticlesImpl::createInternalModuleScene(SceneIntl& scene, RenderDebugInterface* renderDebug)
{
	ModuleSceneIntl* ret = PX_NEW(ParticlesScene)(*this, scene, renderDebug, mParticlesScenes);
	mScenes.pushBack(ret);

	return ret;
}

void ModuleParticlesImpl::releaseModuleSceneIntl(ModuleSceneIntl& scene)
{
	for (uint32_t i = 0; i < mScenes.size(); ++i)
	{
		if (mScenes[i] == &scene)
		{
			mScenes.remove(i);
			break;
		}
	}
	ParticlesScene* es = DYNAMIC_CAST(ParticlesScene*)(&scene);
	es->destroy();

}

uint32_t ModuleParticlesImpl::forceLoadAssets()
{
	uint32_t loadedAssetCount = 0;

	for (uint32_t i = 0; i < mAuthorableObjects.getSize(); i++)
	{
		AuthorableObjectIntl* ao = static_cast<AuthorableObjectIntl*>(mAuthorableObjects.getResource(i));
		loadedAssetCount += ao->forceLoadAssets();
	}

	for (uint32_t i = 0; i < mEffectPackageAuthorableObjects.getSize(); i++)
	{
		AuthorableObjectIntl* ao = static_cast<AuthorableObjectIntl*>(mEffectPackageAuthorableObjects.getResource(i));
		loadedAssetCount += ao->forceLoadAssets();
	}


	return loadedAssetCount;
}

ParticlesScene* ModuleParticlesImpl::getParticlesScene(const Scene& apexScene)
{
	for (uint32_t i = 0 ; i < mParticlesScenes.getSize() ; i++)
	{
		ParticlesScene* es = DYNAMIC_CAST(ParticlesScene*)(mParticlesScenes.getResource(i));
		if (es->mApexScene == &apexScene)
		{
			return es;
		}
	}

	PX_ASSERT(!"Unable to locate an appropriate ParticlesScene");
	return NULL;
}

RenderableIterator* ModuleParticlesImpl::createRenderableIterator(const Scene& apexScene)
{
	WRITE_ZONE();
	ParticlesScene* es = getParticlesScene(apexScene);
	if (es)
	{
		return es->createRenderableIterator();
	}

	return NULL;
}

const NvParameterized::Interface* ModuleParticlesImpl::locateVolumeRenderMaterialData(const char* name) const
{
	const NvParameterized::Interface* ret = NULL;

	if (mGraphicsMaterialsDatabase)
	{
		EffectPackageGraphicsMaterialsParams* d = static_cast< EffectPackageGraphicsMaterialsParams*>(mGraphicsMaterialsDatabase);
		for (int32_t i = 0; i < d->GraphicsMaterials.arraySizes[0]; i++)
		{
			NvParameterized::Interface *ei = d->GraphicsMaterials.buf[i];
			if ( ei && nvidia::strcmp(ei->className(),"VolumeRenderMaterialData") == 0 )
			{
				VolumeRenderMaterialData* e = static_cast< VolumeRenderMaterialData*>(ei);
				if (e)
				{
					if (e->Name && nvidia::stricmp(e->Name, name) == 0)
					{
						ret = e;
						break;
					}
				}
			}
		}
	}
	return ret;
}



const NvParameterized::Interface* ModuleParticlesImpl::locateGraphicsMaterialData(const char* name) const
{
	READ_ZONE();
	const NvParameterized::Interface* ret = NULL;

	if (mGraphicsMaterialsDatabase)
	{
		EffectPackageGraphicsMaterialsParams* d = static_cast< EffectPackageGraphicsMaterialsParams*>(mGraphicsMaterialsDatabase);
		for (int32_t i = 0; i < d->GraphicsMaterials.arraySizes[0]; i++)
		{
			NvParameterized::Interface *ei = d->GraphicsMaterials.buf[i];
			if ( ei && nvidia::strcmp(ei->className(),"GraphicsMaterialData") == 0 || nvidia::strcmp(ei->className(),"VolumeRenderMaterialData") == 0 )
			{
				GraphicsMaterialData* e = static_cast< GraphicsMaterialData*>(ei);
				if (e)
				{
					if (e->Name && nvidia::stricmp(e->Name, name) == 0)
					{
						ret = e;
						break;
					}
				}
			}
		}
	}
	return ret;
}

static bool isUnique(Array< const char* > &nameList, const char* baseName)
{
	bool ret = true;
	for (uint32_t i = 0; i < nameList.size(); i++)
	{
		if (nvidia::stricmp(baseName, nameList[i]) == 0)
		{
			ret = false;
			break;
		}
	}
	return ret;
}

static const char* getUniqueName(Array< const char* > &nameList, const char* baseName)
{
	const char* ret = baseName;

	if (baseName == NULL || strlen(baseName) == 0)
	{
		baseName = "default";
	}
	if (!isUnique(nameList, baseName))
	{
		static char uniqueName[512];
		strncpy(uniqueName, baseName, 512);
		for (uint32_t i = 1; i < 1000; i++)
		{
			sprintf_s(uniqueName, 512, "%s%d", baseName, i);
			if (isUnique(nameList, uniqueName))
			{
				ret = uniqueName;
				break;
			}
		}
	}
	return ret;
}


bool ModuleParticlesImpl::setEffectPackageGraphicsMaterialsDatabase(const NvParameterized::Interface* dataBase)
{
	WRITE_ZONE();
	bool ret = false;
	if (dataBase)
	{
		NvParameterized::Traits* traits = mSdk->getParameterizedTraits();
		if (nvidia::strcmp(dataBase->className(), EffectPackageGraphicsMaterialsParams::staticClassName()) == 0 && dataBase != mGraphicsMaterialsDatabase)
		{
			if (mGraphicsMaterialsDatabase)
			{
				mGraphicsMaterialsDatabase->destroy();
				mGraphicsMaterialsDatabase = NULL;
			}
			dataBase->clone(mGraphicsMaterialsDatabase);
			if ( mGraphicsMaterialsDatabase == NULL )
			{
				APEX_DEBUG_WARNING("Failed to clone asset.");
				mGraphicsMaterialsDatabase = traits->createNvParameterized(EffectPackageGraphicsMaterialsParams::staticClassName());
			}
			ret = true;
		}
		else
		{
			// add it to the end of the existing array..
			if (nvidia::strcmp(dataBase->className(), GraphicsMaterialData::staticClassName()) == 0 || nvidia::strcmp(dataBase->className(), VolumeRenderMaterialData::staticClassName()) == 0 )
			{
				bool revised = false;
				const char* itemName = dataBase->name();
				EffectPackageGraphicsMaterialsParams* d = static_cast< EffectPackageGraphicsMaterialsParams*>(mGraphicsMaterialsDatabase);
				if ( d )
				{
					for (uint32_t i = 0; i < (uint32_t)d->GraphicsMaterials.arraySizes[0]; i++)
					{
						NvParameterized::Interface* ei = d->GraphicsMaterials.buf[i];
						if (!ei)
						{
							continue;
						}
						const char *materialName=NULL;
						if ( nvidia::strcmp(ei->className(),"GraphicsMaterialData") == 0 )
						{
							GraphicsMaterialData* ed = static_cast< GraphicsMaterialData*>(ei);
							materialName = ed->Name.buf;
						}
						else if ( nvidia::strcmp(ei->className(),"VolumeRenderMaterialData") == 0 )
						{
							VolumeRenderMaterialData* ed = static_cast< VolumeRenderMaterialData*>(ei);
							materialName = ed->Name.buf;
						}
						else
						{
							PX_ALWAYS_ASSERT();
						}
						if ( materialName && nvidia::stricmp(materialName, itemName) == 0)
						{
							ei->copy(*dataBase);
							revised = true;
							ret = true;
							break;
						}
					}
					if (!revised && mGraphicsMaterialsDatabase)
					{
						int32_t arraySize = d->GraphicsMaterials.arraySizes[0];
						NvParameterized::Handle handle(mGraphicsMaterialsDatabase);
						NvParameterized::ErrorType err = handle.getParameter("GraphicsMaterials");
						PX_ASSERT(err == NvParameterized::ERROR_NONE);
						err = handle.resizeArray(arraySize + 1);
						PX_ASSERT(err == NvParameterized::ERROR_NONE);
						if (err == NvParameterized::ERROR_NONE)
						{
							NvParameterized::Interface* ei = NULL;
							NvParameterized::ErrorType err = dataBase->clone(ei);
							if ( err != NvParameterized::ERROR_NONE || ei == NULL )
							{
								APEX_DEBUG_WARNING("Failed to clone asset.");
							}
							else
							{
								NvParameterized::Handle item(ei);
								err = item.getParameter("Name");
								PX_ASSERT(err == NvParameterized::ERROR_NONE);
								item.setParamString(itemName);
								ei->setName(itemName);
								d->GraphicsMaterials.buf[arraySize] = ei;
							}
							ret = true;
						}
					}
				}
			}
		}

		if (mGraphicsMaterialsDatabase)
		{
			EffectPackageGraphicsMaterialsParams* ds = static_cast< EffectPackageGraphicsMaterialsParams*>(mGraphicsMaterialsDatabase);
			if (ds->GraphicsMaterials.arraySizes[0] == 0)
			{
				NvParameterized::Handle handle(mGraphicsMaterialsDatabase);
				NvParameterized::ErrorType err = handle.getParameter("GraphicsMaterials");
				PX_ASSERT(err == NvParameterized::ERROR_NONE);
				err = handle.resizeArray(2);
				PX_ASSERT(err == NvParameterized::ERROR_NONE);
				NvParameterized::Interface* ei;
				if (err == NvParameterized::ERROR_NONE)
				{
					ei = traits->createNvParameterized(GraphicsMaterialData::staticClassName());
					ds->GraphicsMaterials.buf[0] = ei;
					ei = traits->createNvParameterized(VolumeRenderMaterialData::staticClassName());
					ds->GraphicsMaterials.buf[1] = ei;
				}
			}

			Array< const char* > nameList;
			for (int32_t i = 0; i < ds->GraphicsMaterials.arraySizes[0]; i++)
			{
				NvParameterized::Interface *ei = ds->GraphicsMaterials.buf[i];
				if ( !ei )
					continue;
				const char *materialName=NULL;
				if ( nvidia::strcmp(ei->className(),"GraphicsMaterialData") == 0 )
				{
					GraphicsMaterialData* e = static_cast< GraphicsMaterialData*>(ei);
					if (e)
					{
						materialName = e->Name;
					}
				}
				else if ( nvidia::strcmp(ei->className(),"VolumeRenderMaterialData") == 0 )
				{
					VolumeRenderMaterialData* e = static_cast< VolumeRenderMaterialData*>(ei);
					if (e)
					{
						materialName = e->Name.buf;
					}
				}
				else
				{
					PX_ALWAYS_ASSERT();
				}
				if ( materialName )
				{
					for (uint32_t j = 0; j < nameList.size(); j++)
					{
						if (nvidia::stricmp(nameList[j], materialName) == 0)
						{
							NvParameterized::Handle handle(ei);
							NvParameterized::ErrorType err = handle.getParameter("Name");
							PX_ASSERT(err == NvParameterized::ERROR_NONE);
							if (err == NvParameterized::ERROR_NONE)
							{
								materialName = getUniqueName(nameList, materialName);
								handle.setParamString(materialName);
								ret = true;
								break;
							}
						}
					}
					nameList.pushBack(materialName);
				}
			}
		}
	}
	return ret;
}


const NvParameterized::Interface* ModuleParticlesImpl::getEffectPackageGraphicsMaterialsDatabase(void) const
{
	READ_ZONE();
	return mGraphicsMaterialsDatabase;
}


NvParameterized::Interface* ModuleParticlesImpl::locateResource(const char* resourceName, const char* nameSpace)
{
	WRITE_ZONE();
	NvParameterized::Interface* ret = NULL;

	if (mEffectPackageDatabaseParams == NULL)
	{
		return NULL;
	}

	if (nvidia::strcmp(nameSpace, PARTICLES_EFFECT_PACKAGE_AUTHORING_TYPE_NAME) == 0)
	{
		EffectPackageDatabaseParams* d = static_cast< EffectPackageDatabaseParams*>(mEffectPackageDatabaseParams);
		if ( d )
		{
			for (uint32_t i = 0; i < (uint32_t)d->EffectPackages.arraySizes[0]; i++)
			{
				NvParameterized::Interface* ei = d->EffectPackages.buf[i];
				if (!ei)
				{
					continue;
				}
				EffectPackageData* ed = static_cast< EffectPackageData*>(ei);
				if (nvidia::stricmp(ed->Name, resourceName) == 0)
				{
					ret = ed->EffectPackage;
					break;
				}
			}
		}
	}
	else if (nvidia::strcmp(nameSpace, APEX_MATERIALS_NAME_SPACE) == 0 || nvidia::strcmp(nameSpace,"GraphicsMaterialData") == 0 )
	{
		EffectPackageGraphicsMaterialsParams* d = static_cast< EffectPackageGraphicsMaterialsParams*>(mGraphicsMaterialsDatabase);
		if ( d )
		{
			for (uint32_t i = 0; i < (uint32_t)d->GraphicsMaterials.arraySizes[0]; i++)
			{
				NvParameterized::Interface* ei = d->GraphicsMaterials.buf[i];
				if (!ei)
				{
					continue;
				}
				if ( nvidia::strcmp(ei->className(),"GraphicsMaterialData") == 0 )
				{
					GraphicsMaterialData* ed = static_cast< GraphicsMaterialData*>(ei);
					if ( nvidia::strcmp(ed->Name,resourceName) == 0 )
					{
						ret = ei;
						break;
					}
				}
			}
		}
	}
	else if (nvidia::strcmp(nameSpace, APEX_VOLUME_RENDER_MATERIALS_NAME_SPACE) == 0 || nvidia::strcmp(nameSpace,"VolumeRenderMaterialData") == 0 )
	{
		EffectPackageGraphicsMaterialsParams* d = static_cast< EffectPackageGraphicsMaterialsParams*>(mGraphicsMaterialsDatabase);
		if ( d )
		{
			for (uint32_t i = 0; i < (uint32_t)d->GraphicsMaterials.arraySizes[0]; i++)
			{
				NvParameterized::Interface* ei = d->GraphicsMaterials.buf[i];
				if (!ei)
				{
					continue;
				}
				if ( nvidia::strcmp(ei->className(),"VolumeRenderMaterialData") == 0 )
				{
					VolumeRenderMaterialData* ed = static_cast< VolumeRenderMaterialData*>(ei);
					if ( nvidia::strcmp(ed->Name,resourceName) == 0 )
					{
						ret = ei;
						break;
					}
				}
			}
		}
	}
	else if (nvidia::strcmp(nameSpace, IOFX_AUTHORING_TYPE_NAME) == 0 || nvidia::strcmp(nameSpace, "GraphicsEffectData") == 0 )
	{
		EffectPackageIOFXDatabaseParams* d = static_cast< EffectPackageIOFXDatabaseParams*>(mEffectPackageIOFXDatabaseParams);
		if ( d )
		{
			for (uint32_t i = 0; i < (uint32_t)d->GraphicsEffects.arraySizes[0]; i++)
			{
				NvParameterized::Interface* ei = d->GraphicsEffects.buf[i];
				if (!ei)
				{
					continue;
				}
				GraphicsEffectData* ed = static_cast< GraphicsEffectData*>(ei);
				if (nvidia::stricmp(resourceName, ed->Name) == 0)
				{
					if (nvidia::strcmp(nameSpace, IOFX_AUTHORING_TYPE_NAME) == 0 )
					{
						ret = ed->IOFX;
					}
					else
					{
						ret = ed;
					}
					break;
				}
			}
		}
	}
	else if (nvidia::strcmp(nameSpace, "ParticleSimulationData") == 0)
	{
		EffectPackageIOSDatabaseParams* d = static_cast< EffectPackageIOSDatabaseParams*>(mEffectPackageIOSDatabaseParams);
		if ( d )
		{
			for (uint32_t i = 0; i < (uint32_t)d->ParticleSimulations.arraySizes[0]; i++)
			{
				NvParameterized::Interface* ei = d->ParticleSimulations.buf[i];
				if (!ei)
				{
					continue;
				}
				ParticleSimulationData* ed = static_cast< ParticleSimulationData*>(ei);
				if (nvidia::stricmp(ed->Name, resourceName) == 0)
				{
					ret = ei;
					break;
				}
			}
		}
	}
	else if (nvidia::strcmp(nameSpace, BASIC_IOS_AUTHORING_TYPE_NAME) == 0)
	{
		EffectPackageIOSDatabaseParams* d = static_cast< EffectPackageIOSDatabaseParams*>(mEffectPackageIOSDatabaseParams);
		if ( d )
		{
			for (uint32_t i = 0; i < (uint32_t)d->ParticleSimulations.arraySizes[0]; i++)
			{
				NvParameterized::Interface* ei = d->ParticleSimulations.buf[i];
				if (!ei)
				{
					continue;
				}
				ParticleSimulationData* ed = static_cast< ParticleSimulationData*>(ei);
				if (nvidia::strcmp(ed->IOS->className(), basicios::BasicIOSAssetParam::staticClassName()) == 0)
				{
					if (nvidia::stricmp(ed->Name, resourceName) == 0)
					{
						ret = ed->IOS;
						break;
					}
				}
			}
		}
	}
	else if (nvidia::strcmp(nameSpace, PARTICLE_IOS_AUTHORING_TYPE_NAME) == 0)
	{
		EffectPackageIOSDatabaseParams* d = static_cast< EffectPackageIOSDatabaseParams*>(mEffectPackageIOSDatabaseParams);
		if ( d )
		{
			for (uint32_t i = 0; i < (uint32_t)d->ParticleSimulations.arraySizes[0]; i++)
			{
				NvParameterized::Interface* ei = d->ParticleSimulations.buf[i];
				if (!ei)
				{
					continue;
				}
				ParticleSimulationData* ed = static_cast< ParticleSimulationData*>(ei);
				if (nvidia::strcmp(ed->IOS->className(), pxparticleios::ParticleIosAssetParam::staticClassName()) == 0)
				{
					if (nvidia::stricmp(ed->Name, resourceName) == 0)
					{
						ret = ed->IOS;
						break;
					}
				}
			}
		}
	}
	else if (nvidia::strcmp(nameSpace, EMITTER_AUTHORING_TYPE_NAME) == 0 || nvidia::strcmp(nameSpace,"EmitterData") == 0 )
	{
		EffectPackageEmitterDatabaseParams* d = static_cast< EffectPackageEmitterDatabaseParams*>(mEffectPackageEmitterDatabaseParams);
		if ( d )
		{
			for (uint32_t i = 0; i < (uint32_t)d->Emitters.arraySizes[0]; i++)
			{
				NvParameterized::Interface* ei = d->Emitters.buf[i];
				if (!ei)
				{
					continue;
				}
				EmitterData* ed = static_cast< EmitterData*>(ei);
				if (nvidia::stricmp(resourceName, ed->Name) == 0)
				{
					if ( nvidia::strcmp(nameSpace, EMITTER_AUTHORING_TYPE_NAME) == 0 )
					{
						ret = ed->Emitter;
					}
					else
					{
						ret = ed;
					}
					break;
				}
			}
		}
	}
	else if (nvidia::strcmp(nameSpace, HEAT_SOURCE_AUTHORING_TYPE_NAME) == 0 || nvidia::strcmp(nameSpace, "HeatSourceData") == 0 )
	{
		EffectPackageFieldSamplerDatabaseParams* d = static_cast< EffectPackageFieldSamplerDatabaseParams*>(mEffectPackageFieldSamplerDatabaseParams);
		if ( d )
		{
			for (uint32_t i = 0; i < (uint32_t)d->FieldSamplers.arraySizes[0]; i++)
			{
				NvParameterized::Interface* ei = d->FieldSamplers.buf[i];
				if (!ei)
				{
					continue;
				}
				if (nvidia::strcmp(ei->className(), HeatSourceData::staticClassName()) != 0)
				{
					continue;
				}
				HeatSourceData* ed = static_cast< HeatSourceData*>(ei);
				if (nvidia::stricmp(ed->Name, resourceName) == 0)
				{
					if (nvidia::strcmp(nameSpace, HEAT_SOURCE_AUTHORING_TYPE_NAME) == 0 )
					{
						ret = ed->HeatSource;
					}
					else
					{
						ret = ed;
					}
					break;
				}
			}
		}
	}
	else if (nvidia::strcmp(nameSpace, SUBSTANCE_SOURCE_AUTHORING_TYPE_NAME) == 0 || nvidia::strcmp(nameSpace, "SubstanceSourceData") == 0 )
	{
		EffectPackageFieldSamplerDatabaseParams* d = static_cast< EffectPackageFieldSamplerDatabaseParams*>(mEffectPackageFieldSamplerDatabaseParams);
		if ( d )
		{
			for (uint32_t i = 0; i < (uint32_t)d->FieldSamplers.arraySizes[0]; i++)
			{
				NvParameterized::Interface* ei = d->FieldSamplers.buf[i];
				if (!ei)
				{
					continue;
				}
				if (nvidia::strcmp(ei->className(), SubstanceSourceData::staticClassName()) != 0)
				{
					continue;
				}
				SubstanceSourceData* ed = static_cast< SubstanceSourceData*>(ei);
				if (nvidia::stricmp(ed->Name, resourceName) == 0)
				{
					if (nvidia::strcmp(nameSpace, SUBSTANCE_SOURCE_AUTHORING_TYPE_NAME) == 0 )
					{
						ret = ed->SubstanceSource;
					}
					else
					{
						ret = ed;
					}
					break;
				}
			}
		}
	}
	else if (nvidia::strcmp(nameSpace, VELOCITY_SOURCE_AUTHORING_TYPE_NAME) == 0 || nvidia::strcmp(nameSpace, "VelocitySourceData") == 0 )
	{
		EffectPackageFieldSamplerDatabaseParams* d = static_cast< EffectPackageFieldSamplerDatabaseParams*>(mEffectPackageFieldSamplerDatabaseParams);
		if ( d )
		{
			for (uint32_t i = 0; i < (uint32_t)d->FieldSamplers.arraySizes[0]; i++)
			{
				NvParameterized::Interface* ei = d->FieldSamplers.buf[i];
				if (!ei)
				{
					continue;
				}
				if (nvidia::strcmp(ei->className(), VelocitySourceData::staticClassName()) != 0)
				{
					continue;
				}
				VelocitySourceData* ed = static_cast< VelocitySourceData*>(ei);
				if (nvidia::stricmp(ed->Name, resourceName) == 0)
				{
					if (nvidia::strcmp(nameSpace, VELOCITY_SOURCE_AUTHORING_TYPE_NAME) == 0 )
					{
						ret = ed->VelocitySource;
					}
					else
					{
						ret = ed;
					}
					break;
				}
			}
		}
	}
	else if (nvidia::strcmp(nameSpace, JET_FS_AUTHORING_TYPE_NAME) == 0 || nvidia::strcmp(nameSpace, "JetFieldSamplerData") == 0 )
	{
		EffectPackageFieldSamplerDatabaseParams* d = static_cast< EffectPackageFieldSamplerDatabaseParams*>(mEffectPackageFieldSamplerDatabaseParams);
		if ( d )
		{
			for (uint32_t i = 0; i < (uint32_t)d->FieldSamplers.arraySizes[0]; i++)
			{
				NvParameterized::Interface* ei = d->FieldSamplers.buf[i];
				if (!ei)
				{
					continue;
				}
				if (nvidia::strcmp(ei->className(), JetFieldSamplerData::staticClassName()) != 0)
				{
					continue;
				}
				JetFieldSamplerData* ed = static_cast< JetFieldSamplerData*>(ei);
				if (nvidia::stricmp(ed->Name, resourceName) == 0)
				{
					if (nvidia::strcmp(nameSpace, JET_FS_AUTHORING_TYPE_NAME) == 0 )
					{
						ret = ed->JetFieldSampler;
					}
					else
					{
						ret = ed;
					}
					break;
				}
			}
		}
	}
	else if (nvidia::strcmp(nameSpace, WIND_FS_AUTHORING_TYPE_NAME) == 0 || nvidia::strcmp(nameSpace, "WindFieldSamplerData") == 0 )
	{
		EffectPackageFieldSamplerDatabaseParams* d = static_cast< EffectPackageFieldSamplerDatabaseParams*>(mEffectPackageFieldSamplerDatabaseParams);
		if ( d )
		{
			for (uint32_t i = 0; i < (uint32_t)d->FieldSamplers.arraySizes[0]; i++)
			{
				NvParameterized::Interface* ei = d->FieldSamplers.buf[i];
				if (!ei)
				{
					continue;
				}
				if (nvidia::strcmp(ei->className(), WindFieldSamplerData::staticClassName()) != 0)
				{
					continue;
				}
				WindFieldSamplerData* ed = static_cast< WindFieldSamplerData*>(ei);
				if (nvidia::stricmp(ed->Name, resourceName) == 0)
				{
					if (nvidia::strcmp(nameSpace, WIND_FS_AUTHORING_TYPE_NAME) == 0 )
					{
						ret = ed->WindFieldSampler;
					}
					else
					{
						ret = ed;
					}
					break;
				}
			}
		}
	}
	else if (nvidia::strcmp(nameSpace, VORTEX_FS_AUTHORING_TYPE_NAME) == 0 || nvidia::strcmp(nameSpace, "VortexFieldSamplerData") == 0 )
	{
		EffectPackageFieldSamplerDatabaseParams* d = static_cast< EffectPackageFieldSamplerDatabaseParams*>(mEffectPackageFieldSamplerDatabaseParams);
		if ( d )
		{
			for (uint32_t i = 0; i < (uint32_t)d->FieldSamplers.arraySizes[0]; i++)
			{
				NvParameterized::Interface* ei = d->FieldSamplers.buf[i];
				if (!ei)
				{
					continue;
				}
				if (nvidia::strcmp(ei->className(), VortexFieldSamplerData::staticClassName()) != 0)
				{
					continue;
				}
				VortexFieldSamplerData* ed = static_cast< VortexFieldSamplerData*>(ei);
				if (nvidia::stricmp(ed->Name, resourceName) == 0)
				{
					if (nvidia::strcmp(nameSpace, VORTEX_FS_AUTHORING_TYPE_NAME) == 0 )
					{
						ret = ed->VortexFieldSampler;
					}
					else
					{
						ret = ed;
					}
					break;
				}
			}
		}
	}
	else if (nvidia::strcmp(nameSpace, NOISE_FS_AUTHORING_TYPE_NAME) == 0 || nvidia::strcmp(nameSpace, "NoiseFieldSamplerData") == 0 )
	{
		EffectPackageFieldSamplerDatabaseParams* d = static_cast< EffectPackageFieldSamplerDatabaseParams*>(mEffectPackageFieldSamplerDatabaseParams);
		if ( d )
		{
			for (uint32_t i = 0; i < (uint32_t)d->FieldSamplers.arraySizes[0]; i++)
			{
				NvParameterized::Interface* ei = d->FieldSamplers.buf[i];
				if (!ei)
				{
					continue;
				}
				if (nvidia::strcmp(ei->className(), NoiseFieldSamplerData::staticClassName()) != 0)
				{
					continue;
				}
				NoiseFieldSamplerData* ed = static_cast< NoiseFieldSamplerData*>(ei);
				if (nvidia::stricmp(ed->Name, resourceName) == 0)
				{
					if (nvidia::strcmp(nameSpace, NOISE_FS_AUTHORING_TYPE_NAME) == 0 )
					{
						ret = ed->NoiseFieldSampler;
					}
					else
					{
						ret = ed;
					}
					break;
				}
			}
		}
	}
	else if (nvidia::strcmp(nameSpace, ATTRACTOR_FS_AUTHORING_TYPE_NAME) == 0 || nvidia::strcmp(nameSpace, "AttractorFieldSamplerData") == 0 )
	{
		EffectPackageFieldSamplerDatabaseParams* d = static_cast< EffectPackageFieldSamplerDatabaseParams*>(mEffectPackageFieldSamplerDatabaseParams);
		if ( d )
		{
			for (uint32_t i = 0; i < (uint32_t)d->FieldSamplers.arraySizes[0]; i++)
			{
				NvParameterized::Interface* ei = d->FieldSamplers.buf[i];
				if (!ei)
				{
					continue;
				}
				if (nvidia::strcmp(ei->className(), AttractorFieldSamplerData::staticClassName()) != 0)
				{
					continue;
				}
				AttractorFieldSamplerData* ed = static_cast< AttractorFieldSamplerData*>(ei);
				if (nvidia::stricmp(ed->Name, resourceName) == 0)
				{
					if (nvidia::strcmp(nameSpace, ATTRACTOR_FS_AUTHORING_TYPE_NAME) == 0 )
					{
						ret = ed->AttractorFieldSampler;
					}
					else
					{
						ret = ed;
					}
					break;
				}
			}
		}
	}
	else if (nvidia::strcmp(nameSpace, TURBULENCE_FS_AUTHORING_TYPE_NAME) == 0 || nvidia::strcmp(nameSpace, "TurbulenceFieldSamplerData") == 0)
	{
		EffectPackageFieldSamplerDatabaseParams* d = static_cast< EffectPackageFieldSamplerDatabaseParams*>(mEffectPackageFieldSamplerDatabaseParams);
		if ( d )
		{
			for (uint32_t i = 0; i < (uint32_t)d->FieldSamplers.arraySizes[0]; i++)
			{
				NvParameterized::Interface* ei = d->FieldSamplers.buf[i];
				if (!ei)
				{
					continue;
				}
				if (nvidia::strcmp(ei->className(), TurbulenceFieldSamplerData::staticClassName()) != 0)
				{
					continue;
				}
				TurbulenceFieldSamplerData* ed = static_cast< TurbulenceFieldSamplerData*>(ei);
				if (nvidia::stricmp(ed->Name, resourceName) == 0)
				{
					if (nvidia::strcmp(nameSpace, TURBULENCE_FS_AUTHORING_TYPE_NAME) == 0 )
					{
						ret = ed->TurbulenceFieldSampler;
					}
					else
					{
						ret = ed;
					}
					break;
				}
			}
		}
	}
	else if (nvidia::strcmp(nameSpace, FORCEFIELD_AUTHORING_TYPE_NAME) == 0 || nvidia::strcmp(nameSpace, "ForceFieldData") == 0)
	{
		EffectPackageFieldSamplerDatabaseParams* d = static_cast< EffectPackageFieldSamplerDatabaseParams*>(mEffectPackageFieldSamplerDatabaseParams);
		if ( d )
		{
			for (uint32_t i = 0; i < (uint32_t)d->FieldSamplers.arraySizes[0]; i++)
			{
				NvParameterized::Interface* ei = d->FieldSamplers.buf[i];
				if (!ei)
				{
					continue;
				}
				if (nvidia::strcmp(ei->className(), ForceFieldData::staticClassName()) != 0)
				{
					continue;
				}
				ForceFieldData* ed = static_cast< ForceFieldData*>(ei);
				if (nvidia::stricmp(ed->Name, resourceName) == 0)
				{
					if (nvidia::strcmp(nameSpace, FORCEFIELD_AUTHORING_TYPE_NAME) == 0 )
					{
						ret = ed->ForceField;
					}
					else
					{
						ret = ed;
					}
					break;
				}
			}
		}
	}
	else if (nvidia::strcmp(nameSpace, FLAME_EMITTER_AUTHORING_TYPE_NAME) == 0 || nvidia::strcmp(nameSpace, "FlameEmitterData") == 0)
	{
		EffectPackageFieldSamplerDatabaseParams* d = static_cast< EffectPackageFieldSamplerDatabaseParams*>(mEffectPackageFieldSamplerDatabaseParams);
		if ( d )
		{
			for (uint32_t i = 0; i < (uint32_t)d->FieldSamplers.arraySizes[0]; i++)
			{
				NvParameterized::Interface* ei = d->FieldSamplers.buf[i];
				if (!ei)
				{
					continue;
				}
				if (nvidia::strcmp(ei->className(), FlameEmitterData::staticClassName()) != 0)
				{
					continue;
				}
				FlameEmitterData* ed = static_cast< FlameEmitterData*>(ei);
				if (nvidia::stricmp(ed->Name, resourceName) == 0)
				{
					if (nvidia::strcmp(nameSpace, FLAME_EMITTER_AUTHORING_TYPE_NAME) == 0 )
					{
						ret = ed->FlameEmitter;
					}
					else
					{
						ret = ed;
					}
					break;
				}
			}
		}
	}

	return ret;
}




const char** ModuleParticlesImpl::getResourceNames(const char* nameSpace, uint32_t& nameCount, const char** &variants)
{
	READ_ZONE();
	const char** ret = NULL;

	variants = NULL;
	mTempNames.clear();
	mTempVariantNames.clear();

	if (nvidia::strcmp(nameSpace, IOFX_AUTHORING_TYPE_NAME) == 0)
	{
		EffectPackageIOFXDatabaseParams* d = static_cast< EffectPackageIOFXDatabaseParams*>(mEffectPackageIOFXDatabaseParams);
		if ( d )
		{
			for (uint32_t i = 0; i < (uint32_t)d->GraphicsEffects.arraySizes[0]; i++)
			{
				NvParameterized::Interface* ei = d->GraphicsEffects.buf[i];
				if (!ei)
				{
					continue;
				}
				GraphicsEffectData* ed = static_cast< GraphicsEffectData*>(ei);
				mTempNames.pushBack(ed->Name);
				mTempVariantNames.pushBack(IOFX_AUTHORING_TYPE_NAME);
			}
		}
	}
	else if (nvidia::strcmp(nameSpace, APEX_MATERIALS_NAME_SPACE) == 0)
	{
		EffectPackageGraphicsMaterialsParams* d = static_cast< EffectPackageGraphicsMaterialsParams*>(mGraphicsMaterialsDatabase);
		if ( d )
		{
			for (uint32_t i = 0; i < (uint32_t)d->GraphicsMaterials.arraySizes[0]; i++)
			{
				NvParameterized::Interface* ei = d->GraphicsMaterials.buf[i];
				if (!ei)
				{
					continue;
				}
				if ( nvidia::strcmp(ei->className(),"GraphicsMaterialData") == 0 )
				{
					GraphicsMaterialData* ed = static_cast< GraphicsMaterialData*>(ei);
					mTempNames.pushBack(ed->Name);
					mTempVariantNames.pushBack(APEX_MATERIALS_NAME_SPACE);
				}
			}
		}
	}
	else if (nvidia::strcmp(nameSpace, APEX_VOLUME_RENDER_MATERIALS_NAME_SPACE) == 0)
	{
		EffectPackageGraphicsMaterialsParams* d = static_cast< EffectPackageGraphicsMaterialsParams*>(mGraphicsMaterialsDatabase);
		if ( d )
		{
			for (uint32_t i = 0; i < (uint32_t)d->GraphicsMaterials.arraySizes[0]; i++)
			{
				NvParameterized::Interface* ei = d->GraphicsMaterials.buf[i];
				if (!ei)
				{
					continue;
				}
				if ( nvidia::strcmp(ei->className(),"VolumeRenderMaterialData") == 0 )
				{
					VolumeRenderMaterialData* ed = static_cast< VolumeRenderMaterialData*>(ei);
					mTempNames.pushBack(ed->Name);
					mTempVariantNames.pushBack(APEX_VOLUME_RENDER_MATERIALS_NAME_SPACE);
				}
			}
		}
	}
	else if (nvidia::strcmp(nameSpace, BASIC_IOS_AUTHORING_TYPE_NAME) == 0 ||  nvidia::strcmp(nameSpace, PARTICLE_IOS_AUTHORING_TYPE_NAME) == 0)
	{
		EffectPackageIOSDatabaseParams* d = static_cast< EffectPackageIOSDatabaseParams*>(mEffectPackageIOSDatabaseParams);
		if ( d )
		{
			for (uint32_t i = 0; i < (uint32_t)d->ParticleSimulations.arraySizes[0]; i++)
			{
				NvParameterized::Interface* ei = d->ParticleSimulations.buf[i];
				if (!ei)
				{
					continue;
				}
				ParticleSimulationData* ed = static_cast< ParticleSimulationData*>(ei);
				mTempNames.pushBack(ed->Name);
				if (nvidia::strcmp(ed->IOS->className(), basicios::BasicIOSAssetParam::staticClassName()) == 0)
				{
					mTempVariantNames.pushBack(BASIC_IOS_AUTHORING_TYPE_NAME);
				}
				else
				{
					mTempVariantNames.pushBack(PARTICLE_IOS_AUTHORING_TYPE_NAME);
				}
			}
		}
	}
	else if (nvidia::strcmp(nameSpace, EMITTER_AUTHORING_TYPE_NAME) == 0)
	{
		EffectPackageEmitterDatabaseParams* d = static_cast< EffectPackageEmitterDatabaseParams*>(mEffectPackageEmitterDatabaseParams);
		if ( d )
		{
			for (uint32_t i = 0; i < (uint32_t)d->Emitters.arraySizes[0]; i++)
			{
				NvParameterized::Interface* ei = d->Emitters.buf[i];
				if (!ei)
				{
					continue;
				}
				EmitterData* ed = static_cast< EmitterData*>(ei);
				mTempNames.pushBack(ed->Name);
				mTempVariantNames.pushBack(EMITTER_AUTHORING_TYPE_NAME);
			}
		}
	}
	else if (nvidia::strcmp(nameSpace, HEAT_SOURCE_AUTHORING_TYPE_NAME) == 0)
	{
		EffectPackageFieldSamplerDatabaseParams* d = static_cast< EffectPackageFieldSamplerDatabaseParams*>(mEffectPackageFieldSamplerDatabaseParams);
		if ( d )
		{
			for (uint32_t i = 0; i < (uint32_t)d->FieldSamplers.arraySizes[0]; i++)
			{
				NvParameterized::Interface* ei = d->FieldSamplers.buf[i];
				if (!ei)
				{
					continue;
				}
				if (nvidia::strcmp(ei->className(), HeatSourceData::staticClassName()) != 0)
				{
					continue;
				}
				HeatSourceData* ed = static_cast< HeatSourceData*>(ei);
				mTempNames.pushBack(ed->Name);
				mTempVariantNames.pushBack(HEAT_SOURCE_AUTHORING_TYPE_NAME);
			}
		}
	}
	else if (nvidia::strcmp(nameSpace, SUBSTANCE_SOURCE_AUTHORING_TYPE_NAME) == 0)
	{
		EffectPackageFieldSamplerDatabaseParams* d = static_cast< EffectPackageFieldSamplerDatabaseParams*>(mEffectPackageFieldSamplerDatabaseParams);
		if ( d )
		{
			for (uint32_t i = 0; i < (uint32_t)d->FieldSamplers.arraySizes[0]; i++)
			{
				NvParameterized::Interface* ei = d->FieldSamplers.buf[i];
				if (!ei)
				{
					continue;
				}
				if (nvidia::strcmp(ei->className(), SubstanceSourceData::staticClassName()) != 0)
				{
					continue;
				}
				SubstanceSourceData* ed = static_cast< SubstanceSourceData*>(ei);
				mTempNames.pushBack(ed->Name);
				mTempVariantNames.pushBack(SUBSTANCE_SOURCE_AUTHORING_TYPE_NAME);
			}
		}
	}
	else if (nvidia::strcmp(nameSpace, VELOCITY_SOURCE_AUTHORING_TYPE_NAME) == 0)
	{
		EffectPackageFieldSamplerDatabaseParams* d = static_cast< EffectPackageFieldSamplerDatabaseParams*>(mEffectPackageFieldSamplerDatabaseParams);
		if ( d )
		{
			for (uint32_t i = 0; i < (uint32_t)d->FieldSamplers.arraySizes[0]; i++)
			{
				NvParameterized::Interface* ei = d->FieldSamplers.buf[i];
				if (!ei)
				{
					continue;
				}
				if (nvidia::strcmp(ei->className(), VelocitySourceData::staticClassName()) != 0)
				{
					continue;
				}
				VelocitySourceData* ed = static_cast< VelocitySourceData*>(ei);
				mTempNames.pushBack(ed->Name);
				mTempVariantNames.pushBack(SUBSTANCE_SOURCE_AUTHORING_TYPE_NAME);
			}
		}
	}
	else if (nvidia::strcmp(nameSpace, JET_FS_AUTHORING_TYPE_NAME) == 0)
	{
		EffectPackageFieldSamplerDatabaseParams* d = static_cast< EffectPackageFieldSamplerDatabaseParams*>(mEffectPackageFieldSamplerDatabaseParams);
		if ( d ) 
		{
			for (uint32_t i = 0; i < (uint32_t)d->FieldSamplers.arraySizes[0]; i++)
			{
				NvParameterized::Interface* ei = d->FieldSamplers.buf[i];
				if (!ei)
				{
					continue;
				}
				if (nvidia::strcmp(ei->className(), JetFieldSamplerData::staticClassName()) != 0)
				{
					continue;
				}
				JetFieldSamplerData* ed = static_cast< JetFieldSamplerData*>(ei);
				mTempNames.pushBack(ed->Name);
				mTempVariantNames.pushBack(JET_FS_AUTHORING_TYPE_NAME);
			}
		}
	}
	else if (nvidia::strcmp(nameSpace, WIND_FS_AUTHORING_TYPE_NAME) == 0)
	{
		EffectPackageFieldSamplerDatabaseParams* d = static_cast< EffectPackageFieldSamplerDatabaseParams*>(mEffectPackageFieldSamplerDatabaseParams);
		if ( d ) 
		{
			for (uint32_t i = 0; i < (uint32_t)d->FieldSamplers.arraySizes[0]; i++)
			{
				NvParameterized::Interface* ei = d->FieldSamplers.buf[i];
				if (!ei)
				{
					continue;
				}
				if (nvidia::strcmp(ei->className(), WindFieldSamplerData::staticClassName()) != 0)
				{
					continue;
				}
				WindFieldSamplerData* ed = static_cast< WindFieldSamplerData*>(ei);
				mTempNames.pushBack(ed->Name);
				mTempVariantNames.pushBack(WIND_FS_AUTHORING_TYPE_NAME);
			}
		}
	}
	else if (nvidia::strcmp(nameSpace, NOISE_FS_AUTHORING_TYPE_NAME) == 0)
	{
		EffectPackageFieldSamplerDatabaseParams* d = static_cast< EffectPackageFieldSamplerDatabaseParams*>(mEffectPackageFieldSamplerDatabaseParams);
		if ( d )
		{
			for (uint32_t i = 0; i < (uint32_t)d->FieldSamplers.arraySizes[0]; i++)
			{
				NvParameterized::Interface* ei = d->FieldSamplers.buf[i];
				if (!ei)
				{
					continue;
				}
				if (nvidia::strcmp(ei->className(), NoiseFieldSamplerData::staticClassName()) != 0)
				{
					continue;
				}
				NoiseFieldSamplerData* ed = static_cast< NoiseFieldSamplerData*>(ei);
				mTempNames.pushBack(ed->Name);
				mTempVariantNames.pushBack(NOISE_FS_AUTHORING_TYPE_NAME);
			}
		}
	}
	else if (nvidia::strcmp(nameSpace, VORTEX_FS_AUTHORING_TYPE_NAME) == 0)
	{
		EffectPackageFieldSamplerDatabaseParams* d = static_cast< EffectPackageFieldSamplerDatabaseParams*>(mEffectPackageFieldSamplerDatabaseParams);
		if ( d ) 
		{
			for (uint32_t i = 0; i < (uint32_t)d->FieldSamplers.arraySizes[0]; i++)
			{
				NvParameterized::Interface* ei = d->FieldSamplers.buf[i];
				if (!ei)
				{
					continue;
				}
				if (nvidia::strcmp(ei->className(), VortexFieldSamplerData::staticClassName()) != 0)
				{
					continue;
				}
				VortexFieldSamplerData* ed = static_cast< VortexFieldSamplerData*>(ei);
				mTempNames.pushBack(ed->Name);
				mTempVariantNames.pushBack(VORTEX_FS_AUTHORING_TYPE_NAME);
			}
		}
	}
	else if (nvidia::strcmp(nameSpace, ATTRACTOR_FS_AUTHORING_TYPE_NAME) == 0)
	{
		EffectPackageFieldSamplerDatabaseParams* d = static_cast< EffectPackageFieldSamplerDatabaseParams*>(mEffectPackageFieldSamplerDatabaseParams);
		if ( d )
		{
			for (uint32_t i = 0; i < (uint32_t)d->FieldSamplers.arraySizes[0]; i++)
			{
				NvParameterized::Interface* ei = d->FieldSamplers.buf[i];
				if (!ei)
				{
					continue;
				}
				if (nvidia::strcmp(ei->className(), AttractorFieldSamplerData::staticClassName()) != 0)
				{
					continue;
				}
				AttractorFieldSamplerData* ed = static_cast< AttractorFieldSamplerData*>(ei);
				mTempNames.pushBack(ed->Name);
				mTempVariantNames.pushBack(ATTRACTOR_FS_AUTHORING_TYPE_NAME);
			}
		}
	}
	else if (nvidia::strcmp(nameSpace, TURBULENCE_FS_AUTHORING_TYPE_NAME) == 0)
	{
		EffectPackageFieldSamplerDatabaseParams* d = static_cast< EffectPackageFieldSamplerDatabaseParams*>(mEffectPackageFieldSamplerDatabaseParams);
		if ( d )
		{
			for (uint32_t i = 0; i < (uint32_t)d->FieldSamplers.arraySizes[0]; i++)
			{
				NvParameterized::Interface* ei = d->FieldSamplers.buf[i];
				if (!ei)
				{
					continue;
				}
				if (nvidia::strcmp(ei->className(), TurbulenceFieldSamplerData::staticClassName()) != 0)
				{
					continue;
				}
				TurbulenceFieldSamplerData* ed = static_cast< TurbulenceFieldSamplerData*>(ei);
				mTempNames.pushBack(ed->Name);
				mTempVariantNames.pushBack(TURBULENCE_FS_AUTHORING_TYPE_NAME);
			}
		}
	}
	else if (nvidia::strcmp(nameSpace, FORCEFIELD_AUTHORING_TYPE_NAME) == 0)
	{
		EffectPackageFieldSamplerDatabaseParams* d = static_cast< EffectPackageFieldSamplerDatabaseParams*>(mEffectPackageFieldSamplerDatabaseParams);
		if ( d )
		{
			for (uint32_t i = 0; i < (uint32_t)d->FieldSamplers.arraySizes[0]; i++)
			{
				NvParameterized::Interface* ei = d->FieldSamplers.buf[i];
				if (!ei)
				{
					continue;
				}
				if (nvidia::strcmp(ei->className(), ForceFieldData::staticClassName()) != 0)
				{
					continue;
				}
				ForceFieldData* ed = static_cast< ForceFieldData*>(ei);
				mTempNames.pushBack(ed->Name);
				mTempVariantNames.pushBack(FORCEFIELD_AUTHORING_TYPE_NAME);
			}
		}
	}
	else if (nvidia::strcmp(nameSpace, FLAME_EMITTER_AUTHORING_TYPE_NAME) == 0)
	{
		EffectPackageFieldSamplerDatabaseParams* d = static_cast< EffectPackageFieldSamplerDatabaseParams*>(mEffectPackageFieldSamplerDatabaseParams);
		if ( d )
		{
			for (uint32_t i = 0; i < (uint32_t)d->FieldSamplers.arraySizes[0]; i++)
			{
				NvParameterized::Interface* ei = d->FieldSamplers.buf[i];
				if (!ei)
				{
					continue;
				}
				if (nvidia::strcmp(ei->className(), FlameEmitterData::staticClassName()) != 0)
				{
					continue;
				}
				FlameEmitterData* ed = static_cast< FlameEmitterData*>(ei);
				mTempNames.pushBack(ed->Name);
				mTempVariantNames.pushBack(FLAME_EMITTER_AUTHORING_TYPE_NAME);
			}
		}
	}
	else
	{
		PX_ALWAYS_ASSERT();
	}

	nameCount = mTempNames.size();
	if (nameCount)
	{
		ret = &mTempNames[0];
		variants = &mTempVariantNames[0];
	}

	return ret;
}


bool ModuleParticlesImpl::setEffectPackageIOSDatabase(const NvParameterized::Interface* dataBase)
{
	WRITE_ZONE();
	bool ret = false;

	if (dataBase)
	{
		NvParameterized::Traits* traits = mSdk->getParameterizedTraits();
		const char* className = dataBase->className();
		if (nvidia::strcmp(className, basicios::BasicIOSAssetParam::staticClassName()) == 0 ||  nvidia::strcmp(className, pxparticleios::ParticleIosAssetParam::staticClassName()) == 0)
		{
			bool revised = false;
			const char* itemName = dataBase->name() ? dataBase->name()  : "defaultBasicIOS";
			EffectPackageIOSDatabaseParams* d = static_cast< EffectPackageIOSDatabaseParams*>(mEffectPackageIOSDatabaseParams);
			if ( d )
			{
				for (uint32_t i = 0; i < (uint32_t)d->ParticleSimulations.arraySizes[0]; i++)
				{
					NvParameterized::Interface* ei = d->ParticleSimulations.buf[i];
					if (!ei)
					{
						continue;
					}
					ParticleSimulationData* ed = static_cast< ParticleSimulationData*>(ei);
					if (nvidia::stricmp(ed->Name.buf, itemName) == 0)
					{
						ed->IOS->copy(*dataBase);
						initParticleSimulationData(ed);
						revised = true;
						ret = true;
						break;
					}
				}
			}
			if (!revised && mEffectPackageIOSDatabaseParams)
			{
				int32_t arraySize = d->ParticleSimulations.arraySizes[0];
				NvParameterized::Handle handle(mEffectPackageIOSDatabaseParams);
				NvParameterized::ErrorType err = handle.getParameter("ParticleSimulations");
				PX_ASSERT(err == NvParameterized::ERROR_NONE);
				err = handle.resizeArray(arraySize + 1);
				PX_ASSERT(err == NvParameterized::ERROR_NONE);
				if (err == NvParameterized::ERROR_NONE)
				{
					NvParameterized::Interface* ei = traits->createNvParameterized(ParticleSimulationData::staticClassName());
					if (ei)
					{
						ParticleSimulationData* ed = static_cast< ParticleSimulationData*>(ei);
						NvParameterized::Handle item(ei);
						err = item.getParameter("Name");
						PX_ASSERT(err == NvParameterized::ERROR_NONE);
						item.setParamString(itemName);
						ei->setName(itemName);
						d->ParticleSimulations.buf[arraySize] = ei;
						ed->IOS = NULL;
						NvParameterized::ErrorType err = dataBase->clone(ed->IOS);
						if ( err != NvParameterized::ERROR_NONE || ed->IOS == NULL )
						{
							APEX_DEBUG_WARNING("Failed to clone asset.");
						}
						else
						{
							initParticleSimulationData(ed);
						}
					}
					ret = true;
				}
			}
		}
		else if (nvidia::strcmp(dataBase->className(), EffectPackageIOSDatabaseParams::staticClassName()) == 0)
		{
			if (mEffectPackageIOSDatabaseParams && mEffectPackageIOSDatabaseParams != dataBase)
			{
				mEffectPackageIOSDatabaseParams->destroy();
				mEffectPackageIOSDatabaseParams = NULL;
			}
			if (mEffectPackageIOSDatabaseParams == NULL)
			{
				dataBase->clone(mEffectPackageIOSDatabaseParams);
				if ( mEffectPackageIOSDatabaseParams == NULL)
				{
					APEX_DEBUG_WARNING("Failed to clone asset.");
					mEffectPackageIOSDatabaseParams = traits->createNvParameterized(EffectPackageIOSDatabaseParams::staticClassName());
				}
			}

			EffectPackageIOSDatabaseParams* d = static_cast< EffectPackageIOSDatabaseParams*>(mEffectPackageIOSDatabaseParams);
			if ( d )
			{
				if (d->ParticleSimulations.arraySizes[0] == 0)
				{
					NvParameterized::Handle handle(mEffectPackageIOSDatabaseParams);
					NvParameterized::ErrorType err = handle.getParameter("ParticleSimulations");
					PX_ASSERT(err == NvParameterized::ERROR_NONE);
					err = handle.resizeArray(3);

					PX_ASSERT(err == NvParameterized::ERROR_NONE);
					if (err == NvParameterized::ERROR_NONE)
					{
						// create the default BasicIOS
						{
							NvParameterized::Interface* ei = traits->createNvParameterized(ParticleSimulationData::staticClassName());
							d->ParticleSimulations.buf[0] = ei;
							ParticleSimulationData* psd = static_cast< ParticleSimulationData*>(ei);
							if (psd)
							{
								NvParameterized::setParamString(*ei, "Name", "defaultBasicIOS");
								psd->IOS = traits->createNvParameterized(basicios::BasicIOSAssetParam::staticClassName());
							}
						}
						// create the default SimpleParticleIOS
						{
							NvParameterized::Interface* ei = traits->createNvParameterized(ParticleSimulationData::staticClassName());
							d->ParticleSimulations.buf[1] = ei;
							ParticleSimulationData* psd = static_cast< ParticleSimulationData*>(ei);
							if (psd)
							{
								NvParameterized::setParamString(*ei, "Name", "defaultSimpleParticleIOS");
								psd->IOS = traits->createNvParameterized(pxparticleios::ParticleIosAssetParam::staticClassName());
								pxparticleios::ParticleIosAssetParam* pia = static_cast< pxparticleios::ParticleIosAssetParam*>(psd->IOS);
								if (pia)
								{
									pia->particleType = traits->createNvParameterized(pxparticleios::SimpleParticleSystemParams::staticClassName());
								}
							}
						}
						// create the default FluidParticleIOS
						{
							NvParameterized::Interface* ei = traits->createNvParameterized(ParticleSimulationData::staticClassName());
							d->ParticleSimulations.buf[2] = ei;
							ParticleSimulationData* psd = static_cast< ParticleSimulationData*>(ei);

							if (psd)
							{
								NvParameterized::setParamString(*ei, "Name", "defaultFluidParticleIOS");
								psd->IOS = traits->createNvParameterized(pxparticleios::ParticleIosAssetParam::staticClassName());
								pxparticleios::ParticleIosAssetParam* pia = static_cast< pxparticleios::ParticleIosAssetParam*>(psd->IOS);

								if (pia)
								{
									pia->particleType = traits->createNvParameterized(pxparticleios::FluidParticleSystemParams::staticClassName());
									pxparticleios::FluidParticleSystemParams* fp = static_cast< pxparticleios::FluidParticleSystemParams*>(pia->particleType);
									if (fp)
									{
										fp->restParticleDistance = 0.1f;
										fp->stiffness = 8;
										fp->viscosity = 5;
									}
									pia->maxParticleCount = 200000;
									pia->particleRadius = 1;
									pia->maxInjectedParticleCount = 0.1f;
									pia->maxMotionDistance = 0.2;
									pia->contactOffset = 0.008f;
									pia->restOffset = 0.004f;
									pia->gridSize = 1.5f;
									pia->damping = 0;
									pia->externalAcceleration = PxVec3(0, 0, 0);
									pia->projectionPlaneNormal = PxVec3(0, 1, 0);
									pia->projectionPlaneDistance = 0;
									pia->particleMass = 0.1f;
									pia->restitution = 0.15f;
									pia->dynamicFriction = 0.008f;
									pia->staticFriction = 0.1f;
									pia->CollisionTwoway = false;
									pia->CollisionWithDynamicActors = true;
									pia->Enable = true;
									pia->ProjectToPlane = false;
									pia->PerParticleRestOffset = false;
									pia->PerParticleCollisionCacheHint = false;
								}
							}
						}
						ret = true;
					}
				}

				Array< const char* > nameList;
				for (int32_t i = 0; i < d->ParticleSimulations.arraySizes[0]; i++)
				{
					ParticleSimulationData* e = static_cast< ParticleSimulationData*>(d->ParticleSimulations.buf[i]);
					if (e)
					{
						for (uint32_t j = 0; j < nameList.size(); j++)
						{
							if (nvidia::stricmp(nameList[j], e->Name) == 0)
							{
								NvParameterized::Handle handle(e);
								NvParameterized::ErrorType err = handle.getParameter("Name");
								PX_ASSERT(err == NvParameterized::ERROR_NONE);
								if (err == NvParameterized::ERROR_NONE)
								{
									handle.setParamString(getUniqueName(nameList, e->Name));
									ret = true;
									break;
								}
							}
						}
						nameList.pushBack(e->Name);
					}
				}


				for (uint32_t i = 0; i < (uint32_t)d->ParticleSimulations.arraySizes[0]; i++)
				{
					ParticleSimulationData* ed = static_cast< ParticleSimulationData* >(d->ParticleSimulations.buf[i]);
					PX_ASSERT(ed);
					if (ed && initParticleSimulationData(ed))
					{
						ret = true;
					}
				}
			}
		}
		else
		{
			PX_ALWAYS_ASSERT(); // not a valid NvParameterized interface
		}
		if (fixupNamedReferences())
		{
			ret = true;
		}
	}
	return ret;
}

static void initColor(iofx::ColorVsLifeCompositeModifierParamsNS::colorLifeStruct_Type&  c, float lifeRemaining, float red, float green, float blue, float alpha)
{
	c.lifeRemaining = lifeRemaining;
	c.color.x = red;
	c.color.y = green;
	c.color.z = blue;
	c.color.w = alpha;
}

static void initScale(iofx::ScaleVsLife2DModifierParamsNS::scaleLifeStruct_Type& s, float lifetime, float scalex, float scaley)
{
	s.lifeRemaining = lifetime;
	s.scale.x = scalex;
	s.scale.y = scaley;
}


bool ModuleParticlesImpl::setEffectPackageIOFXDatabase(const NvParameterized::Interface* dataBase)
{
	WRITE_ZONE();
	bool ret = false;

	if (dataBase)
	{
		NvParameterized::Traits* traits = mSdk->getParameterizedTraits();
		const char* className = dataBase->className();
		if (nvidia::strcmp(className, iofx::IofxAssetParameters::staticClassName()) == 0)
		{
			bool revised = false;
			const char* itemName = dataBase->name();
			EffectPackageIOFXDatabaseParams* d = static_cast< EffectPackageIOFXDatabaseParams*>(mEffectPackageIOFXDatabaseParams);
			if ( d )
			{
				for (uint32_t i = 0; i < (uint32_t)d->GraphicsEffects.arraySizes[0]; i++)
				{
					NvParameterized::Interface* ei = d->GraphicsEffects.buf[i];
					if (!ei)
					{
						continue;
					}
					GraphicsEffectData* ed = static_cast< GraphicsEffectData*>(ei);
					if (nvidia::stricmp(ed->Name.buf, itemName) == 0)
					{
						ed->IOFX->copy(*dataBase);
						revised = true;
						ret = true;
						break;
					}
				}
			}
			if (!revised && mEffectPackageIOFXDatabaseParams)
			{
				int32_t arraySize = d->GraphicsEffects.arraySizes[0];
				NvParameterized::Handle handle(mEffectPackageIOFXDatabaseParams);
				NvParameterized::ErrorType err = handle.getParameter("GraphicsEffects");
				PX_ASSERT(err == NvParameterized::ERROR_NONE);
				err = handle.resizeArray(arraySize + 1);
				PX_ASSERT(err == NvParameterized::ERROR_NONE);
				if (err == NvParameterized::ERROR_NONE)
				{
					NvParameterized::Interface* ei = traits->createNvParameterized(GraphicsEffectData::staticClassName());
					if (ei)
					{
						GraphicsEffectData* ed = static_cast< GraphicsEffectData*>(ei);
						NvParameterized::Handle item(ei);
						err = item.getParameter("Name");
						PX_ASSERT(err == NvParameterized::ERROR_NONE);
						item.setParamString(itemName);
						ei->setName(itemName);
						d->GraphicsEffects.buf[arraySize] = ei;
						ed->IOFX = NULL;
						dataBase->clone(ed->IOFX);
						if ( ed->IOFX == NULL )
						{
							APEX_DEBUG_WARNING("Failed to clone asset.");
						}
					}
					ret = true;
				}
			}
		}
		else if (nvidia::strcmp(dataBase->className(), EffectPackageIOFXDatabaseParams::staticClassName()) == 0)
		{
			if (mEffectPackageIOFXDatabaseParams && mEffectPackageIOFXDatabaseParams != dataBase)
			{
				mEffectPackageIOFXDatabaseParams->destroy();
				mEffectPackageIOFXDatabaseParams = NULL;
			}
			if (mEffectPackageIOFXDatabaseParams == NULL)
			{
				dataBase->clone(mEffectPackageIOFXDatabaseParams);
				if ( mEffectPackageIOFXDatabaseParams == NULL )
				{
					APEX_DEBUG_WARNING("Failed to clone asset.");
					mEffectPackageIOFXDatabaseParams = traits->createNvParameterized(EffectPackageIOFXDatabaseParams::staticClassName());
				}
			}
			EffectPackageIOFXDatabaseParams* d = static_cast< EffectPackageIOFXDatabaseParams*>(mEffectPackageIOFXDatabaseParams);
			if ( d )
			{
				if (d->GraphicsEffects.arraySizes[0] == 0)
				{
					NvParameterized::Handle handle(mEffectPackageIOFXDatabaseParams);
					NvParameterized::ErrorType err = handle.getParameter("GraphicsEffects");
					PX_ASSERT(err == NvParameterized::ERROR_NONE);
					err = handle.resizeArray(2);
					PX_ASSERT(err == NvParameterized::ERROR_NONE);
					if (err == NvParameterized::ERROR_NONE)
					{
						{
							NvParameterized::Interface* ei = traits->createNvParameterized(GraphicsEffectData::staticClassName());
							d->GraphicsEffects.buf[0] = ei;
							if (ei)
							{
								GraphicsEffectData* ged = static_cast< GraphicsEffectData*>(ei);
								if (ged)
								{
									NvParameterized::setParamString(*ei, "Name", "defaultSpriteIOFX");
									ged->IOFX = traits->createNvParameterized(iofx::IofxAssetParameters::staticClassName());
									iofx::IofxAssetParameters* iofx = static_cast< iofx::IofxAssetParameters*>(ged->IOFX);
									if (iofx)
									{
										iofx->iofxType = traits->createNvParameterized(iofx::SpriteIofxParameters::staticClassName());
										iofx->renderOutput.useFloat4Color = true;
									}
								}
							}
						}
						{
							NvParameterized::Interface* ei = traits->createNvParameterized(GraphicsEffectData::staticClassName());
							d->GraphicsEffects.buf[1] = ei;
							if (ei)
							{
								GraphicsEffectData* ged = static_cast< GraphicsEffectData*>(ei);
								if (ged)
								{
									NvParameterized::setParamString(*ei, "Name", "defaultMeshIOFX");
									ged->IOFX = traits->createNvParameterized(iofx::IofxAssetParameters::staticClassName());
									iofx::IofxAssetParameters* iofx = static_cast< iofx::IofxAssetParameters*>(ged->IOFX);
									if (iofx)
									{
										iofx->iofxType = traits->createNvParameterized(iofx::MeshIofxParameters::staticClassName());
										iofx->renderOutput.useFloat4Color = true;
									}
								}
							}
						}

						ret = true;
					}
				}


				Array< const char* > nameList;
				for (int32_t i = 0; i < d->GraphicsEffects.arraySizes[0]; i++)
				{
					GraphicsEffectData* e = static_cast< GraphicsEffectData*>(d->GraphicsEffects.buf[i]);
					if (e)
					{
						for (uint32_t j = 0; j < nameList.size(); j++)
						{
							if (nvidia::stricmp(nameList[j], e->Name) == 0)
							{
								NvParameterized::Handle handle(e);
								NvParameterized::ErrorType err = handle.getParameter("Name");
								PX_ASSERT(err == NvParameterized::ERROR_NONE);
								if (err == NvParameterized::ERROR_NONE)
								{
									handle.setParamString(getUniqueName(nameList, e->Name));
									ret = true;
									break;
								}
							}
						}
						nameList.pushBack(e->Name);
					}
				}


				for (uint32_t i = 0; i < (uint32_t)d->GraphicsEffects.arraySizes[0]; i++)
				{
					GraphicsEffectData* ed = static_cast< GraphicsEffectData*>(d->GraphicsEffects.buf[i]);
					// first, see if it's assigned to a sprite IOFX without valid default values...
					{
						iofx::IofxAssetParameters* ae = static_cast< iofx::IofxAssetParameters*>(ed->IOFX);
						if (ae)
						{
							if (nvidia::strcmp(ae->iofxType->className(), iofx::SpriteIofxParameters::staticClassName()) == 0)
							{
								iofx::SpriteIofxParameters* iofxP = static_cast< iofx::SpriteIofxParameters*>(ae->iofxType);
								if (iofxP->spawnModifierList.arraySizes[0] == 0 && iofxP->continuousModifierList.arraySizes[0] == 0)
								{
									ed->IOFX->destroy();
									ed->IOFX = NULL;
									ret = true;
								}
							}
						}
					}

					if (ed->IOFX == NULL)
					{
						ed->IOFX = traits->createNvParameterized(iofx::IofxAssetParameters::staticClassName());
						iofx::IofxAssetParameters* ae = static_cast< iofx::IofxAssetParameters*>(ed->IOFX);
						if (ae)
						{
							ae->renderOutput.useFloat4Color = true;
							NvParameterized::Interface* iofx = traits->createNvParameterized(iofx::SpriteIofxParameters::staticClassName());
							ae->iofxType = iofx;
							iofx::SpriteIofxParameters* iofxP = static_cast< iofx::SpriteIofxParameters*>(iofx);
							if (iofxP)
							{
								{
									NvParameterized::Handle handle(iofx);
									NvParameterized::ErrorType err = handle.getParameter("spriteMaterialName");
									PX_ASSERT(err == NvParameterized::ERROR_NONE);
									if (err == NvParameterized::ERROR_NONE)
									{
										err = handle.initParamRef(APEX_MATERIALS_NAME_SPACE, true);
										PX_ASSERT(err == NvParameterized::ERROR_NONE);
										if (err == NvParameterized::ERROR_NONE)
										{
											iofxP->spriteMaterialName->setName("defaultGraphicsMaterial");
										}
									}
								}

								{
									NvParameterized::Handle handle(iofx);
									NvParameterized::ErrorType err = handle.getParameter("spawnModifierList");
									PX_ASSERT(err == NvParameterized::ERROR_NONE);
									err = handle.resizeArray(4);
									PX_ASSERT(err == NvParameterized::ERROR_NONE);
									if (err == NvParameterized::ERROR_NONE)
									{
										iofxP->spawnModifierList.buf[0] = traits->createNvParameterized(iofx::InitialColorModifierParams::staticClassName());
										iofxP->spawnModifierList.buf[1] = traits->createNvParameterized(iofx::RandomRotationModifierParams::staticClassName());
										iofxP->spawnModifierList.buf[2] = traits->createNvParameterized(iofx::RandomSubtextureModifierParams::staticClassName());
										iofxP->spawnModifierList.buf[3] = traits->createNvParameterized(iofx::RandomScaleModifierParams::staticClassName());
									}
								}

								{
									NvParameterized::Handle handle(iofx);
									NvParameterized::ErrorType err = handle.getParameter("continuousModifierList");
									PX_ASSERT(err == NvParameterized::ERROR_NONE);
									err = handle.resizeArray(1);
									PX_ASSERT(err == NvParameterized::ERROR_NONE);
									if (err == NvParameterized::ERROR_NONE)
									{
										iofxP->continuousModifierList.buf[0] = traits->createNvParameterized(iofx::ViewDirectionSortingModifierParams::staticClassName());
									}
								}
							}
						}
						ret = true;
					}
					if (ed->IOFX)
					{
						iofx::IofxAssetParameters* ae = static_cast< iofx::IofxAssetParameters*>(ed->IOFX);
						if (ae->iofxType)
						{
							ae->renderOutput.useFloat4Color = true;
							const char* c = ae->iofxType->className();
							if (c && nvidia::strcmp(c, "MeshIofxParameters") == 0)
							{
								iofx::MeshIofxParameters* m = static_cast< iofx::MeshIofxParameters*>(ae->iofxType);
								if (m->renderMeshList.arraySizes[0] == 0)
								{
									ret = true;
									NvParameterized::Handle handle(m);
									NvParameterized::ErrorType err = handle.getParameter("renderMeshList");
									PX_ASSERT(err == NvParameterized::ERROR_NONE);
									err = handle.resizeArray(1);
									PX_ASSERT(err == NvParameterized::ERROR_NONE);
									if (err == NvParameterized::ERROR_NONE)
									{
										iofx::MeshIofxParametersNS::meshProperties_Type& t = m->renderMeshList.buf[0];
										t.weight = 1;
										NvParameterized::Handle elementHandle(m);
										err = handle.getChildHandle(0, elementHandle);
										PX_ASSERT(err == NvParameterized::ERROR_NONE);
										NvParameterized::Handle item(*elementHandle.getInterface());
										NvParameterized::ErrorType err = elementHandle.getChildHandle(elementHandle.getInterface(), "meshAssetName", item);
										PX_ASSERT(err == NvParameterized:: ERROR_NONE);
										if (err == NvParameterized::ERROR_NONE)
										{
											err = item.initParamRef(APEX_OPAQUE_MESH_NAME_SPACE,true);
											PX_ASSERT(err == NvParameterized::ERROR_NONE);
											if (err == NvParameterized::ERROR_NONE)
											{
												t.meshAssetName->setName("SampleMesh.apx");
											}
										}
									}
									// initialize the spawn modifier list
									{
										NvParameterized::Handle handle(m);
										NvParameterized::ErrorType err = handle.getParameter("spawnModifierList");
										PX_ASSERT(err == NvParameterized::ERROR_NONE);
										err = handle.resizeArray(2);
										PX_ASSERT(err == NvParameterized::ERROR_NONE);
										if (err == NvParameterized::ERROR_NONE)
										{
											m->spawnModifierList.buf[0] = traits->createNvParameterized(iofx::SimpleScaleModifierParams::staticClassName());
											iofx::SimpleScaleModifierParams* ss = static_cast< iofx::SimpleScaleModifierParams*>(m->spawnModifierList.buf[0]);
											if (ss)
											{
												ss->scaleFactor = PxVec3(1, 1, 1);
												m->spawnModifierList.buf[1] = traits->createNvParameterized(iofx::RotationModifierParams::staticClassName());
												NvParameterized::setParamEnum(*m->spawnModifierList.buf[1], "rollType", "SPHERICAL");
											}
										}
									}
									// initialize the continuous modifier list
									{
										NvParameterized::Handle handle(m);
										NvParameterized::ErrorType err = handle.getParameter("continuousModifierList");
										PX_ASSERT(err == NvParameterized::ERROR_NONE);
										err = handle.resizeArray(2);
										PX_ASSERT(err == NvParameterized::ERROR_NONE);
										if (err == NvParameterized::ERROR_NONE)
										{
											// init RotationModifierParams
											{
												m->continuousModifierList.buf[0] = traits->createNvParameterized(iofx::RotationModifierParams::staticClassName());
												if (m->continuousModifierList.buf[0])
												{
													NvParameterized::setParamEnum(*m->continuousModifierList.buf[0], "rollType", "SPHERICAL");
													iofx::RotationModifierParams* rmp = static_cast< iofx::RotationModifierParams*>(m->continuousModifierList.buf[0]);
													rmp->maxRotationRatePerSec = 0;
													rmp->maxSettleRatePerSec = 1;
													rmp->collisionRotationMultiplier = 1;
													rmp->inAirRotationMultiplier = 1;
												}
											}
											{
												m->continuousModifierList.buf[1] = traits->createNvParameterized(iofx::ScaleVsLife3DModifierParams::staticClassName());
												if (m->continuousModifierList.buf[1])
												{
													NvParameterized::Handle h(m->continuousModifierList.buf[1]);
													err = h.getParameter("controlPoints");
													PX_ASSERT(err == NvParameterized::ERROR_NONE);
													err = h.resizeArray(3);
													iofx::ScaleVsLife3DModifierParams& svl = *(static_cast< iofx::ScaleVsLife3DModifierParams*>(m->continuousModifierList.buf[1]));
													svl.controlPoints.buf[0].lifeRemaining = 0;
													svl.controlPoints.buf[0].scale = PxVec3(0, 0, 0);

													svl.controlPoints.buf[1].lifeRemaining = 0.5f;
													svl.controlPoints.buf[1].scale = PxVec3(1, 1, 1);

													svl.controlPoints.buf[2].lifeRemaining = 1;
													svl.controlPoints.buf[2].scale = PxVec3(0, 0, 0);

												}
											}
										}

									}
								}
							}
						}
					}
				}
			}
		}
		else
		{
			PX_ALWAYS_ASSERT();
		}
	}


	return ret;

}

bool ModuleParticlesImpl::setEffectPackageEmitterDatabase(const NvParameterized::Interface* dataBase)
{
	WRITE_ZONE();
	bool ret = false;

	if (dataBase)
	{
		NvParameterized::Traits* traits = mSdk->getParameterizedTraits();
		const char* className = dataBase->className();
		if (nvidia::strcmp(className, emitter::ApexEmitterAssetParameters::staticClassName()) == 0)
		{
			bool revised = false;
			const char* itemName = dataBase->name();
			EffectPackageEmitterDatabaseParams* d = static_cast< EffectPackageEmitterDatabaseParams*>(mEffectPackageEmitterDatabaseParams);
			if ( d ) 
			{
				for (uint32_t i = 0; i < (uint32_t)d->Emitters.arraySizes[0]; i++)
				{
					NvParameterized::Interface* ei = d->Emitters.buf[i];
					if (!ei)
					{
						continue;
					}
					EmitterData* ed = static_cast< EmitterData*>(ei);
					if (nvidia::stricmp(ed->Name.buf, itemName) == 0)
					{
						ed->Emitter->copy(*dataBase);
						revised = true;
						ret = true;
						break;
					}
				}
			}
			if (!revised && mEffectPackageEmitterDatabaseParams)
			{
				int32_t arraySize = d->Emitters.arraySizes[0];
				NvParameterized::Handle handle(mEffectPackageEmitterDatabaseParams);
				NvParameterized::ErrorType err = handle.getParameter("Emitters");
				PX_ASSERT(err == NvParameterized::ERROR_NONE);
				err = handle.resizeArray(arraySize + 1);
				PX_ASSERT(err == NvParameterized::ERROR_NONE);
				if (err == NvParameterized::ERROR_NONE)
				{
					NvParameterized::Interface* ei = traits->createNvParameterized(EmitterData::staticClassName());
					if (ei)
					{
						EmitterData* ed = static_cast< EmitterData*>(ei);
						NvParameterized::Handle item(ei);
						err = item.getParameter("Name");
						PX_ASSERT(err == NvParameterized::ERROR_NONE);
						item.setParamString(itemName);
						ei->setName(itemName);
						d->Emitters.buf[arraySize] = ei;
						ed->Emitter = NULL;
						dataBase->clone(ed->Emitter);
						if ( ed->Emitter == NULL )
						{
							APEX_DEBUG_WARNING("Failed to clone asset.");
						}
					}
					ret = true;
				}
			}
		}
		else if (nvidia::strcmp(dataBase->className(), EffectPackageEmitterDatabaseParams::staticClassName()) == 0)
		{
			if (mEffectPackageEmitterDatabaseParams && mEffectPackageEmitterDatabaseParams != dataBase)
			{
				mEffectPackageEmitterDatabaseParams->destroy();
				mEffectPackageEmitterDatabaseParams = NULL;
			}
			if (mEffectPackageEmitterDatabaseParams == NULL)
			{
				dataBase->clone(mEffectPackageEmitterDatabaseParams);
				if ( mEffectPackageEmitterDatabaseParams == NULL )
				{
					APEX_DEBUG_WARNING("Failed to clone asset.");
					mEffectPackageEmitterDatabaseParams = traits->createNvParameterized(EffectPackageEmitterDatabaseParams::staticClassName());
				}
			}

			EffectPackageEmitterDatabaseParams* d = static_cast< EffectPackageEmitterDatabaseParams*>(mEffectPackageEmitterDatabaseParams);
			if ( d )
			{
				if (d->Emitters.arraySizes[0] == 0)
				{
					NvParameterized::Handle handle(mEffectPackageEmitterDatabaseParams);
					NvParameterized::ErrorType err = handle.getParameter("Emitters");
					PX_ASSERT(err == NvParameterized::ERROR_NONE);
					err = handle.resizeArray(1);
					PX_ASSERT(err == NvParameterized::ERROR_NONE);
					if (err == NvParameterized::ERROR_NONE)
					{
						NvParameterized::Interface* ei = traits->createNvParameterized(EmitterData::staticClassName());
						d->Emitters.buf[0] = ei;
						ret = true;
					}
				}


				Array< const char* > nameList;
				for (int32_t i = 0; i < d->Emitters.arraySizes[0]; i++)
				{
					EmitterData* e = static_cast< EmitterData*>(d->Emitters.buf[i]);
					if (e)
					{
						for (uint32_t j = 0; j < nameList.size(); j++)
						{
							if (nvidia::stricmp(nameList[j], e->Name) == 0)
							{
								NvParameterized::Handle handle(e);
								NvParameterized::ErrorType err = handle.getParameter("Name");
								PX_ASSERT(err == NvParameterized::ERROR_NONE);
								if (err == NvParameterized::ERROR_NONE)
								{
									handle.setParamString(getUniqueName(nameList, e->Name));
									ret = true;
									break;
								}
							}
						}
						nameList.pushBack(e->Name);
					}
				}

				for (uint32_t i = 0; i < (uint32_t)d->Emitters.arraySizes[0]; i++)
				{
					EmitterData* ed = static_cast< EmitterData*>(d->Emitters.buf[i]);
					if (ed->Emitter == NULL)
					{
						ed->Emitter = traits->createNvParameterized(emitter::ApexEmitterAssetParameters::staticClassName());
						// Modify "static_cast<emitter::ApexEmitterAssetParameters*>(ed->Emitter)" if default ApexEmitterAssetParameters initialisation is insufficiently
						ret = true;
					}
					if (ed->Emitter)
					{
						emitter::ApexEmitterAssetParameters* ae = static_cast< emitter::ApexEmitterAssetParameters*>(ed->Emitter);
						NvParameterized::Handle handle(ed->Emitter);
						NvParameterized::ErrorType err = ed->Emitter->getParameterHandle("iofxAssetName", handle);
						PX_ASSERT(err == NvParameterized::ERROR_NONE);
						if (err == NvParameterized::ERROR_NONE)
						{
							NvParameterized::Interface* val = NULL;
							err = handle.getParamRef(val);
							if (val == NULL)
							{
								err = handle.initParamRef(IOFX_AUTHORING_TYPE_NAME,true);
								PX_ASSERT(err == NvParameterized::ERROR_NONE);
								if (err == NvParameterized::ERROR_NONE)
								{
									char scratch[512];
									sprintf_s(scratch, 512, "defaultSpriteIOFX");
									ae->iofxAssetName->setName(scratch);
								}
							}
						}
					}
					if (ed->Emitter)
					{
						emitter::ApexEmitterAssetParameters* ae = static_cast< emitter::ApexEmitterAssetParameters*>(ed->Emitter);
						NvParameterized::Handle handle(ed->Emitter);
						NvParameterized::ErrorType err = ed->Emitter->getParameterHandle("iosAssetName", handle);
						PX_ASSERT(err == NvParameterized::ERROR_NONE);
						if (err == NvParameterized::ERROR_NONE)
						{
							NvParameterized::Interface* val = NULL;
							err = handle.getParamRef(val);
							if (val == NULL)
							{
								err = handle.initParamRef(BASIC_IOS_AUTHORING_TYPE_NAME,true);
								PX_ASSERT(err == NvParameterized::ERROR_NONE);
								if (err == NvParameterized::ERROR_NONE)
								{
									char scratch[512];
									sprintf_s(scratch, 512, "defaultBasicIOS");
									ae->iosAssetName->setName(scratch);
								}
							}
						}
					}
				}
			}
		}
		else
		{
			PX_ALWAYS_ASSERT(); // invalid NvParameterized::Interface passed in
		}
	}


	return ret;
}

enum FieldSamplerType
{
	FST_NONE,
	FST_FORCE_FIELD,
	FST_HEAT_SOURCE,
	FST_SUBSTANCE_SOURCE,
	FST_JET,
	FST_ATTRACTOR,
	FST_TURBULENCE,
	FST_NOISE,
	FST_VORTEX,
	FST_WIND,
	FST_VELOCITY_SOURCE,
	FST_FLAME_EMITTER,
	FST_LAST
};

static FieldSamplerType getFieldSamplerType(const NvParameterized::Interface* iface)
{
	FieldSamplerType ret = FST_NONE;

	if (iface)
	{
		const char* className = iface->className();
		if (nvidia::strcmp(className, forcefield::ForceFieldAssetParams::staticClassName()) == 0)
		{
			ret = FST_FORCE_FIELD;
		}
		else if (nvidia::strcmp(className, turbulencefs::HeatSourceAssetParams::staticClassName()) == 0)
		{
			ret = FST_HEAT_SOURCE;
		}
		else if (nvidia::strcmp(className, turbulencefs::SubstanceSourceAssetParams::staticClassName()) == 0)
		{
			ret = FST_SUBSTANCE_SOURCE;
		}
		else if (nvidia::strcmp(className, turbulencefs::VelocitySourceAssetParams::staticClassName()) == 0)
		{
			ret = FST_VELOCITY_SOURCE;
		}
		else if (nvidia::strcmp(className, basicfs::JetFSAssetParams::staticClassName()) == 0)
		{
			ret = FST_JET;
		}
		else if (nvidia::strcmp(className, basicfs::WindFSAssetParams::staticClassName()) == 0)
		{
			ret = FST_WIND;
		}
		else if (nvidia::strcmp(className, basicfs::NoiseFSAssetParams::staticClassName()) == 0)
		{
			ret = FST_NOISE;
		}
		else if (nvidia::strcmp(className, basicfs::VortexFSAssetParams::staticClassName()) == 0)
		{
			ret = FST_VORTEX;
		}
		else if (nvidia::strcmp(className, basicfs::AttractorFSAssetParams::staticClassName()) == 0)
		{
			ret = FST_ATTRACTOR;
		}
		else if (nvidia::strcmp(className, turbulencefs::TurbulenceFSAssetParams::staticClassName()) == 0)
		{
			ret = FST_TURBULENCE;
		}
		else if (nvidia::strcmp(className, turbulencefs::FlameEmitterAssetParams::staticClassName()) == 0)
		{
			ret = FST_FLAME_EMITTER;
		}
		else if (nvidia::strcmp(className, particles::ForceFieldData::staticClassName()) == 0)
		{
			ret = FST_FORCE_FIELD;
		}
		else if (nvidia::strcmp(className, particles::HeatSourceData::staticClassName()) == 0)
		{
			ret = FST_HEAT_SOURCE;
		}
		else if (nvidia::strcmp(className, particles::SubstanceSourceData::staticClassName()) == 0)
		{
			ret = FST_SUBSTANCE_SOURCE;
		}
		else if (nvidia::strcmp(className, particles::VelocitySourceData::staticClassName()) == 0)
		{
			ret = FST_VELOCITY_SOURCE;
		}
		else if (nvidia::strcmp(className, particles::AttractorFieldSamplerData::staticClassName()) == 0)
		{
			ret = FST_ATTRACTOR;
		}
		else if (nvidia::strcmp(className, particles::JetFieldSamplerData::staticClassName()) == 0)
		{
			ret = FST_JET;
		}
		else if (nvidia::strcmp(className, particles::WindFieldSamplerData::staticClassName()) == 0)
		{
			ret = FST_WIND;
		}
		else if (nvidia::strcmp(className, particles::NoiseFieldSamplerData::staticClassName()) == 0)
		{
			ret = FST_NOISE;
		}
		else if (nvidia::strcmp(className, particles::TurbulenceFieldSamplerData::staticClassName()) == 0)
		{
			ret = FST_TURBULENCE;
		}
		else if (nvidia::strcmp(className, particles::VortexFieldSamplerData::staticClassName()) == 0)
		{
			ret = FST_VORTEX;
		}
		else if (nvidia::strcmp(className, particles::FlameEmitterData::staticClassName()) == 0)
		{
			ret = FST_FLAME_EMITTER;
		}
	}

	return ret;
}

static NvParameterized::Interface* getFieldSamplerData(FieldSamplerType fst, NvParameterized::Traits* traits, const NvParameterized::Interface* dataBase)
{
	NvParameterized::Interface* ret = NULL;

	const char* itemName = dataBase->name();
	NvParameterized::ErrorType err;

	switch (fst)
	{
	case FST_FORCE_FIELD:
	{
		NvParameterized::Interface* ei = traits->createNvParameterized(ForceFieldData::staticClassName());
		ForceFieldData* ed = static_cast< ForceFieldData*>(ei);
		if (ed)
		{
			NvParameterized::Handle item(ei);
			err = item.getParameter("Name");
			PX_ASSERT(err == NvParameterized::ERROR_NONE);
			item.setParamString(itemName);
			ei->setName(itemName);
			ret = ei;
			ed->ForceField = NULL;
			dataBase->clone(ed->ForceField);
			if ( ed->ForceField == NULL )
			{
				APEX_DEBUG_WARNING("Failed to clone asset.");
			}
		}
	}
	break;
	case FST_HEAT_SOURCE:
	{
		NvParameterized::Interface* ei = traits->createNvParameterized(HeatSourceData::staticClassName());
		HeatSourceData* ed = static_cast< HeatSourceData*>(ei);
		if (ed)
		{
			NvParameterized::Handle item(ei);
			err = item.getParameter("Name");
			PX_ASSERT(err == NvParameterized::ERROR_NONE);
			item.setParamString(itemName);
			ei->setName(itemName);
			ret = ei;
			ed->HeatSource = NULL;
			dataBase->clone(ed->HeatSource);
			if ( ed->HeatSource == NULL )
			{
				APEX_DEBUG_WARNING("Failed to clone asset.");
			}
		}
	}
	break;
	case FST_SUBSTANCE_SOURCE:
		{
			NvParameterized::Interface* ei = traits->createNvParameterized(SubstanceSourceData::staticClassName());
			SubstanceSourceData* ed = static_cast< SubstanceSourceData*>(ei);
			if (ed)
			{
				NvParameterized::Handle item(ei);
				err = item.getParameter("Name");
				PX_ASSERT(err == NvParameterized::ERROR_NONE);
				item.setParamString(itemName);
				ei->setName(itemName);
				ret = ei;
				ed->SubstanceSource = NULL;
				dataBase->clone(ed->SubstanceSource);
				if ( ed->SubstanceSource == NULL )
				{
					APEX_DEBUG_WARNING("Failed to clone asset.");
				}
			}
		}
		break;
	case FST_VELOCITY_SOURCE:
		{
			NvParameterized::Interface* ei = traits->createNvParameterized(VelocitySourceData::staticClassName());
			VelocitySourceData* ed = static_cast< VelocitySourceData*>(ei);
			if (ed)
			{
				NvParameterized::Handle item(ei);
				err = item.getParameter("Name");
				PX_ASSERT(err == NvParameterized::ERROR_NONE);
				item.setParamString(itemName);
				ei->setName(itemName);
				ret = ei;
				ed->VelocitySource = NULL;
				dataBase->clone(ed->VelocitySource);
				if ( ed->VelocitySource == NULL )
				{
					APEX_DEBUG_WARNING("Failed to clone asset.");
				}
			}
		}
		break;
	case FST_ATTRACTOR:
	{
		NvParameterized::Interface* ei = traits->createNvParameterized(AttractorFieldSamplerData::staticClassName());
		AttractorFieldSamplerData* ed = static_cast< AttractorFieldSamplerData*>(ei);
		if (ed)
		{
			NvParameterized::Handle item(ei);
			err = item.getParameter("Name");
			PX_ASSERT(err == NvParameterized::ERROR_NONE);
			item.setParamString(itemName);
			ei->setName(itemName);
			ret = ei;
			ed->AttractorFieldSampler = NULL;
			dataBase->clone(ed->AttractorFieldSampler);
			if ( ed->AttractorFieldSampler == NULL )
			{
				APEX_DEBUG_WARNING("Failed to clone asset.");
			}
		}
	}
	break;
	case FST_VORTEX:
	{
		NvParameterized::Interface* ei = traits->createNvParameterized(VortexFieldSamplerData::staticClassName());
		VortexFieldSamplerData* ed = static_cast< VortexFieldSamplerData*>(ei);
		if (ed)
		{
			NvParameterized::Handle item(ei);
			err = item.getParameter("Name");
			PX_ASSERT(err == NvParameterized::ERROR_NONE);
			item.setParamString(itemName);
			ei->setName(itemName);
			ret = ei;
			ed->VortexFieldSampler = NULL;
			dataBase->clone(ed->VortexFieldSampler);
			if ( ed->VortexFieldSampler == NULL )
			{
				APEX_DEBUG_WARNING("Failed to clone asset.");
			}
		}
	}
	break;
	case FST_NOISE:
	{
		NvParameterized::Interface* ei = traits->createNvParameterized(NoiseFieldSamplerData::staticClassName());
		NoiseFieldSamplerData* ed = static_cast< NoiseFieldSamplerData*>(ei);
		if (ed)
		{
			NvParameterized::Handle item(ei);
			err = item.getParameter("Name");
			PX_ASSERT(err == NvParameterized::ERROR_NONE);
			item.setParamString(itemName);
			ei->setName(itemName);
			ret = ei;
			ed->NoiseFieldSampler = NULL;
			dataBase->clone(ed->NoiseFieldSampler);
			if ( ed->NoiseFieldSampler == NULL )
			{
				APEX_DEBUG_WARNING("Failed to clone asset.");
			}
		}
	}
	break;
	case FST_JET:
	{
		NvParameterized::Interface* ei = traits->createNvParameterized(JetFieldSamplerData::staticClassName());
		JetFieldSamplerData* ed = static_cast< JetFieldSamplerData*>(ei);
		if (ed)
		{
			NvParameterized::Handle item(ei);
			err = item.getParameter("Name");
			PX_ASSERT(err == NvParameterized::ERROR_NONE);
			item.setParamString(itemName);
			ei->setName(itemName);
			ret = ei;
			ed->JetFieldSampler = NULL;
			dataBase->clone(ed->JetFieldSampler);
			if ( ed->JetFieldSampler == NULL )
			{
				APEX_DEBUG_WARNING("Failed to clone asset.");
			}
		}
	}
	break;
	case FST_WIND:
		{
			NvParameterized::Interface* ei = traits->createNvParameterized(WindFieldSamplerData::staticClassName());
			WindFieldSamplerData* ed = static_cast< WindFieldSamplerData*>(ei);
			if (ed)
			{
				NvParameterized::Handle item(ei);
				err = item.getParameter("Name");
				PX_ASSERT(err == NvParameterized::ERROR_NONE);
				item.setParamString(itemName);
				ei->setName(itemName);
				ret = ei;
				ed->WindFieldSampler = NULL;
				dataBase->clone(ed->WindFieldSampler);
				if ( ed->WindFieldSampler == NULL )
				{
					APEX_DEBUG_WARNING("Failed to clone asset.");
				}
			}
		}
		break;	
	case FST_TURBULENCE:
		{
			NvParameterized::Interface* ei = traits->createNvParameterized(TurbulenceFieldSamplerData::staticClassName());
			TurbulenceFieldSamplerData* ed = static_cast< TurbulenceFieldSamplerData*>(ei);
			if (ed)
			{
				NvParameterized::Handle item(ei);
				err = item.getParameter("Name");
				PX_ASSERT(err == NvParameterized::ERROR_NONE);
				item.setParamString(itemName);
				ei->setName(itemName);
				ret = ei;
				ed->TurbulenceFieldSampler = NULL;
				dataBase->clone(ed->TurbulenceFieldSampler);
				if ( ed->TurbulenceFieldSampler == NULL )
				{
					APEX_DEBUG_WARNING("Failed to clone asset.");
				}
			}
		}
		break;
	case FST_FLAME_EMITTER:
		{
			NvParameterized::Interface* ei = traits->createNvParameterized(FlameEmitterData::staticClassName());
			FlameEmitterData* ed = static_cast< FlameEmitterData*>(ei);
			if (ed)
			{
				NvParameterized::Handle item(ei);
				err = item.getParameter("Name");
				PX_ASSERT(err == NvParameterized::ERROR_NONE);
				item.setParamString(itemName);
				ei->setName(itemName);
				ret = ei;
				ed->FlameEmitter = NULL;
				dataBase->clone(ed->FlameEmitter);
				if ( ed->FlameEmitter == NULL )
				{
					APEX_DEBUG_WARNING("Failed to clone asset.");
				}
			}
		}
		break;

	default:
		PX_ALWAYS_ASSERT(); // not yet implemented!
		break;
	}

	return ret;
}

bool ModuleParticlesImpl::fixFieldSamplerCollisionFilterNames(NvParameterized::Interface *iface)
{
	bool ret = false;

	FieldSamplerType fst = getFieldSamplerType(iface);
	NvParameterized::Interface *asset = NULL;

	const char *filterName=NULL;

	switch (fst)
	{
		case FST_FORCE_FIELD:
			{
				ForceFieldData* ed = static_cast< ForceFieldData*>(iface);
				asset = ed->ForceField;
				filterName = "ForceFieldFS=all";
			}
			break;
		case FST_HEAT_SOURCE:
			{
				HeatSourceData *ed = static_cast< HeatSourceData *>(iface);
				asset = ed->HeatSource;
				filterName = "HeatSource=all";
			}
			break;
		case FST_SUBSTANCE_SOURCE:
			{
				SubstanceSourceData *ed = static_cast< SubstanceSourceData *>(iface);
				asset = ed->SubstanceSource;
				filterName = "SubstanceSource=all";
			}
			break;
		case FST_VELOCITY_SOURCE:
			{
				VelocitySourceData *ed = static_cast< VelocitySourceData *>(iface);
				asset = ed->VelocitySource;
				filterName = "VelocitySource=all";
			}
			break;
		case FST_ATTRACTOR:
			{
				AttractorFieldSamplerData* ed = static_cast< AttractorFieldSamplerData*>(iface);
				asset = ed->AttractorFieldSampler;
				filterName = "AttractorFS=all";
			}
			break;
		case FST_VORTEX:
			{
				VortexFieldSamplerData* ed = static_cast< VortexFieldSamplerData*>(iface);
				asset = ed->VortexFieldSampler;
				filterName = "VortexFS=all";
			}
			break;
		case FST_NOISE:
			{
				NoiseFieldSamplerData* ed = static_cast< NoiseFieldSamplerData*>(iface);
				asset = ed->NoiseFieldSampler;
				filterName = "NoiseFS=all";
			}
			break;
		case FST_JET:
			{
				JetFieldSamplerData* ed = static_cast< JetFieldSamplerData*>(iface);
				asset = ed->JetFieldSampler;
				filterName = "JetFS=all";
			}
			break;
		case FST_WIND:
			{
				WindFieldSamplerData* ed = static_cast< WindFieldSamplerData*>(iface);
				asset = ed->WindFieldSampler;
				filterName = "WindFS=all";
			}
			break;
		case FST_TURBULENCE:
			{
				TurbulenceFieldSamplerData* ed = static_cast< TurbulenceFieldSamplerData*>(iface);
				asset = ed->TurbulenceFieldSampler;
				filterName = "TurbulenceFS=all";
			}
			break;
		case FST_FLAME_EMITTER:
			{
				FlameEmitterData *ed = static_cast< FlameEmitterData *>(iface);
				asset = ed->FlameEmitter;
				filterName = "FlameEmitter=all";
			}
			break;
		default:
			//PX_ASSERT(0);
			break;
	}
	if ( asset && filterName )
	{
		const char *fieldBoundaryFilterDataName=NULL;
		const char *fieldSamplerFilterDataName=NULL;
		bool ok = NvParameterized::getParamString(*asset,"fieldSamplerFilterDataName",fieldSamplerFilterDataName);
		PX_ASSERT( ok );
		ok = NvParameterized::getParamString(*asset,"fieldBoundaryFilterDataName",fieldBoundaryFilterDataName);
		if ( ok )
		{
			if ( fieldBoundaryFilterDataName == NULL || fieldBoundaryFilterDataName[0] == 0 || nvidia::strcmp(fieldBoundaryFilterDataName,"defaultFieldBoundaryFilterDataName") == 0 )
			{
				NvParameterized::setParamString(*asset,"fieldBoundaryFilterDataName",filterName);
				ret = true;
			}
		}
		if ( fieldSamplerFilterDataName == NULL || fieldSamplerFilterDataName[0] == 0 || nvidia::strcmp(fieldSamplerFilterDataName,"defaultFieldSamplerFilterDataName") == 0 )
		{
			NvParameterized::setParamString(*asset,"fieldSamplerFilterDataName",filterName);
			ret = true;
		}
		if ( fst == FST_TURBULENCE )
		{
			const char *collisionFilterDataName=NULL;
			bool ok = NvParameterized::getParamString(*asset,"collisionFilterDataName",collisionFilterDataName);
			PX_UNUSED( ok );
			PX_ASSERT( ok );
			if ( collisionFilterDataName == NULL || collisionFilterDataName[0] == 0 || nvidia::strcmp(collisionFilterDataName,"defaultTurbulenceCollisionFilterDataName") == 0 )
			{
				NvParameterized::setParamString(*asset,"collisionFilterDataName",filterName);
				ret = true;
			}
		}
	}
	return ret;
}

static void setNamedReference(NvParameterized::Interface* parentInterface, const char* parentName, const char* authoringTypeName)
{
	NvParameterized::Handle handle(parentInterface);
	NvParameterized::ErrorType err = parentInterface->getParameterHandle(parentName, handle);
	PX_ASSERT(err == NvParameterized::ERROR_NONE);
	if (err == NvParameterized::ERROR_NONE)
	{
		err = handle.initParamRef(authoringTypeName,true);
		PX_ASSERT(err == NvParameterized::ERROR_NONE);
	}
}



bool ModuleParticlesImpl::setEffectPackageFieldSamplerDatabase(const NvParameterized::Interface* dataBase)
{
	WRITE_ZONE();
	bool ret = false;

	if (dataBase)
	{
		NvParameterized::Traits* traits = mSdk->getParameterizedTraits();
		FieldSamplerType fst = getFieldSamplerType(dataBase);
		if (fst != FST_NONE)
		{
			bool revised = false;
			const char* itemName = dataBase->name();
			EffectPackageFieldSamplerDatabaseParams* d = static_cast< EffectPackageFieldSamplerDatabaseParams*>(mEffectPackageFieldSamplerDatabaseParams);
			if ( d )
			{
				for (uint32_t i = 0; i < (uint32_t)d->FieldSamplers.arraySizes[0]; i++)
				{
					NvParameterized::Interface* ei = d->FieldSamplers.buf[i];
					if (!ei)
					{
						continue;
					}
					const char* dataName;
					NvParameterized::getParamString(*ei, "Name", dataName);
					if (nvidia::stricmp(dataName, itemName) == 0)
					{
						ei->destroy();
						d->FieldSamplers.buf[i] = getFieldSamplerData(fst, traits, dataBase);
						fixFieldSamplerCollisionFilterNames( d->FieldSamplers.buf[i] );
						revised = true;
						ret = true;
						break;
					}
				}
			}
			if (!revised && mEffectPackageFieldSamplerDatabaseParams)
			{
				int32_t arraySize = d->FieldSamplers.arraySizes[0];
				NvParameterized::Handle handle(mEffectPackageFieldSamplerDatabaseParams);
				NvParameterized::ErrorType err = handle.getParameter("FieldSamplers");
				PX_ASSERT(err == NvParameterized::ERROR_NONE);
				err = handle.resizeArray(arraySize + 1);
				PX_ASSERT(err == NvParameterized::ERROR_NONE);
				if (err == NvParameterized::ERROR_NONE)
				{
					d->FieldSamplers.buf[arraySize] = getFieldSamplerData(fst, traits, dataBase);
					fixFieldSamplerCollisionFilterNames( d->FieldSamplers.buf[arraySize] );
				}
			}
		}
		else if (nvidia::strcmp(dataBase->className(), EffectPackageFieldSamplerDatabaseParams::staticClassName()) == 0)
		{
			if (mEffectPackageFieldSamplerDatabaseParams && mEffectPackageFieldSamplerDatabaseParams != dataBase)
			{
				mEffectPackageFieldSamplerDatabaseParams->destroy();
				mEffectPackageFieldSamplerDatabaseParams = NULL;
			}

			if (mEffectPackageFieldSamplerDatabaseParams == NULL)
			{
				dataBase->clone(mEffectPackageFieldSamplerDatabaseParams);
				if ( mEffectPackageFieldSamplerDatabaseParams == NULL )
				{
					APEX_DEBUG_WARNING("Failed to clone asset.");
					mEffectPackageFieldSamplerDatabaseParams = traits->createNvParameterized(EffectPackageFieldSamplerDatabaseParams::staticClassName());
				}
			}

			EffectPackageFieldSamplerDatabaseParams* d = static_cast< EffectPackageFieldSamplerDatabaseParams*>(mEffectPackageFieldSamplerDatabaseParams);
			if ( d )
			{
				if (d->FieldSamplers.arraySizes[0] == 0)
				{
					NvParameterized::Handle handle(mEffectPackageFieldSamplerDatabaseParams);
					NvParameterized::ErrorType err = handle.getParameter("FieldSamplers");
					PX_ASSERT(err == NvParameterized::ERROR_NONE);
					err = handle.resizeArray(11);
					PX_ASSERT(err == NvParameterized::ERROR_NONE);
					NvParameterized::Interface* ei;
					if (err == NvParameterized::ERROR_NONE)
					{
						ei = traits->createNvParameterized(AttractorFieldSamplerData::staticClassName());
						d->FieldSamplers.buf[0] = ei;

						ei = traits->createNvParameterized(ForceFieldData::staticClassName());
						d->FieldSamplers.buf[1] = ei;

						ei = traits->createNvParameterized(HeatSourceData::staticClassName());
						d->FieldSamplers.buf[2] = ei;

						ei = traits->createNvParameterized(JetFieldSamplerData::staticClassName());
						d->FieldSamplers.buf[3] = ei;

						ei = traits->createNvParameterized(TurbulenceFieldSamplerData::staticClassName());
						d->FieldSamplers.buf[4] = ei;

						ei = traits->createNvParameterized(VortexFieldSamplerData::staticClassName());
						d->FieldSamplers.buf[5] = ei;

						ei = traits->createNvParameterized(NoiseFieldSamplerData::staticClassName());
						d->FieldSamplers.buf[6] = ei;

						ei = traits->createNvParameterized(SubstanceSourceData::staticClassName());
						d->FieldSamplers.buf[7] = ei;

						ei = traits->createNvParameterized(WindFieldSamplerData::staticClassName());
						d->FieldSamplers.buf[8] = ei;

						ei = traits->createNvParameterized(VelocitySourceData::staticClassName());
						d->FieldSamplers.buf[9] = ei;

						ei = traits->createNvParameterized(FlameEmitterData::staticClassName());
						d->FieldSamplers.buf[10] = ei;

						ret = true;
					}
				}


				Array< const char* > nameList;
				for (int32_t i = 0; i < d->FieldSamplers.arraySizes[0]; i++)
				{
					ForceFieldData* e = static_cast< ForceFieldData*>(d->FieldSamplers.buf[i]);
					if (e)
					{
						for (uint32_t j = 0; j < nameList.size(); j++)
						{
							if (nvidia::stricmp(nameList[j], e->Name) == 0)
							{
								NvParameterized::Handle handle(e);
								NvParameterized::ErrorType err = handle.getParameter("Name");
								PX_ASSERT(err == NvParameterized::ERROR_NONE);
								if (err == NvParameterized::ERROR_NONE)
								{
									handle.setParamString(getUniqueName(nameList, e->Name));
									ret = true;
									break;
								}
							}
						}
						nameList.pushBack(e->Name);
					}
				}


				for (uint32_t i = 0; i < (uint32_t)d->FieldSamplers.arraySizes[0]; i++)
				{
					NvParameterized::Interface* iface = d->FieldSamplers.buf[i];
					FieldSamplerType fst = getFieldSamplerType(iface);
					switch (fst)
					{
					case FST_FORCE_FIELD:
					{
						ForceFieldData* ed = static_cast< ForceFieldData*>(d->FieldSamplers.buf[i]);
						if (ed->ForceField == NULL)
						{
							ed->ForceField = traits->createNvParameterized(forcefield::ForceFieldAssetParams::staticClassName());
							ret = true;
						}
						forcefield::ForceFieldAssetParams* fap = static_cast< forcefield::ForceFieldAssetParams*>(ed->ForceField);
						if (fap && fap->strength == 0 && fap->lifetime == 0)    // initialize the forcefield to reasonable values; if lifetime is zero, then the force field is not considered initalized.
						{
							ret = true;
							fap->defScale = 1;
							fap->strength = 20.0f;
							fap->lifetime = 1000;

							fap->includeShapeParameters.dimensions = PxVec3(5, 5, 5);

							//TODO: Verify correctness of changes, add case for GenericForceFieldKernelType
							if (fap->forceFieldKernelType == NULL)
							{
								fap->forceFieldKernelType = traits->createNvParameterized(forcefield::RadialForceFieldKernelParams::staticClassName());
								forcefield::RadialForceFieldKernelParams* rfap = static_cast< forcefield::RadialForceFieldKernelParams*>(fap->forceFieldKernelType);
								if (rfap)
								{
									rfap->falloffParameters = traits->createNvParameterized(forcefield::ForceFieldFalloffParams::staticClassName());
									forcefield::ForceFieldFalloffParams* ffp = static_cast< forcefield::ForceFieldFalloffParams*>(rfap->falloffParameters);
									rfap->noiseParameters = traits->createNvParameterized(forcefield::ForceFieldNoiseParams::staticClassName());
									forcefield::ForceFieldNoiseParams* fnp = static_cast< forcefield::ForceFieldNoiseParams*>(rfap->noiseParameters);

									ffp->multiplier = 0.5f;
									ffp->start = 100;
									ffp->end = 0;

									fnp->strength = 0.01f;
									fnp->spaceScale = 8;
									fnp->timeScale = 1;
									fnp->octaves = 3;
								}
							}
						}
					}
					break;
					case FST_HEAT_SOURCE:
					{
						HeatSourceData* ed = static_cast< HeatSourceData*>(d->FieldSamplers.buf[i]);
						if (ed->HeatSource == NULL)
						{
							ed->HeatSource = traits->createNvParameterized(turbulencefs::HeatSourceAssetParams::staticClassName());
							ret = true;
						}
						turbulencefs::HeatSourceAssetParams* ap = static_cast< turbulencefs::HeatSourceAssetParams*>(ed->HeatSource);
						if (ap && ap->geometryType == NULL)
						{
							ap->averageTemperature = 4;
							ap->stdTemperature = 2;
							ap->geometryType = traits->createNvParameterized(turbulencefs::HeatSourceGeomSphereParams::staticClassName());
							ret = true;
						}
					}
					break;
					case FST_SUBSTANCE_SOURCE:
						{
							SubstanceSourceData* ed = static_cast< SubstanceSourceData*>(d->FieldSamplers.buf[i]);
							if (ed->SubstanceSource == NULL)
							{
								ed->SubstanceSource = traits->createNvParameterized(turbulencefs::SubstanceSourceAssetParams::staticClassName());
								ret = true;
							}
							turbulencefs::SubstanceSourceAssetParams* ap = static_cast< turbulencefs::SubstanceSourceAssetParams*>(ed->SubstanceSource);
							if (ap && ap->geometryType == NULL)
							{
								ap->averageDensity = 32;
								ap->stdDensity = 16;
								ap->geometryType = traits->createNvParameterized(turbulencefs::HeatSourceGeomSphereParams::staticClassName());
								ret = true;
							}
						}
						break;
					case FST_VELOCITY_SOURCE:
						{
							VelocitySourceData* ed = static_cast< VelocitySourceData*>(d->FieldSamplers.buf[i]);
							if (ed->VelocitySource == NULL)
							{
								ed->VelocitySource = traits->createNvParameterized(turbulencefs::VelocitySourceAssetParams::staticClassName());
								ret = true;
							}
							turbulencefs::VelocitySourceAssetParams* ap = static_cast< turbulencefs::VelocitySourceAssetParams*>(ed->VelocitySource);
							if (ap && ap->geometryType == NULL)
							{
								ap->geometryType = traits->createNvParameterized(turbulencefs::HeatSourceGeomSphereParams::staticClassName());
								ret = true;
							}
						}
						break;
					case FST_ATTRACTOR:
					{
						AttractorFieldSamplerData* ed = static_cast< AttractorFieldSamplerData*>(d->FieldSamplers.buf[i]);
						if (ed->AttractorFieldSampler == NULL)
						{
							ed->AttractorFieldSampler = traits->createNvParameterized(basicfs::AttractorFSAssetParams::staticClassName());
							basicfs::AttractorFSAssetParams* a = static_cast< basicfs::AttractorFSAssetParams*>(ed->AttractorFieldSampler);
							if (a)
							{
								a->boundaryFadePercentage = 0.3f;
								a->radius = 2;
								a->constFieldStrength = 20;
								a->variableFieldStrength = 1;
							}
							ret = true;
						}
					}
					break;
					case FST_VORTEX:
					{
						VortexFieldSamplerData* ed = static_cast< VortexFieldSamplerData*>(d->FieldSamplers.buf[i]);
						if (ed->VortexFieldSampler == NULL)
						{
							ed->VortexFieldSampler = traits->createNvParameterized(basicfs::VortexFSAssetParams::staticClassName());
							basicfs::VortexFSAssetParams* j = static_cast< basicfs::VortexFSAssetParams*>(ed->VortexFieldSampler);
							if (j)
							{
								j->boundaryFadePercentage = 0.1f;
								j->height = 1;
								j->bottomRadius = 1;
								j->topRadius = 1;
								j->rotationalStrength = 4;
								j->radialStrength = 4;
								j->liftStrength = 1;
							}
							ret = true;
						}
					}
					break;
					case FST_NOISE:
					{
						NoiseFieldSamplerData* ed = static_cast< NoiseFieldSamplerData*>(d->FieldSamplers.buf[i]);
						if (ed->NoiseFieldSampler == NULL)
						{
							ed->NoiseFieldSampler = traits->createNvParameterized(basicfs::NoiseFSAssetParams::staticClassName());
							ret = true;
						}
					}
					break;
					case FST_JET:
					{
						JetFieldSamplerData* ed = static_cast< JetFieldSamplerData*>(d->FieldSamplers.buf[i]);
						if (ed->JetFieldSampler == NULL)
						{
							ed->JetFieldSampler = traits->createNvParameterized(basicfs::JetFSAssetParams::staticClassName());
							basicfs::JetFSAssetParams* j = static_cast< basicfs::JetFSAssetParams*>(ed->JetFieldSampler);
							if (j)
							{
								j->defaultScale = 1;
								j->boundaryFadePercentage = 0.3f;
								j->fieldDirection = PxVec3(1, 0, 0);
								j->fieldDirectionDeviationAngle = 0;
								j->fieldDirectionOscillationPeriod = 0;
								j->fieldStrength = 10;
								j->fieldStrengthDeviationPercentage = 0;
								j->fieldStrengthOscillationPeriod = 0;
								j->gridShapeRadius = 2;
								j->gridShapeHeight = 2;
								j->gridBoundaryFadePercentage = 0.01f;
								j->nearRadius = 0.1f;
								j->pivotRadius = 0.2f;
								j->farRadius = 4;
								j->directionalStretch = 1;
								j->averageStartDistance = 1;
								j->averageEndDistance = 5;
								j->noisePercentage = 0.1f;
								j->noiseTimeScale = 1;
								j->noiseOctaves = 1;
							}
							ret = true;
						}
					}
					break;
					case FST_WIND:
						{
							WindFieldSamplerData* ed = static_cast< WindFieldSamplerData*>(d->FieldSamplers.buf[i]);
							if (ed->WindFieldSampler == NULL)
							{
								ed->WindFieldSampler = traits->createNvParameterized(basicfs::WindFSAssetParams::staticClassName());
								basicfs::WindFSAssetParams* j = static_cast< basicfs::WindFSAssetParams*>(ed->WindFieldSampler);
								if (j)
								{
									// TODO initialize to reasonable default values
								}
								ret = true;
							}
						}
						break;
					case FST_TURBULENCE:
						{
							TurbulenceFieldSamplerData* ed = static_cast< TurbulenceFieldSamplerData*>(d->FieldSamplers.buf[i]);
							if (ed->TurbulenceFieldSampler == NULL)
							{
								ed->TurbulenceFieldSampler = traits->createNvParameterized(turbulencefs::TurbulenceFSAssetParams::staticClassName());
								turbulencefs::TurbulenceFSAssetParams* fs = static_cast< turbulencefs::TurbulenceFSAssetParams*>(ed->TurbulenceFieldSampler);
								if (fs)
								{
									fs->gridSizeWorld = PxVec3(15, 15, 15);
									fs->updatesPerFrame = 1;
									fs->angularVelocityMultiplier = 1;
									fs->angularVelocityClamp = 100000;
									fs->linearVelocityMultiplier = 1;
									fs->linearVelocityClamp = 100000;
									fs->boundaryFadePercentage = 0.3f;
									fs->boundarySizePercentage = 1;
									fs->maxCollidingObjects = 32;
									fs->maxHeatSources = 8;
									fs->dragCoeff = 0;
									fs->externalVelocity = PxVec3(0, 0, 0);
									fs->fieldVelocityMultiplier = 1;
									fs->fieldVelocityWeight = 0.75;
									fs->useHeat = true;
									fs->heatParams.temperatureBasedForceMultiplier = 0.02f;
									fs->heatParams.ambientTemperature = 0;
									fs->heatParams.heatForceDirection = PxVec3(0, 1, 0);
									fs->isEnabledOptimizedLOD = false;
									fs->updatesPerFrame = 1.f;
									fs->noiseParams.noiseStrength = 0;
									fs->noiseParams.noiseSpacePeriod = PxVec3(0.9f, 0.9f, 0.9f);
									fs->noiseParams.noiseTimePeriod = 10;
									fs->noiseParams.noiseOctaves = 1;
									fs->dragCoeffForRigidBody = 0;
									fs->fluidViscosity = 0;
								}

								ret = true;
							}
						}
						break;
					case FST_FLAME_EMITTER:
						{
							FlameEmitterData* ed = static_cast< FlameEmitterData*>(d->FieldSamplers.buf[i]);
							if (ed->FlameEmitter == NULL)
							{
								ed->FlameEmitter = traits->createNvParameterized(turbulencefs::FlameEmitterAssetParams::staticClassName());
								ret = true;
							}
						}
						break;
					default:
						PX_ALWAYS_ASSERT(); // not yet implemented
						break;
					}
					if ( fixFieldSamplerCollisionFilterNames( iface ) )
					{
						ret = true;
					}
				}
			}
		}
		else
		{
			PX_ALWAYS_ASSERT(); // invalid NvParameterized::Interface passed in
		}
	}


	return ret;
}


static void initPathData(NvParameterized::Interface *ei,RigidBodyEffectNS::EffectProperties_Type &ep,bool &ret)
{
	if ( ep.Path.Scale.arraySizes[0] == 0 )
	{
		NvParameterized::Handle handle(ei);
		NvParameterized::ErrorType err = handle.getParameter("EffectProperties.Path.Scale");
		PX_ASSERT(err == NvParameterized::ERROR_NONE);
		err = handle.resizeArray(2);
		PX_ASSERT(err == NvParameterized::ERROR_NONE);
		if (err == NvParameterized::ERROR_NONE)
		{
			RigidBodyEffectNS::ControlPoint_Type &c1 = ep.Path.Scale.buf[0];
			RigidBodyEffectNS::ControlPoint_Type &c2 = ep.Path.Scale.buf[1];

			c1.x = 0;
			c1.y = 1;

			c2.x = 1;
			c2.y = 1;
			ret = true;
		}
	}
	if ( ep.Path.Speed.arraySizes[0] == 0 )
	{
		NvParameterized::Handle handle(ei);
		NvParameterized::ErrorType err = handle.getParameter("EffectProperties.Path.Speed");
		PX_ASSERT(err == NvParameterized::ERROR_NONE);
		err = handle.resizeArray(2);
		PX_ASSERT(err == NvParameterized::ERROR_NONE);
		if (err == NvParameterized::ERROR_NONE)
		{
			RigidBodyEffectNS::ControlPoint_Type &c1 = ep.Path.Speed.buf[0];
			RigidBodyEffectNS::ControlPoint_Type &c2 = ep.Path.Speed.buf[1];

			c1.x = 0;
			c1.y = 1;

			c2.x = 1;
			c2.y = 1;
			ret = true;
		}
	}
}

static void initPathData(NvParameterized::Interface *ei,RigidBodyEffectNS::EffectPath_Type &Path,bool &ret)
{
	if ( Path.Scale.arraySizes[0] == 0 )
	{
		NvParameterized::Handle handle(ei);
		NvParameterized::ErrorType err = handle.getParameter("Path.Scale");
		PX_ASSERT(err == NvParameterized::ERROR_NONE);
		err = handle.resizeArray(2);
		PX_ASSERT(err == NvParameterized::ERROR_NONE);
		if (err == NvParameterized::ERROR_NONE)
		{
			RigidBodyEffectNS::ControlPoint_Type &c1 = Path.Scale.buf[0];
			RigidBodyEffectNS::ControlPoint_Type &c2 = Path.Scale.buf[1];

			c1.x = 0;
			c1.y = 1;

			c2.x = 1;
			c2.y = 1;
			ret = true;
		}
	}
	if ( Path.Speed.arraySizes[0] == 0 )
	{
		NvParameterized::Handle handle(ei);
		NvParameterized::ErrorType err = handle.getParameter("Path.Speed");
		PX_ASSERT(err == NvParameterized::ERROR_NONE);
		err = handle.resizeArray(2);
		PX_ASSERT(err == NvParameterized::ERROR_NONE);
		if (err == NvParameterized::ERROR_NONE)
		{
			RigidBodyEffectNS::ControlPoint_Type &c1 = Path.Speed.buf[0];
			RigidBodyEffectNS::ControlPoint_Type &c2 = Path.Speed.buf[1];

			c1.x = 0;
			c1.y = 1;

			c2.x = 1;
			c2.y = 1;
			ret = true;
		}
	}
}


bool ModuleParticlesImpl::setEffectPackageDatabase(const NvParameterized::Interface* dataBase)
{
	WRITE_ZONE();
	bool ret = false;

	if (dataBase)
	{
		NvParameterized::Traits* traits = mSdk->getParameterizedTraits();
		const char* className = dataBase->className();
		if (nvidia::strcmp(className, EffectPackageAssetParams::staticClassName()) == 0)
		{
			bool revised = false;
			const char* itemName = dataBase->name();
			EffectPackageDatabaseParams* d = static_cast< EffectPackageDatabaseParams*>(mEffectPackageDatabaseParams);
			for (uint32_t i = 0; i < (uint32_t)d->EffectPackages.arraySizes[0]; i++)
			{
				NvParameterized::Interface* ei = d->EffectPackages.buf[i];
				EffectPackageData* ed = static_cast< EffectPackageData*>(ei);
				if (nvidia::stricmp(ed->Name.buf, itemName) == 0)
				{
					ed->EffectPackage->copy(*dataBase);
					revised = true;
					ret = true;
					break;
				}
			}
			if (!revised && mEffectPackageDatabaseParams)
			{
				int32_t arraySize = d->EffectPackages.arraySizes[0];
				NvParameterized::Handle handle(mEffectPackageDatabaseParams);
				NvParameterized::ErrorType err = handle.getParameter("EffectPackages");
				PX_ASSERT(err == NvParameterized::ERROR_NONE);
				err = handle.resizeArray(arraySize + 1);
				PX_ASSERT(err == NvParameterized::ERROR_NONE);
				if (err == NvParameterized::ERROR_NONE)
				{
					NvParameterized::Interface* ei = traits->createNvParameterized(EffectPackageData::staticClassName());
					EffectPackageData* ed = static_cast< EffectPackageData*>(ei);
					if (ed)
					{
						NvParameterized::Handle item(ei);
						err = item.getParameter("Name");
						PX_ASSERT(err == NvParameterized::ERROR_NONE);
						item.setParamString(itemName);
						ei->setName(itemName);
						d->EffectPackages.buf[arraySize] = ei;
						dataBase->clone(ed->EffectPackage);
						if ( ed->EffectPackage == NULL )
						{
							APEX_DEBUG_WARNING("Failed to clone asset.");
						}
					}
					ret = true;
				}
			}
		}
		else if (nvidia::strcmp(className, EffectPackageDatabaseParams::staticClassName()) == 0)
		{
			if (mEffectPackageDatabaseParams && mEffectPackageDatabaseParams != dataBase)
			{
				mEffectPackageDatabaseParams->destroy();
				mEffectPackageDatabaseParams = NULL;
			}
			if (mEffectPackageDatabaseParams == NULL)
			{
				dataBase->clone(mEffectPackageDatabaseParams);
				if ( mEffectPackageDatabaseParams == NULL )
				{
					APEX_DEBUG_WARNING("Failed to clone asset.");
					mEffectPackageDatabaseParams = traits->createNvParameterized(EffectPackageDatabaseParams::staticClassName());
				}
			}
		}
		// initialize anything which needs default values assigned...
		if ( mEffectPackageDatabaseParams )
		{
			EffectPackageDatabaseParams* d = static_cast< EffectPackageDatabaseParams*>(mEffectPackageDatabaseParams);

			{
				if (d->EffectPackages.arraySizes[0] == 0)
				{
					NvParameterized::Handle handle(mEffectPackageDatabaseParams);
					NvParameterized::ErrorType err = handle.getParameter("EffectPackages");
					PX_ASSERT(err == NvParameterized::ERROR_NONE);
					err = handle.resizeArray(1);
					PX_ASSERT(err == NvParameterized::ERROR_NONE);
					if (err == NvParameterized::ERROR_NONE)
					{
						NvParameterized::Interface* ei = traits->createNvParameterized(EffectPackageData::staticClassName());
						EffectPackageData* ed = static_cast< EffectPackageData*>(ei);
						d->EffectPackages.buf[0] = ei;
						if (ed)
						{
							ed->EffectPackage = traits->createNvParameterized(EffectPackageAssetParams::staticClassName());
							EffectPackageAssetParams* p = static_cast< EffectPackageAssetParams*>(ed->EffectPackage);
							if (p->Effects.arraySizes[0] == 0 && p)
							{
								NvParameterized::Handle handle(p);
								NvParameterized::ErrorType err = handle.getParameter("Effects");
								PX_ASSERT(err == NvParameterized::ERROR_NONE);
								err = handle.resizeArray(1);
								PX_ASSERT(err == NvParameterized::ERROR_NONE);
								if (err == NvParameterized::ERROR_NONE)
								{
									NvParameterized::Interface* ei = traits->createNvParameterized(EmitterEffect::staticClassName());
									p->Effects.buf[0] = ei;
									ret = true;
								}
							}
						}
						ret = true;
					}
				}
				Array< const char* > nameList;
				for (int32_t i = 0; i < d->EffectPackages.arraySizes[0]; i++)
				{
					EffectPackageData* e = static_cast< EffectPackageData*>(d->EffectPackages.buf[i]);
					if (e)
					{
						for (uint32_t j = 0; j < nameList.size(); j++)
						{
							if (nvidia::stricmp(nameList[j], e->Name) == 0)
							{
								NvParameterized::Handle handle(e);
								NvParameterized::ErrorType err = handle.getParameter("Name");
								PX_ASSERT(err == NvParameterized::ERROR_NONE);
								if (err == NvParameterized::ERROR_NONE)
								{
									handle.setParamString(getUniqueName(nameList, e->Name));
									ret = true;
									break;
								}
							}
						}
						nameList.pushBack(e->Name);
					}
				}



				for (uint32_t i = 0; i < (uint32_t)d->EffectPackages.arraySizes[0]; i++)
				{
					NvParameterized::Interface* ei = d->EffectPackages.buf[i];
					EffectPackageData* d = static_cast< EffectPackageData*>(ei);


					EffectPackageAssetParams* p = static_cast< EffectPackageAssetParams*>(d->EffectPackage);
					if (p == NULL)
					{
						NvParameterized::Interface* ep = traits->createNvParameterized(EffectPackageAssetParams::staticClassName());
						p = static_cast< EffectPackageAssetParams*>(ep);
						d->EffectPackage = ep;
					}

					{
						RigidBodyEffectNS::EffectPath_Type &path = *(RigidBodyEffectNS::EffectPath_Type *)&p->Path;
						initPathData(d->EffectPackage,path,ret);
					}



					for (uint32_t i = 0; i < (uint32_t)p->Effects.arraySizes[0]; i++)
					{
						NvParameterized::Interface* ei = p->Effects.buf[i];
						if (!ei)
						{
							continue;
						}
						if (nvidia::strcmp(ei->className(), EmitterEffect::staticClassName()) == 0)
						{
							EmitterEffect* e = static_cast< EmitterEffect*>(ei);
							RigidBodyEffectNS::EffectProperties_Type &ep = *(RigidBodyEffectNS::EffectProperties_Type *)&e->EffectProperties;
							initPathData(ei,ep,ret);
							if (e->Emitter == NULL)
							{
								setNamedReference(ei, "Emitter", EMITTER_AUTHORING_TYPE_NAME);
								PX_ASSERT(e->Emitter);
								e->Emitter->setName("defaultEmitter");
								ret = true;
							}
						}
						else if (nvidia::strcmp(ei->className(), RigidBodyEffect::staticClassName()) == 0)
						{
							RigidBodyEffect* e = static_cast< RigidBodyEffect*>(ei);
							RigidBodyEffectNS::EffectProperties_Type &ep = e->EffectProperties;
							initPathData(ei,ep,ret);
						}
						else if (nvidia::strcmp(ei->className(), HeatSourceEffect::staticClassName()) == 0)
						{
							HeatSourceEffect* e = static_cast< HeatSourceEffect*>(ei);
							RigidBodyEffectNS::EffectProperties_Type &ep = *(RigidBodyEffectNS::EffectProperties_Type *)&e->EffectProperties;
							initPathData(ei,ep,ret);
							if (e->HeatSource == NULL)
							{
								setNamedReference(ei, "HeatSource", HEAT_SOURCE_AUTHORING_TYPE_NAME);
								PX_ASSERT(e->HeatSource);
								e->HeatSource->setName("defaultHeatSource");
								ret = true;
							}
						}
						else if (nvidia::strcmp(ei->className(), SubstanceSourceEffect::staticClassName()) == 0)
						{
							SubstanceSourceEffect* e = static_cast< SubstanceSourceEffect*>(ei);
							RigidBodyEffectNS::EffectProperties_Type &ep = *(RigidBodyEffectNS::EffectProperties_Type *)&e->EffectProperties;
							initPathData(ei,ep,ret);
							if (e->SubstanceSource == NULL)
							{
								setNamedReference(ei, "SubstanceSource", SUBSTANCE_SOURCE_AUTHORING_TYPE_NAME);
								PX_ASSERT(e->SubstanceSource);
								e->SubstanceSource->setName("defaultSubstanceSource");
								ret = true;
							}
						}
						else if (nvidia::strcmp(ei->className(), VelocitySourceEffect::staticClassName()) == 0)
						{
							VelocitySourceEffect* e = static_cast< VelocitySourceEffect*>(ei);
							RigidBodyEffectNS::EffectProperties_Type &ep = *(RigidBodyEffectNS::EffectProperties_Type *)&e->EffectProperties;
							initPathData(ei,ep,ret);
							if (e->VelocitySource == NULL)
							{
								setNamedReference(ei, "VelocitySource", VELOCITY_SOURCE_AUTHORING_TYPE_NAME);
								PX_ASSERT(e->VelocitySource);
								e->VelocitySource->setName("defaultVelocitySource");
								ret = true;
							}
						}
						else if (nvidia::strcmp(ei->className(), JetFieldSamplerEffect::staticClassName()) == 0)
						{
							JetFieldSamplerEffect* e = static_cast< JetFieldSamplerEffect*>(ei);
							RigidBodyEffectNS::EffectProperties_Type &ep = *(RigidBodyEffectNS::EffectProperties_Type *)&e->EffectProperties;
							initPathData(ei,ep,ret);
							if (e->JetFieldSampler == NULL)
							{
								setNamedReference(ei, "JetFieldSampler", JET_FS_AUTHORING_TYPE_NAME);
								PX_ASSERT(e->JetFieldSampler);
								e->JetFieldSampler->setName("defaultJetFieldSampler");
								ret = true;
							}
						}
						else if (nvidia::strcmp(ei->className(), WindFieldSamplerEffect::staticClassName()) == 0)
						{
							WindFieldSamplerEffect* e = static_cast< WindFieldSamplerEffect*>(ei);
							RigidBodyEffectNS::EffectProperties_Type &ep = *(RigidBodyEffectNS::EffectProperties_Type *)&e->EffectProperties;
							initPathData(ei,ep,ret);
							if (e->WindFieldSampler == NULL)
							{
								setNamedReference(ei, "WindFieldSampler", WIND_FS_AUTHORING_TYPE_NAME);
								PX_ASSERT(e->WindFieldSampler);
								e->WindFieldSampler->setName("defaultWindFieldSampler");
								ret = true;
							}
						}
						else if (nvidia::strcmp(ei->className(), NoiseFieldSamplerEffect::staticClassName()) == 0)
						{
							NoiseFieldSamplerEffect* e = static_cast< NoiseFieldSamplerEffect*>(ei);
							RigidBodyEffectNS::EffectProperties_Type &ep = *(RigidBodyEffectNS::EffectProperties_Type *)&e->EffectProperties;
							initPathData(ei,ep,ret);
							if (e->NoiseFieldSampler == NULL)
							{
								setNamedReference(ei, "NoiseFieldSampler", NOISE_FS_AUTHORING_TYPE_NAME);
								PX_ASSERT(e->NoiseFieldSampler);
								e->NoiseFieldSampler->setName("defaultNoiseFieldSampler");
								ret = true;
							}
						}
						else if (nvidia::strcmp(ei->className(), VortexFieldSamplerEffect::staticClassName()) == 0)
						{
							VortexFieldSamplerEffect* e = static_cast< VortexFieldSamplerEffect*>(ei);
							RigidBodyEffectNS::EffectProperties_Type &ep = *(RigidBodyEffectNS::EffectProperties_Type *)&e->EffectProperties;
							initPathData(ei,ep,ret);
							if (e->VortexFieldSampler == NULL)
							{
								setNamedReference(ei, "VortexFieldSampler", VORTEX_FS_AUTHORING_TYPE_NAME);
								PX_ASSERT(e->VortexFieldSampler);
								e->VortexFieldSampler->setName("defaultVortexFieldSampler");
								ret = true;
							}
						}
						else if (nvidia::strcmp(ei->className(), AttractorFieldSamplerEffect::staticClassName()) == 0)
						{
							AttractorFieldSamplerEffect* e = static_cast< AttractorFieldSamplerEffect*>(ei);
							RigidBodyEffectNS::EffectProperties_Type &ep = *(RigidBodyEffectNS::EffectProperties_Type *)&e->EffectProperties;
							initPathData(ei,ep,ret);
							if (e->AttractorFieldSampler == NULL)
							{
								setNamedReference(ei, "AttractorFieldSampler", ATTRACTOR_FS_AUTHORING_TYPE_NAME);
								PX_ASSERT(e->AttractorFieldSampler);
								e->AttractorFieldSampler->setName("defaultAttractorFieldSampler");
								ret = true;
							}
						}
						else if (nvidia::strcmp(ei->className(), TurbulenceFieldSamplerEffect::staticClassName()) == 0)
						{
							TurbulenceFieldSamplerEffect* e = static_cast< TurbulenceFieldSamplerEffect*>(ei);
							RigidBodyEffectNS::EffectProperties_Type &ep = *(RigidBodyEffectNS::EffectProperties_Type *)&e->EffectProperties;
							initPathData(ei,ep,ret);
							if (e->TurbulenceFieldSampler == NULL)
							{
								setNamedReference(ei, "TurbulenceFieldSampler", TURBULENCE_FS_AUTHORING_TYPE_NAME);
								PX_ASSERT(e->TurbulenceFieldSampler);
								e->TurbulenceFieldSampler->setName("defaultTurbulenceFieldSampler");
								ret = true;
							}
						}
						else if (nvidia::strcmp(ei->className(), ForceFieldEffect::staticClassName()) == 0)
						{
							ForceFieldEffect* e = static_cast< ForceFieldEffect*>(ei);
							RigidBodyEffectNS::EffectProperties_Type &ep = *(RigidBodyEffectNS::EffectProperties_Type *)&e->EffectProperties;
							initPathData(ei,ep,ret);
							if (e->ForceField == NULL)
							{
								setNamedReference(ei, "ForceField", FORCEFIELD_AUTHORING_TYPE_NAME);
								PX_ASSERT(e->ForceField);
								e->ForceField->setName("defaultForceField");
								ret = true;
							}
						}
						else if (nvidia::strcmp(ei->className(), FlameEmitterEffect::staticClassName()) == 0)
						{
							FlameEmitterEffect* e = static_cast< FlameEmitterEffect*>(ei);
							RigidBodyEffectNS::EffectProperties_Type &ep = *(RigidBodyEffectNS::EffectProperties_Type *)&e->EffectProperties;
							initPathData(ei,ep,ret);
							if (e->FlameEmitter == NULL)
							{
								setNamedReference(ei, "FlameEmitter", FLAME_EMITTER_AUTHORING_TYPE_NAME);
								PX_ASSERT(e->FlameEmitter);
								e->FlameEmitter->setName("defaultFlameEmitter");
								ret = true;
							}
						}

					}
				}
			}
		}
	}


	return ret;
}


bool ModuleParticlesImpl::fixupNamedReferences(void)
{
	bool ret = false;

	if (mEffectPackageEmitterDatabaseParams)
	{
		EffectPackageEmitterDatabaseParams* d = static_cast< EffectPackageEmitterDatabaseParams*>(mEffectPackageEmitterDatabaseParams);

		for (int32_t i = 0; i < d->Emitters.arraySizes[0]; i++)
		{
			EmitterData* ed = static_cast< EmitterData*>(d->Emitters.buf[i]);

			if (ed->Emitter)
			{
				NvParameterized::Handle handle(ed->Emitter);
				NvParameterized::ErrorType err = ed->Emitter->getParameterHandle("iosAssetName", handle);
				PX_ASSERT(err == NvParameterized::ERROR_NONE);
				if (err == NvParameterized::ERROR_NONE)
				{
					NvParameterized::Interface* val = NULL;
					err = handle.getParamRef(val);
					// ok..need to see if it has changed!
					if (val)
					{
						const char* refName = val->name();
						EffectPackageIOSDatabaseParams* d = static_cast< EffectPackageIOSDatabaseParams*>(mEffectPackageIOSDatabaseParams);
						for (uint32_t i = 0; i < (uint32_t)d->ParticleSimulations.arraySizes[0]; i++)
						{
							NvParameterized::Interface* ei = d->ParticleSimulations.buf[i];
							emitter::ApexEmitterAssetParameters* ae = static_cast< emitter::ApexEmitterAssetParameters*>(ed->Emitter);
							ParticleSimulationData* ed = static_cast< ParticleSimulationData*>(ei);
							if (nvidia::stricmp(ed->Name.buf, refName) == 0)
							{
								NvParameterized::Interface* ios = ed->IOS;
								if ( ios )
								{
									const char* paramRef = getAuthoringTypeName(ios->className());
									err = handle.initParamRef(paramRef,true);
									PX_ASSERT(err == NvParameterized::ERROR_NONE);
									if (err == NvParameterized::ERROR_NONE)
									{
										ae->iosAssetName->setName(ed->Name.buf);
									}
								}
								break;
							}
						}
					}
				}
			}
		}
	}
	return ret;
}

bool ModuleParticlesImpl::initParticleSimulationData(ParticleSimulationData* ed)
{
	bool ret = false;

	NvParameterized::Traits* traits = mSdk->getParameterizedTraits();

	if (ed->IOS == NULL)
	{
		ed->IOS = traits->createNvParameterized(basicios::BasicIOSAssetParam::staticClassName());
		basicios::BasicIOSAssetParam* ios = static_cast< basicios::BasicIOSAssetParam*>(ed->IOS);
		if (ios)
		{
			ios->restDensity = 1;
			ios->particleRadius = 0.1f;
			ios->maxParticleCount = 16384;
			ios->maxInjectedParticleCount = 0.05;
			ios->sceneGravityScale = 1;
			ios->externalAcceleration.x = 0;
			ios->externalAcceleration.y = 0;
			ios->externalAcceleration.z = 0;
			ios->particleMass.center = 1;
			ios->particleMass.spread = 0;
			ios->staticCollision = true;
			ios->restitutionForStaticShapes = 0.2f;
			ios->dynamicCollision = true;
			ios->restitutionForDynamicShapes = 0.2f;
			ios->collisionDistanceMultiplier = 1;
			ios->collisionThreshold = 0.001f;
			ios->collisionWithConvex = true;
			ios->collisionWithTriangleMesh = false;
		}
		ret = true;
	}

	if (ed->IOS && nvidia::strcmp(ed->IOS->className(), "ParticleIosAssetParam") == 0)
	{
		pxparticleios::ParticleIosAssetParam* p = static_cast< pxparticleios::ParticleIosAssetParam*>(ed->IOS);
		if (p->particleType == NULL)
		{
			p->particleType = traits->createNvParameterized(pxparticleios::FluidParticleSystemParams::staticClassName());
			ret = true;
		}
	}

	if ( ed->IOS )
	{
		if ( nvidia::strcmp(ed->IOS->className(), "ParticleIosAssetParam") == 0 )
		{
			const char *filterName = "ParticlesSimple=all";
			pxparticleios::ParticleIosAssetParam* p = static_cast< pxparticleios::ParticleIosAssetParam*>(ed->IOS);
			if (p->particleType && nvidia::strcmp(p->particleType->className(), pxparticleios::FluidParticleSystemParams::staticClassName()) == 0 )
			{
				filterName = "ParticlesFluid=all";
			}

			{
				const char *value=NULL;
				NvParameterized::getParamString(*ed->IOS,"simulationFilterData",value);
				if ( value == NULL || value[0] == 0 )
				{
					NvParameterized::setParamString(*ed->IOS,"simulationFilterData",filterName);
					ret = true;
				}
			}
			{
				const char *value=NULL;
				NvParameterized::getParamString(*ed->IOS,"fieldSamplerFilterData",value);
				if ( value == NULL || value[0] == 0 )
				{
					NvParameterized::setParamString(*ed->IOS,"fieldSamplerFilterData",filterName);
					ret = true;
				}
			}
		}
		else
		{
			const char *filterName = "ParticlesBasicIOS=all";
			{
				const char *value=NULL;
				NvParameterized::getParamString(*ed->IOS,"collisionFilterDataName",value);
				if ( value == NULL || value[0] == 0 )
				{
					NvParameterized::setParamString(*ed->IOS,"collisionFilterDataName",filterName);
					ret = true;
				}
			}
			{
				const char *value=NULL;
				NvParameterized::getParamString(*ed->IOS,"fieldSamplerFilterDataName",value);
				if ( value == NULL || value[0] == 0 )
				{
					NvParameterized::setParamString(*ed->IOS,"fieldSamplerFilterDataName",filterName);
					ret = true;
				}
			}

		}
	}
	return ret;
}

void ModuleParticlesImpl::resetEmitterPool(void)
{
	WRITE_ZONE();
	for (uint32_t i = 0; i < mScenes.size(); i++)
	{
		ParticlesScene* ds = static_cast< ParticlesScene*>(mScenes[i]);
		ds->resetEmitterPool();
	}
}

#define MODULE_CHECK(x) if ( x == module ) { x = NULL; }

/**
Notification from ApexSDK when a module has been released
*/
void ModuleParticlesImpl::notifyChildGone(ModuleIntl* imodule)
{
	Module* module = mSdk->getModule(imodule);
	PX_ASSERT(module);
	MODULE_CHECK(mModuleBasicIos);
	MODULE_CHECK(mModuleEmitter);
	MODULE_CHECK(mModuleIofx);
	MODULE_CHECK(mModuleFieldSampler);
	MODULE_CHECK(mModuleBasicFS);
	MODULE_CHECK(mModuleParticleIos);			// PhysX 3.x only : Instantiate the ParticleIOS module
	MODULE_CHECK(mModuleForceField);			// PhysX 3.x only : Instantiate the ForceField module
};

// This is a notification that the ApexSDK is being released.  During the shutdown process
// the APEX SDK will automatically release all currently registered modules; therefore we are no longer
// responsible for releasing these modules ourselves.

#define SAFE_MODULE_NULL(x) if ( x ) { ModuleIntl *m = mSdk->getInternalModule(x); PX_ASSERT(m); m->setParent(NULL); x = NULL; }

void ModuleParticlesImpl::notifyReleaseSDK(void)
{
	SAFE_MODULE_NULL(mModuleBasicIos);
	SAFE_MODULE_NULL(mModuleEmitter);
	SAFE_MODULE_NULL(mModuleIofx);
	SAFE_MODULE_NULL(mModuleFieldSampler);
	SAFE_MODULE_NULL(mModuleBasicFS);
	SAFE_MODULE_NULL(mModuleParticleIos);			// PhysX 3.x only : Instantiate the ParticleIOS module
	SAFE_MODULE_NULL(mModuleForceField);			// PhysX 3.x only : Instantiate the ForceField module
}

void ModuleParticlesImpl::initializeDefaultDatabases(void)
{
	WRITE_ZONE();
	/* Register the NvParameterized factories */
	NvParameterized::Traits* traits = mSdk->getParameterizedTraits();

	if (mTurbulenceModule == NULL)
	{
		uint32_t count = mSdk->getNbModules();
		Module** modules = mSdk->getModules();
		for (uint32_t i = 0; i < count; i++)
		{
			Module* m = modules[i];
			const char* name = m->getName();
			if (nvidia::strcmp(name, "TurbulenceFS") == 0)
			{
				mTurbulenceModule = static_cast< ModuleTurbulenceFS*>(m);
				break;
			}
		}
	}

	// Initialize the effect package databases
	if (mEffectPackageIOSDatabaseParams == NULL)
	{
		NvParameterized::Interface* iface = traits->createNvParameterized(EffectPackageIOSDatabaseParams::staticClassName());
		if (iface)
		{
			setEffectPackageIOSDatabase(iface);
			iface->destroy();
		}
	}
	if (mEffectPackageIOFXDatabaseParams == NULL)
	{
		NvParameterized::Interface* iface = traits->createNvParameterized(EffectPackageIOFXDatabaseParams::staticClassName());
		if (iface)
		{
			setEffectPackageIOFXDatabase(iface);
			iface->destroy();
		}
	}
	if (mEffectPackageEmitterDatabaseParams == NULL)
	{
		NvParameterized::Interface* iface = traits->createNvParameterized(EffectPackageEmitterDatabaseParams::staticClassName());
		if (iface)
		{
			setEffectPackageEmitterDatabase(iface);
			iface->destroy();
		}
	}
	if (mEffectPackageFieldSamplerDatabaseParams == NULL)
	{
		NvParameterized::Interface* iface = traits->createNvParameterized(EffectPackageFieldSamplerDatabaseParams::staticClassName());
		setEffectPackageFieldSamplerDatabase(iface);
		iface->destroy();
	}
	if (mEffectPackageDatabaseParams == NULL)
	{
		NvParameterized::Interface* iface = traits->createNvParameterized(EffectPackageDatabaseParams::staticClassName());
		if (iface)
		{
			setEffectPackageDatabase(iface);
			iface->destroy();
		}
	}
	if (mGraphicsMaterialsDatabase == NULL)
	{
		NvParameterized::Interface* iface = traits->createNvParameterized(EffectPackageGraphicsMaterialsParams::staticClassName());
		if (iface)
		{
			setEffectPackageGraphicsMaterialsDatabase(iface);
			iface->destroy();
		}
	}

}

#define MODULE_NAME_CHECK(x) if ( x ) { if ( nvidia::strcmp(x->getName(),moduleName) == 0) ret = x; }

nvidia::apex::Module* ModuleParticlesImpl::getModule(const char* moduleName)
{
	READ_ZONE();
	nvidia::apex::Module* ret = NULL;
	MODULE_NAME_CHECK(mModuleBasicIos);
	MODULE_NAME_CHECK(mModuleEmitter);
	MODULE_NAME_CHECK(mModuleIofx);
	MODULE_NAME_CHECK(mModuleFieldSampler);
	MODULE_NAME_CHECK(mModuleBasicFS);
	MODULE_NAME_CHECK(mModuleParticleIos);
	MODULE_NAME_CHECK(mModuleForceField);
	return ret;
}

}
} // end namespace nvidia

