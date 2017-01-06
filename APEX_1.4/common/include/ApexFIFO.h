/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __APEX_FIFO_H__
#define __APEX_FIFO_H__

#include "Apex.h"
#include "PsUserAllocated.h"

namespace nvidia
{
namespace apex
{

template <typename T>
struct FIFOEntry
{
	T data;
	uint32_t next;
	bool isValidEntry;
};

template<typename T>
class ApexFIFO : public UserAllocated
{
public:
	ApexFIFO() : first((uint32_t) - 1), last((uint32_t) - 1), count(0) {}

	bool popFront(T& frontElement)
	{
		if (first == (uint32_t)-1)
		{
			return false;
		}

		PX_ASSERT(first < list.size());
		frontElement = list[first].data;

		if (first == last)
		{
			list.clear();
			first = (uint32_t) - 1;
			last = (uint32_t) - 1;
		}
		else
		{
			list[first].isValidEntry = false;

			if (list[last].next == (uint32_t)-1)
			{
				list[last].next = first;
			}
			first = list[first].next;
		}

		count--;
		return true;
	}


	void pushBack(const T& newElement)
	{
		if (list.size() == 0 || list[last].next == (uint32_t)-1)
		{
			FIFOEntry<T> newEntry;
			newEntry.data = newElement;
			newEntry.next = (uint32_t) - 1;
			newEntry.isValidEntry = true;
			list.pushBack(newEntry);

			if (first == (uint32_t) - 1)
			{
				PX_ASSERT(last == (uint32_t) - 1);
				first = list.size() - 1;
			}
			else
			{
				PX_ASSERT(last != (uint32_t) - 1);
				list[last].next = list.size() - 1;
			}

			last = list.size() - 1;
		}
		else
		{
			uint32_t freeIndex = list[last].next;
			PX_ASSERT(freeIndex < list.size());

			FIFOEntry<T>& freeEntry = list[freeIndex];
			freeEntry.data = newElement;
			freeEntry.isValidEntry = true;

			if (freeEntry.next == first)
			{
				freeEntry.next = (uint32_t) - 1;
			}

			last = freeIndex;
		}
		count++;
	}

	uint32_t size()
	{
		return count;
	}

	PX_INLINE void reserve(const uint32_t capacity)
	{
		list.reserve(capacity);
	}

	PX_INLINE uint32_t capacity() const
	{
		return list.capacity();
	}

private:
	uint32_t first;
	uint32_t last;
	uint32_t count;
	physx::Array<FIFOEntry<T> > list;
};

}
} // end namespace nvidia::apex

#endif