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

#pragma once

#include "Simd.h"

// platform specific helpers

namespace physx
{
namespace cloth
{

inline uint32_t findBitSet(uint32_t mask);

// intFloor(-1.0f) returns -2 on SSE and NEON!
inline Simd4i intFloor(const Simd4f& v);

inline Simd4i horizontalOr(const Simd4i& mask);

template <typename>
struct Gather;

#if NV_SIMD_SIMD
template <>
struct Gather<Simd4i>
{
	inline Gather(const Simd4i& index);
	inline Simd4i operator()(const Simd4i*) const;

#if NV_SIMD_SSE2
	Simd4i mSelectQ, mSelectD, mSelectW;
	static const Simd4i sIntSignBit;
	static const Simd4i sSignedMask;
#elif NV_SIMD_NEON
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
} // namespace physx

#if NV_SIMD_SSE2
#include "sse2/SwCollisionHelpers.h"
#elif NV_SIMD_NEON
#include "neon/SwCollisionHelpers.h"
#endif

#if NV_SIMD_SCALAR
#include "scalar/SwCollisionHelpers.h"
#endif
