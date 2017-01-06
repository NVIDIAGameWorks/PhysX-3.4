/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef APEX_CONTEXT_H
#define APEX_CONTEXT_H

#include "ApexUsingNamespace.h"
#include "Context.h"
#include "PsMutex.h"
#include "PsArray.h"
#include "PsUserAllocated.h"
#include "ApexRWLockable.h"

namespace nvidia
{
namespace apex
{

class ApexActor;
class ApexRenderableIterator;

class ApexContext
{
public:
	ApexContext() : mIterator(NULL) {}
	virtual ~ApexContext();

	virtual uint32_t	addActor(ApexActor& actor, ApexActor* actorPtr = NULL);
	virtual void	callContextCreationCallbacks(ApexActor* actorPtr);
	virtual void	callContextDeletionCallbacks(ApexActor* actorPtr);
	virtual void	removeActorAtIndex(uint32_t index);

	void			renderLockAllActors();
	void			renderUnLockAllActors();

	void			removeAllActors();
	RenderableIterator* createRenderableIterator();
	void			releaseRenderableIterator(RenderableIterator&);

protected:
	physx::Array<ApexActor*> mActorArray;
	physx::Array<ApexActor*> mActorArrayCallBacks;
	nvidia::ReadWriteLock	mActorListLock;
	ApexRenderableIterator* mIterator;

	friend class ApexRenderableIterator;
	friend class ApexActor;
};

class ApexRenderableIterator : public RenderableIterator, public ApexRWLockable, public UserAllocated
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	Renderable* getFirst();
	Renderable* getNext();
	void			  reset();
	void			  release();

protected:
	void			  destroy();
	ApexRenderableIterator(ApexContext&);
	virtual ~ApexRenderableIterator() {}
	void			  removeCachedActor(ApexActor&);

	ApexContext*	  ctx;
	uint32_t             curActor;
	ApexActor*        mLockedActor;
	physx::Array<ApexActor*> mCachedActors;
	physx::Array<ApexActor*> mSkippedActors;

	friend class ApexContext;
};

}
} // end namespace nvidia::apex

#endif // APEX_CONTEXT_H