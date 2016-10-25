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
#include "VortexFSAsset.h"
#include "VortexFSActorImpl.h"
#include "ModuleBasicFSImpl.h"
#include "BasicFSScene.h"

namespace nvidia
{
namespace basicfs
{

AuthObjTypeID	VortexFSAsset::mAssetTypeID;

VortexFSAsset::VortexFSAsset(ModuleBasicFSImpl* module, ResourceList& list, const char* name) 
			: BasicFSAssetImpl(module, name)
			, mDefaultActorParams(NULL)
			, mDefaultPreviewParams(NULL)
{
	NvParameterized::Traits* traits = GetInternalApexSDK()->getParameterizedTraits();
	mParams = static_cast<VortexFSAssetParams*>(traits->createNvParameterized(VortexFSAssetParams::staticClassName()));
	PX_ASSERT(mParams);

	list.add(*this);
}

VortexFSAsset::VortexFSAsset(ModuleBasicFSImpl* module, ResourceList& list, NvParameterized::Interface* params, const char* name) 
			: BasicFSAssetImpl(module, name)
			, mParams(static_cast<VortexFSAssetParams*>(params))
			, mDefaultActorParams(NULL)
			, mDefaultPreviewParams(NULL)
{
	list.add(*this);
}

VortexFSAsset::~VortexFSAsset()
{
}


void VortexFSAsset::destroy()
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

NvParameterized::Interface* VortexFSAsset::getDefaultActorDesc()
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
		NvParameterized::Interface* param = traits->createNvParameterized(VortexFSActorParams::staticClassName());
		mDefaultActorParams = static_cast<VortexFSActorParams*>(param);

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

Actor* VortexFSAsset::createApexActor(const NvParameterized::Interface& params, Scene& apexScene)
{
	Actor* ret = 0;

	if (nvidia::strcmp(params.className(), VortexFSActorParams::staticClassName()) == 0)
	{
		const VortexFSActorParams& actorParams = static_cast<const VortexFSActorParams&>(params);

		BasicFSScene* es = mModule->getBasicFSScene(apexScene);
		ret = es->createVortexFSActor(actorParams, *this, mFSActors);
	}
	return ret;
}


VortexFSPreview* VortexFSAsset::createVortexFSPreview(const VortexFSPreviewDesc& desc, AssetPreviewScene* previewScene)
{
	return createVortexFSPreviewImpl(desc, this, previewScene);
}

VortexFSPreview* VortexFSAsset::createVortexFSPreviewImpl(const VortexFSPreviewDesc& desc, VortexFSAsset* TurboAsset, AssetPreviewScene* previewScene)
{
	return PX_NEW(VortexFSAssetPreview)(desc, mModule->mSdk, TurboAsset, previewScene);
}

void VortexFSAsset::releaseVortexFSPreview(VortexFSPreview& nxpreview)
{
	VortexFSAssetPreview* preview = DYNAMIC_CAST(VortexFSAssetPreview*)(&nxpreview);
	preview->destroy();
}

NvParameterized::Interface* VortexFSAsset::getDefaultAssetPreviewDesc()
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
		const char* className = VortexFSPreviewParams::staticClassName();
		NvParameterized::Interface* param = traits->createNvParameterized(className);
		mDefaultPreviewParams = static_cast<VortexFSPreviewParams*>(param);

		PX_ASSERT(param);
		if (!param)
		{
			return NULL;
		}
	}

	return mDefaultPreviewParams;
}

AssetPreview* VortexFSAsset::createApexAssetPreview(const NvParameterized::Interface& params, AssetPreviewScene* previewScene)
{
	AssetPreview* ret = 0;

	const char* className = params.className();
	if (nvidia::strcmp(className, VortexFSPreviewParams::staticClassName()) == 0)
	{
		VortexFSPreviewDesc desc;
		const VortexFSPreviewParams* pDesc = static_cast<const VortexFSPreviewParams*>(&params);

		desc.mPose = pDesc->globalPose;

		desc.mPreviewDetail = 0;
		if (pDesc->drawShape)
		{
			desc.mPreviewDetail |= APEX_VORTEX::VORTEX_DRAW_SHAPE;
		}
		if (pDesc->drawAssetInfo)
		{
			desc.mPreviewDetail |= APEX_VORTEX::VORTEX_DRAW_ASSET_INFO;
		}

		ret = createVortexFSPreview(desc, previewScene);
	}

	return ret;
}


}
} // end namespace nvidia::apex


