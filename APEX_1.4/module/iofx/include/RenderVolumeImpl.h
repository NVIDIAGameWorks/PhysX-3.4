/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __RENDER_VOLUME_IMPL_H__
#define __RENDER_VOLUME_IMPL_H__

#include "Apex.h"
#include "RenderVolume.h"
#include "PsArray.h"
#include "ApexResource.h"
#include "ApexRenderable.h"
#include "ApexRWLockable.h"
#include "ReadCheck.h"
#include "WriteCheck.h"

namespace nvidia
{
namespace apex
{
class IofxAsset;
class IofxActor;
}
namespace iofx
{
class IofxScene;

class RenderVolumeImpl : public RenderVolume, public ApexResourceInterface, public ApexResource, public ApexRWLockable
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	RenderVolumeImpl(IofxScene& scene, const PxBounds3& b, uint32_t priority, bool allIofx);
	~RenderVolumeImpl();

	// ApexResourceInterface methods
	void				release();
	void			    destroy();

	uint32_t	    getListIndex() const
	{
		return m_listIndex;
	}
	void	            setListIndex(ResourceList& list, uint32_t index)
	{
		m_listIndex = index;
		m_list = &list;
	}

	void				setOwnershipBounds(const PxBounds3& b)
	{
		WRITE_ZONE();
		mOwnershipBounds = b;
	}
	PxBounds3			getOwnershipBounds(void) const
	{
		READ_ZONE();
		return mOwnershipBounds;
	}
	PxBounds3			getBounds() const;

	// methods for use by IOS or IOFX actor
	bool				addIofxActor(IofxActor& iofx);
	bool				removeIofxActor(const IofxActor& iofx);

	bool				addIofxAsset(IofxAsset& iofx);
	void				setPosition(const PxVec3& pos);

	bool				getAffectsAllIofx() const
	{
		READ_ZONE();
		return mAllIofx;
	}

	IofxActor* const* lockIofxActorList(uint32_t& count)
	{
		READ_ZONE();
		mLock.lockReader();
		count = mIofxActors.size();
		return count ? &mIofxActors.front() : NULL;
	}
	void unlockIofxActorList()
	{
		mLock.unlockReader();
	}


	IofxAsset* const* getIofxAssetList(uint32_t& count) const
	{
		READ_ZONE();
		count = mIofxAssets.size();
		return count ? &mIofxAssets.front() : NULL;
	}
	PxVec3				getPosition() const
	{
		READ_ZONE();
		return mOwnershipBounds.getCenter();
	}
	uint32_t				getPriority() const
	{
		READ_ZONE();
		return mPriority;
	}
	bool				affectsIofxAsset(const IofxAsset& iofx) const;

	PX_INLINE void	lockReader()
	{
		mLock.lockReader();
	}
	PX_INLINE void	unlockReader()
	{
		mLock.unlockReader();
	}

protected:
	// bounds is stored in ApexRenderable::mRenderBounds
	uint32_t						mPriority;
	bool							mAllIofx;
	bool							mPendingDelete;
	IofxScene&						mScene;
	nvidia::Array<IofxAsset*>		mIofxAssets;
	nvidia::Array<IofxActor*>		mIofxActors;
	physx::PxBounds3				mOwnershipBounds;
	physx::shdfnd::ReadWriteLock	mLock;
};

}
} // namespace nvidia

#endif // __RENDER_VOLUME_IMPL_H__
