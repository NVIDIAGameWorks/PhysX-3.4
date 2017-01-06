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

#include "Simd4i.h"

// platform specific helpers

namespace nvidia
{
namespace cloth
{

inline uint32_t findBitSet(uint32_t mask);

// intFloor(-1.0f) returns -2 on SSE and NEON!
inline Simd4i intFloor(const Simd4f& v);

inline Simd4i horizontalOr(Simd4i mask);

template <typename>
struct Gather;

#if NVMATH_SIMD
template <>
struct Gather<Simd4i>
{
	inline Gather(const Simd4i& index);
	inline Simd4i operator()(const Simd4i*) const;

#if NVMATH_SSE2
	Simd4i mSelectQ, mSelectD, mSelectW;
	static const Simd4i sIntSignBit;
	static const Simd4i sSignedMask;
#elif NVMATH_NEON
	Simd4i mPermute;
	static const Simd4i sPack;
	static const Simd4i sOffset;
	static const Simd4i sShift;
	static const Simd4i sMask;
#endif
	Simd4i mOutOfRange;
};
#endif

} // namespace cloth
} // namespace nvidia

#if NVMATH_SSE2
#include "sse2/SwCollisionHelpers.h"
#elif NVMATH_NEON
#include "neon/SwCollisionHelpers.h"
#endif

#if NVMATH_SCALAR
#include "scalar/SwCollisionHelpers.h"
#endif
