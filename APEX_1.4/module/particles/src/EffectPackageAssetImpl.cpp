/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "EffectPackageAssetImpl.h"
#include "EffectPackageActorParams.h"
#include "EffectPackageActorImpl.h"
#include "ModuleParticlesImpl.h"
#include "EmitterEffect.h"
#include "ReadCheck.h"
#include "WriteCheck.h"

#pragma warning(disable:4100)

namespace nvidia
{

namespace particles
{

EffectPackageAssetImpl::EffectPackageAssetImpl(ModuleParticlesImpl*, ResourceList&, const char* name)
{
	PX_ALWAYS_ASSERT();
}

EffectPackageAssetImpl::EffectPackageAssetImpl(ModuleParticlesImpl* moduleParticles, ResourceList& resourceList, NvParameterized::Interface* params, const char* name)
{
	mDefaultActorParams = NULL;
	mName = name;
	mModule = moduleParticles;
	mParams = static_cast< EffectPackageAssetParams*>(params);
	initializeAssetNameTable();
	resourceList.add(*this);
}

EffectPackageAssetImpl::~EffectPackageAssetImpl()
{
}

uint32_t	EffectPackageAssetImpl::forceLoadAssets()
{
	WRITE_ZONE();
	return 0;
}

NvParameterized::Interface* EffectPackageAssetImpl::getDefaultActorDesc()
{
	READ_ZONE();
	NvParameterized::Traits* traits = GetInternalApexSDK()->getParameterizedTraits();
	PX_ASSERT(traits);
	if (!traits)
	{
		return NULL;
	}
	// create if not yet created
	if (!mDefaultActorParams)
	{
		const char* className = EffectPackageActorParams::staticClassName();
		NvParameterized::Interface* param = traits->createNvParameterized(className);
		NvParameterized::Handle h(param);
		mDefaultActorParams = static_cast<EffectPackageActorParams*>(param);
		PX_ASSERT(param);
		if (!param)
		{
			return NULL;
		}
	}
	return mDefaultActorParams;

}

void	EffectPackageAssetImpl::release()
{
	mModule->mSdk->releaseAsset(*this);
}


NvParameterized::Interface* EffectPackageAssetImpl::getDefaultAssetPreviewDesc()
{
	READ_ZONE();
	PX_ALWAYS_ASSERT();
	return NULL;
}

Actor* EffectPackageAssetImpl::createApexActor(const NvParameterized::Interface& parms, Scene& apexScene)
{
	WRITE_ZONE();
	Actor* ret = NULL;

	ParticlesScene* ds = mModule->getParticlesScene(apexScene);
	if (ds)
	{
		const EffectPackageAssetParams* assetParams = mParams;
		const EffectPackageActorParams* actorParams = static_cast<const EffectPackageActorParams*>(&parms);
		EffectPackageActorImpl* ea = PX_NEW(EffectPackageActorImpl)(this, assetParams, actorParams,
		                         *GetInternalApexSDK(),
		                         apexScene,
		                         *ds,
		                         mModule->getModuleTurbulenceFS());

		ret = static_cast< Actor*>(ea);
	}
	return ret;
}

void	EffectPackageAssetImpl::destroy()
{
	if (mDefaultActorParams)
	{
		mDefaultActorParams->destroy();
		mDefaultActorParams = 0;
	}

	if (mParams)
	{
		mParams->destroy();
		mParams = NULL;
	}

	/* Actors are automatically cleaned up on deletion by ResourceList dtor */
	delete this;

}

void EffectPackageAssetImpl::initializeAssetNameTable()
{
}

float EffectPackageAssetImpl::getDuration() const
{
	READ_ZONE();
	float ret = 0;

	for (int32_t i = 0; i < mParams->Effects.arraySizes[0]; i++)
	{
		NvParameterized::Interface* iface = mParams->Effects.buf[i];
		if (iface && nvidia::strcmp(iface->className(), EmitterEffect::staticClassName()) == 0)
		{
			EmitterEffect* ee = static_cast< EmitterEffect*>(iface);
			float v = 0;
			if (ee->EffectProperties.Duration != 0 && ee->EffectProperties.RepeatCount != 9999)
			{
				v = ee->EffectProperties.InitialDelayTime + ee->EffectProperties.RepeatCount * ee->EffectProperties.Duration + ee->EffectProperties.RepeatCount * ee->EffectProperties.RepeatDelay;
			}
			if (v == 0)	// any infinite lifespan sub-effect means the entire effect package has an infinite life
			{
				ret = 0;
				break;
			}
			else if (v > ret)
			{
				ret = v;
			}
		}
	}

	return ret;
}

bool EffectPackageAssetImpl::useUniqueRenderVolume() const
{
	READ_ZONE();
	return mParams ? mParams->LODSettings.UniqueRenderVolume : false;
}

void EffectPackageAssetAuthoringImpl::setToolString(const char* toolString)
{
	if (mParams != NULL)
	{
		NvParameterized::Handle handle(*mParams, "toolString");
		PX_ASSERT(handle.isValid());
		if (handle.isValid())
		{
			PX_ASSERT(handle.parameterDefinition()->type() == NvParameterized::TYPE_STRING);
			handle.setParamString(toolString);
		}
	}
}

} // end of particles namespace
} // end of nvidia namespace
