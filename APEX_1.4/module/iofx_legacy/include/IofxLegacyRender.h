/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __IOFX_LEGACY_RENDER_H__
#define __IOFX_LEGACY_RENDER_H__

#include "Px.h"
#include "Renderable.h"
#include "PsUserAllocated.h"
#include "ApexRenderable.h"
#include "ApexRWLockable.h"

#include "iofx/IofxRenderable.h"
#include "iofx/IofxRenderCallback.h"

#include "ApexSDKIntl.h"
#include "Scene.h"
#include "PxTaskManager.h"


namespace nvidia
{
namespace apex
{
namespace legacy
{


class IofxLegacyRenderable : public Renderable, public ApexRenderable, public UserAllocated, public ApexRWLockable
{
public:
	virtual	physx::PxBounds3	getBounds() const
	{
		return ApexRenderable::getBounds();
	}
	virtual	void				lockRenderResources()
	{
		ApexRenderable::renderDataLock();
	}
	virtual	void				unlockRenderResources()
	{
		ApexRenderable::renderDataUnLock();
	}

	void update()
	{
		ApexRenderable::mRenderBounds = mRenderable.getBounds();
	}

protected:
	IofxLegacyRenderable(const IofxRenderable& renderable)
		: mRenderable(renderable)
	{
	}

	const IofxRenderable&	mRenderable;
};


class IofxLegacySpriteRenderable : public IofxLegacyRenderable
{
public:
	IofxLegacySpriteRenderable(const IofxRenderable& renderable)
		: IofxLegacyRenderable(renderable)
		, mRenderResource(NULL)
	{
	}
	virtual ~IofxLegacySpriteRenderable()
	{
		if (mRenderResource != NULL)
		{
			GetInternalApexSDK()->getUserRenderResourceManager()->releaseResource(*mRenderResource);
		}
	}

	virtual void updateRenderResources(bool rewriteBuffers, void* userRenderData);
	virtual void dispatchRenderResources(UserRenderer& renderer);

private:
	UserRenderResource*	mRenderResource;
};


class IofxLegacyMeshRenderable : public IofxLegacyRenderable
{
public:
	IofxLegacyMeshRenderable(const IofxRenderable& renderable)
		: IofxLegacyRenderable(renderable)
		, mRenderMeshActor(NULL)
	{
	}
	virtual ~IofxLegacyMeshRenderable()
	{
		if (mRenderMeshActor != NULL)
		{
			mRenderMeshActor->release();
		}
	}

	virtual void updateRenderResources(bool rewriteBuffers, void* userRenderData);
	virtual void dispatchRenderResources(UserRenderer& renderer);

private:
	RenderMeshActor* loadRenderMeshActor(const IofxMeshRenderData* meshRenderData);

	RenderMeshActor*		mRenderMeshActor;
};


class IofxLegacyRenderCallback : public IofxRenderCallback, public UserAllocated
{
public:
	IofxLegacyRenderCallback(const Scene* apexScene)
	{
		mGpuDispatcher = apexScene->getTaskManager()->getGpuDispatcher();
	}

	virtual void onCreatedIofxRenderable(IofxRenderable& renderable)
	{
		IofxLegacyRenderable* legacyRenderable = NULL;
		switch (renderable.getType())
		{
		case IofxRenderable::SPRITE:
			legacyRenderable = PX_NEW(IofxLegacySpriteRenderable)(renderable);
			break;
		case IofxRenderable::MESH:
			legacyRenderable = PX_NEW(IofxLegacyMeshRenderable)(renderable);
			break;
		default:
			PX_ALWAYS_ASSERT();
		}
		renderable.userData = legacyRenderable;
	}

	virtual void onUpdatedIofxRenderable(IofxRenderable& renderable)
	{
		IofxLegacyRenderable* legacyRenderable = static_cast<IofxLegacyRenderable*>(renderable.userData);
		if (legacyRenderable != NULL)
		{
			legacyRenderable->update();
		}
	}

	virtual void onReleasingIofxRenderable(IofxRenderable& renderable)
	{
		IofxLegacyRenderable* legacyRenderable = static_cast<IofxLegacyRenderable*>(renderable.userData);
		if (legacyRenderable != NULL)
		{
			PX_DELETE(legacyRenderable);
			renderable.userData = NULL;
		}
	}

	virtual bool getIofxSpriteRenderLayout(IofxSpriteRenderLayout& spriteRenderLayout, uint32_t spriteCount, uint32_t spriteSemanticsBitmap, RenderInteropFlags::Enum interopFlags);

	virtual bool getIofxMeshRenderLayout(IofxMeshRenderLayout& meshRenderLayout, uint32_t meshCount, uint32_t meshSemanticsBitmap, RenderInteropFlags::Enum interopFlags);

	virtual UserRenderBuffer* createRenderBuffer(const UserRenderBufferDesc& );

	virtual UserRenderSurface* createRenderSurface(const UserRenderSurfaceDesc& );

private:
	PxGpuDispatcher* mGpuDispatcher;
};


}
}
} // namespace nvidia

#endif /* __IOFX_LEGACY_RENDER_H__ */
