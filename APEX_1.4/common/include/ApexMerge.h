/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef APEX_MERGE_H
#define APEX_MERGE_H

namespace nvidia
{
namespace apex
{
	// merge 2 increasingly sorted arrays
	// it's ok if one of the input buffers is also the results array
	template<class Sortable>
	bool ApexMerge(Sortable* bufferA, uint32_t numEntriesA, Sortable* bufferB, uint32_t numEntriesB, Sortable* resultBuffer, uint32_t numEntriesResult, int (*compare)(const void*, const void*))
	{
		if (numEntriesResult != numEntriesA + numEntriesB)
			return false;

#if PX_CHECKED
		if (numEntriesA > 0)
		{
			for (uint32_t i = 1; i < numEntriesA; ++i)
			{
				PX_ASSERT(compare(bufferA + i - 1, bufferA + i) <= 0);
			}
		}

		if (numEntriesB > 0)
		{
			for (uint32_t i = 1; i < numEntriesB; ++i)
			{
				PX_ASSERT(compare(bufferB + i - 1, bufferB + i) <= 0);
			}
		}
#endif

		int32_t iA = (int32_t)numEntriesA-1;
		int32_t iB = (int32_t)numEntriesB-1;
		uint32_t iResult = numEntriesA + numEntriesB - 1;

		while (iA >= 0 && iB >= 0)
		{
			if (compare(&bufferA[iA], &bufferB[iB]) > 0)
			{
				resultBuffer[iResult] = bufferA[iA--];
			}
			else
			{
				resultBuffer[iResult] = bufferB[iB--];
			}

			--iResult;
		}

		if (iA < 0)
		{
			if (resultBuffer != bufferB)
			{
				memcpy(resultBuffer, bufferB, (iB + 1) * sizeof(Sortable));
			}
		}
		else
		{
			if (resultBuffer != bufferA)
			{
				memcpy(resultBuffer, bufferA, (iA + 1) * sizeof(Sortable));
			}
		}

		return true;
	}

} // namespace apex
} // namespace nvidia

#endif // APEX_MERGE_H
