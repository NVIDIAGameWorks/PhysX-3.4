/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef APEX_QUICK_SELECT_SMALLEST_K_H
#define APEX_QUICK_SELECT_SMALLEST_K_H

namespace nvidia
{
namespace apex
{
//A variant of quick sort to move the smallest k members of a sequence to its start.
//Does much less work than a full sort.

template<class Sortable, class Predicate>
PX_INLINE void ApexQuickSelectSmallestK(Sortable* start, Sortable* end, uint32_t k, const Predicate& p = Predicate())
{
	Sortable* origStart = start;
	Sortable* i;
	Sortable* j;
	Sortable m;

	for (;;)
	{
		i = start;
		j = end;
		m = *(i + ((j - i) >> 1));

		while (i <= j)
		{
			while (p(*i, m))
			{
				i++;
			}
			while (p(m, *j))
			{
				j--;
			}
			if (i <= j)
			{
				if (i != j)
				{
					nvidia::swap(*i, *j);
				}
				i++;
				j--;
			}
		}



		if (start < j
		        && k + origStart - 1 < j)	//we now have found the (j - start+1) smallest.  we need to continue sorting these only if k < (j - start+1)
			//if we sort this we definitely won't need to sort the right hand side.
		{
			end = j;
		}
		else if (i < end
		         && k + origStart > i)	//only continue sorting these if left side is not larger than k.
			//we do this instead of recursing
		{
			start = i;
		}
		else
		{
			return;
		}
	}
}

} // namespace apex
} // namespace nvidia

#endif // APEX_QUICK_SELECT_SMALLEST_K_H
