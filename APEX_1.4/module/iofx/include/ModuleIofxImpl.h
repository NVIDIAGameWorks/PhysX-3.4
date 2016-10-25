/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __MODULE_IOFX_IMPL_H__
#define __MODULE_IOFX_IMPL_H__

#include "Apex.h"
#include "ModuleIofx.h"
#include "ApexSDKIntl.h"
#include "ModuleBase.h"
#include "ModuleIofxIntl.h"
#include "ResourceProviderIntl.h"
#include "ApexSharedUtils.h"
#include "ApexSDKHelpers.h"
#include "ModulePerfScope.h"
#include "ApexAuthorableObject.h"
#include "ApexRWLockable.h"
#include "ModuleIofxRegistration.h"
#include "ReadCheck.h"
#include "WriteCheck.h"


namespace nvidia
{
namespace apex
{
class RenderVolume;
}
namespace iofx
{
class IofxAssetImpl;
class IofxScene;

class ModuleIofxDesc : public ApexDesc
{
public:

	/**
	\brief constructor sets to default.
	*/
	PX_INLINE ModuleIofxDesc() : ApexDesc()
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

class ModuleIofxImpl : public ModuleIofx, public ModuleIofxIntl, public ModuleBase, public ApexRWLockable
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	ModuleIofxImpl(ApexSDKIntl* sdk);
	~ModuleIofxImpl();

	void							init(const ModuleIofxDesc& ModuleIofxDesc);

	// base class methods
	void							init(NvParameterized::Interface&) {}
	NvParameterized::Interface* 	getDefaultModuleDesc();
	void							release()
	{
		ModuleBase::release();
	}
	void							destroy();
	const char*						getName() const
	{
		READ_ZONE();
		return ModuleBase::getName();
	}
	
	RenderableIterator*		createRenderableIterator(const Scene&)
	{
		WRITE_ZONE();
		return NULL; //obsolete!
	}

	void							disableCudaInterop()
	{
		WRITE_ZONE();
		mInteropDisabled = true;
	}
	void							disableCudaModifiers()
	{
		WRITE_ZONE();
		mCudaDisabled = true;
	}
	void							disableDeferredRenderableAllocation()
	{
		WRITE_ZONE();
		mDeferredDisabled = true;
	}

	const TestBase*				getTestBase(Scene* apexScene) const;

	bool							setIofxRenderCallback(const Scene& apexScene, IofxRenderCallback* );
	IofxRenderCallback*			getIofxRenderCallback(const Scene& apexScene) const;

	IofxRenderableIterator*		createIofxRenderableIterator(const Scene&);

	void							prepareRenderables(const Scene&);

	// ModuleIofxIntl methods
	IofxManagerIntl*					createActorManager(const Scene& scene, const IofxAsset& asset, const IofxManagerDescIntl& desc);

	uint32_t					forceLoadAssets();
	AuthObjTypeID					getModuleID() const;

	ModuleSceneIntl* 					createInternalModuleScene(SceneIntl&, RenderDebugInterface*);
	void							releaseModuleSceneIntl(ModuleSceneIntl&);

	IofxScene* 						getIofxScene(const Scene& scene);
	const IofxScene* 				getIofxScene(const Scene& scene) const;

	RenderVolume* 			createRenderVolume(const Scene& apexScene, const PxBounds3& b, uint32_t priority, bool allIofx);
	void							releaseRenderVolume(RenderVolume& volume);

protected:

	ResourceList								mAuthorableObjects;

	IofxModuleParameters* 						mModuleParams;
	bool										mInteropDisabled;
	bool										mCudaDisabled;
	bool										mDeferredDisabled;

	ResourceList								mIofxScenes;

	friend class IofxActorImpl;
	friend class IofxScene;
	friend class IofxManager;
	friend class IofxManagerGPU;
};

}
} // namespace nvidia

#endif // __MODULE_IOFX_IMPL_H__
