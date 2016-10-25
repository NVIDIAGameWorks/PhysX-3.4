/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __MODULE_PARTICLES_IMPL_H__
#define __MODULE_PARTICLES_IMPL_H__

#include "Apex.h"
#include "ModuleParticles.h"
#include "ApexSDKIntl.h"
#include "ModuleIntl.h"
#include "ModuleBase.h"

#include "ApexRWLockable.h"
#include "ApexSDKHelpers.h"
#include "ParticlesDebugRenderParams.h"
#include "ParticlesModuleParameters.h"
#include "EffectPackageGraphicsMaterialsParams.h"

#include "EffectPackageAssetParams.h"
#include "EffectPackageActorParams.h"

#include "GraphicsMaterialData.h"
#include "VolumeRenderMaterialData.h"
#include "EmitterEffect.h"
#include "RigidBodyEffect.h"

#include "HeatSourceEffect.h"
#include "SubstanceSourceEffect.h"
#include "VelocitySourceEffect.h"
#include "ForceFieldEffect.h"
#include "JetFieldSamplerEffect.h"
#include "WindFieldSamplerEffect.h"
#include "NoiseFieldSamplerEffect.h"
#include "VortexFieldSamplerEffect.h"
#include "AttractorFieldSamplerEffect.h"
#include "TurbulenceFieldSamplerEffect.h"
#include "FlameEmitterEffect.h"

#include "EffectPackageData.h"
#include "AttractorFieldSamplerData.h"
#include "JetFieldSamplerData.h"
#include "WindFieldSamplerData.h"
#include "NoiseFieldSamplerData.h"
#include "VortexFieldSamplerData.h"
#include "TurbulenceFieldSamplerData.h"
#include "HeatSourceData.h"
#include "SubstanceSourceData.h"
#include "VelocitySourceData.h"
#include "ForceFieldData.h"
#include "EmitterData.h"
#include "GraphicsEffectData.h"
#include "ParticleSimulationData.h"
#include "FlameEmitterData.h"

#include "EffectPackageIOSDatabaseParams.h"
#include "EffectPackageIOFXDatabaseParams.h"
#include "EffectPackageEmitterDatabaseParams.h"
#include "EffectPackageDatabaseParams.h"
#include "EffectPackageFieldSamplerDatabaseParams.h"

#include "ReadCheck.h"
#include "WriteCheck.h"

namespace nvidia
{
namespace apex
{
class SceneIntl;
class ModuleTurbulenceFS;
class EmitterActor;
class EmitterAsset;
}
namespace particles
{
class ParticlesAsset;
class ParticlesAssetAuthoring;
class ParticlesScene;

typedef Array< ModuleSceneIntl* > ModuleSceneVector;

class ModuleParticlesDesc : public ApexDesc
{
public:

	/**
	\brief Constructor sets to default.
	*/
	PX_INLINE ModuleParticlesDesc()
	{
		setToDefault();
	}
	/**
	\brief (re)sets the structure to the default.
	*/
	PX_INLINE void	setToDefault()
	{
		ApexDesc::setToDefault();
		moduleValue = 0;
	}

	/**
	Returns true if an object can be created using this descriptor.
	*/
	PX_INLINE bool	isValid() const
	{
		return ApexDesc::isValid();
	}

	/**
	ModuleBase configurable parameter.
	*/
	uint32_t moduleValue;
};


class ModuleParticlesImpl : public ModuleParticles, public ModuleIntl, public ModuleBase, public ApexRWLockable
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	ModuleParticlesImpl(ApexSDKIntl* sdk);
	~ModuleParticlesImpl();

	void						init(const ModuleParticlesDesc& desc);

	// base class methods
	void						init(NvParameterized::Interface&) {}
	NvParameterized::Interface* getDefaultModuleDesc();
	void release()
	{
		ModuleBase::release();
	}
	void destroy();
	const char*					getName() const
	{
		READ_ZONE();
		return ModuleBase::getName();
	}

	virtual bool setEffectPackageGraphicsMaterialsDatabase(const NvParameterized::Interface* dataBase);

	virtual const NvParameterized::Interface* getEffectPackageGraphicsMaterialsDatabase() const;

	virtual bool setEffectPackageIOSDatabase(const NvParameterized::Interface* dataBase);
	virtual bool setEffectPackageIOFXDatabase(const NvParameterized::Interface* dataBase);
	virtual bool setEffectPackageEmitterDatabase(const NvParameterized::Interface* dataBase);
	virtual bool setEffectPackageDatabase(const NvParameterized::Interface* dataBase);
	virtual bool setEffectPackageFieldSamplerDatabase(const NvParameterized::Interface* dataBase);

	virtual const NvParameterized::Interface* getEffectPackageIOSDatabase(void) const
	{
		READ_ZONE();
		return mEffectPackageIOSDatabaseParams;
	};
	virtual const NvParameterized::Interface* getEffectPackageIOFXDatabase(void) const
	{
		READ_ZONE();
		return mEffectPackageIOFXDatabaseParams;
	};
	virtual const NvParameterized::Interface* getEffectPackageEmitterDatabase(void) const
	{
		READ_ZONE();
		return mEffectPackageEmitterDatabaseParams;
	};
	virtual const NvParameterized::Interface* getEffectPackageDatabase(void) const
	{
		READ_ZONE();
		return mEffectPackageDatabaseParams;
	};
	virtual const NvParameterized::Interface* getEffectPackageFieldSamplerDatabase(void) const
	{
		READ_ZONE();
		return mEffectPackageFieldSamplerDatabaseParams;
	};

	bool initParticleSimulationData(ParticleSimulationData* ed);

	virtual NvParameterized::Interface* locateResource(const char* resourceName,		// the name of the resource
	        const char* nameSpace);

	virtual const char** getResourceNames(const char* nameSpace, uint32_t& nameCount, const char** &variants);

	virtual const NvParameterized::Interface* locateGraphicsMaterialData(const char* name) const;
	virtual const NvParameterized::Interface* locateVolumeRenderMaterialData(const char* name) const;


	ModuleSceneIntl* 				createInternalModuleScene(SceneIntl&, RenderDebugInterface*);
	void						releaseModuleSceneIntl(ModuleSceneIntl&);
	uint32_t				forceLoadAssets();
	AuthObjTypeID				getModuleID() const;
	RenderableIterator* 	createRenderableIterator(const Scene&);

	AuthObjTypeID             getParticlesAssetTypeID() const;

	uint32_t				getModuleValue() const
	{
		return mModuleValue;
	}

	ModuleTurbulenceFS* getModuleTurbulenceFS(void)
	{
		return mTurbulenceModule;
	}

	ParticlesScene* 			getParticlesScene(const Scene& apexScene);

	virtual void setEnableScreenCulling(bool state, bool znegative)
	{
		WRITE_ZONE();
		mEnableScreenCulling = state;
		mZnegative = znegative;
	}

	bool getEnableScreenCulling(void) const
	{
		return mEnableScreenCulling;
	}

	bool getZnegative(void) const
	{
		return mZnegative;
	}

	virtual void resetEmitterPool(void);

	virtual void setUseEmitterPool(bool state)
	{
		WRITE_ZONE();
		mUseEmitterPool = state;
	}

	virtual bool getUseEmitterPool(void) const
	{
		READ_ZONE();
		return mUseEmitterPool;
	}

	PxMaterial *getDefaultMaterial(void) const
	{
		return mDefaultMaterial;
	}

	virtual void notifyReleaseSDK(void);

	virtual void notifyChildGone(ModuleIntl* imodule);

protected:
	bool						mUseEmitterPool;
	bool						mEnableScreenCulling;
	bool						mZnegative;

	ResourceList				mAuthorableObjects;
	ResourceList				mEffectPackageAuthorableObjects;

	ResourceList				mParticlesScenes;

	uint32_t				mModuleValue;

	friend class ParticlesAsset;

	/**
	\brief Used by the ParticleEffectTool to initialize the default database values for the editor
	*/
	virtual void initializeDefaultDatabases(void);

	virtual nvidia::apex::Module* getModule(const char* moduleName);

private:

	bool fixFieldSamplerCollisionFilterNames(NvParameterized::Interface *fs);

	bool fixupNamedReferences(void);

	NvParameterized::Interface*									mEffectPackageIOSDatabaseParams;
	NvParameterized::Interface*									mEffectPackageIOFXDatabaseParams;
	NvParameterized::Interface*									mEffectPackageEmitterDatabaseParams;
	NvParameterized::Interface*									mEffectPackageDatabaseParams;
	NvParameterized::Interface*									mEffectPackageFieldSamplerDatabaseParams;

	ParticlesModuleParameters*									mModuleParams;
	NvParameterized::Interface*									mGraphicsMaterialsDatabase;
	ModuleTurbulenceFS*							mTurbulenceModule;
	ModuleSceneVector											mScenes;
	Array< const char*>										mTempNames;
	Array< const char*>										mTempVariantNames;

	nvidia::apex::Module* 	mModuleBasicIos;			// Instantiate the BasicIOS module statically
	nvidia::apex::Module* 	mModuleEmitter;				// Instantiate the Emitter module statically
	nvidia::apex::Module* 	mModuleIofx;				// Instantiate the IOFX module statically
	nvidia::apex::Module* 	mModuleFieldSampler;		// Instantiate the field sampler module statically
	nvidia::apex::Module* 	mModuleBasicFS;				// Instantiate the BasicFS module statically
	nvidia::apex::Module* 	mModuleParticleIos;			// PhysX 3.x only : Instantiate the ParticleIOS module
	nvidia::apex::Module* 	mModuleForceField;			// PhysX 3.x only : Instantiate the ForceField module
	PxMaterial		*mDefaultMaterial;
};

}
} // end namespace nvidia

#endif // __MODULE_PARTICLES_H__
