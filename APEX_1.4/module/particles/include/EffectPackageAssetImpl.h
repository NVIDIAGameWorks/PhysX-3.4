/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef PARTICLES_EFFECT_PACKAGE_ASSET_H
#define PARTICLES_EFFECT_PACKAGE_ASSET_H

#include "Apex.h"
#include "ApexUsingNamespace.h"
#include "EffectPackageAsset.h"
#include "EffectPackageActor.h"
#include "ApexSDKHelpers.h"
#include "ModuleParticlesImpl.h"
#include "ApexAssetAuthoring.h"
#include "ApexString.h"
#include "ApexAssetTracker.h"
#include "ApexAuthorableObject.h"
#include "EffectPackageAssetParams.h"
#include "ApexRWLockable.h"

#include "ReadCheck.h"
#include "WriteCheck.h"
#include "ApexAuthorableObject.h"

namespace nvidia
{
namespace particles
{

class EffectPackageActorImpl;
class ModuleParticlesImpl;

class EffectPackageAssetImpl : public EffectPackageAsset, public ApexResourceInterface, public ApexResource, public ApexRWLockable
{
	friend class EffectPackageAssetDummyAuthoring;
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	EffectPackageAssetImpl(ModuleParticlesImpl*, ResourceList&, const char* name);
	EffectPackageAssetImpl(ModuleParticlesImpl*, ResourceList&, NvParameterized::Interface*, const char*);

	~EffectPackageAssetImpl();

	/* Asset */
	const char* 					getName() const
	{
		READ_ZONE();
		return mName.c_str();
	}
	AuthObjTypeID					getObjTypeID() const
	{
		READ_ZONE();
		return mAssetTypeID;
	}
	const char* 					getObjTypeName() const
	{
		READ_ZONE();
		return getClassName();
	}

	uint32_t					forceLoadAssets();

	/* ApexResource */
	virtual void					release();

	/* ApexResourceInterface, ApexResource */
	uint32_t					getListIndex() const
	{
		return m_listIndex;
	}

	void							setListIndex(class ResourceList& list, uint32_t index)
	{
		m_list = &list;
		m_listIndex = index;
	}

	/* EffectPackageAsset specific methods */
	void							releaseEffectPackageActor(EffectPackageActor&);
	const EffectPackageAssetParams&	getEffectPackageParameters() const
	{
		return *mParams;
	}
	float					getDefaultScale() const
	{
		return 1;
	}
	void							destroy();

	const NvParameterized::Interface* getAssetNvParameterized() const
	{
		READ_ZONE();
		return mParams;
	}

	virtual float getDuration() const;
	virtual bool useUniqueRenderVolume() const;

	/**
	 * \brief Releases the ApexAsset but returns the NvParameterized::Interface and *ownership* to the caller.
	 */
	virtual NvParameterized::Interface* releaseAndReturnNvParameterizedInterface()
	{
		WRITE_ZONE();
		NvParameterized::Interface* ret = mParams;
		mParams = NULL;
		release();
		return ret;
	}
	NvParameterized::Interface* getDefaultActorDesc();
	NvParameterized::Interface* getDefaultAssetPreviewDesc();
	virtual Actor* createApexActor(const NvParameterized::Interface& /*parms*/, Scene& /*apexScene*/);
	virtual AssetPreview* createApexAssetPreview(const ::NvParameterized::Interface& /*params*/, AssetPreviewScene* /*previewScene*/)
	{
		WRITE_ZONE();
		PX_ALWAYS_ASSERT();
		return NULL;
	}

	virtual bool isValidForActorCreation(const ::NvParameterized::Interface& /*parms*/, Scene& /*apexScene*/) const
	{
		READ_ZONE();
		return true; // TODO implement this method
	}

	virtual bool isDirty() const
	{
		READ_ZONE();
		return false;
	}

	static AuthObjTypeID			mAssetTypeID;

protected:
	static const char* 				getClassName()
	{
		return PARTICLES_EFFECT_PACKAGE_AUTHORING_TYPE_NAME;
	}

	ModuleParticlesImpl*				mModule;

	ResourceList					mEffectPackageActors;
	ApexSimpleString				mName;
	EffectPackageAssetParams*			mParams;
	EffectPackageActorParams*			mDefaultActorParams;

	void							initializeAssetNameTable();

	friend class ModuleParticlesE;
	friend class EffectPackageActorImpl;
	template <class T_Module, class T_Asset, class T_AssetAuthoring> friend class nvidia::apex::ApexAuthorableObject;
};

#ifndef WITHOUT_APEX_AUTHORING
class EffectPackageAssetAuthoringImpl : public EffectPackageAssetImpl, public ApexAssetAuthoring, public EffectPackageAssetAuthoring
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	/* EffectPackageAssetAuthoring */
	EffectPackageAssetAuthoringImpl(ModuleParticlesImpl* m, ResourceList& l) :
		EffectPackageAssetImpl(m, l, "EffectPackageAssetAuthoringImpl") {}

	EffectPackageAssetAuthoringImpl(ModuleParticlesImpl* m, ResourceList& l, const char* name) :
		EffectPackageAssetImpl(m, l, name) {}

	EffectPackageAssetAuthoringImpl(ModuleParticlesImpl* m, ResourceList& l, NvParameterized::Interface* params, const char* name) :
		EffectPackageAssetImpl(m, l, params, name) {}

	~EffectPackageAssetAuthoringImpl() {}

	void							destroy()
	{
		delete this;
	}

	/* AssetAuthoring */
	const char* 					getName() const
	{
		READ_ZONE();
		return EffectPackageAssetImpl::getName();
	}
	const char* 					getObjTypeName() const
	{
		READ_ZONE();
		return EffectPackageAssetImpl::getClassName();
	}
	virtual bool					prepareForPlatform(nvidia::apex::PlatformTag)
	{
		WRITE_ZONE();
		APEX_INVALID_OPERATION("Not Implemented.");
		return false;
	}

	void setToolString(const char* toolName, const char* toolVersion, uint32_t toolChangelist)
	{
		WRITE_ZONE();
		ApexAssetAuthoring::setToolString(toolName, toolVersion, toolChangelist);
	}

	// from ApexAssetAuthoring
	virtual void setToolString(const char* toolString);

	/* ApexResource */
	virtual void					release()
	{
		mModule->mSdk->releaseAssetAuthoring(*this);
	}

	NvParameterized::Interface* getNvParameterized() const
	{
		return (NvParameterized::Interface*)getAssetNvParameterized();
	}
	/**
	 * \brief Releases the ApexAsset but returns the NvParameterized::Interface and *ownership* to the caller.
	 */
	virtual NvParameterized::Interface* releaseAndReturnNvParameterizedInterface()
	{
		WRITE_ZONE();
		NvParameterized::Interface* ret = mParams;
		mParams = NULL;
		release();
		return ret;
	}
};
#endif

}
} // end namespace nvidia

#endif // PARTICLES_EFFECT_PACKAGE_ASSET_H
