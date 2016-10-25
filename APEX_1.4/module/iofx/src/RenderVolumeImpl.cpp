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

#include "RenderVolumeImpl.h"
#include "ModuleIofxImpl.h"
#include "IofxAssetImpl.h"
#include "IofxActorImpl.h"
#include "IofxScene.h"
#include "SceneIntl.h"

#include "PsArray.h"

namespace nvidia
{
namespace iofx
{

RenderVolumeImpl::RenderVolumeImpl(IofxScene& scene, const PxBounds3& b, uint32_t priority, bool allIofx)
	: mPriority(priority)
	, mAllIofx(allIofx)
	, mPendingDelete(false)
	, mScene(scene)
{
	setOwnershipBounds(b);

	mScene.mAddedRenderVolumesLock.lock();
	mScene.mAddedRenderVolumes.pushBack(this);
	mScene.mAddedRenderVolumesLock.unlock();
}

RenderVolumeImpl::~RenderVolumeImpl()
{
	mLock.lockWriter();
	while (mIofxActors.size())
	{
		IofxActor* iofx = mIofxActors.popBack();
		iofx->release();
	}
	mLock.unlockWriter();
}

void RenderVolumeImpl::destroy()
{
	if (!mPendingDelete)
	{
		mPendingDelete = true;

		mScene.mDeletedRenderVolumesLock.lock();
		mScene.mDeletedRenderVolumes.pushBack(this);
		mScene.mDeletedRenderVolumesLock.unlock();
	}
}

void RenderVolumeImpl::release()
{
	if (!mPendingDelete)
	{
		mScene.mModule->releaseRenderVolume(*this);
	}
}

bool RenderVolumeImpl::addIofxAsset(IofxAsset& iofx)
{
	WRITE_ZONE();
	if (mAllIofx || mPendingDelete)
	{
		return false;
	}

	mLock.lockWriter();
	mIofxAssets.pushBack(&iofx);
	mLock.unlockWriter();
	return true;
}

bool RenderVolumeImpl::addIofxActor(IofxActor& iofx)
{
	if (mPendingDelete)
	{
		return false;
	}

	mLock.lockWriter();
	mIofxActors.pushBack(&iofx);
	mLock.unlockWriter();
	return true;
}

bool RenderVolumeImpl::removeIofxActor(const IofxActor& iofx)
{
	bool res = false;
	mLock.lockWriter();
	for (uint32_t i = 0 ; i < mIofxActors.size() ; i++)
	{
		if (mIofxActors[ i ] == &iofx)
		{
			mIofxActors.replaceWithLast(i);
			res = true;
			break;
		}
	}
	mLock.unlockWriter();
	return res;
}

void RenderVolumeImpl::setPosition(const PxVec3& pos)
{
	WRITE_ZONE();
	PxVec3 ext = mOwnershipBounds.getExtents();
	mOwnershipBounds = physx::PxBounds3(pos - ext, pos + ext);
}

PxBounds3 RenderVolumeImpl::getBounds() const
{
	READ_ZONE();
	if (mPendingDelete)
	{
		return PxBounds3::empty();
	}

	PxBounds3 b = PxBounds3::empty();
	nvidia::Array<IofxActor*>::ConstIterator i;
	for (i = mIofxActors.begin() ; i != mIofxActors.end() ; i++)
	{
		b.include((*i)->getBounds());
	}

	return b;
}

// Callers must acquire render lock for thread safety
bool RenderVolumeImpl::affectsIofxAsset(const IofxAsset& iofx) const
{
	READ_ZONE();
	if (mPendingDelete)
	{
		return false;
	}

	if (mAllIofx)
	{
		return true;
	}

	nvidia::Array<IofxAsset*>::ConstIterator i;
	for (i = mIofxAssets.begin() ; i != mIofxAssets.end() ; i++)
	{
		if (&iofx == *i)
		{
			return true;
		}
	}

	return false;
}

}
} // namespace nvidia
