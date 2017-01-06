/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef USER_RENDER_RESOURCE_H
#define USER_RENDER_RESOURCE_H

/*!
\file
\brief class UserRenderResource
*/

#include "ApexUsingNamespace.h"

namespace nvidia
{
namespace apex
{

class UserRenderVertexBuffer;
class UserRenderIndexBuffer;
class UserRenderBoneBuffer;
class UserRenderInstanceBuffer;
class UserRenderSpriteBuffer;


PX_PUSH_PACK_DEFAULT

/**
\brief An abstract interface to a renderable resource
*/
class UserRenderResource
{
public:
	virtual ~UserRenderResource() {}

	/** \brief Set vertex buffer range */
	virtual void setVertexBufferRange(uint32_t firstVertex, uint32_t numVerts) = 0;
	/** \brief Set index buffer range */
	virtual void setIndexBufferRange(uint32_t firstIndex, uint32_t numIndices) = 0;
	/** \brief Set bone buffer range */
	virtual void setBoneBufferRange(uint32_t firstBone, uint32_t numBones) = 0;
	/** \brief Set instance buffer range */
	virtual void setInstanceBufferRange(uint32_t firstInstance, uint32_t numInstances) = 0;
	/** \brief Set sprite buffer range */
	virtual void setSpriteBufferRange(uint32_t firstSprite, uint32_t numSprites) = 0;
	/** \brief Set sprite visible count */
	virtual void setSpriteVisibleCount(uint32_t visibleCount) { PX_UNUSED(visibleCount); }
	/** \brief Set material */
	virtual void setMaterial(void* material) = 0;

	/** \brief Get number of vertex buffers */
	virtual uint32_t getNbVertexBuffers() const = 0;
	/** \brief Get vertex buffer */
	virtual UserRenderVertexBuffer*	getVertexBuffer(uint32_t index) const = 0;
	/** \brief Get index buffer */
	virtual UserRenderIndexBuffer* getIndexBuffer() const = 0;
	/** \brief Get bone buffer */
	virtual UserRenderBoneBuffer* getBoneBuffer() const = 0;
	/** \brief Get instance buffer */
	virtual UserRenderInstanceBuffer* getInstanceBuffer() const = 0;
	/** \brief Get sprite buffer */
	virtual UserRenderSpriteBuffer*	getSpriteBuffer() const = 0;
};

PX_POP_PACK

}
} // end namespace nvidia::apex

#endif
