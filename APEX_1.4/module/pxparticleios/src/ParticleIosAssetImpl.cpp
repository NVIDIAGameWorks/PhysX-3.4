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
#include "ParticleIosAssetImpl.h"
#include "ParticleIosActorImpl.h"
#include "ModuleParticleIosImpl.h"
#include "ApexUsingNamespace.h"
#include "FluidParticleSystemParams.h"

#if APEX_CUDA_SUPPORT
#include "ParticleIosActorGPU.h"
#endif

namespace nvidia
{
namespace pxparticleios
{

ParticleIosAssetImpl::ParticleIosAssetImpl(ModuleParticleIosImpl* module, ResourceList& list, NvParameterized::Interface* params, const char* name) :
	mModule(module),
	mName(name),
	mParams((ParticleIosAssetParam*)params)
{
	list.add(*this);
}

ParticleIosAssetImpl::ParticleIosAssetImpl(ModuleParticleIosImpl* module, ResourceList& list, const char* name):
	mModule(module),
	mName(name),
	mParams(0)
{
	NvParameterized::Traits* traits = GetInternalApexSDK()->getParameterizedTraits();
	mParams = (ParticleIosAssetParam*)traits->createNvParameterized(ParticleIosAssetParam::staticClassName());

	list.add(*this);
}

void ParticleIosAssetImpl::release()
{
	mModule->mSdk->releaseAsset(*this);
}

void ParticleIosAssetImpl::destroy()
{
	if (mParams)
	{
		mParams->destroy();
		mParams = NULL;
	}

	delete this;
}

ParticleIosAssetImpl::~ParticleIosAssetImpl()
{
}

ParticleIosActorImpl* ParticleIosAssetImpl::getIosActorInScene(Scene& scene, bool mesh) const
{
	ParticleIosScene* iosScene = mModule->getParticleIosScene(scene);
	if (iosScene != 0)
	{
		for (uint32_t i = 0 ; i < mIosActorList.getSize() ; i++)
		{
			ParticleIosActorImpl* iosActor = DYNAMIC_CAST(ParticleIosActorImpl*)(mIosActorList.getResource(i));
			if (iosActor->mParticleIosScene == iosScene && iosActor->mIsMesh == mesh)
			{
				return iosActor;
			}
		}
	}
	return NULL;
}

Actor* ParticleIosAssetImpl::createIosActor(Scene& scene, IofxAsset* iofxAsset)
{
	WRITE_ZONE();
	ParticleIosActorImpl* iosActor = getIosActorInScene(scene, iofxAsset->getMeshAssetCount() > 0);
	if (iosActor == 0)
	{
		ParticleIosScene* iosScene = mModule->getParticleIosScene(scene);
		if (iosScene != 0)
		{
			iosActor = iosScene->createIosActor(mIosActorList, *this, *iofxAsset);
			iosActor->mIsMesh = iofxAsset->getMeshAssetCount() > 0;
		}
	}
	PX_ASSERT(iosActor);
	return iosActor;
}

void ParticleIosAssetImpl::releaseIosActor(Actor& actor)
{
	WRITE_ZONE();
	ParticleIosActorImpl* iosActor = DYNAMIC_CAST(ParticleIosActorImpl*)(&actor);
	iosActor->destroy();
}

uint32_t ParticleIosAssetImpl::forceLoadAssets()
{
	WRITE_ZONE();
	return 0;
}


#ifndef WITHOUT_APEX_AUTHORING
/*******************   ParticleIosAssetAuthoringImpl *******************/
ParticleIosAssetAuthoringImpl::ParticleIosAssetAuthoringImpl(ModuleParticleIosImpl* module, ResourceList& list):
	ParticleIosAssetImpl(module, list, "Authoring")
{
}
ParticleIosAssetAuthoringImpl::ParticleIosAssetAuthoringImpl(ModuleParticleIosImpl* module, ResourceList& list, const char* name):
	ParticleIosAssetImpl(module, list, name)
{
}

ParticleIosAssetAuthoringImpl::ParticleIosAssetAuthoringImpl(ModuleParticleIosImpl* module, ResourceList& list, NvParameterized::Interface* params, const char* name) :
	ParticleIosAssetImpl(module, list, params, name)
{
}

void ParticleIosAssetAuthoringImpl::release()
{
	delete this;
}

void ParticleIosAssetAuthoringImpl::setCollisionGroupName(const char* collisionGroupName)
{
	NvParameterized::Handle h(*mParams, "collisionGroupName");
	h.setParamString(collisionGroupName);
}

void ParticleIosAssetAuthoringImpl::setCollisionGroupMaskName(const char* collisionGroupMaskName)
{
	NvParameterized::Handle h(*mParams, "collisionGroupMaskName");
	h.setParamString(collisionGroupMaskName);
}
#endif // !WITHOUT_APEX_AUTHORING

}
} // namespace nvidia


