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

#include "AttractorFSAsset.h"
#include "AttractorFSActorImpl.h"
#include "ModuleBasicFSImpl.h"
#include "BasicFSScene.h"

namespace nvidia
{
namespace basicfs
{

AuthObjTypeID	AttractorFSAsset::mAssetTypeID;

AttractorFSAsset::AttractorFSAsset(ModuleBasicFSImpl* module, ResourceList& list, const char* name) 
			: BasicFSAssetImpl(module, name)
			, mDefaultActorParams(NULL)
			, mDefaultPreviewParams(NULL)
{
	NvParameterized::Traits* traits = GetInternalApexSDK()->getParameterizedTraits();
	mParams = static_cast<AttractorFSAssetParams*>(traits->createNvParameterized(AttractorFSAssetParams::staticClassName()));
	PX_ASSERT(mParams);

	list.add(*this);
}

AttractorFSAsset::AttractorFSAsset(ModuleBasicFSImpl* module, ResourceList& list, NvParameterized::Interface* params, const char* name) 
			: BasicFSAssetImpl(module, name)
			, mParams(static_cast<AttractorFSAssetParams*>(params))
			, mDefaultActorParams(NULL)
			, mDefaultPreviewParams(NULL)
{
	list.add(*this);
}

AttractorFSAsset::~AttractorFSAsset()
{
}


void AttractorFSAsset::destroy()
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

	if (mDefaultPreviewParams)
	{
		mDefaultPreviewParams->destroy();
		mDefaultPreviewParams = 0;
	}


	/* Actors are automatically cleaned up on deletion by ResourceList dtor */
	delete this;
}

NvParameterized::Interface* AttractorFSAsset::getDefaultActorDesc()
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
		NvParameterized::Interface* param = traits->createNvParameterized(AttractorFSActorParams::staticClassName());
		mDefaultActorParams = static_cast<AttractorFSActorParams*>(param);

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

Actor* AttractorFSAsset::createApexActor(const NvParameterized::Interface& params, Scene& apexScene)
{
	Actor* ret = 0;

	if (nvidia::strcmp(params.className(), AttractorFSActorParams::staticClassName()) == 0)
	{
		const AttractorFSActorParams& actorParams = static_cast<const AttractorFSActorParams&>(params);

		BasicFSScene* es = mModule->getBasicFSScene(apexScene);
		ret = es->createAttractorFSActor(actorParams, *this, mFSActors);
	}
	return ret;
}


AttractorFSPreview* AttractorFSAsset::createAttractorFSPreview(const AttractorFSPreviewDesc& desc, AssetPreviewScene* previewScene)
{
	return createAttractorFSPreviewImpl(desc, this, previewScene);
}

AttractorFSPreview* AttractorFSAsset::createAttractorFSPreviewImpl(const AttractorFSPreviewDesc& desc, AttractorFSAsset* TurboAsset, AssetPreviewScene* previewScene)
{
	return PX_NEW(AttractorFSAssetPreview)(desc, mModule->mSdk, TurboAsset, previewScene);
}

void AttractorFSAsset::releaseAttractorFSPreview(AttractorFSPreview& nxpreview)
{
	AttractorFSAssetPreview* preview = DYNAMIC_CAST(AttractorFSAssetPreview*)(&nxpreview);
	preview->destroy();
}

NvParameterized::Interface* AttractorFSAsset::getDefaultAssetPreviewDesc()
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
		const char* className = AttractorFSPreviewParams::staticClassName();
		NvParameterized::Interface* param = traits->createNvParameterized(className);
		mDefaultPreviewParams = static_cast<AttractorFSPreviewParams*>(param);

		PX_ASSERT(param);
		if (!param)
		{
			return NULL;
		}
	}

	return mDefaultPreviewParams;
}

AssetPreview* AttractorFSAsset::createApexAssetPreview(const NvParameterized::Interface& params, AssetPreviewScene* previewScene)
{
	AssetPreview* ret = 0;

	const char* className = params.className();
	if (nvidia::strcmp(className, AttractorFSPreviewParams::staticClassName()) == 0)
	{
		AttractorFSPreviewDesc desc;
		const AttractorFSPreviewParams* pDesc = static_cast<const AttractorFSPreviewParams*>(&params);

		desc.mPose = pDesc->globalPose;

		desc.mPreviewDetail = 0;
		if (pDesc->drawShape)
		{
			desc.mPreviewDetail |= APEX_ATTRACT::ATTRACT_DRAW_SHAPE;
		}
		if (pDesc->drawAssetInfo)
		{
			desc.mPreviewDetail |= APEX_ATTRACT::ATTRACT_DRAW_ASSET_INFO;
		}

		ret = createAttractorFSPreview(desc, previewScene);
	}

	return ret;
}

}
} // end namespace nvidia::apex


