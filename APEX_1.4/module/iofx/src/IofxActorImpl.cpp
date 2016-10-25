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
#include "ApexSDKIntl.h"
#include "SceneIntl.h"
#include "IofxActor.h"
#include "IofxActorImpl.h"
#include "IofxSceneCPU.h"
#include "IofxSceneGPU.h"
#include "RenderVolumeImpl.h"
#include "IosObjectData.h"
#include "IofxRenderData.h"

#include "ModuleIofxImpl.h"

namespace nvidia
{
namespace iofx
{

IofxActorImpl::IofxActorImpl(ResID renderResID, IofxScene* iscene, IofxManager& mgr)
	: mRenderResID(renderResID)
	, mIofxScene(iscene)
	, mMgr(mgr)
	, mRenderVolume(NULL) // IOS will set this after creation
	, mSemantics(0)
	, mActiveRenderable(NULL)
{
	//asset.add(*this);

	mResultBounds.setEmpty();
	mResultRange.startIndex = 0;
	mResultRange.objectCount = 0;
	mResultVisibleCount = 0;

	addSelfToContext(*iscene->mApexScene->getApexContext());    // Add self to ApexScene
	addSelfToContext(*iscene);							      // Add self to IofxScene
}

IofxActorImpl::~IofxActorImpl()
{
}

void IofxActorImpl::getLodRange(float& min, float& max, bool& intOnly) const
{
	READ_ZONE();
	PX_UNUSED(min);
	PX_UNUSED(max);
	PX_UNUSED(intOnly);
	APEX_INVALID_OPERATION("not implemented");
}


float IofxActorImpl::getActiveLod() const
{
	READ_ZONE();
	APEX_INVALID_OPERATION("BasicIosActor does not support this operation");
	return -1.0f;
}


void IofxActorImpl::forceLod(float lod)
{
	WRITE_ZONE();
	PX_UNUSED(lod);
	APEX_INVALID_OPERATION("not implemented");
}

void IofxActorImpl::release()
{
	if (mInRelease)
	{
		return;
	}
	mInRelease = true;
	destroy();
}


void IofxActorImpl::destroy()
{
	if (mRenderVolume)
	{
		mRenderVolume->removeIofxActor(*this);
	}

	// Removes self from scenes and IOFX manager
	// should be called after mRenderVolume->removeIofxActor to avoid dead-lock!!!
	ApexActor::destroy();

	for (uint32_t i = 0 ; i < mRenderableArray.size() ; i++)
	{
		IofxRenderableImpl* renderable = mRenderableArray[i];
		if (renderable != NULL)
		{
			renderable->release();
		}
	}
	mRenderableArray.clear();

	delete this;
}

bool IofxActorImpl::prepareRenderables(IosObjectBaseData* obj)
{
	if (mResultRange.objectCount > 0 && obj->renderData->checkSemantics(mSemantics))
	{
		const uint32_t instanceID = obj->renderData->getInstanceID();
		if (mRenderableArray.size() <= instanceID)
		{
			mRenderableArray.resize(instanceID + 1, NULL);
		}
		mActiveRenderable = mRenderableArray[instanceID];
		if (mActiveRenderable == NULL)
		{
			if (mMgr.mIsMesh)
			{
				mActiveRenderable = PX_NEW(IofxMeshRenderable)(mIofxScene, mRenderResID);
			}
			else
			{
				mActiveRenderable = PX_NEW(IofxSpriteRenderable)(mIofxScene, mRenderResID);
			}
			mRenderableArray[instanceID] = mActiveRenderable;
		}

		mActiveRenderable->update(obj->renderData, mResultBounds, mResultRange.startIndex, mResultRange.objectCount, mResultVisibleCount);
		return true;
	}
	else
	{
		mActiveRenderable = NULL;
		return false;
	}
}

IofxRenderable* IofxActorImpl::acquireRenderableReference()
{
	return mActiveRenderable ? mActiveRenderable->incrementReferenceCount() : NULL;
}


}
} // namespace nvidia
