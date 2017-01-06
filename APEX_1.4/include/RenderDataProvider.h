/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef RENDER_DATA_PROVIDER_H
#define RENDER_DATA_PROVIDER_H

/*!
\file
\brief class RenderDataProvider
*/

#include "ApexUsingNamespace.h"

namespace nvidia
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

/**
\brief An actor instance that provides renderable data
*/
class RenderDataProvider
{
public:
	/**
	\brief Lock the renderable resources of this Renderable actor

	Locks the renderable data of this Renderable actor.  If the user uses an RenderableIterator
	to retrieve the list of Renderables, then locking is handled for them automatically by APEX.  If the
	user is storing Renderable pointers and using them ad-hoc, then they must use this API to lock the
	actor while updateRenderResources() and/or dispatchRenderResources() is called.  If an iterator is not being
	used, the user is also responsible for insuring the Renderable has not been deleted by another game
	thread.
	*/
	virtual void lockRenderResources() = 0;

	/**
	\brief Unlocks the renderable data of this Renderable actor.

	See locking semantics for RenderDataProvider::lockRenderResources().
	*/
	virtual void unlockRenderResources() = 0;

	/**
	\brief Update the renderable data of this Renderable actor.

	When called, this method will use the UserRenderResourceManager interface to inform the user
	about its render resource needs.  It will also call the writeBuffer() methods of various graphics
	buffers.  It must be called by the user each frame before any calls to dispatchRenderResources().
	If the actor is not being rendered, this function may also be skipped.

	\param [in] rewriteBuffers If true then static buffers will be rewritten (in the case of a graphics 
	device context being lost if managed buffers aren't being used)

	\param [in] userRenderData A pointer used by the application for context information which will be sent in
	the UserRenderResourceManager::createResource() method as a member of the UserRenderResourceDesc class.
	*/
	virtual void updateRenderResources(bool rewriteBuffers = false, void* userRenderData = 0) = 0;
};

PX_POP_PACK

}
} // end namespace nvidia::apex

#endif // RENDER_DATA_PROVIDER_H
