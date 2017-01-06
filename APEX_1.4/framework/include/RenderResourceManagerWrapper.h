/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef RENDER_RESOURCE_MANAGER_WRAPPER_H
#define RENDER_RESOURCE_MANAGER_WRAPPER_H

#include "ApexSDKImpl.h"

namespace nvidia
{
namespace apex
{


class RenderResourceManagerWrapper : public UserRenderResourceManager, public UserAllocated
{
	PX_NOCOPY(RenderResourceManagerWrapper);
public:
	RenderResourceManagerWrapper(UserRenderResourceManager& rrm) :
	  mRrm(rrm)
	{
	}


	virtual UserRenderVertexBuffer*   createVertexBuffer(const UserRenderVertexBufferDesc& desc)
	{
		URR_CHECK;
		return mRrm.createVertexBuffer(desc);
	}

	virtual void                        releaseVertexBuffer(UserRenderVertexBuffer& buffer)
	{
		mRrm.releaseVertexBuffer(buffer);
	}

	virtual UserRenderIndexBuffer*    createIndexBuffer(const UserRenderIndexBufferDesc& desc)
	{
		URR_CHECK;
		return mRrm.createIndexBuffer(desc);
	}

	virtual void                        releaseIndexBuffer(UserRenderIndexBuffer& buffer) 
	{
		mRrm.releaseIndexBuffer(buffer);
	}

	virtual UserRenderBoneBuffer*     createBoneBuffer(const UserRenderBoneBufferDesc& desc)
	{
		URR_CHECK;
		return mRrm.createBoneBuffer(desc);
	}

	virtual void                        releaseBoneBuffer(UserRenderBoneBuffer& buffer)
	{
		mRrm.releaseBoneBuffer(buffer);
	}

	virtual UserRenderInstanceBuffer* createInstanceBuffer(const UserRenderInstanceBufferDesc& desc)
	{
		URR_CHECK;
		return mRrm.createInstanceBuffer(desc);
	}

	virtual void                        releaseInstanceBuffer(UserRenderInstanceBuffer& buffer)
	{
		mRrm.releaseInstanceBuffer(buffer);
	}

	virtual UserRenderSpriteBuffer*   createSpriteBuffer(const UserRenderSpriteBufferDesc& desc)
	{
		URR_CHECK;
		return mRrm.createSpriteBuffer(desc);
	}

	virtual void                        releaseSpriteBuffer(UserRenderSpriteBuffer& buffer)
	{
		mRrm.releaseSpriteBuffer(buffer);
	}

	virtual UserRenderSurfaceBuffer*  createSurfaceBuffer(const UserRenderSurfaceBufferDesc& desc)
	{
		URR_CHECK;
		return mRrm.createSurfaceBuffer(desc);
	}

	virtual void                        releaseSurfaceBuffer(UserRenderSurfaceBuffer& buffer)
	{
		mRrm.releaseSurfaceBuffer(buffer);
	}

	virtual UserRenderResource*       createResource(const UserRenderResourceDesc& desc)
	{
		URR_CHECK;
		return mRrm.createResource(desc);
	}

	virtual void                        releaseResource(UserRenderResource& resource)
	{
		mRrm.releaseResource(resource);
	}

	virtual uint32_t                       getMaxBonesForMaterial(void* material)
	{
		return mRrm.getMaxBonesForMaterial(material);
	}

	virtual bool getSpriteLayoutData(uint32_t spriteCount, uint32_t spriteSemanticsBitmap, UserRenderSpriteBufferDesc* textureDescArray)
	{
		return mRrm.getSpriteLayoutData(spriteCount, spriteSemanticsBitmap, textureDescArray);
	}

	virtual bool getInstanceLayoutData(uint32_t spriteCount, uint32_t spriteSemanticsBitmap, UserRenderInstanceBufferDesc* instanceDescArray)
	{
		return mRrm.getInstanceLayoutData(spriteCount, spriteSemanticsBitmap, instanceDescArray);
	}

private:
	UserRenderResourceManager& mRrm;
};


}
} // end namespace nvidia::apex


#endif // RENDER_RESOURCE_MANAGER_WRAPPER_H
