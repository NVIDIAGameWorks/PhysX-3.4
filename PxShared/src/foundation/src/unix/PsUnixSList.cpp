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

#include "PsAllocator.h"
#include "PsAtomic.h"
#include "PsSList.h"
#include "PsThread.h"
#include <pthread.h>

#if PX_IOS || PX_EMSCRIPTEN
#define USE_MUTEX
#endif

namespace physx
{
namespace shdfnd
{
namespace
{
#if defined(USE_MUTEX)
class ScopedMutexLock
{
	pthread_mutex_t& mMutex;

  public:
	PX_INLINE ScopedMutexLock(pthread_mutex_t& mutex) : mMutex(mutex)
	{
		pthread_mutex_lock(&mMutex);
	}

	PX_INLINE ~ScopedMutexLock()
	{
		pthread_mutex_unlock(&mMutex);
	}
};

typedef ScopedMutexLock ScopedLock;
#else
struct ScopedSpinLock
{
	PX_FORCE_INLINE ScopedSpinLock(volatile int32_t& lock) : mLock(lock)
	{
		while(__sync_lock_test_and_set(&mLock, 1))
		{
			// spinning without atomics is usually
			// causing less bus traffic. -> only one
			// CPU is modifying the cache line.
			while(lock)
				PxSpinLockPause();
		}
	}

	PX_FORCE_INLINE ~ScopedSpinLock()
	{
		__sync_lock_release(&mLock);
	}

  private:
	volatile int32_t& mLock;
};

typedef ScopedSpinLock ScopedLock;
#endif

struct SListDetail
{
	SListEntry* head;
#if defined(USE_MUTEX)
	pthread_mutex_t lock;
#else
	volatile int32_t lock;
#endif
};

template <typename T>
SListDetail* getDetail(T* impl)
{
	return reinterpret_cast<SListDetail*>(impl);
}
}

SListImpl::SListImpl()
{
	getDetail(this)->head = NULL;

#if defined(USE_MUTEX)
	pthread_mutex_init(&getDetail(this)->lock, NULL);
#else
	getDetail(this)->lock = 0; // 0 == unlocked
#endif
}

SListImpl::~SListImpl()
{
#if defined(USE_MUTEX)
	pthread_mutex_destroy(&getDetail(this)->lock);
#endif
}

void SListImpl::push(SListEntry* entry)
{
	ScopedLock lock(getDetail(this)->lock);
	entry->mNext = getDetail(this)->head;
	getDetail(this)->head = entry;
}

SListEntry* SListImpl::pop()
{
	ScopedLock lock(getDetail(this)->lock);
	SListEntry* result = getDetail(this)->head;
	if(result != NULL)
		getDetail(this)->head = result->mNext;
	return result;
}

SListEntry* SListImpl::flush()
{
	ScopedLock lock(getDetail(this)->lock);
	SListEntry* result = getDetail(this)->head;
	getDetail(this)->head = NULL;
	return result;
}

static const uint32_t gSize = sizeof(SListDetail);

const uint32_t& SListImpl::getSize()
{
	return gSize;
}

} // namespace shdfnd
} // namespace physx
