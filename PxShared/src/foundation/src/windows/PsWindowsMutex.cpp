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

#include "windows/PsWindowsInclude.h"
#include "PsFoundation.h"
#include "PsUserAllocated.h"
#include "PsMutex.h"
#include "PsThread.h"
#include "foundation/PxErrorCallback.h"

namespace physx
{
namespace shdfnd
{

namespace
{
struct MutexWinImpl
{
	CRITICAL_SECTION mLock;
	Thread::Id mOwner;
};

MutexWinImpl* getMutex(MutexImpl* impl)
{
	return reinterpret_cast<MutexWinImpl*>(impl);
}
}

MutexImpl::MutexImpl()
{
	InitializeCriticalSection(&getMutex(this)->mLock);
	getMutex(this)->mOwner = 0;
}

MutexImpl::~MutexImpl()
{
	DeleteCriticalSection(&getMutex(this)->mLock);
}

void MutexImpl::lock()
{
	EnterCriticalSection(&getMutex(this)->mLock);

#if PX_DEBUG
	getMutex(this)->mOwner = Thread::getId();
#endif
}

bool MutexImpl::trylock()
{
	bool success = TryEnterCriticalSection(&getMutex(this)->mLock) != 0;
#if PX_DEBUG
	if(success)
		getMutex(this)->mOwner = Thread::getId();
#endif
	return success;
}

void MutexImpl::unlock()
{
#if PX_DEBUG
	// ensure we are already holding the lock
	if(getMutex(this)->mOwner != Thread::getId())
	{
		shdfnd::getFoundation().error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__,
		                              "Mutex must be unlocked only by thread that has already acquired lock");
		return;
	}

#endif

	LeaveCriticalSection(&getMutex(this)->mLock);
}

uint32_t MutexImpl::getSize()
{
	return sizeof(MutexWinImpl);
}

class ReadWriteLockImpl
{
	PX_NOCOPY(ReadWriteLockImpl)
  public:
	ReadWriteLockImpl()
	{
	}
	Mutex mutex;
	volatile LONG readerCount; // handle recursive writer locking
};

ReadWriteLock::ReadWriteLock()
{
	mImpl = reinterpret_cast<ReadWriteLockImpl*>(PX_ALLOC(sizeof(ReadWriteLockImpl), "ReadWriteLockImpl"));
	PX_PLACEMENT_NEW(mImpl, ReadWriteLockImpl);

	mImpl->readerCount = 0;
}

ReadWriteLock::~ReadWriteLock()
{
	mImpl->~ReadWriteLockImpl();
	PX_FREE(mImpl);
}

void ReadWriteLock::lockReader()
{
	mImpl->mutex.lock();

	InterlockedIncrement(&mImpl->readerCount);

	mImpl->mutex.unlock();
}

void ReadWriteLock::lockWriter()
{
	mImpl->mutex.lock();

	// spin lock until no readers
	while(mImpl->readerCount)
		;
}

void ReadWriteLock::unlockReader()
{
	InterlockedDecrement(&mImpl->readerCount);
}

void ReadWriteLock::unlockWriter()
{
	mImpl->mutex.unlock();
}

} // namespace shdfnd
} // namespace physx
