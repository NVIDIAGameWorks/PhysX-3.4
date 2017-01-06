/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __DESTRUCTIBLEPREVIEW_PROXY_H__
#define __DESTRUCTIBLEPREVIEW_PROXY_H__

#include "Apex.h"
#include "DestructiblePreview.h"
#include "DestructiblePreviewImpl.h"
#include "PsUserAllocated.h"
#include "ApexRWLockable.h"
#include "ReadCheck.h"
#include "WriteCheck.h"

namespace nvidia
{
namespace destructible
{

class DestructiblePreviewProxy : public DestructiblePreview, public ApexResourceInterface, public UserAllocated, public ApexRWLockable
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	DestructiblePreviewImpl impl;

#pragma warning(disable : 4355) // disable warning about this pointer in argument list
	DestructiblePreviewProxy(DestructibleAssetImpl& asset, ResourceList& list, const NvParameterized::Interface* params)
		: impl(this, asset, params)
	{
		list.add(*this);
	}

	virtual ~DestructiblePreviewProxy()
	{
	}

	// AssetPreview methods
	virtual void setPose(const PxMat44& pose)
	{
		WRITE_ZONE();
		impl.setPose(pose);
	}

	virtual const PxMat44 getPose() const
	{
		READ_ZONE();
		return impl.getPose();
	}

	// DestructiblePreview methods

	virtual const RenderMeshActor* getRenderMeshActor() const
	{
		READ_ZONE();
		return const_cast<const RenderMeshActor*>(impl.getRenderMeshActor());
	}

	virtual void setExplodeView(uint32_t depth, float explode)
	{
		WRITE_ZONE();
		return impl.setExplodeView(depth, explode);
	}

	// ApexInterface methods
	virtual void release()
	{
		WRITE_ZONE();
		impl.release();
		delete this;
	}
	virtual void destroy()
	{
		impl.destroy();
	}

	// Renderable methods
	virtual void updateRenderResources(bool rewriteBuffers, void* userRenderData)
	{
		URR_SCOPE;
		impl.updateRenderResources(rewriteBuffers, userRenderData);
	}
	virtual void dispatchRenderResources(UserRenderer& api)
	{
		impl.dispatchRenderResources(api);
	}
	virtual PxBounds3 getBounds() const
	{
		return impl.getBounds();
	}
	virtual void lockRenderResources()
	{
		impl.ApexRenderable::renderDataLock();
	}
	virtual void unlockRenderResources()
	{
		impl.ApexRenderable::renderDataUnLock();
	}

	// ApexResourceInterface methods
	virtual void	setListIndex(ResourceList& list, uint32_t index)
	{
		impl.m_listIndex = index;
		impl.m_list = &list;
	}
	virtual uint32_t	getListIndex() const
	{
		return impl.m_listIndex;
	}
};

}
} // end namespace nvidia

#endif // __DESTRUCTIBLEPREVIEW_PROXY_H__
