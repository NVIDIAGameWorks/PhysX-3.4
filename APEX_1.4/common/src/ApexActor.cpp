/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "Apex.h"
#include "ApexActor.h"
#include "ApexContext.h"

#include "RenderDebugInterface.h"
#include "SceneIntl.h"

#include "PsAtomic.h"

namespace nvidia
{
namespace apex
{

#if UNIQUE_ACTOR_ID
int32_t ApexActor::mUniqueActorIdCounter = 0;
#endif

ApexActor::ApexActor() : mInRelease(false), mEnableDebugVisualization(true)
{
#if UNIQUE_ACTOR_ID
	mUniqueActorId = shdfnd::atomicIncrement(&mUniqueActorIdCounter);
#endif
}


ApexActor::~ApexActor()
{
	destroy();
}

void ApexActor::addSelfToContext(ApexContext& ctx, ApexActor* actorPtr)
{
	ContextTrack t;

	t.ctx = &ctx;
	t.index = ctx.addActor(*this, actorPtr);
	mContexts.pushBack(t);
}

void ApexActor::updateIndex(ApexContext& ctx, uint32_t index)
{
	for (uint32_t i = 0 ; i < mContexts.size() ; i++)
	{
		ContextTrack& t = mContexts[i];
		if (t.ctx == &ctx)
		{
			t.index = index;
			break;
		}
	}
}

bool ApexActor::findSelfInContext(ApexContext& ctx)
{
	for (uint32_t i = 0 ; i < mContexts.size() ; i++)
	{
		ContextTrack& t = mContexts[i];
		if (t.ctx == &ctx)
		{
			return true;
		}
	}

	return false;
}

void ApexActor::destroy()
{
	mInRelease = true;

	renderDataLock();

	for (uint32_t i = 0 ; i < mContexts.size() ; i++)
	{
		ContextTrack& t = mContexts[i];
		t.ctx->removeActorAtIndex(t.index);
	}
	mContexts.clear();
}

}
} // end namespace nvidia::apex
