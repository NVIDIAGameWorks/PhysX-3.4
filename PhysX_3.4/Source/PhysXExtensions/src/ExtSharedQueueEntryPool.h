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


#ifndef PX_PHYSICS_EXTENSIONS_NP_SHARED_QUEUE_ENTRY_POOL_H
#define PX_PHYSICS_EXTENSIONS_NP_SHARED_QUEUE_ENTRY_POOL_H

#include "CmPhysXCommon.h"
#include "PsAllocator.h"
#include "PsArray.h"
#include "PsSList.h"

namespace physx
{
namespace Ext
{
	class SharedQueueEntry : public Ps::SListEntry
	{
	public:
		SharedQueueEntry(void* objectRef) : mObjectRef(objectRef), mPooledEntry(false) {}
		SharedQueueEntry() : mObjectRef(NULL), mPooledEntry(true) {}

	public:
		void* mObjectRef;
		bool mPooledEntry; // True if the entry was preallocated in a pool
	};

#if PX_VC
#pragma warning(push)
#pragma warning(disable:4324)	// Padding was added at the end of a structure because of a __declspec(align) value.
#endif							// Because of the SList member I assume*/

	template<class Alloc = typename Ps::AllocatorTraits<SharedQueueEntry>::Type >
	class SharedQueueEntryPool : private Alloc
	{
	public:
		SharedQueueEntryPool(PxU32 poolSize, const Alloc& alloc = Alloc(PX_DEBUG_EXP("SharedQueueEntryPool")));
		~SharedQueueEntryPool();

		SharedQueueEntry* getEntry(void* objectRef);
		void putEntry(SharedQueueEntry& entry);

	private:
		SharedQueueEntry*					mTaskEntryPool;
		Ps::SList							mTaskEntryPtrPool;
	};

#if PX_VC
#pragma warning(pop)
#endif

} // namespace Ext


template <class Alloc>
Ext::SharedQueueEntryPool<Alloc>::SharedQueueEntryPool(PxU32 poolSize, const Alloc& alloc)
	: Alloc(alloc)
{
	Ps::AlignedAllocator<PX_SLIST_ALIGNMENT, Alloc> alignedAlloc("SharedQueueEntryPool");

	mTaskEntryPool = poolSize ? reinterpret_cast<SharedQueueEntry*>(alignedAlloc.allocate(sizeof(SharedQueueEntry) * poolSize, __FILE__, __LINE__)) : NULL;

	if (mTaskEntryPool)
	{
		for(PxU32 i=0; i < poolSize; i++)
		{
			PX_ASSERT((size_t(&mTaskEntryPool[i]) & (PX_SLIST_ALIGNMENT-1)) == 0);  // The SList entry must be aligned according to PX_SLIST_ALIGNMENT

			PX_PLACEMENT_NEW(&mTaskEntryPool[i], SharedQueueEntry)();
			PX_ASSERT(mTaskEntryPool[i].mPooledEntry == true);
			mTaskEntryPtrPool.push(mTaskEntryPool[i]);
		}
	}
}


template <class Alloc>
Ext::SharedQueueEntryPool<Alloc>::~SharedQueueEntryPool()
{
	if (mTaskEntryPool)
	{
		Ps::AlignedAllocator<PX_SLIST_ALIGNMENT, Alloc> alignedAlloc("SharedQueueEntryPool");
		alignedAlloc.deallocate(mTaskEntryPool);
	}
}


template <class Alloc>
Ext::SharedQueueEntry* Ext::SharedQueueEntryPool<Alloc>::getEntry(void* objectRef)
{
	SharedQueueEntry* e = static_cast<SharedQueueEntry*>(mTaskEntryPtrPool.pop());
	if (e)
	{
		PX_ASSERT(e->mPooledEntry == true);
		e->mObjectRef = objectRef;
		return e;
	}
	else
	{
		Ps::AlignedAllocator<PX_SLIST_ALIGNMENT, Alloc> alignedAlloc;
		e = reinterpret_cast<SharedQueueEntry*>(alignedAlloc.allocate(sizeof(SharedQueueEntry), __FILE__, __LINE__));
		if (e)
		{
			PX_PLACEMENT_NEW(e, SharedQueueEntry)(objectRef);
			PX_ASSERT(e->mPooledEntry == false);
		}

		return e;
	}
}


template <class Alloc>
void Ext::SharedQueueEntryPool<Alloc>::putEntry(Ext::SharedQueueEntry& entry)
{
	if (entry.mPooledEntry)
	{
		entry.mObjectRef = NULL;
		mTaskEntryPtrPool.push(entry);
	}
	else
	{
		Ps::AlignedAllocator<PX_SLIST_ALIGNMENT, Alloc> alignedAlloc;
		alignedAlloc.deallocate(&entry);
	}
}

}

#endif
