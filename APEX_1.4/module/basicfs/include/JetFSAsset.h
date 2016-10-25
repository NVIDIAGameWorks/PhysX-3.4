/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __JET_FS_ASSET_H__
#define __JET_FS_ASSET_H__

#include "BasicFSAssetImpl.h"
#include "JetFSAssetPreview.h"
#include "JetFSAssetParams.h"
#include "JetFSActorParams.h"
#include "ApexAuthorableObject.h"

namespace nvidia
{
namespace apex
{
class RenderMeshAsset;
}
namespace basicfs
{

class JetFSActorImpl;

class JetFSAsset : public BasicFSAssetImpl
{
	friend class BasicFSAssetDummyAuthoring;
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	JetFSAsset(ModuleBasicFSImpl*, ResourceList&, const char*);
	JetFSAsset(ModuleBasicFSImpl*, ResourceList&, NvParameterized::Interface*, const char*);
	~JetFSAsset();

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

	JetFSPreview*	createJetFSPreview(const JetFSPreviewDesc& desc, AssetPreviewScene* previewScene);
	JetFSPreview*	createJetFSPreviewImpl(const JetFSPreviewDesc& desc, JetFSAsset* TurboAsset, AssetPreviewScene* previewScene);
	void			releaseJetFSPreview(JetFSPreview& preview);

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
		return JET_FS_AUTHORING_TYPE_NAME;
	}

protected:

	static AuthObjTypeID		mAssetTypeID;

	JetFSAssetParams*			mParams;
	JetFSActorParams*			mDefaultActorParams;
	JetFSPreviewParams*			mDefaultPreviewParams;

	friend class ModuleBasicFSImpl;
	friend class JetFSActorImpl;
	friend class JetFSAssetPreview;
	template <class T_Module, class T_Asset, class T_AssetAuthoring> friend class nvidia::apex::ApexAuthorableObject;

};

class JetFSAssetAuthoring : public JetFSAsset, public ApexAssetAuthoring, public BasicFSAssetAuthoring
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	/* BasicFSAssetAuthoring */
	JetFSAssetAuthoring(ModuleBasicFSImpl* m, ResourceList& l) :
		JetFSAsset(m, l, "JetFSAssetAuthoring") {}

	JetFSAssetAuthoring(ModuleBasicFSImpl* m, ResourceList& l, const char* name) :
		JetFSAsset(m, l, name) {}

	JetFSAssetAuthoring(ModuleBasicFSImpl* m, ResourceList& l, NvParameterized::Interface* params, const char* name) :
		JetFSAsset(m, l, params, name) {}

	~JetFSAssetAuthoring() {}
	void                    destroy()
	{
		delete this;
	}

	/* AssetAuthoring */
	const char* 			getName(void) const
	{
		return JetFSAsset::getName();
	}
	const char* 			getObjTypeName() const
	{
		return JetFSAsset::getClassName();
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
		return JetFSAsset::getAssetNvParameterized();
	}

	NvParameterized::Interface* releaseAndReturnNvParameterizedInterface(void)
	{
		return JetFSAsset::releaseAndReturnNvParameterizedInterface();
	}

};

}
} // end namespace nvidia::apex

#endif