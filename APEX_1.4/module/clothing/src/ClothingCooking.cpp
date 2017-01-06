/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "ApexDefs.h"

#include "ClothingCooking.h"
#include "PsMemoryBuffer.h"
#include "ParamArray.h"

#include "CookingAbstract.h"

#include "ClothingScene.h"

#include "PsAtomic.h"

namespace nvidia
{
namespace clothing
{

void ClothingCookingLock::lockCooking()
{
	atomicIncrement(&mNumCookingDependencies);
}


void ClothingCookingLock::unlockCooking()
{
	atomicDecrement(&mNumCookingDependencies);
}


ClothingCookingTask::ClothingCookingTask(ClothingScene* clothingScene, CookingAbstract& _job) : job(&_job), nextTask(NULL),
	mState(Uninit), mClothingScene(clothingScene), mResult(NULL), mLockedObject(NULL)
{
	PX_ASSERT(((size_t)(void*)(&mState) & 0x00000003) == 0); // check alignment, mState must be aligned for the atomic exchange operations

	PX_ASSERT(job != NULL);
}


ClothingCookingTask::~ClothingCookingTask()
{
	if (job != NULL)
	{
		PX_DELETE_AND_RESET(job);
	}
}


void ClothingCookingTask::initCooking(PxTaskManager& tm, PxBaseTask* c)
{
	PxLightCpuTask::setContinuation(tm, c);

	int32_t oldState = atomicCompareExchange((int32_t*)(&mState), WaitForRun, Uninit);
	PX_ASSERT(oldState == Uninit);
	PX_UNUSED(oldState);
}


void ClothingCookingTask::run()
{
	// run
	NvParameterized::Interface* result = NULL;
	int32_t oldState = atomicCompareExchange((int32_t*)(&mState), Running, WaitForRun);
	PX_ASSERT(oldState != ReadyForRelease && oldState != Aborting && oldState != WaitForFetch);
	if (oldState == WaitForRun) // the change was successful
	{
		result = job->execute();
	}

	unlockObject();

	// try to run the next task. Must be called before in a state where it will be deleted
	mClothingScene->submitCookingTask(NULL);

	// finished
	oldState = atomicCompareExchange((int32_t*)(&mState), WaitForFetch, Running);
	if (oldState == Running)
	{
		mResult = result;
	}
	else
	{
		if (result != NULL)
		{
			result->destroy();
			result = NULL;
		}
		atomicExchange((int32_t*)(&mState), ReadyForRelease);
		PX_ASSERT(mResult == NULL);
	}
}

NvParameterized::Interface* ClothingCookingTask::getResult()
{
	if (mResult != NULL)
	{
		int32_t oldState = atomicCompareExchange((int32_t*)(&mState), ReadyForRelease, WaitForFetch);
		PX_ASSERT(oldState == WaitForFetch);
		PX_UNUSED(oldState);
		return mResult;
	}
	return NULL;
}

void ClothingCookingTask::lockObject(ClothingCookingLock* lockedObject)
{
	if (mLockedObject != NULL)
	{
		mLockedObject->unlockCooking();
	}

	mLockedObject = lockedObject;
	if (mLockedObject != NULL)
	{
		mLockedObject->lockCooking();
	}
}

void ClothingCookingTask::unlockObject()
{
	if (mLockedObject != NULL)
	{
		mLockedObject->unlockCooking();
		mLockedObject = NULL;
	}
}

void ClothingCookingTask::abort()
{
	int32_t oldState = atomicExchange((int32_t*)(&mState), Aborting);
	PX_ASSERT(oldState >= WaitForRun);
	if (oldState == WaitForFetch)
	{
		atomicExchange((int32_t*)(&mState), ReadyForRelease);
		if (mResult != NULL)
		{
			NvParameterized::Interface* oldParam = mResult;
			mResult = NULL;
			oldParam->destroy();
		}
		unlockObject();
	}
	else if (oldState != Running)
	{
		PX_ASSERT(mResult == NULL);
		atomicExchange((int32_t*)(&mState), ReadyForRelease);
		unlockObject();
	}
}

}
}

