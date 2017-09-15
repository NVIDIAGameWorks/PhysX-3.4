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

#include "foundation/PxAssert.h"
#include "foundation/PxErrorCallback.h"

#include "Ps.h"
#include "PsFoundation.h"
#include "PsUserAllocated.h"
#include "PsMutex.h"
#include "PsAtomic.h"
#include "PsThread.h"

#include <pthread.h>

namespace physx
{
namespace shdfnd
{

namespace
{
struct MutexUnixImpl
{
	pthread_mutex_t lock;
	Thread::Id owner;
};

MutexUnixImpl* getMutex(MutexImpl* impl)
{
	return reinterpret_cast<MutexUnixImpl*>(impl);
}
}

MutexImpl::MutexImpl()
{
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
#if !PX_ANDROID
	// mimic default windows behavior where applicable
	pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT);
#endif
	pthread_mutex_init(&getMutex(this)->lock, &attr);
	pthread_mutexattr_destroy(&attr);
}

MutexImpl::~MutexImpl()
{
	pthread_mutex_destroy(&getMutex(this)->lock);
}

void MutexImpl::lock()
{
	int err = pthread_mutex_lock(&getMutex(this)->lock);
	PX_ASSERT(!err);
	PX_UNUSED(err);

#if PX_DEBUG
	getMutex(this)->owner = Thread::getId();
#endif
}

bool MutexImpl::trylock()
{
	bool success = !pthread_mutex_trylock(&getMutex(this)->lock);
#if PX_DEBUG
	if(success)
		getMutex(this)->owner = Thread::getId();
#endif
	return success;
}

void MutexImpl::unlock()
{
#if PX_DEBUG
	if(getMutex(this)->owner != Thread::getId())
	{
		shdfnd::getFoundation().error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__,
		                              "Mutex must be unlocked only by thread that has already acquired lock");
		return;
	}
#endif

	int err = pthread_mutex_unlock(&getMutex(this)->lock);
	PX_ASSERT(!err);
	PX_UNUSED(err);
}

uint32_t MutexImpl::getSize()
{
	return sizeof(MutexUnixImpl);
}

class ReadWriteLockImpl
{
  public:
	Mutex mutex;
	volatile int readerCounter;
};

ReadWriteLock::ReadWriteLock()
{
	mImpl = reinterpret_cast<ReadWriteLockImpl*>(PX_ALLOC(sizeof(ReadWriteLockImpl), "ReadWriteLockImpl"));
	PX_PLACEMENT_NEW(mImpl, ReadWriteLockImpl);

	mImpl->readerCounter = 0;
}

ReadWriteLock::~ReadWriteLock()
{
	mImpl->~ReadWriteLockImpl();
	PX_FREE(mImpl);
}

void ReadWriteLock::lockReader()
{
	mImpl->mutex.lock();

	atomicIncrement(&mImpl->readerCounter);

	mImpl->mutex.unlock();
}

void ReadWriteLock::lockWriter()
{
	mImpl->mutex.lock();

	while(mImpl->readerCounter != 0)
		;
}

void ReadWriteLock::unlockReader()
{
	atomicDecrement(&mImpl->readerCounter);
}

void ReadWriteLock::unlockWriter()
{
	mImpl->mutex.unlock();
}

} // namespace shdfnd
} // namespace physx
