/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef USER_RENDER_SURFACE_BUFFER_DESC_H
#define USER_RENDER_SURFACE_BUFFER_DESC_H

/*!
\file
\brief class UserRenderSurfaceBufferDesc, structs RenderDataFormat and RenderSurfaceSemantic
*/

#include "ApexUsingNamespace.h"
#include "UserRenderResourceManager.h"
#include "RenderDataFormat.h"
#include "ApexSDK.h"

namespace physx
{
	class PxCudaContextManager;
}

namespace nvidia
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

/**
\brief Describes the semantics and layout of a Surface buffer
*/
class UserRenderSurfaceBufferDesc
{
public:
	UserRenderSurfaceBufferDesc(void)
	{
		setDefaults();
	}

	/**
	\brief Default values
	*/
	void setDefaults()
	{
//		hint   = RenderBufferHint::STATIC;		
		format = RenderDataFormat::UNSPECIFIED;
		
		width = 0;
		height = 0;
		depth = 1;

		//moduleIdentifier = 0;
		
		registerInCUDA = false;
		interopContext = 0;
//		stride = 0;
	}

	/**
	\brief Checks if the surface buffer descriptor is valid
	*/
	bool isValid(void) const
	{
		uint32_t numFailed = 0;
		numFailed += (format == RenderDataFormat::UNSPECIFIED);
		numFailed += (width == 0) && (height == 0) && (depth == 0);
		numFailed += registerInCUDA && (interopContext == 0);
//		numFailed += registerInCUDA && (stride == 0);
		
		return (numFailed == 0);
	}

public:
	/**
	\brief The size of U-dimension.
	*/
	uint32_t				width;
	
	/**
	\brief The size of V-dimension.
	*/
	uint32_t				height;

	/**
	\brief The size of W-dimension.
	*/
	uint32_t				depth;

	/**
	\brief Data format of suface buffer.
	*/
	RenderDataFormat::Enum	format;

	bool					registerInCUDA;  //!< Declare if the resource must be registered in CUDA upon creation

	/**
	This context must be used to register and unregister the resource every time the
	device is lost and recreated.
	*/
	PxCudaContextManager*	interopContext;
};

PX_POP_PACK

}
} // end namespace nvidia::apex

#endif // USER_RENDER_SURFACE_BUFFER_DESC_H
