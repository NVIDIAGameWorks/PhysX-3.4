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

#include "Simd4f.h"
#include <float.h>

namespace nvidia
{

namespace cloth
{

template <typename Simd4f>
struct BoundingBox
{
	Simd4f mLower;
	Simd4f mUpper;
};

template <typename Simd4f>
inline BoundingBox<Simd4f> loadBounds(const float* ptr)
{
	BoundingBox<Simd4f> result;
	result.mLower = load(ptr);
	result.mUpper = load(ptr + 3);
	return result;
}

template <typename Simd4f>
inline BoundingBox<Simd4f> emptyBounds()
{
	BoundingBox<Simd4f> result;

	result.mLower = simd4f(FLT_MAX);
	result.mUpper = -result.mLower;

	return result;
}

template <typename Simd4f>
inline BoundingBox<Simd4f> expandBounds(const BoundingBox<Simd4f>& bounds, const Simd4f* pIt, const Simd4f* pEnd)
{
	BoundingBox<Simd4f> result = bounds;
	for(; pIt != pEnd; ++pIt)
	{
		result.mLower = min(result.mLower, *pIt);
		result.mUpper = max(result.mUpper, *pIt);
	}
	return result;
}

template <typename Simd4f>
inline BoundingBox<Simd4f> expandBounds(const BoundingBox<Simd4f>& a, const BoundingBox<Simd4f>& b)
{
	BoundingBox<Simd4f> result;
	result.mLower = min(a.mLower, b.mLower);
	result.mUpper = max(a.mUpper, b.mUpper);
	return result;
}

template <typename Simd4f>
inline BoundingBox<Simd4f> intersectBounds(const BoundingBox<Simd4f>& a, const BoundingBox<Simd4f>& b)
{
	BoundingBox<Simd4f> result;
	result.mLower = max(a.mLower, b.mLower);
	result.mUpper = min(a.mUpper, b.mUpper);
	return result;
}

template <typename Simd4f>
inline bool isEmptyBounds(const BoundingBox<Simd4f>& a)
{
	return anyGreater(a.mLower, a.mUpper) != 0;
}
}
}
