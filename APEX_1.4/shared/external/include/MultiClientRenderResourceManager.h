/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MULTI_CLIENT_RENDER_RESOURCE_MANAGER_H
#define MULTI_CLIENT_RENDER_RESOURCE_MANAGER_H


#include "UserRenderResourceManager.h"
#include "UserRenderer.h"

#include <vector>


class MultiClientRenderResourceManager : public nvidia::apex::UserRenderResourceManager
{
public:

	MultiClientRenderResourceManager();
	~MultiClientRenderResourceManager();

	void addChild(nvidia::apex::UserRenderResourceManager* rrm, bool destroyAutomatic);


	virtual nvidia::apex::UserRenderVertexBuffer*   createVertexBuffer(const nvidia::apex::UserRenderVertexBufferDesc& desc);
	virtual void                                     releaseVertexBuffer(nvidia::apex::UserRenderVertexBuffer& buffer);

	virtual nvidia::apex::UserRenderIndexBuffer*    createIndexBuffer(const nvidia::apex::UserRenderIndexBufferDesc& desc);
	virtual void                                     releaseIndexBuffer(nvidia::apex::UserRenderIndexBuffer& buffer);

	virtual nvidia::apex::UserRenderBoneBuffer*     createBoneBuffer(const nvidia::apex::UserRenderBoneBufferDesc& desc);
	virtual void                                     releaseBoneBuffer(nvidia::apex::UserRenderBoneBuffer& buffer);

	virtual nvidia::apex::UserRenderInstanceBuffer* createInstanceBuffer(const nvidia::apex::UserRenderInstanceBufferDesc& desc);
	virtual void                                     releaseInstanceBuffer(nvidia::apex::UserRenderInstanceBuffer& buffer);

	virtual nvidia::apex::UserRenderSpriteBuffer*   createSpriteBuffer(const nvidia::apex::UserRenderSpriteBufferDesc& desc);
	virtual void                                     releaseSpriteBuffer(nvidia::apex::UserRenderSpriteBuffer& buffer);

	virtual nvidia::apex::UserRenderSurfaceBuffer*	 createSurfaceBuffer( const nvidia::apex::UserRenderSurfaceBufferDesc &desc );
	virtual void									 releaseSurfaceBuffer( nvidia::apex::UserRenderSurfaceBuffer &buffer );

	virtual nvidia::apex::UserRenderResource*       createResource(const nvidia::apex::UserRenderResourceDesc& desc);

	virtual void                                     releaseResource(nvidia::apex::UserRenderResource& resource);

	virtual uint32_t                             getMaxBonesForMaterial(void* material);

	virtual bool									getSpriteLayoutData(uint32_t spriteCount, 
																		uint32_t spriteSemanticsBitmap, 
																		nvidia::apex::UserRenderSpriteBufferDesc* bufferDesc);
	virtual bool									getInstanceLayoutData(uint32_t particleCount, 
																		uint32_t particleSemanticsBitmap, 
																		nvidia::apex::UserRenderInstanceBufferDesc* bufferDesc);
protected:

	struct Child
	{
		Child(nvidia::apex::UserRenderResourceManager* _rrm, bool destroy) : rrm(_rrm), destroyRrm(destroy) {}

		nvidia::apex::UserRenderResourceManager* rrm;
		bool destroyRrm;
	};

	std::vector<Child> mChildren;
};


class MultiClientUserRenderer : public nvidia::apex::UserRenderer
{
public:
	MultiClientUserRenderer() {}
	virtual ~MultiClientUserRenderer() {}

	void addChild(nvidia::apex::UserRenderer* child);
	virtual void renderResource(const nvidia::apex::RenderContext& context);

protected:
	std::vector<nvidia::apex::UserRenderer*> mChildren;
};


#endif // MULTI_CLIENT_RENDER_RESOURCE_MANAGER_H
