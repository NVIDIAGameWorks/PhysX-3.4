/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef USERALLOCATOR_H
#define USERALLOCATOR_H

#include "ApexDefs.h"
#include "PxAllocatorCallback.h"
#include "ApexUsingNamespace.h"

#pragma warning(push)
#pragma warning(disable:4512)

#if PX_CHECKED || defined(_DEBUG)
#if PX_WINDOWS_FAMILY
#define USE_MEM_TRACKER
#endif
#endif

namespace MEM_TRACKER
{
	class MemTracker;
};

/* User allocator for APEX and 3.0 PhysX SDK */
class UserPxAllocator : public physx::PxAllocatorCallback
{
public:
	UserPxAllocator(const char* context, const char* dllName, bool useTrackerIfSupported = true);
	virtual		   ~UserPxAllocator();

	uint32_t	getHandle(const char* name);


	void*			allocate(size_t size, const char* typeName, const char* filename, int line);
	void			deallocate(void* ptr);

	size_t			getAllocatedMemoryBytes()
	{
		return mMemoryAllocated;
	}

	static bool dumpMemoryLeaks(const char* filename);

private:
	bool				trackerEnabled() const { return mUseTracker && (NULL != mMemoryTracker); }

	const char*			mContext;
	size_t				mMemoryAllocated;
	const bool			mUseTracker;

	static MEM_TRACKER::MemTracker	*mMemoryTracker;
	static int gMemoryTrackerClients;

	// Poor man's memory leak check
	static unsigned int mNumAllocations;
	static unsigned int mNumFrees;
};

#pragma warning(pop)

#endif
