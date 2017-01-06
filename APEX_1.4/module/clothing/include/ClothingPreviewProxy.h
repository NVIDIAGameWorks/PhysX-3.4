/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef CLOTHING_PREVIEW_PROXY_H
#define CLOTHING_PREVIEW_PROXY_H

#include "ClothingPreview.h"
#include "ClothingActorImpl.h"
#include "Renderable.h"
#include "ApexRWLockable.h"
#include "WriteCheck.h"
#include "ReadCheck.h"

namespace nvidia
{
namespace clothing
{

class ClothingPreviewProxy : public ClothingPreview, public UserAllocated, public ApexResourceInterface, public ApexRWLockable
{
	ClothingActorImpl impl;

public:
	APEX_RW_LOCKABLE_BOILERPLATE
#pragma warning( disable : 4355 ) // disable warning about this pointer in argument list
	ClothingPreviewProxy(const NvParameterized::Interface& desc, ClothingAssetImpl* asset, ResourceList* list) :
		impl(desc, NULL, this, asset, NULL)
	{
		list->add(*this);
	}

	virtual void release()
	{
		impl.release();
	}

	virtual uint32_t getListIndex() const
	{
		return impl.m_listIndex;
	}

	virtual void setListIndex(class ResourceList& list, uint32_t index)
	{
		impl.m_list = &list;
		impl.m_listIndex = index;
	}

	virtual void setPose(const PxMat44& pose)
	{
		WRITE_ZONE();
		impl.updateState(pose, NULL, 0, 0, ClothingTeleportMode::Continuous);
	}

	virtual const PxMat44 getPose() const
	{
		READ_ZONE();
		return impl.getGlobalPose();
	}

	virtual void lockRenderResources()
	{
		impl.lockRenderResources();
	}

	virtual void unlockRenderResources()
	{
		impl.unlockRenderResources();
	}

	virtual void updateRenderResources(bool rewriteBuffers = false, void* userRenderData = 0)
	{
		Renderable* renderable = impl.getRenderable();
		if (renderable != NULL)
		{
			renderable->updateRenderResources(rewriteBuffers, userRenderData);
		}
	}

	virtual void dispatchRenderResources(UserRenderer& renderer)
	{
		WRITE_ZONE();
		Renderable* renderable = impl.getRenderable();
		if (renderable != NULL)
		{
			renderable->dispatchRenderResources(renderer);
		}
	}

	virtual PxBounds3 getBounds() const
	{
		READ_ZONE();
		return impl.getBounds();
	}


	virtual void updateState(const PxMat44& globalPose, const PxMat44* newBoneMatrices, uint32_t boneMatricesByteStride, uint32_t numBoneMatrices)
	{
		WRITE_ZONE();
		impl.updateState(globalPose, newBoneMatrices, boneMatricesByteStride, numBoneMatrices, ClothingTeleportMode::Continuous);
	}

	void destroy()
	{
		impl.destroy();
		delete this;
	}

	virtual ~ClothingPreviewProxy() {}
};

}
} // namespace nvidia

#endif // CLOTHING_PREVIEW_PROXY_H
