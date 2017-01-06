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

#ifndef PX_FOUNDATION_PSFOUNDATION_H
#define PX_FOUNDATION_PSFOUNDATION_H

#include "foundation/PxFoundation.h"
#include "foundation/PxErrors.h"
#include "foundation/PxProfiler.h"

#include "PsBroadcast.h"
#include "PsAllocator.h"
#include "PsTempAllocator.h"
#include "PsMutex.h"
#include "PsHashMap.h"
#include "PsUserAllocated.h"

#include <stdarg.h>

namespace physx
{
namespace shdfnd
{

#if PX_VC
#pragma warning(push)
#pragma warning(disable : 4251) // class needs to have dll-interface to be used by clients of class
#endif

class PX_FOUNDATION_API Foundation : public PxFoundation, public UserAllocated
{
	PX_NOCOPY(Foundation)

  public:
	typedef MutexT<Allocator> Mutex;

	typedef HashMap<const NamedAllocator*, const char*, Hash<const NamedAllocator*>, NonTrackingAllocator> AllocNameMap;
	typedef Array<TempAllocatorChunk*, Allocator> AllocFreeTable;

  public:
	// factory
	// note, you MUST eventually call release if createInstance returned true!
	static Foundation* createInstance(PxU32 version, PxErrorCallback& errc, PxAllocatorCallback& alloc);
	static Foundation& getInstance();
	void release();
	static void incRefCount(); // this call requires a foundation object to exist already
	static void decRefCount(); // this call requires a foundation object to exist already

	// Begin Errors
	virtual PxErrorCallback& getErrorCallback()
	{
		return mErrorCallback;
	} // Return the user's error callback
	PxErrorCallback& getInternalErrorCallback()
	{
		return mBroadcastingError;
	} // Return the broadcasting error callback

	void registerErrorCallback(PxErrorCallback& listener);
	void deregisterErrorCallback(PxErrorCallback& listener);

	virtual void setErrorLevel(PxErrorCode::Enum mask)
	{
		mErrorMask = mask;
	}
	virtual PxErrorCode::Enum getErrorLevel() const
	{
		return mErrorMask;
	}

	void error(PxErrorCode::Enum, const char* file, int line, const char* messageFmt, ...); // Report errors with the
	                                                                                        // broadcasting
	void errorImpl(PxErrorCode::Enum, const char* file, int line, const char* messageFmt, va_list); // error callback
	static PxU32 getWarnOnceTimestamp();

	// End errors

	// Begin Allocations
	virtual PxAllocatorCallback& getAllocatorCallback()
	{
		return mAllocatorCallback;
	} // Return the user's allocator callback
	PxAllocatorCallback& getAllocator()
	{
		return mBroadcastingAllocator;
	} // Return the broadcasting allocator

	void registerAllocationListener(physx::shdfnd::AllocationListener& listener);
	void deregisterAllocationListener(physx::shdfnd::AllocationListener& listener);

	virtual bool getReportAllocationNames() const
	{
		return mReportAllocationNames;
	}
	virtual void setReportAllocationNames(bool value)
	{
		mReportAllocationNames = value;
	}

	PX_INLINE AllocNameMap& getNamedAllocMap()
	{
		return mNamedAllocMap;
	}
	PX_INLINE Mutex& getNamedAllocMutex()
	{
		return mNamedAllocMutex;
	}

	PX_INLINE AllocFreeTable& getTempAllocFreeTable()
	{
		return mTempAllocFreeTable;
	}
	PX_INLINE Mutex& getTempAllocMutex()
	{
		return mTempAllocMutex;
	}
	// End allocations

  private:
	static void destroyInstance();

	Foundation(PxErrorCallback& errc, PxAllocatorCallback& alloc);
	~Foundation();

	// init order is tricky here: the mutexes require the allocator, the allocator may require the error stream
	PxAllocatorCallback& mAllocatorCallback;
	PxErrorCallback& mErrorCallback;

	BroadcastingAllocator mBroadcastingAllocator;
	BroadcastingErrorCallback mBroadcastingError;

	bool mReportAllocationNames;

	PxErrorCode::Enum mErrorMask;
	Mutex mErrorMutex;

	AllocNameMap mNamedAllocMap;
	Mutex mNamedAllocMutex;

	AllocFreeTable mTempAllocFreeTable;
	Mutex mTempAllocMutex;

	Mutex mListenerMutex;

	static Foundation* mInstance;
	static PxU32 mRefCount;
	static PxU32 mWarnOnceTimestap;
};
#if PX_VC
#pragma warning(pop)
#endif

PX_INLINE Foundation& getFoundation()
{
	return Foundation::getInstance();
}

} // namespace shdfnd
} // namespace physx

// shortcut macros:
// usage: Foundation::error(PX_WARN, "static friction %f is is lower than dynamic friction %d", sfr, dfr);
#define PX_WARN ::physx::PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__
#define PX_INFO ::physx::PxErrorCode::eDEBUG_INFO, __FILE__, __LINE__

#if PX_DEBUG || PX_CHECKED
#define PX_WARN_ONCE(string)                                                                                           \
	{                                                                                                                  \
		static PxU32 timestamp = 0;                                                                                    \
		if(timestamp != Ps::getFoundation().getWarnOnceTimestamp())                                                    \
		{                                                                                                              \
			timestamp = Ps::getFoundation().getWarnOnceTimestamp();                                                    \
			Ps::getFoundation().error(PX_WARN, string);                                                                \
		}                                                                                                              \
	\
}
#define PX_WARN_ONCE_IF(condition, string)                                                                             \
	{                                                                                                                  \
		if(condition)                                                                                                  \
		{                                                                                                              \
			PX_WARN_ONCE(string)                                                                                       \
		}                                                                                                              \
	\
}
#else
#define PX_WARN_ONCE(string) ((void)0)
#define PX_WARN_ONCE_IF(condition, string) ((void)0)
#endif

#endif
