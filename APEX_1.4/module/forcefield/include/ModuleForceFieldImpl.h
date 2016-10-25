/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __MODULE_FORCEFIELD_IMPL_H__
#define __MODULE_FORCEFIELD_IMPL_H__

#include "Apex.h"
#include "ModuleForceField.h"
#include "ApexSDKIntl.h"
#include "ModuleIntl.h"
#include "ModuleBase.h"

#include "ApexSDKHelpers.h"
#include "ApexRWLockable.h"
#include "ReadCheck.h"
#include "WriteCheck.h"
#include "ModuleForceFieldRegistration.h"

namespace nvidia
{
namespace apex
{
class SceneIntl;
class ModuleFieldSamplerIntl;
}
namespace forcefield
{

class ForceFieldAssetImpl;
class ForceFieldAssetAuthoringImpl;
class ForceFieldScene;

class ModuleForceFieldDesc : public ApexDesc
{
public:

	/**
	\brief Constructor sets to default.
	*/
	PX_INLINE ModuleForceFieldDesc()
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


class ModuleForceFieldImpl : public ModuleForceField, public ModuleIntl, public ModuleBase, public ApexRWLockable
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	ModuleForceFieldImpl(ApexSDKIntl* sdk);
	~ModuleForceFieldImpl();

	void						init(const ModuleForceFieldDesc& explosionDesc);

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
		return ModuleBase::getName();
	}
	
	ModuleSceneIntl* 				createInternalModuleScene(SceneIntl&, RenderDebugInterface*);
	void						releaseModuleSceneIntl(ModuleSceneIntl&);
	uint32_t				forceLoadAssets();
	AuthObjTypeID				getModuleID() const;
	RenderableIterator* 	createRenderableIterator(const Scene&);

	AuthObjTypeID             getForceFieldAssetTypeID() const;

	uint32_t				getModuleValue() const
	{
		READ_ZONE();
		return mModuleValue;
	}

	ModuleFieldSamplerIntl*		getInternalModuleFieldSampler();

protected:
	ForceFieldScene* 			getForceFieldScene(const Scene& apexScene);

	ResourceList				mAuthorableObjects;

	ResourceList				mForceFieldScenes;

	uint32_t				mModuleValue;

	ModuleFieldSamplerIntl*		mFieldSamplerModule;

	friend class ForceFieldAssetImpl;

private:

	ForceFieldModuleParams*				mModuleParams;
};

}
} // end namespace nvidia

#endif // __MODULE_FORCEFIELD_IMPL_H__
