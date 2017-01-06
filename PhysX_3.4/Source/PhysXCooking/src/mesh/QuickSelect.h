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

#ifndef QUICKSELECT_H
#define QUICKSELECT_H

#include "foundation/PxSimpleTypes.h"

// Google "wikipedia QuickSelect" for algorithm explanation
namespace physx { namespace quickSelect {


	#define SWAP32(x, y) { PxU32 tmp = y; y = x; x = tmp; }

	// left is the index of the leftmost element of the subarray
	// right is the index of the rightmost element of the subarray (inclusive)
	// number of elements in subarray = right-left+1
	template<typename LtEq>
	PxU32 partition(PxU32* PX_RESTRICT a, PxU32 left, PxU32 right, PxU32 pivotIndex, const LtEq& cmpLtEq)
	{
		PX_ASSERT(pivotIndex >= left && pivotIndex <= right);
		PxU32 pivotValue = a[pivotIndex];
		SWAP32(a[pivotIndex], a[right]) // Move pivot to end
		PxU32 storeIndex = left;
		for (PxU32 i = left; i < right; i++)  // left <= i < right
			if (cmpLtEq(a[i], pivotValue))
			{
				SWAP32(a[i], a[storeIndex]);
				storeIndex++;
			}
		SWAP32(a[storeIndex], a[right]); // Move pivot to its final place
		for (PxU32 i = left; i < storeIndex; i++)
			PX_ASSERT(cmpLtEq(a[i], a[storeIndex]));
		for (PxU32 i = storeIndex+1; i <= right; i++)
			PX_ASSERT(cmpLtEq(a[storeIndex], a[i]));
		return storeIndex;
	}

	// left is the index of the leftmost element of the subarray
	// right is the index of the rightmost element of the subarray (inclusive)
	// number of elements in subarray = right-left+1
	// recursive version
	template<typename LtEq>
	void quickFindFirstK(PxU32* PX_RESTRICT a, PxU32 left, PxU32 right, PxU32 k, const LtEq& cmpLtEq)
	{
		PX_ASSERT(k <= right-left+1);
		if (right > left)
		{
			// select pivotIndex between left and right
			PxU32 pivotIndex = (left + right) >> 1;
			PxU32 pivotNewIndex = partition(a, left, right, pivotIndex, cmpLtEq);
			// now all elements to the left of pivotNewIndex are < old value of a[pivotIndex] (bottom half values)
			if (pivotNewIndex > left + k) // new condition
				quickFindFirstK(a, left, pivotNewIndex-1, k, cmpLtEq);
			if (pivotNewIndex < left + k)
				quickFindFirstK(a, pivotNewIndex+1, right, k+left-pivotNewIndex-1, cmpLtEq);
		}
	}

	// non-recursive version
	template<typename LtEq>
	void quickSelectFirstK(PxU32* PX_RESTRICT a, PxU32 left, PxU32 right, PxU32 k, const LtEq& cmpLtEq)
	{
		PX_ASSERT(k <= right-left+1);
		for (;;)
		{
			PxU32 pivotIndex = (left+right) >> 1;
			PxU32 pivotNewIndex = partition(a, left, right, pivotIndex, cmpLtEq);
			PxU32 pivotDist = pivotNewIndex - left + 1;
			if (pivotDist == k)
				return;
			else if (k < pivotDist)
			{
				PX_ASSERT(pivotNewIndex > 0);
				right = pivotNewIndex - 1;
			}
			else
			{
				k = k - pivotDist;
				left = pivotNewIndex+1;
			}
		}
	}

} }  // namespace quickSelect, physx

#endif

