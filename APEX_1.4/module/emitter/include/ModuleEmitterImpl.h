/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __MODULE_EMITTER_IMPL_H__
#define __MODULE_EMITTER_IMPL_H__

#include "Apex.h"
#include "ModuleEmitter.h"
#include "ApexSDKIntl.h"
#include "ModuleBase.h"
#include "ModuleIntl.h"
#include "EmitterAssetImpl.h"
#include "GroundEmitterAssetImpl.h"
#include "ModulePerfScope.h"
#include "ImpactObjectEvent.h"

#include "ModuleEmitterRegistration.h"
#include "ApexRWLockable.h"
#include "ReadCheck.h"
#include "WriteCheck.h"

namespace nvidia
{
namespace emitter
{

class EmitterScene;

class ModuleEmitterDesc : public ApexDesc
{
public:

	/**
	\brief constructor sets to default.
	*/
	PX_INLINE ModuleEmitterDesc() : ApexDesc()
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

class ModuleEmitterImpl : public ModuleEmitter, public ModuleIntl, public ModuleBase, public ApexRWLockable
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	ModuleEmitterImpl(ApexSDKIntl* inSdk);
	~ModuleEmitterImpl();

	void						init(const ModuleEmitterDesc& moduleEmitterDesc);

	// base class methods
	void						init(NvParameterized::Interface&) {}
	NvParameterized::Interface* getDefaultModuleDesc();
	void						release()
	{
		ModuleBase::release();
	}
	void						destroy();
	const char*					getName() const
	{
		READ_ZONE();
		return ModuleBase::getName();
	}

	RenderableIterator* 	createRenderableIterator(const Scene&);
	ModuleSceneIntl* 				createInternalModuleScene(SceneIntl&, RenderDebugInterface*);
	void						releaseModuleSceneIntl(ModuleSceneIntl&);
	uint32_t				forceLoadAssets();

	AuthObjTypeID				getModuleID() const;
	AuthObjTypeID             getEmitterAssetTypeID() const;

	ApexActor* 					getApexActor(Actor*, AuthObjTypeID) const;

	float getRateScale() const
	{
		return mRateScale;
	}
	float getDensityScale() const
	{
		return mDensityScale;
	}
	float getGroundDensityScale() const
	{
		return mGroundDensityScale;
	}

	void setRateScale(float rateScale)
	{
		mRateScale = rateScale;
	}
	void setDensityScale(float densityScale)
	{
		mDensityScale = densityScale;
	}
	void setGroundDensityScale(float groundDensityScale)
	{
		mGroundDensityScale = groundDensityScale;
	}

private:
	EmitterScene* 				getEmitterScene(const Scene& apexScene);
	ResourceList				mAuthorableObjects;
	ResourceList				mEmitterScenes;

private:

	EmitterModuleParameters* 			mModuleParams;

	float		mRateScale;
	float		mDensityScale;
	float		mGroundDensityScale;

	friend class EmitterAssetImpl;
	friend class GroundEmitterAssetImpl;
	friend class GroundEmitterActorImpl;
	friend class ImpactEmitterAssetImpl;
	friend class ImpactEmitterActorImpl;
};

}
} // namespace nvidia::apex

#endif // __MODULE_EMITTER_IMPL_H__
