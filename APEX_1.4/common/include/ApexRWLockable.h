/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef APEX_RW_LOCKABLE_H
#define APEX_RW_LOCKABLE_H

#include "RWLockable.h"
#include "PsThread.h"
#include "PsMutex.h"
#include "PsHashMap.h"

namespace nvidia
{
namespace apex
{

struct ThreadReadWriteCount 
{
	ThreadReadWriteCount() : value(0) {}
	union {
		struct {
			uint8_t readDepth;			// depth of re-entrant reads
			uint8_t writeDepth;		// depth of re-entrant writes 
			uint8_t readLockDepth;		// depth of read-locks
			uint8_t writeLockDepth;	// depth of write-locks
		} counters;
		uint32_t value;
	};
};

class ApexRWLockable : public RWLockable
{
public:
	ApexRWLockable();
	virtual ~ApexRWLockable();

	virtual void acquireReadLock(const char *fileName, const uint32_t lineno) const;
	virtual void acquireWriteLock(const char *fileName, const uint32_t lineno)const;
	virtual void releaseReadLock(void) const;
	virtual void releaseWriteLock(void) const;
	virtual uint32_t getReadWriteErrorCount() const;
	bool startWrite(bool allowReentry);
	void stopWrite(bool allowReentry);
	nvidia::Thread::Id getCurrentWriter() const;

	bool startRead() const;
	void stopRead() const;

	void setEnabled(bool);
	bool isEnabled() const;
private:
	bool											mEnabled;
	mutable volatile nvidia::Thread::Id		mCurrentWriter;
	mutable nvidia::ReadWriteLock			mRWLock;
	volatile int32_t									mConcurrentWriteCount;
	mutable volatile int32_t							mConcurrentReadCount;
	mutable volatile int32_t							mConcurrentErrorCount;
	nvidia::Mutex							mDataLock;
	typedef nvidia::HashMap<nvidia::ThreadImpl::Id, ThreadReadWriteCount> DepthsHashMap_t;
	mutable DepthsHashMap_t							mData;
};

#define APEX_RW_LOCKABLE_BOILERPLATE								\
	virtual void acquireReadLock(const char *fileName, const uint32_t lineno) const \
	{																\
		ApexRWLockable::acquireReadLock(fileName, lineno);					\
	}																\
	virtual void acquireWriteLock(const char *fileName, const uint32_t lineno) const\
	{																\
		ApexRWLockable::acquireWriteLock(fileName, lineno);			\
	}																\
	virtual void releaseReadLock(void)									const\
	{																\
		ApexRWLockable::releaseReadLock();								\
	}																\
	virtual void releaseWriteLock(void)									const\
	{																\
		ApexRWLockable::releaseWriteLock();								\
	}																\
	virtual uint32_t getReadWriteErrorCount() const				\
	{																\
		return ApexRWLockable::getReadWriteErrorCount();			\
	}																\
	bool startWrite(bool allowReentry)								\
	{																\
		return ApexRWLockable::startWrite(allowReentry);			\
	}																\
	void stopWrite(bool allowReentry)								\
	{																\
		ApexRWLockable::stopWrite(allowReentry);					\
	}																\
	bool startRead() const											\
	{																\
		return ApexRWLockable::startRead();							\
	}																\
	void stopRead() const											\
	{																\
		ApexRWLockable::stopRead();									\
	}																\

class ApexRWLockableScopedDisable
{
public:
	ApexRWLockableScopedDisable(RWLockable*);
	~ApexRWLockableScopedDisable();
private:
	ApexRWLockableScopedDisable(const ApexRWLockableScopedDisable&);
	ApexRWLockableScopedDisable& operator=(const ApexRWLockableScopedDisable&);

	ApexRWLockable* mLockable;
};

#define APEX_RW_LOCKABLE_SCOPED_DISABLE(lockable) ApexRWLockableScopedDisable __temporaryDisable(lockable);

}
}

#endif // APEX_RW_LOCKABLE_H
