/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  


#include "ReadCheck.h"
#include "ApexRWLockable.h"
#include "PsThread.h"
#include "ApexSDKIntl.h"

namespace nvidia
{
namespace apex
{

ReadCheck::ReadCheck(const ApexRWLockable* scene, const char* functionName)
	: mLockable(scene), mName(functionName), mErrorCount(0)
{
	if (GetApexSDK()->isConcurrencyCheckEnabled() && mLockable && !mLockable->isEnabled())
	{
		if (!mLockable->startRead() && !mLockable->isEnabled())
		{
			APEX_INTERNAL_ERROR("An API read call (%s) was made from thread %d but acquireReadLock() was not called first, note that "
				"when ApexSDKDesc::enableConcurrencyCheck is enabled all API reads and writes must be "
				"wrapped in the appropriate locks.", mName, uint32_t(nvidia::Thread::getId()));
		}

		// Record the NpScene read/write error counter which is
		// incremented any time a NpScene::startWrite/startRead fails
		// (see destructor for additional error checking based on this count)
		mErrorCount = mLockable->getReadWriteErrorCount();
	}
}


ReadCheck::~ReadCheck()
{
	if (GetApexSDK()->isConcurrencyCheckEnabled() && mLockable)
	{
		// By checking if the NpScene::mConcurrentErrorCount has been incremented
		// we can detect if an erroneous read/write was performed during 
		// this objects lifetime. In this case we also print this function's
		// details so that the user can see which two API calls overlapped
		if (mLockable->getReadWriteErrorCount() != mErrorCount && !mLockable->isEnabled())
		{
			APEX_INTERNAL_ERROR("Leaving %s on thread %d, an API overlapping write on another thread was detected.", mName, uint32_t(nvidia::Thread::getId()));
		}

		mLockable->stopRead();
	}
}

}
}