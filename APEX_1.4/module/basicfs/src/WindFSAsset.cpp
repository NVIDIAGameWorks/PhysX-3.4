/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "ApexDefs.h"
#include "Apex.h"
#include "WindFSAsset.h"
#include "WindFSActorImpl.h"
#include "ModuleBasicFSImpl.h"

#include "BasicFSScene.h"

namespace nvidia
{
namespace basicfs
{

AuthObjTypeID	WindFSAsset::mAssetTypeID;

WindFSAsset::WindFSAsset(ModuleBasicFSImpl* module, ResourceList& list, const char* name) 
			: BasicFSAssetImpl(module, name)
			, mDefaultActorParams(NULL)
			, mDefaultPreviewParams(NULL)
{
	NvParameterized::Traits* traits = GetInternalApexSDK()->getParameterizedTraits();
	mParams = static_cast<WindFSAssetParams*>(traits->createNvParameterized(WindFSAssetParams::staticClassName()));
	PX_ASSERT(mParams);

	list.add(*this);
}

WindFSAsset::WindFSAsset(ModuleBasicFSImpl* module, ResourceList& list, NvParameterized::Interface* params, const char* name) 
			: BasicFSAssetImpl(module, name)
			, mParams(static_cast<WindFSAssetParams*>(params))
			, mDefaultActorParams(NULL)
			, mDefaultPreviewParams(NULL)
{
	list.add(*this);
}

WindFSAsset::~WindFSAsset()
{
}


void WindFSAsset::destroy()
{
	if (mParams)
	{
		mParams->destroy();
		mParams = 0;
	}

	if (mDefaultActorParams)
	{
		mDefaultActorParams->destroy();
		mDefaultActorParams = 0;
	}

	/* Actors are automatically cleaned up on deletion by ResourceList dtor */
	delete this;
}

NvParameterized::Interface* WindFSAsset::getDefaultActorDesc()
{
	NvParameterized::Traits* traits = GetInternalApexSDK()->getParameterizedTraits();
	PX_ASSERT(traits);
	if (!traits)
	{
		return NULL;
	}

	// create if not yet created
	if (!mDefaultActorParams)
	{
		NvParameterized::Interface* param = traits->createNvParameterized(WindFSActorParams::staticClassName());
		mDefaultActorParams = static_cast<WindFSActorParams*>(param);

		PX_ASSERT(param);
		if (!param)
		{
			return NULL;
		}
	}
	else
	{
		mDefaultActorParams->initDefaults();
	}

	return mDefaultActorParams;
}

Actor* WindFSAsset::createApexActor(const NvParameterized::Interface& params, Scene& apexScene)
{
	Actor* ret = 0;

	if (nvidia::strcmp(params.className(), WindFSActorParams::staticClassName()) == 0)
	{
		const WindFSActorParams& actorParams = static_cast<const WindFSActorParams&>(params);

		BasicFSScene* es = mModule->getBasicFSScene(apexScene);
		ret = es->createWindFSActor(actorParams, *this, mFSActors);
	}
	return ret;
}

NvParameterized::Interface* WindFSAsset::getDefaultAssetPreviewDesc()
{
	NvParameterized::Traits* traits = GetInternalApexSDK()->getParameterizedTraits();
	PX_ASSERT(traits);
	if (!traits)
	{
		return NULL;
	}

	// create if not yet created
	if (!mDefaultPreviewParams)
	{
		const char* className = WindFSPreviewParams::staticClassName();
		NvParameterized::Interface* param = traits->createNvParameterized(className);
		mDefaultPreviewParams = static_cast<WindFSPreviewParams*>(param);

		PX_ASSERT(param);
		if (!param)
		{
			return NULL;
		}
	}

	return mDefaultPreviewParams;
}

AssetPreview* WindFSAsset::createApexAssetPreview(const NvParameterized::Interface& params, AssetPreviewScene* previewScene)
{
	AssetPreview* ret = 0;

	const char* className = params.className();
	if (nvidia::strcmp(className, WindFSPreviewParams::staticClassName()) == 0)
	{
		WindFSPreviewDesc desc;
		const WindFSPreviewParams* pDesc = static_cast<const WindFSPreviewParams*>(&params);

		desc.mPose = pDesc->globalPose;

		desc.mPreviewDetail = 0;
		if (pDesc->drawAssetInfo)
		{
			desc.mPreviewDetail |= APEX_WIND::WIND_DRAW_ASSET_INFO;
		}

		ret = createWindFSPreview(desc, previewScene);
	}

	return ret;
}

WindFSPreview* WindFSAsset::createWindFSPreview(const WindFSPreviewDesc& desc, AssetPreviewScene* previewScene)
{
	return createWindFSPreviewImpl(desc, this, previewScene);
}

WindFSPreview* WindFSAsset::createWindFSPreviewImpl(const WindFSPreviewDesc& desc, WindFSAsset* jetAsset, AssetPreviewScene* previewScene)
{
	return PX_NEW(WindFSAssetPreview)(desc, mModule->mSdk, jetAsset, previewScene);
}

void WindFSAsset::releaseWindFSPreview(WindFSPreview& nxpreview)
{
	WindFSAssetPreview* preview = DYNAMIC_CAST(WindFSAssetPreview*)(&nxpreview);
	preview->destroy();
}

}
} // end namespace nvidia::apex

