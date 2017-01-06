// This code contains NVIDIA Confidential Information and is disclosed to you
// under a form of NVIDIA software license agreement provided separately to you.
//
// Notice
// NVIDIA Corporation and its licensors retain all intellectual property and
// proprietary rights in and to this software and related documentation and
// any modifications thereto. Any use, reproduction, disclosure, or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA Corporation is strictly prohibited.
//
// ALL NVIDIA DESIGN SPECIFICATIONS, CODE ARE PROVIDED "AS IS.". NVIDIA MAKES
// NO WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES OF NONINFRINGEMENT,
// MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Information and code furnished is believed to be accurate and reliable.
// However, NVIDIA Corporation assumes no responsibility for the consequences of use of such
// information or for any infringement of patents or other rights of third parties that may
// result from its use. No license is granted by implication or otherwise under any patent
// or patent rights of NVIDIA Corporation. Details are subject to change without notice.
// This code supersedes and replaces all information previously supplied.
// NVIDIA Corporation products are not authorized for use as critical
// components in life support devices or systems without express written approval of
// NVIDIA Corporation.
//
// Copyright (c) 2008-2017 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  


#include "NpWriteCheck.h"

#include "NpScene.h"

using namespace physx;

NpWriteCheck::NpWriteCheck(NpScene* scene, const char* functionName, bool allowReentry) 
: mScene(scene), mName(functionName), mAllowReentry(allowReentry), mErrorCount(0)
{
	if (mScene)
	{
		switch (mScene->startWrite(mAllowReentry))
		{
		case NpScene::StartWriteResult::eOK:
			break;
		case NpScene::StartWriteResult::eNO_LOCK:
			Ps::getFoundation().error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__,
				"An API write call (%s) was made from thread %d but PxScene::lockWrite() was not called first, note that "
				"when PxSceneFlag::eREQUIRE_RW_LOCK is enabled all API reads and writes must be "
				"wrapped in the appropriate locks.", mName, PxU32(Ps::Thread::getId()));
			break;
		case NpScene::StartWriteResult::eRACE_DETECTED:
			Ps::getFoundation().error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__,
				"Concurrent API write call or overlapping API read and write call detected during %s from thread %d! "
				"Note that write operations to the SDK must be sequential, i.e., no overlap with "
				"other write or read calls, else the resulting behavior is undefined. "
				"Also note that API writes during a callback function are not permitted.", mName, PxU32(Ps::Thread::getId()));
			break;

		case NpScene::StartWriteResult::eIN_FETCHRESULTS:
			Ps::getFoundation().error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__,
				"Illegal write call detected in %s from thread %d during split fetchResults! "
				"Note that write operations to the SDK are not permitted between the start of fetchResultsStart() and end of fetchResultsFinish(). "
				"Behavior will be undefined. ", mName, PxU32(Ps::Thread::getId()));
			break;
		}

		// Record the NpScene read/write error counter which is
		// incremented any time a NpScene::startWrite/startRead fails
		// (see destructor for additional error checking based on this count)
		mErrorCount = mScene->getReadWriteErrorCount();
	}
}


NpWriteCheck::~NpWriteCheck()
{
	if (mScene)
	{
		// By checking if the NpScene::mConcurrentErrorCount has been incremented
		// we can detect if an erroneous read/write was performed during 
		// this objects lifetime. In this case we also print this function's
		// details so that the user can see which two API calls overlapped
		if (mScene->getReadWriteErrorCount() != mErrorCount && !(mScene->getScene().getFlags() & PxSceneFlag::eREQUIRE_RW_LOCK))
		{
			Ps::getFoundation().error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__, 
			"Leaving %s on thread %d, an overlapping API read or write by another thread was detected.", mName, PxU32(Ps::Thread::getId()));
		}

		mScene->stopWrite(mAllowReentry);
	}
}
