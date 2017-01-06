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

#ifndef PSFOUNDATION_PSSORT_H
#define PSFOUNDATION_PSSORT_H

/** \addtogroup foundation
@{
*/

#include "PsSortInternals.h"
#include "PsAlloca.h"

#define PX_SORT_PARANOIA PX_DEBUG

/**
\brief Sorts an array of objects in ascending order, assuming
that the predicate implements the < operator:

\see Less, Greater
*/

#if PX_VC
#pragma warning(push)
#pragma warning(disable : 4706) // disable the warning that we did an assignment within a conditional expression, as
// this was intentional.
#endif

namespace physx
{
namespace shdfnd
{
template <class T, class Predicate, class Allocator>
void sort(T* elements, uint32_t count, const Predicate& compare, const Allocator& inAllocator,
          const uint32_t initialStackSize = 32)
{
	static const uint32_t SMALL_SORT_CUTOFF = 5; // must be >= 3 since we need 3 for median

	PX_ALLOCA(stackMem, int32_t, initialStackSize);
	internal::Stack<Allocator> stack(stackMem, initialStackSize, inAllocator);

	int32_t first = 0, last = int32_t(count - 1);
	if(last > first)
	{
		for(;;)
		{
			while(last > first)
			{
				PX_ASSERT(first >= 0 && last < int32_t(count));
				if(uint32_t(last - first) < SMALL_SORT_CUTOFF)
				{
					internal::smallSort(elements, first, last, compare);
					break;
				}
				else
				{
					const int32_t partIndex = internal::partition(elements, first, last, compare);

					// push smaller sublist to minimize stack usage
					if((partIndex - first) < (last - partIndex))
					{
						stack.push(first, partIndex - 1);
						first = partIndex + 1;
					}
					else
					{
						stack.push(partIndex + 1, last);
						last = partIndex - 1;
					}
				}
			}

			if(stack.empty())
				break;

			stack.pop(first, last);
		}
	}
#if PX_SORT_PARANOIA
	for(uint32_t i = 1; i < count; i++)
		PX_ASSERT(!compare(elements[i], elements[i - 1]));
#endif
}

template <class T, class Predicate>
void sort(T* elements, uint32_t count, const Predicate& compare)
{
	sort(elements, count, compare, typename shdfnd::AllocatorTraits<T>::Type());
}

template <class T>
void sort(T* elements, uint32_t count)
{
	sort(elements, count, shdfnd::Less<T>(), typename shdfnd::AllocatorTraits<T>::Type());
}

} // namespace shdfnd
} // namespace physx

#if PX_VC
#pragma warning(pop)
#endif

#endif // #ifndef PSFOUNDATION_PSSORT_H
