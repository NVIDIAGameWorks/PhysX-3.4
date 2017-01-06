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

#include "ApexRWLockable.h"
#include "PsThread.h"
#include "PsAtomic.h"
#include "ApexSDKIntl.h"

namespace nvidia
{
namespace apex
{

ApexRWLockable::ApexRWLockable()
	: 
	mEnabled (true)
	, mCurrentWriter(0)
	, mRWLock()
	, mConcurrentWriteCount	(0)
	, mConcurrentReadCount	(0)
	, mConcurrentErrorCount	(0)
{
}

ApexRWLockable::~ApexRWLockable()
{
}

void ApexRWLockable::setEnabled(bool e)
{
	mEnabled = e;
}

bool ApexRWLockable::isEnabled() const
{
	return mEnabled;
}

void ApexRWLockable::acquireReadLock(const char*, uint32_t) const
{
	mRWLock.lockReader();
}

void ApexRWLockable::acquireWriteLock(const char*, uint32_t) const
{
	mRWLock.lockWriter();
	mCurrentWriter = Thread::getId();
}

void ApexRWLockable::releaseReadLock(void) const
{
	mRWLock.unlockReader();
}

void ApexRWLockable::releaseWriteLock(void) const
{
	mCurrentWriter = 0;
	mRWLock.unlockWriter();
}

bool ApexRWLockable::startWrite(bool allowReentry) 
{
	PX_COMPILE_TIME_ASSERT(sizeof(ThreadReadWriteCount) == 4);
	mDataLock.lock();
	bool error = false;

	ThreadReadWriteCount& rwc = mData[Thread::getId()];
	// check that we are the only thread reading (this allows read->write order on a single thread)
	error |= mConcurrentReadCount != rwc.counters.readDepth;

	// check no other threads are writing 
	error |= mConcurrentWriteCount != rwc.counters.writeDepth;

	// increment shared write counter
	shdfnd::atomicIncrement(&mConcurrentWriteCount);

	// in the normal case (re-entry is allowed) then we simply increment
	// the writeDepth by 1, otherwise (re-entry is not allowed) increment
	// by 2 to force subsequent writes to fail by creating a mismatch between
	// the concurrent write counter and the local counter, any value > 1 will do
	if (allowReentry)
		rwc.counters.writeDepth++;
	else
		rwc.counters.writeDepth += 2;
	mDataLock.unlock();

	if (error)
		shdfnd::atomicIncrement(&mConcurrentErrorCount);

	return !error;
}

void ApexRWLockable::stopWrite(bool allowReentry)
{
	shdfnd::atomicDecrement(&mConcurrentWriteCount);

	// decrement depth of writes for this thread
	mDataLock.lock();
	nvidia::ThreadImpl::Id id = nvidia::Thread::getId();

	// see comment in startWrite()
	if (allowReentry)
		mData[id].counters.writeDepth--;
	else
		mData[id].counters.writeDepth -= 2;

	mDataLock.unlock();
}

nvidia::Thread::Id ApexRWLockable::getCurrentWriter() const
{
	return mCurrentWriter;
}

bool ApexRWLockable::startRead() const
{
	shdfnd::atomicIncrement(&mConcurrentReadCount);

	mDataLock.lock();
	nvidia::ThreadImpl::Id id = nvidia::Thread::getId();
	ThreadReadWriteCount& rwc = mData[id];
	rwc.counters.readDepth++;
	bool success = (rwc.counters.writeDepth > 0 || mConcurrentWriteCount == 0); 
	mDataLock.unlock();

	if (!success)
		shdfnd::atomicIncrement(&mConcurrentErrorCount);

	return success;
}

void ApexRWLockable::stopRead() const
{
	shdfnd::atomicDecrement(&mConcurrentReadCount);
	mDataLock.lock();
	nvidia::ThreadImpl::Id id = nvidia::Thread::getId();
	mData[id].counters.readDepth--;
	mDataLock.unlock();
}

uint32_t ApexRWLockable::getReadWriteErrorCount() const
{
	return static_cast<uint32_t>(mConcurrentErrorCount);
}

ApexRWLockableScopedDisable::ApexRWLockableScopedDisable(RWLockable* rw) : mLockable(static_cast<ApexRWLockable*>(rw))
{
	mLockable->setEnabled(false);
}

ApexRWLockableScopedDisable::~ApexRWLockableScopedDisable()
{
	mLockable->setEnabled(true);
}

ApexRWLockableScopedDisable::ApexRWLockableScopedDisable(const ApexRWLockableScopedDisable& o) : mLockable(o.mLockable)
{
}

ApexRWLockableScopedDisable& ApexRWLockableScopedDisable::operator=(const ApexRWLockableScopedDisable&)
{
	return *this;
}

}
}