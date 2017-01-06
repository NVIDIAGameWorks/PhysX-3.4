/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef CONTEXT_H
#define CONTEXT_H

/*!
\file
\brief class Context
*/

#include "ApexInterface.h"

namespace nvidia
{
namespace apex
{

class Renderable;
class RenderableIterator;

PX_PUSH_PACK_DEFAULT

/**
\brief A container for Actors
*/
class Context
{
public:
	/**
	\brief Removes all actors from the context and releases them
	*/
	virtual void removeAllActors() = 0;

	/**
	\brief Create an iterator for all renderables in this context
	*/
	virtual RenderableIterator*	createRenderableIterator() = 0;

	/**
	\brief Release a renderable iterator

	Equivalent to calling the iterator's release method.
	*/
	virtual void releaseRenderableIterator(RenderableIterator&) = 0;

protected:
	virtual ~Context() {}
};

/**
\brief Iterate over all renderable Actors in an Context

An RenderableIterator is a lock-safe iterator over all renderable
Actors in an Context.  Actors which are locked are skipped in the initial
pass and deferred till the end.  The returned Renderable is locked by the
iterator and remains locked until you call getNext().

The RenderableIterator is also deletion safe.  If an actor is deleted
from the Context in another thread, the iterator will skip that actor.

An RenderableIterator should not be held for longer than a single simulation
step.  It should be allocated on demand and released after use.
*/
class RenderableIterator : public ApexInterface
{
public:
	/**
	\brief Return the first renderable in an Context
	*/
	virtual Renderable* getFirst() = 0;
	/**
	\brief Return the next unlocked renderable in an Context
	*/
	virtual Renderable* getNext() = 0;
	/**
	\brief Refresh the renderable actor list for this context

	This function is only necessary if you believe actors have been added or
	deleted since the iterator was created.
	*/
	virtual void reset() = 0;
};

PX_POP_PACK

}
} // end namespace nvidia::apex

#endif // CONTEXT_H
