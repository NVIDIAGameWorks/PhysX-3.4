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

#pragma once

#include "foundation/PxAssert.h"

#if PX_LINUX_FAMILY
#include <stdint.h> // intptr_t
#endif

template <size_t align>
class StackAllocator
{
	typedef unsigned char byte;

	// todo: switch to offsets so size is consistent on x64
	// mSize is just for book keeping so could be 4 bytes
	struct Header
	{
		Header* mPrev;
		size_t mSize : 31;
		size_t mFree : 1;
	};

	StackAllocator(const StackAllocator&);
	StackAllocator& operator=(const StackAllocator&);

  public:
	StackAllocator(void* buffer, size_t bufferSize)
	: mBuffer(reinterpret_cast<byte*>(buffer)), mBufferSize(bufferSize), mFreeStart(mBuffer), mTop(0)
	{
	}

	~StackAllocator()
	{
		PX_ASSERT(userBytes() == 0);
	}

	void* allocate(size_t numBytes)
	{
		// this is non-standard
		if(!numBytes)
			return 0;

		uintptr_t unalignedStart = uintptr_t(mFreeStart) + sizeof(Header);

		byte* allocStart = reinterpret_cast<byte*>((unalignedStart + (align - 1)) & ~(align - 1));
		byte* allocEnd = allocStart + numBytes;

		// ensure there is space for the alloc
		PX_ASSERT(allocEnd <= mBuffer + mBufferSize);

		Header* h = getHeader(allocStart);
		h->mPrev = mTop;
		h->mSize = numBytes;
		h->mFree = false;

		mTop = h;
		mFreeStart = allocEnd;

		return allocStart;
	}

	void deallocate(void* p)
	{
		if(!p)
			return;

		Header* h = getHeader(p);
		h->mFree = true;

		// unwind the stack to the next live alloc
		while(mTop && mTop->mFree)
		{
			mFreeStart = reinterpret_cast<byte*>(mTop);
			mTop = mTop->mPrev;
		}
	}

  private:
	// return the header for an allocation
	inline Header* getHeader(void* p) const
	{
		PX_ASSERT((reinterpret_cast<uintptr_t>(p) & (align - 1)) == 0);
		PX_ASSERT(reinterpret_cast<byte*>(p) >= mBuffer + sizeof(Header));
		PX_ASSERT(reinterpret_cast<byte*>(p) < mBuffer + mBufferSize);

		return reinterpret_cast<Header*>(p) - 1;
	}

  public:
	// total user-allocated bytes not including any overhead
	size_t userBytes() const
	{
		size_t total = 0;
		Header* iter = mTop;
		while(iter)
		{
			total += iter->mSize;
			iter = iter->mPrev;
		}

		return total;
	}

	// total user-allocated bytes + overhead
	size_t totalUsedBytes() const
	{
		return mFreeStart - mBuffer;
	}

	size_t remainingBytes() const
	{
		return mBufferSize - totalUsedBytes();
	}

	size_t wastedBytes() const
	{
		return totalUsedBytes() - userBytes();
	}

  private:
	byte* const mBuffer;
	const size_t mBufferSize;

	byte* mFreeStart; // start of free space
	Header* mTop;     // top allocation header
};
