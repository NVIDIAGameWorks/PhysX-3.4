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


#ifndef PX_PHYSICS_NP_PTRTABLESTORAGEMANAGER_H
#define PX_PHYSICS_NP_PTRTABLESTORAGEMANAGER_H

#include "CmPhysXCommon.h"
#include "PsMutex.h"
#include "PsUserAllocated.h"
#include "CmPtrTable.h"
#include "PsBitUtils.h"

namespace physx
{
class NpPtrTableStorageManager : public Cm::PtrTableStorageManager, public Ps::UserAllocated
{
	PX_NOCOPY(NpPtrTableStorageManager)

public:

	NpPtrTableStorageManager() {}
	~NpPtrTableStorageManager() {}

	void**	allocate(PxU32 capacity)
	{
		PX_ASSERT(Ps::isPowerOfTwo(capacity));

		Ps::Mutex::ScopedLock lock(mMutex);

		return capacity<=4*sizeof(void*)  ? reinterpret_cast<void**>(mPool4.construct())
			 : capacity<=16*sizeof(void*) ? reinterpret_cast<void**>(mPool16.construct())
			 : capacity<=64*sizeof(void*) ? reinterpret_cast<void**>(mPool64.construct())
			 : reinterpret_cast<void**>(PX_ALLOC(capacity*sizeof(void*), "CmPtrTable pointer array"));							
	}

	void deallocate(void** addr, PxU32 capacity)
	{
		PX_ASSERT(Ps::isPowerOfTwo(capacity));

		Ps::Mutex::ScopedLock lock(mMutex);

		if(capacity<=4*sizeof(void*))			mPool4.destroy(reinterpret_cast< PtrBlock<4>*>(addr));
		else if(capacity<=16*sizeof(void*))		mPool16.destroy(reinterpret_cast< PtrBlock<16>*>(addr));
		else if(capacity<=64*sizeof(void*))		mPool64.destroy(reinterpret_cast< PtrBlock<64>*>(addr));
		else									PX_FREE(addr);
	}

	// originalCapacity is the only way we know which pool the alloc request belongs to,
	// so if those are no longer going to match, we need to realloc.

	bool canReuse(PxU32 originalCapacity, PxU32 newCapacity)
	{
		PX_ASSERT(Ps::isPowerOfTwo(originalCapacity));
		PX_ASSERT(Ps::isPowerOfTwo(newCapacity));

		return poolId(originalCapacity) == poolId(newCapacity) && newCapacity<=64;
	}

private:
	Ps::Mutex mMutex;

	int poolId(PxU32 size)
	{
		return size<=4	? 0
			 : size<=16 ? 1
			 : size<=64 ? 2
			 : 3;
	}

	template<int N> class PtrBlock { void* ptr[N]; };

	Ps::Pool2<PtrBlock<4>, 4096 >		mPool4; 
	Ps::Pool2<PtrBlock<16>, 4096 >		mPool16;
	Ps::Pool2<PtrBlock<64>, 4096 >		mPool64;
};

}
#endif
