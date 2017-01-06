/*
* Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#ifndef APEX_RENDER_RESOURCE_MANAGER_H
#define APEX_RENDER_RESOURCE_MANAGER_H

#include "Apex.h"
#include "ApexRenderResources.h"

#include "PsString.h"

#pragma warning(push)
#pragma warning(disable : 4917)
#pragma warning(disable : 4365)
#include <d3d11.h>
#pragma warning(pop)

using namespace nvidia::apex;

class ApexRenderResourceManager : public UserRenderResourceManager
{
  public:
	ApexRenderResourceManager(ID3D11Device* d) : mDevice(d) {}

	virtual UserRenderVertexBuffer* createVertexBuffer(const UserRenderVertexBufferDesc& desc)
	{
		return new SampleApexRendererVertexBuffer(*mDevice, desc);
	}
	virtual void releaseVertexBuffer(UserRenderVertexBuffer& buffer)
	{
		delete &buffer;
	}

	virtual UserRenderIndexBuffer* createIndexBuffer(const UserRenderIndexBufferDesc& desc)
	{
		return new SampleApexRendererIndexBuffer(*mDevice, desc);
	}
	virtual void releaseIndexBuffer(UserRenderIndexBuffer& buffer)
	{
		delete &buffer;
	}

	virtual UserRenderBoneBuffer* createBoneBuffer(const UserRenderBoneBufferDesc& desc)
	{
		return new SampleApexRendererBoneBuffer(*mDevice, desc);
	}
	virtual void releaseBoneBuffer(UserRenderBoneBuffer& buffer)
	{
		delete &buffer;
	}

	virtual UserRenderInstanceBuffer* createInstanceBuffer(const UserRenderInstanceBufferDesc& desc)
	{
		return new SampleApexRendererInstanceBuffer(*mDevice, desc);
	}
	virtual void releaseInstanceBuffer(UserRenderInstanceBuffer& buffer)
	{
		delete &buffer;
	}

	virtual UserRenderSpriteBuffer* createSpriteBuffer(const UserRenderSpriteBufferDesc& desc)
	{
		return new SampleApexRendererSpriteBuffer(*mDevice, desc);
	}
	virtual void releaseSpriteBuffer(UserRenderSpriteBuffer& buffer)
	{
		delete &buffer;
	}

	virtual UserRenderSurfaceBuffer* createSurfaceBuffer(const UserRenderSurfaceBufferDesc&)
	{
		physx::shdfnd::printString("createSurfaceBuffer");
		return NULL;
	}
	virtual void releaseSurfaceBuffer(UserRenderSurfaceBuffer&)
	{
	}

	virtual UserRenderResource* createResource(const UserRenderResourceDesc& desc)
	{
		return new SampleApexRendererMesh(*mDevice, desc);
	}
	virtual void releaseResource(UserRenderResource& buffer)
	{
		delete &buffer;
	}

	virtual uint32_t getMaxBonesForMaterial(void*)
	{
		return 0;
	}

	virtual bool getSpriteLayoutData(uint32_t spriteCount, uint32_t spriteSemanticsBitmap, UserRenderSpriteBufferDesc* vertexDescArray);
	virtual bool getInstanceLayoutData(uint32_t spriteCount, uint32_t particleSemanticsBitmap, UserRenderInstanceBufferDesc* instanceDescArray);

  private:
	ID3D11Device* mDevice;
};

#endif