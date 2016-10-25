/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __MODULE_PARTICLEIOS_IMPL_H__
#define __MODULE_PARTICLEIOS_IMPL_H__

#include "Apex.h"
#include "ModuleParticleIos.h"
#include "ApexSDKIntl.h"
#include "ModuleBase.h"
#include "ModuleIntl.h"
#include "ResourceProviderIntl.h"
#include "ApexSharedUtils.h"
#include "ApexSDKHelpers.h"
#include "ModulePerfScope.h"
#include "ApexAuthorableObject.h"
#include "ParticleIosAssetImpl.h"
#include "ModuleParticleIosRegistration.h"
#include "ApexRWLockable.h"

namespace nvidia
{
namespace apex
{
class ModuleIofxIntl;
class ModuleFieldSamplerIntl;
class ParticleIosActor;
}
namespace pxparticleios
{

class ParticleIosScene;


/**
\brief ModuleBase descriptor for ParticleIOS module
*/
class ModuleParticleIosDesc : public ApexDesc
{
public:

	/**
	\brief constructor sets to default.
	*/
	PX_INLINE ModuleParticleIosDesc() : ApexDesc()
	{
		init();
	}

	/**
	\brief sets members to default values.
	*/
	PX_INLINE void setToDefault()
	{
		ApexDesc::setToDefault();
		init();
	}

	/**
	\brief checks if this is a valid descriptor.
	*/
	PX_INLINE bool isValid() const
	{
		bool retVal = ApexDesc::isValid();
		return retVal;
	}

private:

	PX_INLINE void init()
	{
	}
};

class ModuleParticleIosImpl : public ModuleParticleIos, public ModuleIntl, public ModuleBase, public ApexRWLockable
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	ModuleParticleIosImpl(ApexSDKIntl* sdk);
	~ModuleParticleIosImpl();

	void											init(const ModuleParticleIosDesc& desc);

	// base class methods
	void											init(NvParameterized::Interface&);
	NvParameterized::Interface* 					getDefaultModuleDesc();
	void											release()
	{
		ModuleBase::release();
	}
	void											destroy();
	const char*										getName() const
	{
		READ_ZONE();
		return ModuleBase::getName();
	}

	//ParticleIosActor *							getApexActor( Scene* scene, AuthObjTypeID type ) const;
	ApexActor* 										getApexActor(Actor* nxactor, AuthObjTypeID type) const;

	ModuleSceneIntl* 									createInternalModuleScene(SceneIntl&, RenderDebugInterface*);
	void											releaseModuleSceneIntl(ModuleSceneIntl&);
	uint32_t									forceLoadAssets();
	AuthObjTypeID									getModuleID() const;
	RenderableIterator* 						createRenderableIterator(const Scene&);

	virtual const char*                             getParticleIosTypeName();

	ParticleIosScene* 								getParticleIosScene(const Scene& scene);
	const ParticleIosScene* 						getParticleIosScene(const Scene& scene) const;

	ModuleIofxIntl* 									getInternalModuleIofx();
	ModuleFieldSamplerIntl* 							getInternalModuleFieldSampler();

protected:

	ResourceList								mParticleIosSceneList;
	ResourceList								mAuthorableObjects;

	friend class ParticleIosScene;
private:

	ParticleIosModuleParameters*				mModuleParams;

	ModuleIofxIntl*                               mIofxModule;
	ModuleFieldSamplerIntl*                       mFieldSamplerModule;
};

}
} // namespace nvidia

#endif // __MODULE_PARTICLEIOS_IMPL_H__
