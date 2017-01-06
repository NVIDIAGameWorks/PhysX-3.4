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

#pragma once

#include <PxAssert.h>

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
