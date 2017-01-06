/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef AUTHORABLE_OBJECT_INTL_H
#define AUTHORABLE_OBJECT_INTL_H

#include "ApexString.h"
#include "ApexSDKIntl.h"
#include "ApexSDKHelpers.h"
#include "ResourceProviderIntl.h"
#include "ApexResource.h"

class ResourceList;

namespace NvParameterized
{
class Interface;
};

namespace nvidia
{
namespace apex
{

// This class currently contains implementation, this will be removed and put in APEXAuthorableObject
class AuthorableObjectIntl : public ApexResourceInterface, public ApexResource
{
public:

	AuthorableObjectIntl(ModuleIntl* m, ResourceList& list, const char* aoTypeName)
		:	mAOTypeName(aoTypeName),
		    mModule(m)
	{
		list.add(*this);
	}

	virtual Asset* 			createAsset(AssetAuthoring& author, const char* name) = 0;
	virtual Asset* 			createAsset(NvParameterized::Interface* params, const char* name) = 0;
	virtual void					releaseAsset(Asset& nxasset) = 0;

	virtual AssetAuthoring* 	createAssetAuthoring() = 0;
	virtual AssetAuthoring* 	createAssetAuthoring(const char* name) = 0;
	virtual AssetAuthoring* 	createAssetAuthoring(NvParameterized::Interface* params, const char* name) = 0;
	virtual void					releaseAssetAuthoring(AssetAuthoring& nxauthor) = 0;

	virtual uint32_t					forceLoadAssets() = 0;
	virtual uint32_t					getAssetCount() = 0;
	virtual bool					getAssetList(Asset** outAssets, uint32_t& outAssetCount, uint32_t inAssetCount) = 0;


	virtual ResID					getResID() = 0;
	virtual ApexSimpleString&		getName() = 0;

	// ApexResourceInterface methods
	virtual void					release() = 0;
	virtual void					destroy() = 0;

	// ApexResourceInterface methods
	uint32_t							getListIndex() const
	{
		return m_listIndex;
	}

	void							setListIndex(ResourceList& list, uint32_t index)
	{
		m_listIndex = index;
		m_list = &list;
	}

	ResID				mAOResID;
	ResID				mAOPtrResID;
	ApexSimpleString	mAOTypeName;
	ApexSimpleString	mParameterizedName;

	ResourceList		mAssets;
	ResourceList		mAssetAuthors;

	ModuleIntl* 			mModule;
};

}
} // end namespace nvidia::apex

#endif	// AUTHORABLE_OBJECT_INTL_H
