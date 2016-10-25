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
#include "ForceFieldAssetPreviewImpl.h"
#include "ForceFieldAssetImpl.h"
#include "ForceFieldActorImpl.h"
#include "ModuleForceFieldImpl.h"
#include "ForceFieldScene.h"
#include "ApexResourceHelper.h"
#include "AssetPreviewScene.h"


namespace nvidia
{
namespace forcefield
{

ForceFieldAssetImpl::ForceFieldAssetImpl(ModuleForceFieldImpl* module, ResourceList& list, const char* name) :
	mModule(module),
	mName(name),
	mDefaultActorParams(NULL),
	mDefaultPreviewParams(NULL)
{
	NvParameterized::Traits* traits = GetInternalApexSDK()->getParameterizedTraits();
	mParams = (ForceFieldAssetParams*)traits->createNvParameterized(ForceFieldAssetParams::staticClassName());

	initializeAssetNameTable();

	list.add(*this);
}

ForceFieldAssetImpl::ForceFieldAssetImpl(ModuleForceFieldImpl* module, ResourceList& list, NvParameterized::Interface* params, const char* name) :
	mModule(module),
	mName(name),
	mParams((ForceFieldAssetParams*)params),
	mDefaultActorParams(NULL),
	mDefaultPreviewParams(NULL)
{
	initializeAssetNameTable();

	list.add(*this);
}

ForceFieldAssetImpl::~ForceFieldAssetImpl()
{
}

void ForceFieldAssetImpl::destroy()
{
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
	if (mParams)
	{
		mParams->destroy();
		mParams = NULL;
	}
	/* Assets that were forceloaded or loaded by actors will be automatically
	 * released by the ApexAssetTracker member destructors.
	 */

	/* Actors are automatically cleaned up on deletion by ResourceList dtor */
	delete this;
}

uint32_t ForceFieldAssetImpl::forceLoadAssets()
{
	WRITE_ZONE();
	// Is there anything to be done here?
	return NULL;
}

void ForceFieldAssetImpl::initializeAssetNameTable()
{
	ApexSimpleString tmpStr;
	NvParameterized::Handle h(*mParams);
	NvParameterized::Interface* refPtr;

	if (mParams->forceFieldKernelType == NULL)
	{
		NvParameterized::Handle h(mParams);
		h.getParameter("forceFieldKernelType");
		h.initParamRef(h.parameterDefinition()->refVariantVal(0), true);
	}

	mParams->getParameterHandle("forceFieldKernelType", h);
	mParams->getParamRef(h, refPtr);
	PX_ASSERT(refPtr);
	if (!refPtr)
	{
		APEX_INTERNAL_ERROR("No force field kernel specified");
		return;
	}

	tmpStr = refPtr->className();

	mGenericParams = NULL;
	mRadialParams = NULL;
	mFalloffParams = NULL;
	mNoiseParams = NULL;

	if (tmpStr == GenericForceFieldKernelParams::staticClassName())
	{
		mGenericParams = static_cast<GenericForceFieldKernelParams*>(refPtr);
	}
	else if (tmpStr == RadialForceFieldKernelParams::staticClassName())
	{
		mRadialParams = static_cast<RadialForceFieldKernelParams*>(refPtr);

		NvParameterized::Handle h(*mRadialParams);

		mRadialParams->getParameterHandle("falloffParameters", h);
		mRadialParams->getParamRef(h, refPtr);
		PX_ASSERT(refPtr);
		
		mFalloffParams = static_cast<ForceFieldFalloffParams*>(refPtr);
		
		mRadialParams->getParameterHandle("noiseParameters", h);
		mRadialParams->getParamRef(h, refPtr);
		PX_ASSERT(refPtr);

		mNoiseParams = static_cast<ForceFieldNoiseParams*>(refPtr);		
	}
	else
	{
		PX_ASSERT(0 && "Invalid force field kernel type for APEX_ForceField.");
		return;
	}
}

NvParameterized::Interface* ForceFieldAssetImpl::getDefaultActorDesc()
{
	WRITE_ZONE();
	NvParameterized::Traits* traits = GetInternalApexSDK()->getParameterizedTraits();
	PX_ASSERT(traits);
	if (!traits)
	{
		return NULL;
	}

	// create if not yet created
	if (!mDefaultActorParams)
	{
		NvParameterized::ErrorType error = NvParameterized::ERROR_NONE;
		PX_UNUSED(error);

		const char* className = ForceFieldActorParams::staticClassName();
		NvParameterized::Interface* param = traits->createNvParameterized(className);
		NvParameterized::Handle h(param);
		mDefaultActorParams = static_cast<ForceFieldActorParams*>(param);

		PX_ASSERT(param);
		if (!param)
		{
			return NULL;
		}
	}

	return mDefaultActorParams;
}


NvParameterized::Interface* ForceFieldAssetImpl::getDefaultAssetPreviewDesc()
{
	WRITE_ZONE();
	NvParameterized::Traits* traits = GetInternalApexSDK()->getParameterizedTraits();
	PX_ASSERT(traits);
	if (!traits)
	{
		return NULL;
	}

	// create if not yet created
	if (!mDefaultPreviewParams)
	{
		const char* className = ForceFieldAssetPreviewParams::staticClassName();
		NvParameterized::Interface* param = traits->createNvParameterized(className);
		mDefaultPreviewParams = static_cast<ForceFieldAssetPreviewParams*>(param);

		PX_ASSERT(param);
		if (!param)
		{
			return NULL;
		}
	}
	else
	{
		mDefaultPreviewParams->initDefaults();
	}

	return mDefaultPreviewParams;
}

Actor* ForceFieldAssetImpl::createApexActor(const NvParameterized::Interface& parms, Scene& apexScene)
{
	WRITE_ZONE();
	if (!isValidForActorCreation(parms, apexScene))
	{
		return NULL;
	}

	Actor* ret = 0;

	if (nvidia::strcmp(parms.className(), ForceFieldActorParams::staticClassName()) == 0)
	{
		ForceFieldActorDesc desc;

		const ForceFieldActorParams* pDesc = static_cast<const ForceFieldActorParams*>(&parms);
		desc.initialPose		= pDesc->initialPose;
		desc.scale				= pDesc->scale;
		desc.samplerFilterData  = ApexResourceHelper::resolveCollisionGroup128(pDesc->fieldSamplerFilterDataName ? pDesc->fieldSamplerFilterDataName : mParams->fieldSamplerFilterDataName);
		desc.boundaryFilterData = ApexResourceHelper::resolveCollisionGroup128(pDesc->fieldBoundaryFilterDataName ? pDesc->fieldBoundaryFilterDataName : mParams->fieldBoundaryFilterDataName);

		ForceFieldScene* es = mModule->getForceFieldScene(apexScene);
		ret = es->createForceFieldActor(desc, *this, mForceFieldActors);
	}

	return ret;
}

AssetPreview* ForceFieldAssetImpl::createApexAssetPreview(const NvParameterized::Interface& parms, AssetPreviewScene* previewScene)
{
	WRITE_ZONE();
	AssetPreview* ret = 0;

	const char* className = parms.className();
	if (nvidia::strcmp(className, ForceFieldAssetPreviewParams::staticClassName()) == 0)
	{
		ForceFieldPreviewDesc desc;
		const ForceFieldAssetPreviewParams* pDesc = static_cast<const ForceFieldAssetPreviewParams*>(&parms);

		desc.mPose	= pDesc->pose;
		desc.mIconScale = pDesc->iconScale;
		desc.mPreviewDetail = 0;
		if (pDesc->drawIcon)
		{
			desc.mPreviewDetail |= APEX_FORCEFIELD::FORCEFIELD_DRAW_ICON;
		}
		if (pDesc->drawBoundaries)
		{
			desc.mPreviewDetail |= APEX_FORCEFIELD::FORCEFIELD_DRAW_BOUNDARIES;
		}
		if (pDesc->drawBold)
		{
			desc.mPreviewDetail |= APEX_FORCEFIELD::FORCEFIELD_DRAW_WITH_CYLINDERS;
		}

		ret = createForceFieldPreview(desc, previewScene);
	}

	return ret;
}

void ForceFieldAssetImpl::releaseForceFieldActor(ForceFieldActor& nxactor)
{
	WRITE_ZONE();
	ForceFieldActorImpl* actor = DYNAMIC_CAST(ForceFieldActorImpl*)(&nxactor);
	actor->destroy();
}

ForceFieldPreview* ForceFieldAssetImpl::createForceFieldPreview(const ForceFieldPreviewDesc& desc, AssetPreviewScene* previewScene)
{
	return(createForceFieldPreviewImpl(desc, this, previewScene));
}

ForceFieldPreview* ForceFieldAssetImpl::createForceFieldPreviewImpl(const ForceFieldPreviewDesc& desc, ForceFieldAssetImpl* forceFieldAsset, AssetPreviewScene* previewScene)
{
	return(PX_NEW(ForceFieldAssetPreviewImpl)(desc, mModule->mSdk, forceFieldAsset, previewScene));
}

void ForceFieldAssetImpl::releaseForceFieldPreview(ForceFieldPreview& nxpreview)
{
	ForceFieldAssetPreviewImpl* preview = DYNAMIC_CAST(ForceFieldAssetPreviewImpl*)(&nxpreview);
	preview->destroy();
}

}
} // end namespace nvidia

