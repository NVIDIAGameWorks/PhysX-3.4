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

#include "foundation/PxMath.h"

#include "PsFoundation.h"
#include "PsTempAllocator.h"
#include "PsArray.h"
#include "PsMutex.h"
#include "PsAtomic.h"
#include "PsIntrinsics.h"
#include "PsBitUtils.h"

#if PX_VC
#pragma warning(disable : 4706) // assignment within conditional expression
#endif

namespace physx
{
namespace shdfnd
{
namespace
{
typedef TempAllocatorChunk Chunk;
typedef Array<Chunk*, NonTrackingAllocator> AllocFreeTable;

PX_INLINE Foundation::AllocFreeTable& getFreeTable()
{
	return getFoundation().getTempAllocFreeTable();
}
PX_INLINE Foundation::Mutex& getMutex()
{
	return getFoundation().getTempAllocMutex();
}

const PxU32 sMinIndex = 8;  // 256B min
const PxU32 sMaxIndex = 17; // 128kB max
}

void* TempAllocator::allocate(size_t size, const char* filename, int line)
{
	if(!size)
		return 0;

	uint32_t index = PxMax(highestSetBit(uint32_t(size) + sizeof(Chunk) - 1), sMinIndex);

	Chunk* chunk = 0;
	if(index < sMaxIndex)
	{
		Foundation::Mutex::ScopedLock lock(getMutex());

		// find chunk up to 16x bigger than necessary
		Chunk** it = getFreeTable().begin() + index - sMinIndex;
		Chunk** end = PxMin(it + 3, getFreeTable().end());
		while(it < end && !(*it))
			++it;

		if(it < end)
		{
			// pop top off freelist
			chunk = *it;
			*it = chunk->mNext;
			index = uint32_t(it - getFreeTable().begin() + sMinIndex);
		}
		else
			// create new chunk
			chunk = reinterpret_cast<Chunk*>(NonTrackingAllocator().allocate(size_t(2 << index), filename, line));
	}
	else
	{
		// too big for temp allocation, forward to base allocator
		chunk = reinterpret_cast<Chunk*>(NonTrackingAllocator().allocate(size + sizeof(Chunk), filename, line));
	}

	chunk->mIndex = index;
	void* ret = chunk + 1;
	PX_ASSERT((size_t(ret) & 0xf) == 0); // SDK types require at minimum 16 byte allignment.
	return ret;
}

void TempAllocator::deallocate(void* ptr)
{
	if(!ptr)
		return;

	Chunk* chunk = reinterpret_cast<Chunk*>(ptr) - 1;
	uint32_t index = chunk->mIndex;

	if(index >= sMaxIndex)
		return NonTrackingAllocator().deallocate(chunk);

	Foundation::Mutex::ScopedLock lock(getMutex());

	index -= sMinIndex;
	if(getFreeTable().size() <= index)
		getFreeTable().resize(index + 1);

	chunk->mNext = getFreeTable()[index];
	getFreeTable()[index] = chunk;
}

} // namespace shdfnd
} // namespace physx
