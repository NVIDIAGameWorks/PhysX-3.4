/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __NOISE_FS_ASSET_H__
#define __NOISE_FS_ASSET_H__

#include "BasicFSAssetImpl.h"
#include "NoiseFSAssetPreview.h"
#include "NoiseFSAssetParams.h"
#include "NoiseFSActorParams.h"
#include "ApexAuthorableObject.h"

namespace nvidia
{
namespace apex
{
class RenderMeshAsset;
}
namespace basicfs
{

class NoiseFSActorImpl;

class NoiseFSAsset : public BasicFSAssetImpl
{
	friend class BasicFSAssetDummyAuthoring;
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	NoiseFSAsset(ModuleBasicFSImpl*, ResourceList&, const char*);
	NoiseFSAsset(ModuleBasicFSImpl*, ResourceList&, NvParameterized::Interface*, const char*);
	~NoiseFSAsset();

	/* Asset */
//	const char* 			getName() const
//	{
//		return mName.c_str();
//	}
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

	NoiseFSPreview*	createNoiseFSPreview(const NoiseFSPreviewDesc& desc, AssetPreviewScene* previewScene);
	NoiseFSPreview*	createNoiseFSPreviewImpl(const NoiseFSPreviewDesc& desc, NoiseFSAsset* TurboAsset, AssetPreviewScene* previewScene);
	void				releaseNoiseFSPreview(NoiseFSPreview& preview);

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
		return NOISE_FS_AUTHORING_TYPE_NAME;
	}

protected:

	static AuthObjTypeID		mAssetTypeID;

	NoiseFSAssetParams*			mParams;
	NoiseFSActorParams*			mDefaultActorParams;
	NoiseFSPreviewParams*		mDefaultPreviewParams;

	friend class ModuleBasicFSImpl;
	friend class NoiseFSActorImpl;
	friend class NoiseFSAssetPreview;
	template <class T_Module, class T_Asset, class T_AssetAuthoring> friend class nvidia::apex::ApexAuthorableObject;

};

class NoiseFSAssetAuthoring : public NoiseFSAsset, public ApexAssetAuthoring, public BasicFSAssetAuthoring
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	/* BasicFSAssetAuthoring */
	NoiseFSAssetAuthoring(ModuleBasicFSImpl* m, ResourceList& l) :
		NoiseFSAsset(m, l, "NoiseFSAssetAuthoring") {}

	NoiseFSAssetAuthoring(ModuleBasicFSImpl* m, ResourceList& l, const char* name) :
		NoiseFSAsset(m, l, name) {}

	NoiseFSAssetAuthoring(ModuleBasicFSImpl* m, ResourceList& l, NvParameterized::Interface* params, const char* name) :
		NoiseFSAsset(m, l, params, name) {}

	~NoiseFSAssetAuthoring() {}
	void                    destroy()
	{
		delete this;
	}

	/* AssetAuthoring */
	const char* 			getName(void) const
	{
		return NoiseFSAsset::getName();
	}
	const char* 			getObjTypeName() const
	{
		return NoiseFSAsset::getClassName();
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
		return NoiseFSAsset::getAssetNvParameterized();
	}

	NvParameterized::Interface* releaseAndReturnNvParameterizedInterface(void)
	{
		return NoiseFSAsset::releaseAndReturnNvParameterizedInterface();
	}

};

}
} // end namespace nvidia::apex

#endif