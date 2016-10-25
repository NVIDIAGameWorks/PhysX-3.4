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
#include "JetFSAsset.h"
#include "JetFSActorImpl.h"
#include "ModuleBasicFSImpl.h"

#include "BasicFSScene.h"

namespace nvidia
{
namespace basicfs
{

AuthObjTypeID JetFSAsset::mAssetTypeID; 

JetFSAsset::JetFSAsset(ModuleBasicFSImpl* module, ResourceList& list, const char* name) 
			: BasicFSAssetImpl(module, name)
			, mDefaultActorParams(NULL)
			, mDefaultPreviewParams(NULL)
{
	NvParameterized::Traits* traits = GetInternalApexSDK()->getParameterizedTraits();
	mParams = static_cast<JetFSAssetParams*>(traits->createNvParameterized(JetFSAssetParams::staticClassName()));
	PX_ASSERT(mParams);

	list.add(*this);
}

JetFSAsset::JetFSAsset(ModuleBasicFSImpl* module, ResourceList& list, NvParameterized::Interface* params, const char* name) 
			: BasicFSAssetImpl(module, name)
			, mParams(static_cast<JetFSAssetParams*>(params))
			, mDefaultActorParams(NULL)
			, mDefaultPreviewParams(NULL)
{
	list.add(*this);
}

JetFSAsset::~JetFSAsset()
{
}


void JetFSAsset::destroy()
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

NvParameterized::Interface* JetFSAsset::getDefaultActorDesc()
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
		NvParameterized::Interface* param = traits->createNvParameterized(JetFSActorParams::staticClassName());
		mDefaultActorParams = static_cast<JetFSActorParams*>(param);

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

Actor* JetFSAsset::createApexActor(const NvParameterized::Interface& params, Scene& apexScene)
{
	Actor* ret = 0;

	if (nvidia::strcmp(params.className(), JetFSActorParams::staticClassName()) == 0)
	{
		const JetFSActorParams& actorParams = static_cast<const JetFSActorParams&>(params);

		BasicFSScene* es = mModule->getBasicFSScene(apexScene);
		ret = es->createJetFSActor(actorParams, *this, mFSActors);
	}
	return ret;
}

NvParameterized::Interface* JetFSAsset::getDefaultAssetPreviewDesc()
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
		const char* className = JetFSPreviewParams::staticClassName();
		NvParameterized::Interface* param = traits->createNvParameterized(className);
		mDefaultPreviewParams = static_cast<JetFSPreviewParams*>(param);

		PX_ASSERT(param);
		if (!param)
		{
			return NULL;
		}
	}

	return mDefaultPreviewParams;
}

AssetPreview* JetFSAsset::createApexAssetPreview(const NvParameterized::Interface& params, AssetPreviewScene* previewScene)
{
	AssetPreview* ret = 0;

	const char* className = params.className();
	if (nvidia::strcmp(className, JetFSPreviewParams::staticClassName()) == 0)
	{
		JetFSPreviewDesc desc;
		const JetFSPreviewParams* pDesc = static_cast<const JetFSPreviewParams*>(&params);

		desc.mPose = pDesc->globalPose;

		desc.mPreviewDetail = 0;
		if (pDesc->drawShape)
		{
			desc.mPreviewDetail |= APEX_JET::JET_DRAW_SHAPE;
		}
		if (pDesc->drawAssetInfo)
		{
			desc.mPreviewDetail |= APEX_JET::JET_DRAW_ASSET_INFO;
		}

		ret = createJetFSPreview(desc, previewScene);
	}

	return ret;
}

JetFSPreview* JetFSAsset::createJetFSPreview(const JetFSPreviewDesc& desc, AssetPreviewScene* previewScene)
{
	return createJetFSPreviewImpl(desc, this, previewScene);
}

JetFSPreview* JetFSAsset::createJetFSPreviewImpl(const JetFSPreviewDesc& desc, JetFSAsset* jetAsset, AssetPreviewScene* previewScene)
{
	return PX_NEW(JetFSAssetPreview)(desc, mModule->mSdk, jetAsset, previewScene);
}

void JetFSAsset::releaseJetFSPreview(JetFSPreview& nxpreview)
{
	JetFSAssetPreview* preview = DYNAMIC_CAST(JetFSAssetPreview*)(&nxpreview);
	preview->destroy();
}

}
} // end namespace nvidia::apex


