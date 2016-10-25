/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __ATTRACTOR_FS_ASSET_H__
#define __ATTRACTOR_FS_ASSET_H__

#include "BasicFSAssetImpl.h"
#include "AttractorFSAssetPreview.h"
#include "AttractorFSAssetParams.h"
#include "AttractorFSActorParams.h"
#include "ApexAuthorableObject.h"

namespace nvidia
{
namespace apex
{
class RenderMeshAsset;
}
namespace basicfs
{

class AttractorFSActorImpl;

class AttractorFSAsset : public BasicFSAssetImpl
{
	friend class BasicFSAssetDummyAuthoring;
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	AttractorFSAsset(ModuleBasicFSImpl*, ResourceList&, const char*);
	AttractorFSAsset(ModuleBasicFSImpl*, ResourceList&, NvParameterized::Interface*, const char*);
	~AttractorFSAsset();

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

	AttractorFSPreview*	createAttractorFSPreview(const AttractorFSPreviewDesc& desc, AssetPreviewScene* previewScene);
	AttractorFSPreview*	createAttractorFSPreviewImpl(const AttractorFSPreviewDesc& desc, AttractorFSAsset* TurboAsset, AssetPreviewScene* previewScene);
	void					releaseAttractorFSPreview(AttractorFSPreview& preview);

	NvParameterized::Interface* getDefaultAssetPreviewDesc();

	virtual AssetPreview* createApexAssetPreview(const NvParameterized::Interface& /*params*/, AssetPreviewScene* previewScene);

	virtual bool isValidForActorCreation(const ::NvParameterized::Interface& /*parms*/, Scene& /*apexScene*/) const
	{
		return true; // todo implement this method
	}

	/* Typical asset members */
	static const char* 			getClassName() // return to protected
	{
		return ATTRACTOR_FS_AUTHORING_TYPE_NAME;
	}

protected:

	static AuthObjTypeID		mAssetTypeID;

	AttractorFSAssetParams*		mParams;
	AttractorFSActorParams*		mDefaultActorParams;
	AttractorFSPreviewParams*	mDefaultPreviewParams;

	friend class ModuleBasicFSImpl;
	friend class AttractorFSActorImpl;
	template <class T_Module, class T_Asset, class T_AssetAuthoring> friend class nvidia::apex::ApexAuthorableObject;

};

class AttractorFSAssetAuthoring : public AttractorFSAsset, public ApexAssetAuthoring, public BasicFSAssetAuthoring
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	/* BasicFSAssetAuthoring */
	AttractorFSAssetAuthoring(ModuleBasicFSImpl* m, ResourceList& l) :
		AttractorFSAsset(m, l, "AttractorFSAssetAuthoring") {}

	AttractorFSAssetAuthoring(ModuleBasicFSImpl* m, ResourceList& l, const char* name) :
		AttractorFSAsset(m, l, name) {}

	AttractorFSAssetAuthoring(ModuleBasicFSImpl* m, ResourceList& l, NvParameterized::Interface* params, const char* name) :
		AttractorFSAsset(m, l, params, name) {}

	~AttractorFSAssetAuthoring() {}
	void                    destroy()
	{
		delete this;
	}

	/* AssetAuthoring */
	const char* 			getName(void) const
	{
		return AttractorFSAsset::getName();
	}
	const char* 			getObjTypeName() const
	{
		return AttractorFSAsset::getClassName();
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
		return AttractorFSAsset::getAssetNvParameterized();
	}

	NvParameterized::Interface* releaseAndReturnNvParameterizedInterface(void)
	{
		return AttractorFSAsset::releaseAndReturnNvParameterizedInterface();
	}

};

}
} // end namespace nvidia::apex

#endif