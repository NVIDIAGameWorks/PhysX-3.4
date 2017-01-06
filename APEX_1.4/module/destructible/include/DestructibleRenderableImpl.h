/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __DESTRUCTIBLE_RENDERABLE_IMPL_H__
#define __DESTRUCTIBLE_RENDERABLE_IMPL_H__

#include "Apex.h"
#include "DestructibleActor.h"
#include "DestructibleRenderable.h"
#include "ApexRWLockable.h"
#include "ApexActor.h"
#if APEX_RUNTIME_FRACTURE
#include "../fracture/Renderable.h"
#endif

#include "ReadCheck.h"

namespace nvidia
{
namespace apex
{
class RenderMeshActor;
}

namespace destructible
{

class DestructibleActorImpl;

class DestructibleRenderableImpl : public DestructibleRenderable, public ApexRenderable, public UserAllocated, public ApexRWLockable
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	DestructibleRenderableImpl(RenderMeshActor* renderMeshActors[DestructibleActorMeshType::Count], DestructibleAssetImpl* asset, int32_t listIndex);
	~DestructibleRenderableImpl();

	// Begin DestructibleRenderable methods
	virtual RenderMeshActor*	getRenderMeshActor(DestructibleActorMeshType::Enum type = DestructibleActorMeshType::Skinned) const
	{
		READ_ZONE();
		return (uint32_t)type < DestructibleActorMeshType::Count ? mRenderMeshActors[type] : NULL;
	}

	virtual void				release();
	// End DestructibleRenderable methods

	// Begin Renderable methods
	virtual	void				updateRenderResources(bool rewriteBuffers, void* userRenderData);

	virtual	void				dispatchRenderResources(UserRenderer& api);

	virtual	PxBounds3			getBounds() const
	{
		PxBounds3 bounds = ApexRenderable::getBounds();
#if APEX_RUNTIME_FRACTURE
		bounds.include(mRTrenderable.getBounds());
#endif
		return bounds;
	}

	virtual	void				lockRenderResources()
	{
		ApexRenderable::renderDataLock();
	}

	virtual	void				unlockRenderResources()
	{
		ApexRenderable::renderDataUnLock();
	}
	// End Renderable methods

	// Begin DestructibleRenderable methods
	// Returns this if successful, NULL otherwise
	DestructibleRenderableImpl*		incrementReferenceCount();

	int32_t						getReferenceCount()
	{
		return mRefCount;
	}

	void						setBounds(const PxBounds3& bounds)
	{
		mRenderBounds = bounds;
	}
	// End DestructibleRenderable methods

#if APEX_RUNTIME_FRACTURE
	::nvidia::fracture::Renderable& getRTrenderable() { return mRTrenderable; }
#endif

private:

	RenderMeshActor*	mRenderMeshActors[DestructibleActorMeshType::Count];	// Indexed by DestructibleActorMeshType::Enum
	DestructibleAssetImpl*	mAsset;
	int32_t				mListIndex;
	volatile int32_t		mRefCount;
#if APEX_RUNTIME_FRACTURE
	::nvidia::fracture::Renderable mRTrenderable;
#endif
};

}
} // end namespace nvidia

#endif // __DESTRUCTIBLE_RENDERABLE_IMPL_H__
