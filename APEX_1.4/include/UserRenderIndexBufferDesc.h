/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef USER_RENDER_INDEX_BUFFER_DESC_H
#define USER_RENDER_INDEX_BUFFER_DESC_H

/*!
\file
\brief class UserRenderIndexBufferDesc, structs RenderDataFormat and UserRenderIndexBufferDesc
*/

#include "RenderDataFormat.h"
#include "UserRenderResourceManager.h"

namespace physx
{
	class PxCudaContextManager;
};

namespace nvidia
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

#if !PX_PS4
	#pragma warning(push)
	#pragma warning(disable:4121)
#endif	//!PX_PS4

/**
\brief describes the semantics and layout of an index buffer
*/
class UserRenderIndexBufferDesc
{
public:
	UserRenderIndexBufferDesc(void)
	{
		registerInCUDA = false;
		interopContext = 0;
		maxIndices = 0;
		hint       = RenderBufferHint::STATIC;
		format     = RenderDataFormat::UNSPECIFIED;
		primitives = RenderPrimitiveType::TRIANGLES;
	}

	/**
	\brief Check if parameter's values are correct
	*/
	bool isValid(void) const
	{
		uint32_t numFailed = 0;
		numFailed += (registerInCUDA && !interopContext) ? 1 : 0;
		return (numFailed == 0);
	}

public:

	/**
	\brief The maximum amount of indices this buffer will ever hold.
	*/
	uint32_t					maxIndices;

	/**
	\brief Hint on how often this buffer is updated
	*/
	RenderBufferHint::Enum		hint;

	/**
	\brief The format of this buffer (only one implied semantic)
	*/
	RenderDataFormat::Enum		format;

	/**
	\brief Rendering primitive type (triangle, line strip, etc)
	*/
	RenderPrimitiveType::Enum	primitives;

	/**
	\brief Declare if the resource must be registered in CUDA upon creation
	*/
	bool						registerInCUDA;

	/**
	\brief The CUDA context

	This context must be used to register and unregister the resource every time the
	device is lost and recreated.
	*/
	PxCudaContextManager*		interopContext;
};

#if !PX_PS4
	#pragma warning(pop)
#endif	//!PX_PS4

PX_POP_PACK

}
} // end namespace nvidia::apex

#endif // USER_RENDER_INDEX_BUFFER_DESC_H
