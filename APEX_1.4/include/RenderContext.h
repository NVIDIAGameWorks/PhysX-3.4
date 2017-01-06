/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef RENDER_CONTEXT_H
#define RENDER_CONTEXT_H

/*!
\file
\brief class RenderContext
*/

#include "ApexUsingNamespace.h"
#include "foundation/PxMat44.h"

namespace nvidia
{
namespace apex
{

class UserRenderResource;
class UserOpaqueMesh;

PX_PUSH_PACK_DEFAULT

/**
\brief Describes the context of a renderable object
*/
class RenderContext
{
public:
	RenderContext(void)
	{
		renderResource = 0;
		isScreenSpace = false;
        renderMeshName = NULL;
	}

public:
	UserRenderResource*	renderResource;		//!< The renderable resource to be rendered
	bool				isScreenSpace;		//!< The data is in screenspace and should use a screenspace projection that transforms X -1 to +1 and Y -1 to +1 with zbuffer disabled.
	PxMat44				local2world;		//!< Reverse world pose transform for this renderable
	PxMat44				world2local;		//!< World pose transform for this renderable
    const char*         renderMeshName;     //!< The name of the render mesh this context is associated with.
};

PX_POP_PACK

}
} // end namespace nvidia::apex

#endif // RENDER_CONTEXT_H
