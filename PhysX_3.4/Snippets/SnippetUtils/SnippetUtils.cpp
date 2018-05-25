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
// Copyright (c) 2008-2018 NVIDIA Corporation. All rights reserved.

#include "SnippetUtils.h"

#include "foundation/PxSimpleTypes.h"

#include "CmPhysXCommon.h"

#include "PsAtomic.h"
#include "PsString.h"
#include "PsSync.h"
#include "PsThread.h"
#include "PsTime.h"
#include "PsMutex.h"
#include "PsAllocator.h"
#include "extensions/PxDefaultAllocator.h"


namespace physx
{

namespace
{
PxDefaultAllocator gUtilAllocator;

struct UtilAllocator // since we're allocating internal classes here, make sure we align properly
{
	void* allocate(size_t size,const char* file,  PxU32 line) 	{ return gUtilAllocator.allocate(size, NULL, file, int(line));		}
	void deallocate(void* ptr)									{ gUtilAllocator.deallocate(ptr);								}
};
}


namespace SnippetUtils
{

	PxI32 atomicIncrement(volatile PxI32* val)
	{
		return Ps::atomicIncrement(val);
	}

	PxI32 atomicDecrement(volatile PxI32* val)
	{
		return Ps::atomicDecrement(val);
	}

	//******************************************************************************//

	PxU32 getNbPhysicalCores()
	{
		return Ps::Thread::getNbPhysicalCores();
	}

	//******************************************************************************//

	PxU32 getThreadId()
	{
		return static_cast<PxU32>(Ps::Thread::getId());
	}

	//******************************************************************************//

	PxU64 getCurrentTimeCounterValue()
	{
		return Ps::Time::getCurrentCounterValue();
	}

	PxReal getElapsedTimeInMilliseconds(const PxU64 elapsedTime)
	{
		return Ps::Time::getCounterFrequency().toTensOfNanos(elapsedTime)/(100.0f * 1000.0f);
	}

	PxReal getElapsedTimeInMicroSeconds(const PxU64 elapsedTime)
	{
		return Ps::Time::getCounterFrequency().toTensOfNanos(elapsedTime)/(100.0f);
	}

	//******************************************************************************//

	struct Sync: public Ps::SyncT<UtilAllocator> {};

	Sync* syncCreate()
	{
		return new(gUtilAllocator.allocate(sizeof(Sync), 0, 0, 0)) Sync();
	}

	void syncWait(Sync* sync)
	{
		sync->wait();
	}

	void syncSet(Sync* sync)
	{
		sync->set();
	}

	void syncReset(Sync* sync)
	{
		sync->reset();
	}
	
	void syncRelease(Sync* sync)
	{
		sync->~Sync();
		gUtilAllocator.deallocate(sync);
	}

	//******************************************************************************//

	struct Thread: public Ps::ThreadT<UtilAllocator>
	{
		Thread(ThreadEntryPoint entryPoint, void* data): 
			Ps::ThreadT<UtilAllocator>(),
			mEntryPoint(entryPoint),
			mData(data)
		{
		}

		virtual void execute(void)											
		{ 
			mEntryPoint(mData);
		}

		ThreadEntryPoint mEntryPoint;
		void* mData;
	};

	Thread* threadCreate(ThreadEntryPoint entryPoint, void* data)
	{
		Thread* createThread = static_cast<Thread*>(gUtilAllocator.allocate(sizeof(Thread), 0, 0, 0));
		PX_PLACEMENT_NEW(createThread, Thread(entryPoint, data));
		createThread->start();
		return createThread;
	}

	void threadQuit(Thread* thread)
	{
		thread->quit();
	}

	void threadSignalQuit(Thread* thread)
	{
		thread->signalQuit();
	}

	bool threadWaitForQuit(Thread* thread)
	{
		return thread->waitForQuit();
	}

	bool threadQuitIsSignalled(Thread* thread)
	{
		return thread->quitIsSignalled();
	}

	void threadRelease(Thread* thread)
	{
		thread->~Thread();
		gUtilAllocator.deallocate(thread);
	}

	//******************************************************************************//

	struct Mutex: public Ps::MutexT<UtilAllocator> {};

	Mutex* mutexCreate()
	{
		return new(gUtilAllocator.allocate(sizeof(Mutex), 0, 0, 0)) Mutex();
	}

	void mutexLock(Mutex* mutex)
	{
		mutex->lock();
	}

	void mutexUnlock(Mutex* mutex)
	{
		mutex->unlock();
	}

	void mutexRelease(Mutex* mutex)
	{
		mutex->~Mutex();
		gUtilAllocator.deallocate(mutex);
	}


} // namespace physXUtils
} // namespace physx
