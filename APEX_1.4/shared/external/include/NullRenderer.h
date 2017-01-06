/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef __NULL_RENDERER_H_
#define __NULL_RENDERER_H_

#include "Apex.h"

/* This class is intended for use by command line tools that require an
 * APEX SDK.  Apps which use this renderer should _NOT_ call
 * updateRenderResources() or dispatchRenderResources().  You _WILL_
 * crash.
 */

namespace nvidia
{
namespace apex
{

class NullRenderResourceManager : public UserRenderResourceManager
{
public:
	UserRenderVertexBuffer*	createVertexBuffer(const UserRenderVertexBufferDesc&)
	{
		return NULL;
	}
	UserRenderIndexBuffer*	createIndexBuffer(const UserRenderIndexBufferDesc&)
	{
		return NULL;
	}
	UserRenderBoneBuffer*		createBoneBuffer(const UserRenderBoneBufferDesc&)
	{
		return NULL;
	}
	UserRenderInstanceBuffer*	createInstanceBuffer(const UserRenderInstanceBufferDesc&)
	{
		return NULL;
	}
	UserRenderSpriteBuffer*   createSpriteBuffer(const UserRenderSpriteBufferDesc&)
	{
		return NULL;
	}
	UserRenderSurfaceBuffer*   createSurfaceBuffer(const UserRenderSurfaceBufferDesc&)
	{
		return NULL;
	}
	UserRenderResource*		createResource(const UserRenderResourceDesc&)
	{
		return NULL;
	}
	void						releaseVertexBuffer(UserRenderVertexBuffer&) {}
	void						releaseIndexBuffer(UserRenderIndexBuffer&) {}
	void						releaseBoneBuffer(UserRenderBoneBuffer&) {}
	void						releaseInstanceBuffer(UserRenderInstanceBuffer&) {}
	void						releaseSpriteBuffer(UserRenderSpriteBuffer&) {}
	void						releaseSurfaceBuffer(UserRenderSurfaceBuffer&) {}
	void						releaseResource(UserRenderResource&) {}
	uint32_t						getMaxBonesForMaterial(void*)
	{
		return 0;
	}

	/** \brief Get the sprite layout data */
	virtual bool getSpriteLayoutData(uint32_t spriteCount, uint32_t spriteSemanticsBitmap, nvidia::apex::UserRenderSpriteBufferDesc* textureDescArray) 
	{
		PX_UNUSED(spriteCount);
		PX_UNUSED(spriteSemanticsBitmap);
		PX_UNUSED(textureDescArray);
		return false;
	}

	/** \brief Get the instance layout data */
	virtual bool getInstanceLayoutData(uint32_t particleCount, uint32_t particleSemanticsBitmap, nvidia::apex::UserRenderInstanceBufferDesc* instanceDescArray) 
	{
		PX_UNUSED(particleCount);
		PX_UNUSED(particleSemanticsBitmap);
		PX_UNUSED(instanceDescArray);
		return false;
	}


};

}
} // end namespace nvidia::apex

#endif
