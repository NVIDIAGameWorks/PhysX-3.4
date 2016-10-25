/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __VORTEX_FS_ASSET_H__
#define __VORTEX_FS_ASSET_H__

#include "BasicFSAssetImpl.h"
#include "VortexFSAssetPreview.h"
#include "VortexFSAssetParams.h"
#include "VortexFSActorParams.h"
#include "ApexAuthorableObject.h"

#define VORTEX_FS_AUTHORING_TYPE_NAME "VortexFSAsset"

namespace nvidia
{
namespace apex
{
class RenderMeshAsset;	
}	
namespace basicfs
{

class VortexFSActorImpl;

class VortexFSAsset : public BasicFSAssetImpl
{
	friend class BasicFSAssetDummyAuthoring;
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	VortexFSAsset(ModuleBasicFSImpl*, ResourceList&, const char*);
	VortexFSAsset(ModuleBasicFSImpl*, ResourceList&, NvParameterized::Interface*, const char*);
	~VortexFSAsset();

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

	NvParameterized::Interface* getDefaultActorDesc();
	virtual Actor* createApexActor(const NvParameterized::Interface& /*parms*/, Scene& /*apexScene*/);

	VortexFSPreview*	createVortexFSPreview(const VortexFSPreviewDesc& desc, AssetPreviewScene* previewScene);
	VortexFSPreview*	createVortexFSPreviewImpl(const VortexFSPreviewDesc& desc, VortexFSAsset* TurboAsset, AssetPreviewScene* previewScene);
	void				releaseVortexFSPreview(VortexFSPreview& preview);

	NvParameterized::Interface* getDefaultAssetPreviewDesc();

	virtual AssetPreview* createApexAssetPreview(const NvParameterized::Interface& /*params*/, AssetPreviewScene* previewScene);

	virtual bool isValidForActorCreation(const ::NvParameterized::Interface& /*parms*/, Scene& /*apexScene*/) const
	{
		return true; // todo implement this method
	}

	/* Typical asset members */
	static const char* 			getClassName() // return to protected
	{
		return VORTEX_FS_AUTHORING_TYPE_NAME;
	}

protected:

	static AuthObjTypeID		mAssetTypeID;

	VortexFSAssetParams*		mParams;
	VortexFSActorParams*		mDefaultActorParams;
	VortexFSPreviewParams*		mDefaultPreviewParams;

	friend class ModuleBasicFSImpl;
	friend class VortexFSActorImpl;
	template <class T_Module, class T_Asset, class T_AssetAuthoring> friend class nvidia::apex::ApexAuthorableObject;

};

class VortexFSAssetAuthoring : public VortexFSAsset, public ApexAssetAuthoring, public BasicFSAssetAuthoring
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	/* BasicFSAssetAuthoring */
	VortexFSAssetAuthoring(ModuleBasicFSImpl* m, ResourceList& l) :
		VortexFSAsset(m, l, "VortexFSAssetAuthoring") {}

	VortexFSAssetAuthoring(ModuleBasicFSImpl* m, ResourceList& l, const char* name) :
		VortexFSAsset(m, l, name) {}

	VortexFSAssetAuthoring(ModuleBasicFSImpl* m, ResourceList& l, NvParameterized::Interface* params, const char* name) :
		VortexFSAsset(m, l, params, name) {}

	~VortexFSAssetAuthoring() {}
	void                    destroy()
	{
		delete this;
	}

	/* AssetAuthoring */
	const char* 			getName(void) const
	{
		return VortexFSAsset::getName();
	}
	const char* 			getObjTypeName() const
	{
		return VortexFSAsset::getClassName();
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
		return VortexFSAsset::getAssetNvParameterized();
	}

	NvParameterized::Interface* releaseAndReturnNvParameterizedInterface(void)
	{
		return VortexFSAsset::releaseAndReturnNvParameterizedInterface();
	}

};

}
} // end namespace nvidia::apex

#endif