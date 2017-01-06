/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef USER_RENDERER_H
#define USER_RENDERER_H

/*!
\file
\brief class UserRenderer
*/

#include "ApexUsingNamespace.h"

namespace nvidia
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

class RenderContext;

/**
\brief User provided renderer interface
*/
class UserRenderer
{
public:
	virtual ~UserRenderer() {}
	/**
	\brief Render a resource

	Renderable::dispatchRenderResouces() will call this
	function as many times as possible to render all of the actor's
	sub-meshes.
	*/
	virtual void renderResource(const RenderContext& context) = 0;
};

PX_POP_PACK

}
} // end namespace nvidia::apex

#endif
