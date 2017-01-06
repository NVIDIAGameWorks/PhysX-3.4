/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef RENDERABLE_H
#define RENDERABLE_H

/*!
\file
\brief class Renderable
*/

#include "RenderDataProvider.h"

namespace nvidia
{
namespace apex
{

class UserRenderer;

PX_PUSH_PACK_DEFAULT

/**
\brief Base class of any actor that can be rendered
 */
class Renderable : public RenderDataProvider
{
public:
	/**
	When called, this method will use the UserRenderer interface to render itself (if visible, etc)
	by calling renderer.renderResource( RenderContext& ) as many times as necessary.   See locking
	semantics for RenderDataProvider::lockRenderResources().
	*/
	virtual void dispatchRenderResources(UserRenderer& renderer) = 0;

	/**
	Returns AABB covering rendered data.  The actor's world bounds is updated each frame
	during Scene::fetchResults().  This function does not require the Renderable actor to be locked.
	*/
	virtual PxBounds3 getBounds() const = 0;
};

PX_POP_PACK

}
} // end namespace nvidia::apex

#endif
