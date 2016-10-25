/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __MODULE_BASIC_IOS_IMPL_H__
#define __MODULE_BASIC_IOS_IMPL_H__

#include "Apex.h"
#include "ModuleBasicIos.h"
#include "ApexSDKIntl.h"
#include "ModuleBase.h"
#include "ModuleIntl.h"
#include "ResourceProviderIntl.h"
#include "ApexSharedUtils.h"
#include "ApexSDKHelpers.h"
#include "ModulePerfScope.h"
#include "ApexAuthorableObject.h"
#include "BasicIosAssetImpl.h"
#include "ModuleBasicIosRegistration.h"
#include "ApexRWLockable.h"

namespace nvidia
{
namespace apex
{
class ModuleIofxIntl;
class ModuleFieldSamplerIntl;
}
namespace basicios
{

class BasicIosScene;

/**
\brief ModuleBase descriptor for BasicIOS module
*/
class ModuleBasicIosDesc : public ApexDesc
{
public:

	/**
	\brief constructor sets to default.
	*/
	PX_INLINE ModuleBasicIosDesc() : ApexDesc()
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

class ModuleBasicIosImpl : public ModuleBasicIos, public ModuleIntl, public ModuleBase, public ApexRWLockable
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	ModuleBasicIosImpl(ApexSDKIntl* sdk);
	~ModuleBasicIosImpl();

	void											init(const ModuleBasicIosDesc& desc);

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
		return ModuleBase::getName();
	}

	//BasicIosActor *								getApexActor( Scene* scene, AuthObjTypeID type ) const;
	ApexActor* 										getApexActor(Actor* nxactor, AuthObjTypeID type) const;

	ModuleSceneIntl* 									createInternalModuleScene(SceneIntl&, RenderDebugInterface*);
	void											releaseModuleSceneIntl(ModuleSceneIntl&);
	uint32_t									forceLoadAssets();
	AuthObjTypeID									getModuleID() const;
	RenderableIterator* 						createRenderableIterator(const Scene&);

	virtual const char*                             getBasicIosTypeName();

	BasicIosScene* 									getBasicIosScene(const Scene& scene);
	const BasicIosScene* 							getBasicIosScene(const Scene& scene) const;

	ModuleIofxIntl* 									getInternalModuleIofx();
	ModuleFieldSamplerIntl* 							getInternalModuleFieldSampler();

	const TestBase*								getTestBase(Scene* apexScene) const;

protected:

	ResourceList								mBasicIosSceneList;
	ResourceList								mAuthorableObjects;

	friend class BasicIosScene;
private:
	BasicIosModuleParameters*					mModuleParams;

	ModuleIofxIntl*                               mIofxModule;
	ModuleFieldSamplerIntl*                       mFieldSamplerModule;
};

}
} // namespace nvidia

#endif // __MODULE_BASIC_IOS_IMPL_H__
