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
#include "NoiseFSAsset.h"
#include "NoiseFSActorImpl.h"
#include "ModuleBasicFSImpl.h"

#include "BasicFSScene.h"

namespace nvidia
{
namespace basicfs
{

AuthObjTypeID	NoiseFSAsset::mAssetTypeID;

NoiseFSAsset::NoiseFSAsset(ModuleBasicFSImpl* module, ResourceList& list, const char* name) 
			: BasicFSAssetImpl(module, name)
			, mDefaultActorParams(NULL)
			, mDefaultPreviewParams(NULL)
{
	NvParameterized::Traits* traits = GetInternalApexSDK()->getParameterizedTraits();
	mParams = static_cast<NoiseFSAssetParams*>(traits->createNvParameterized(NoiseFSAssetParams::staticClassName()));
	PX_ASSERT(mParams);

	list.add(*this);
}

NoiseFSAsset::NoiseFSAsset(ModuleBasicFSImpl* module, ResourceList& list, NvParameterized::Interface* params, const char* name) 
			: BasicFSAssetImpl(module, name)
			, mParams(static_cast<NoiseFSAssetParams*>(params))
			, mDefaultActorParams(NULL)
			, mDefaultPreviewParams(NULL)
{
	list.add(*this);
}

NoiseFSAsset::~NoiseFSAsset()
{
}


void NoiseFSAsset::destroy()
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

NvParameterized::Interface* NoiseFSAsset::getDefaultActorDesc()
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
		NvParameterized::Interface* param = traits->createNvParameterized(NoiseFSActorParams::staticClassName());
		mDefaultActorParams = static_cast<NoiseFSActorParams*>(param);

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

Actor* NoiseFSAsset::createApexActor(const NvParameterized::Interface& params, Scene& apexScene)
{
	Actor* ret = 0;

	if (nvidia::strcmp(params.className(), NoiseFSActorParams::staticClassName()) == 0)
	{
		const NoiseFSActorParams& actorParams = static_cast<const NoiseFSActorParams&>(params);

		BasicFSScene* es = mModule->getBasicFSScene(apexScene);
		ret = es->createNoiseFSActor(actorParams, *this, mFSActors);
	}
	return ret;
}

NvParameterized::Interface* NoiseFSAsset::getDefaultAssetPreviewDesc()
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
		const char* className = NoiseFSPreviewParams::staticClassName();
		NvParameterized::Interface* param = traits->createNvParameterized(className);
		mDefaultPreviewParams = static_cast<NoiseFSPreviewParams*>(param);

		PX_ASSERT(param);
		if (!param)
		{
			return NULL;
		}
	}

	return mDefaultPreviewParams;
}

AssetPreview* NoiseFSAsset::createApexAssetPreview(const NvParameterized::Interface& params, AssetPreviewScene* previewScene)
{
	AssetPreview* ret = 0;

	const char* className = params.className();
	if (nvidia::strcmp(className, NoiseFSPreviewParams::staticClassName()) == 0)
	{
		NoiseFSPreviewDesc desc;
		const NoiseFSPreviewParams* pDesc = static_cast<const NoiseFSPreviewParams*>(&params);

		desc.mPose = pDesc->globalPose;

		desc.mPreviewDetail = 0;
		if (pDesc->drawShape)
		{
			desc.mPreviewDetail |= APEX_NOISE::NOISE_DRAW_SHAPE;
		}
		if (pDesc->drawAssetInfo)
		{
			desc.mPreviewDetail |= APEX_NOISE::NOISE_DRAW_ASSET_INFO;
		}

		ret = createNoiseFSPreview(desc, previewScene);
	}

	return ret;
}

NoiseFSPreview* NoiseFSAsset::createNoiseFSPreview(const NoiseFSPreviewDesc& desc, AssetPreviewScene* previewScene)
{
	return createNoiseFSPreviewImpl(desc, this, previewScene);
}

NoiseFSPreview* NoiseFSAsset::createNoiseFSPreviewImpl(const NoiseFSPreviewDesc& desc, NoiseFSAsset* jetAsset, AssetPreviewScene* previewScene)
{
	return PX_NEW(NoiseFSAssetPreview)(desc, mModule->mSdk, jetAsset, previewScene);
}

void NoiseFSAsset::releaseNoiseFSPreview(NoiseFSPreview& nxpreview)
{
	NoiseFSAssetPreview* preview = DYNAMIC_CAST(NoiseFSAssetPreview*)(&nxpreview);
	preview->destroy();
}

}
} // end namespace nvidia::apex


