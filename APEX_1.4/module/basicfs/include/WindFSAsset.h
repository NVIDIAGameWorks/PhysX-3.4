/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __WIND_FS_ASSET_H__
#define __WIND_FS_ASSET_H__

#include "BasicFSAssetImpl.h"
#include "WindFSAssetPreview.h"
#include "WindFSAssetParams.h"
#include "WindFSActorParams.h"
#include "ApexAuthorableObject.h"

namespace nvidia
{
namespace apex
{
class RenderMeshAsset;
}
namespace basicfs
{

class WindFSActorImpl;

class WindFSAsset : public BasicFSAssetImpl
{
	friend class BasicFSAssetDummyAuthoring;
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	WindFSAsset(ModuleBasicFSImpl*, ResourceList&, const char*);
	WindFSAsset(ModuleBasicFSImpl*, ResourceList&, NvParameterized::Interface*, const char*);
	~WindFSAsset();

	/* Asset */
	AuthObjTypeID			getObjTypeID() const
	{
		return mAssetTypeID;
	}
	const char* 			getObjTypeName() const
	{
		return getClassName();
	}

	/* ApexInterface */
	virtual void			release()
	{
		mModule->mSdk->releaseAsset(*this);
	}

	// TODO: implement forceLoadAssets
	uint32_t					forceLoadAssets()
	{
		return 0;
	}

	NvParameterized::Interface* getAssetNvParameterized() const
	{
		return mParams;
	}

	NvParameterized::Interface* releaseAndReturnNvParameterizedInterface(void)
	{
		NvParameterized::Interface* ret = mParams;
		mParams = NULL;
		release();
		return ret;
	}

	/* BasicFSAsset specific methods */
	void                    destroy();

	/**
	* \brief Apply any changes that may been made to the NvParameterized::Interface on this asset.
	*/
	virtual void applyEditingChanges(void)
	{
		APEX_INVALID_OPERATION("Not yet implemented!");
	}

	WindFSPreview*	createWindFSPreview(const WindFSPreviewDesc& desc, AssetPreviewScene* previewScene);
	WindFSPreview*	createWindFSPreviewImpl(const WindFSPreviewDesc& desc, WindFSAsset* TurboAsset, AssetPreviewScene* previewScene);
	void				releaseWindFSPreview(WindFSPreview& preview);

	NvParameterized::Interface* getDefaultActorDesc();
	virtual Actor* createApexActor(const NvParameterized::Interface& /*parms*/, Scene& /*apexScene*/);

	NvParameterized::Interface* getDefaultAssetPreviewDesc();
	virtual AssetPreview* createApexAssetPreview(const NvParameterized::Interface& /*params*/, AssetPreviewScene* previewScene);

	virtual bool isValidForActorCreation(const ::NvParameterized::Interface& /*parms*/, Scene& /*apexScene*/) const
	{
		return true; // todo implement this method
	}

	/* Typical asset members */
	static const char* 			getClassName() // return to protected
	{
		return WIND_FS_AUTHORING_TYPE_NAME;
	}

protected:

	static AuthObjTypeID		mAssetTypeID;

	WindFSAssetParams*			mParams;
	WindFSActorParams*			mDefaultActorParams;
	WindFSPreviewParams*		mDefaultPreviewParams;

	friend class ModuleBasicFSImpl;
	friend class WindFSActorImpl;
	friend class WindFSAssetPreview;
	template <class T_Module, class T_Asset, class T_AssetAuthoring> friend class nvidia::apex::ApexAuthorableObject;

};

class WindFSAssetAuthoring : public WindFSAsset, public ApexAssetAuthoring, public BasicFSAssetAuthoring
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	WindFSAssetAuthoring(ModuleBasicFSImpl* m, ResourceList& l) :
		WindFSAsset(m, l, "WindFSAssetAuthoring") {}

	WindFSAssetAuthoring(ModuleBasicFSImpl* m, ResourceList& l, const char* name) :
		WindFSAsset(m, l, name) {}

	WindFSAssetAuthoring(ModuleBasicFSImpl* m, ResourceList& l, NvParameterized::Interface* params, const char* name) :
		WindFSAsset(m, l, params, name) {}

	~WindFSAssetAuthoring() {}
	void                    destroy()
	{
		delete this;
	}

	/* AssetAuthoring */
	const char* 			getName(void) const
	{
		return WindFSAsset::getName();
	}
	const char* 			getObjTypeName() const
	{
		return WindFSAsset::getClassName();
	}
	bool					prepareForPlatform(nvidia::apex::PlatformTag)
	{
		APEX_INVALID_OPERATION("Not Implemented.");
		return false;
	}

	void					setToolString(const char* toolName, const char* toolVersion, uint32_t toolChangelist)
	{
		ApexAssetAuthoring::setToolString(toolName, toolVersion, toolChangelist);
	}

	/* ApexInterface */
	virtual void			release()
	{
		mModule->mSdk->releaseAssetAuthoring(*this);
	}

	NvParameterized::Interface* getNvParameterized() const
	{
		return WindFSAsset::getAssetNvParameterized();
	}

	NvParameterized::Interface* releaseAndReturnNvParameterizedInterface(void)
	{
		return WindFSAsset::releaseAndReturnNvParameterizedInterface();
	}

};

}
} // end namespace nvidia::apex

#endif