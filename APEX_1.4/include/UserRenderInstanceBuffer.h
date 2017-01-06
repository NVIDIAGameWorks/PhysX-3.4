/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef USER_RENDER_INSTANCE_BUFFER_H
#define USER_RENDER_INSTANCE_BUFFER_H

/*!
\file
\brief class UserRenderInstanceBuffer
*/

#include "RenderBufferData.h"
#include "UserRenderInstanceBufferDesc.h"

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
\brief The instance buffer type (deprecated)
*/
class PX_DEPRECATED RenderInstanceBufferData : public RenderBufferData<RenderInstanceSemantic, RenderInstanceSemantic::Enum> {};

/**
\brief Used for storing per-instance data for rendering.
*/
class UserRenderInstanceBuffer
{
public:
	virtual		~UserRenderInstanceBuffer() {}

	///Get the low-level handle of the buffer resource
	///\return true if succeeded, false otherwise
	virtual bool getInteropResourceHandle(CUgraphicsResource& handle)
#if APEX_DEFAULT_NO_INTEROP_IMPLEMENTATION
	{
		PX_UNUSED(&handle);
		return false;
	}
#else
	= 0;
#endif
	/**
	\brief Called when APEX wants to update the contents of the instance buffer.

	The source data type is assumed to be the same as what was defined in the descriptor.
	APEX should call this function and supply data for ALL semantics that were originally
	requested during creation every time its called.

	\param [in] data				Contains the source data for the instance buffer.
	\param [in] firstInstance		first instance to start writing to.
	\param [in] numInstances		number of instance to write.
	*/
	virtual void writeBuffer(const void* data, uint32_t firstInstance, uint32_t numInstances)
	{
		PX_UNUSED(data);
		PX_UNUSED(firstInstance);
		PX_UNUSED(numInstances);
	}
};

PX_POP_PACK

}
} // end namespace nvidia::apex

#endif // USER_RENDER_INSTANCE_BUFFER_H
