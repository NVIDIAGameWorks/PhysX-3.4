/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef USER_RENDER_INDEX_BUFFER_H
#define USER_RENDER_INDEX_BUFFER_H

/*!
\file
\brief class UserRenderIndexBuffer
*/

#include "ApexUsingNamespace.h"

/**
\brief Cuda graphics resource
*/
typedef struct CUgraphicsResource_st* CUgraphicsResource;

namespace nvidia
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

/**
\brief Used for storing index data for rendering.
*/
class UserRenderIndexBuffer
{
public:
	virtual		~UserRenderIndexBuffer() {}

	///Get the low-level handle of the buffer resource (D3D resource pointer or GL buffer object ID)
	///\return true id succeeded, false otherwise
	virtual bool getInteropResourceHandle(CUgraphicsResource& handle)
#if APEX_DEFAULT_NO_INTEROP_IMPLEMENTATION
	{
		PX_UNUSED(&handle);
		return false;
	}
#else
	= 0;
#endif

	//! write some data into the buffer.
	//  the source data type is assumed to be the same as what was defined in the descriptor.
	virtual void writeBuffer(const void* srcData, uint32_t srcStride, uint32_t firstDestElement, uint32_t numElements) = 0;
};

PX_POP_PACK

}
} // namespace nvidia::apex

#endif // USER_RENDER_INDEX_BUFFER_H
