/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "Apex.h"

#include "BasicIosAssetImpl.h"
#include "BasicIosActorImpl.h"
//#include "ApexSharedSerialization.h"
#include "ModuleBasicIosImpl.h"
#include "ApexUsingNamespace.h"

#if APEX_CUDA_SUPPORT
#include "BasicIosActorGPU.h"
#endif

namespace nvidia
{
namespace basicios
{

void BasicIosAssetImpl::processParams()
{
	NvParameterized::Handle handle(mParams);
	if (NvParameterized::ERROR_NONE != mParams->getParameterHandle("particleMass.type", handle))
	{
		PX_ALWAYS_ASSERT();
		return;
	}

	const char* type = 0;
	if (NvParameterized::ERROR_NONE != handle.getParamEnum(type))
	{
		PX_ALWAYS_ASSERT();
		return;
	}

	mMassDistribType = 0 == nvidia::strcmp("uniform", type) ? UNIFORM : NORMAL;
}

BasicIosAssetImpl::BasicIosAssetImpl(ModuleBasicIosImpl* module, ResourceList& list, NvParameterized::Interface* params, const char* name) :
	mModule(module),
	mName(name),
	mParams((BasicIOSAssetParam*)params)
{
	list.add(*this);
	processParams();
}

BasicIosAssetImpl::BasicIosAssetImpl(ModuleBasicIosImpl* module, ResourceList& list, const char* name):
	mModule(module),
	mName(name),
	mParams(0)
{
	NvParameterized::Traits* traits = GetInternalApexSDK()->getParameterizedTraits();
	mParams = (BasicIOSAssetParam*)traits->createNvParameterized(BasicIOSAssetParam::staticClassName());

	list.add(*this);

	processParams();
}

float BasicIosAssetImpl::getParticleMass() const
{
	READ_ZONE();
	float m = 0.0f;
	switch (mMassDistribType)
	{
	case UNIFORM:
		m = mParams->particleMass.center + mParams->particleMass.spread * mSRand.getNext();
		break;
	case NORMAL:
		m = mNormRand.getScaled(mParams->particleMass.center, mParams->particleMass.spread);
		break;
	default:
		PX_ALWAYS_ASSERT();
	}

	return m <= 0 ? mParams->particleMass.center : m; // Clamp
}

void BasicIosAssetImpl::release()
{
	mModule->mSdk->releaseAsset(*this);
}

void BasicIosAssetImpl::destroy()
{
	if (mParams)
	{
		mParams->destroy();
		mParams = NULL;
	}

	delete this;
}

BasicIosAssetImpl::~BasicIosAssetImpl()
{
}

BasicIosActorImpl* BasicIosAssetImpl::getIosActorInScene(Scene& scene, bool mesh) const
{
	BasicIosScene* iosScene = mModule->getBasicIosScene(scene);
	if (iosScene != 0)
	{
		for (uint32_t i = 0 ; i < mIosActorList.getSize() ; i++)
		{
			BasicIosActorImpl* iosActor = DYNAMIC_CAST(BasicIosActorImpl*)(mIosActorList.getResource(i));
			if (iosActor->mBasicIosScene == iosScene && iosActor->mIsMesh == mesh)
			{
				return iosActor;
			}
		}
	}
	return NULL;
}

Actor* BasicIosAssetImpl::createIosActor(Scene& scene, nvidia::apex::IofxAsset* iofxAsset)
{
	BasicIosActorImpl* iosActor = getIosActorInScene(scene, iofxAsset->getMeshAssetCount() > 0);
	if (iosActor == 0)
	{
		BasicIosScene* iosScene = mModule->getBasicIosScene(scene);
		if (iosScene != 0)
		{
			iosActor = iosScene->createIosActor(mIosActorList, *this, *iofxAsset);
			iosActor->mIsMesh = iofxAsset->getMeshAssetCount() > 0;
		}
	}
	PX_ASSERT(iosActor);
	return iosActor;
}

void BasicIosAssetImpl::releaseIosActor(Actor& actor)
{
	BasicIosActorImpl* iosActor = DYNAMIC_CAST(BasicIosActorImpl*)(&actor);
	iosActor->destroy();
}

uint32_t BasicIosAssetImpl::forceLoadAssets()
{
	return 0;
}

bool BasicIosAssetImpl::getSupportsDensity() const
{
	BasicIOSAssetParam* gridParams = (BasicIOSAssetParam*)(getAssetNvParameterized());
	return (gridParams->GridDensity.Enabled);
}

#ifndef WITHOUT_APEX_AUTHORING
/*******************   BasicIosAssetAuthoringImpl *******************/
BasicIosAssetAuthoringImpl::BasicIosAssetAuthoringImpl(ModuleBasicIosImpl* module, ResourceList& list):
	BasicIosAssetImpl(module, list, "Authoring")
{
}
BasicIosAssetAuthoringImpl::BasicIosAssetAuthoringImpl(ModuleBasicIosImpl* module, ResourceList& list, const char* name):
	BasicIosAssetImpl(module, list, name)
{
}

BasicIosAssetAuthoringImpl::BasicIosAssetAuthoringImpl(ModuleBasicIosImpl* module, ResourceList& list, NvParameterized::Interface* params, const char* name) :
	BasicIosAssetImpl(module, list, params, name)
{
}

void BasicIosAssetAuthoringImpl::release()
{
	delete this;
}

void BasicIosAssetAuthoringImpl::setCollisionGroupName(const char* collisionGroupName)
{
	NvParameterized::Handle h(*mParams, "collisionGroupName");
	h.setParamString(collisionGroupName);
}

void BasicIosAssetAuthoringImpl::setCollisionGroupMaskName(const char* collisionGroupMaskName)
{
	NvParameterized::Handle h(*mParams, "collisionGroupMaskName");
	h.setParamString(collisionGroupMaskName);
}


#endif

}
} // namespace nvidia
