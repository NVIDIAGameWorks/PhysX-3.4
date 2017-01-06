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

#ifndef PSFOUNDATION_PSSORTINTERNALS_H
#define PSFOUNDATION_PSSORTINTERNALS_H

/** \addtogroup foundation
@{
*/

#include "foundation/PxAssert.h"
#include "foundation/PxIntrinsics.h"
#include "PsBasicTemplates.h"
#include "PsUserAllocated.h"

namespace physx
{
namespace shdfnd
{
namespace internal
{
template <class T, class Predicate>
PX_INLINE void median3(T* elements, int32_t first, int32_t last, Predicate& compare)
{
	/*
	This creates sentinels because we know there is an element at the start minimum(or equal)
	than the pivot and an element at the end greater(or equal) than the pivot. Plus the
	median of 3 reduces the chance of degenerate behavour.
	*/

	int32_t mid = (first + last) / 2;

	if(compare(elements[mid], elements[first]))
		swap(elements[first], elements[mid]);

	if(compare(elements[last], elements[first]))
		swap(elements[first], elements[last]);

	if(compare(elements[last], elements[mid]))
		swap(elements[mid], elements[last]);

	// keep the pivot at last-1
	swap(elements[mid], elements[last - 1]);
}

template <class T, class Predicate>
PX_INLINE int32_t partition(T* elements, int32_t first, int32_t last, Predicate& compare)
{
	median3(elements, first, last, compare);

	/*
	WARNING: using the line:

	T partValue = elements[last-1];

	and changing the scan loops to:

	while(comparator.greater(partValue, elements[++i]));
	while(comparator.greater(elements[--j], partValue);

	triggers a compiler optimizer bug on xenon where it stores a double to the stack for partValue
	then loads it as a single...:-(
	*/

	int32_t i = first;    // we know first is less than pivot(but i gets pre incremented)
	int32_t j = last - 1; // pivot is in last-1 (but j gets pre decremented)

	for(;;)
	{
		while(compare(elements[++i], elements[last - 1]))
			;
		while(compare(elements[last - 1], elements[--j]))
			;

		if(i >= j)
			break;

		PX_ASSERT(i <= last && j >= first);
		swap(elements[i], elements[j]);
	}
	// put the pivot in place

	PX_ASSERT(i <= last && first <= (last - 1));
	swap(elements[i], elements[last - 1]);

	return i;
}

template <class T, class Predicate>
PX_INLINE void smallSort(T* elements, int32_t first, int32_t last, Predicate& compare)
{
	// selection sort - could reduce to fsel on 360 with floats.

	for(int32_t i = first; i < last; i++)
	{
		int32_t m = i;
		for(int32_t j = i + 1; j <= last; j++)
			if(compare(elements[j], elements[m]))
				m = j;

		if(m != i)
			swap(elements[m], elements[i]);
	}
}

template <class Allocator>
class Stack
{
	Allocator mAllocator;
	uint32_t mSize, mCapacity;
	int32_t* mMemory;
	bool mRealloc;

  public:
	Stack(int32_t* memory, uint32_t capacity, const Allocator& inAllocator)
	: mAllocator(inAllocator), mSize(0), mCapacity(capacity), mMemory(memory), mRealloc(false)
	{
	}
	~Stack()
	{
		if(mRealloc)
			mAllocator.deallocate(mMemory);
	}

	void grow()
	{
		mCapacity *= 2;
		int32_t* newMem =
		    reinterpret_cast<int32_t*>(mAllocator.allocate(sizeof(int32_t) * mCapacity, __FILE__, __LINE__));
		intrinsics::memCopy(newMem, mMemory, mSize * sizeof(int32_t));
		if(mRealloc)
			mAllocator.deallocate(mMemory);
		mRealloc = true;
		mMemory = newMem;
	}

	PX_INLINE void push(int32_t start, int32_t end)
	{
		if(mSize >= mCapacity - 1)
			grow();
		mMemory[mSize++] = start;
		mMemory[mSize++] = end;
	}

	PX_INLINE void pop(int32_t& start, int32_t& end)
	{
		PX_ASSERT(!empty());
		end = mMemory[--mSize];
		start = mMemory[--mSize];
	}

	PX_INLINE bool empty()
	{
		return mSize == 0;
	}
};
} // namespace internal

} // namespace shdfnd
} // namespace physx

#endif // #ifndef PSFOUNDATION_PSSORTINTERNALS_H
