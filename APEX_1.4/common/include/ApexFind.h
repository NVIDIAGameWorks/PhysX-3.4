/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef APEX_FIND_H
#define APEX_FIND_H

namespace nvidia
{
namespace apex
{
	// binary search
	template<class Sortable>
	int32_t ApexFind(const Sortable* buffer, uint32_t numEntries, const Sortable& element, int (*compare)(const void*, const void*))
	{

#if PX_CHECKED
		if (numEntries > 0)
		{
			for (uint32_t i = 1; i < numEntries; ++i)
			{
				PX_ASSERT(compare(buffer + i - 1, buffer + i) <= 0);
			}
		}
#endif

		int32_t curMin = 0;
		int32_t curMax = (int32_t)numEntries;
		int32_t testIndex = 0;

		while (curMin < curMax)
		{
			testIndex = (curMin + curMax) / 2;
			int32_t compResult = compare(&element, buffer+testIndex);
			if (compResult < 0)
			{
				curMax = testIndex;
			}
			else if (compResult > 0)
			{
				curMin = testIndex;
			}
			else
			{
				return testIndex;
			}

		}

		return -1;
	}

} // namespace apex
} // namespace nvidia

#endif // APEX_FIND_H
