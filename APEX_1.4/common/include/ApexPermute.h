/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef APEX_PERMUTE_H
#define APEX_PERMUTE_H

namespace nvidia
{
namespace apex
{

// permutationBuffer has to contain the indices that map from the new to the old index
template<class Sortable>
inline void ApexPermute(Sortable* sortBuffer, const uint32_t* permutationBuffer, uint32_t numElements, uint32_t numElementsPerPermutation = 1)
{
	nvidia::Array<Sortable> temp;
	temp.resize(numElementsPerPermutation);

	// TODO remove used buffer
	nvidia::Array<bool> used(numElements, false);

	for (uint32_t i = 0; i < numElements; i++)
	{
		//if (permutationBuffer[i] == (uint32_t)-1 || permutationBuffer[i] == i)
		if (used[i] || permutationBuffer[i] == i)
		{
			continue;
		}

		uint32_t dst = i;
		uint32_t src = permutationBuffer[i];
		for (uint32_t j = 0; j < numElementsPerPermutation; j++)
		{
			temp[j] = sortBuffer[numElementsPerPermutation * dst + j];
		}
		do
		{
			for (uint32_t j = 0; j < numElementsPerPermutation; j++)
			{
				sortBuffer[numElementsPerPermutation * dst + j] = sortBuffer[numElementsPerPermutation * src + j];
			}
			//permutationBuffer[dst] = (uint32_t)-1;
			used[dst] = true;
			dst = src;
			src = permutationBuffer[src];
			//} while (permutationBuffer[src] != (uint32_t)-1);
		}
		while (!used[src]);
		for (uint32_t j = 0; j < numElementsPerPermutation; j++)
		{
			sortBuffer[numElementsPerPermutation * dst + j] = temp[j];
		}
		//permutationBuffer[dst] = (uint32_t)-1;
		used[dst] = true;
	}
}

} // namespace apex
} // namespace nvidia

#endif // APEX_PERMUTE_H
