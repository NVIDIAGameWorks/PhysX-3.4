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

#ifdef _M_ARM
#include <arm_neon.h>
#endif

namespace nvidia
{
namespace cloth
{

uint32_t findBitSet(uint32_t mask)
{
#ifdef _M_ARM
	__n64 t = { mask };
	return 31 - (vclz_u32(t)).n64_u32[0];
#else
	return 31 - __builtin_clz(mask);
#endif
}

Simd4i intFloor(const Simd4f& v)
{
	int32x4_t neg = vreinterpretq_s32_u32(vshrq_n_u32(v.u4, 31));
	return vsubq_s32(vcvtq_s32_f32(v.f4), neg);
}

Simd4i horizontalOr(Simd4i mask)
{
	using namespace simdi;
	uint32x2_t hi = vget_high_u32(mask.u4);
	uint32x2_t lo = vget_low_u32(mask.u4);
	uint32x2_t tmp = vorr_u32(lo, hi);
	uint32x2_t rev = vrev64_u32(tmp);
	uint32x2_t res = vorr_u32(tmp, rev);
	return vcombine_u32(res, res);
}

Gather<Simd4i>::Gather(const Simd4i& index)
{
#ifdef __arm64__
	using namespace simdi;
	PX_ALIGN(16, uint8x8x2_t) byteIndex = reinterpret_cast<const uint8x8x2_t&>(sPack);
	uint8x16_t lohiIndex = reinterpret_cast<const uint8x16_t&>(index);
	byteIndex.val[0] = vtbl1q_u8(lohiIndex, byteIndex.val[0]);
	byteIndex.val[1] = vtbl1q_u8(lohiIndex, byteIndex.val[1]);
	mPermute = vshlq_n_u32(reinterpret_cast<const uint32x4_t&>(byteIndex), 2);
	mPermute = mPermute | sOffset | vcgtq_u32(index.u4, sMask.u4);
#else
	using namespace simdi;
	PX_ALIGN(16, uint8x8x2_t) byteIndex = reinterpret_cast<const uint8x8x2_t&>(sPack);
	uint8x8x2_t lohiIndex = reinterpret_cast<const uint8x8x2_t&>(index);
	byteIndex.val[0] = vtbl2_u8(lohiIndex, byteIndex.val[0]);
	byteIndex.val[1] = vtbl2_u8(lohiIndex, byteIndex.val[1]);
	mPermute = vshlq_n_u32(reinterpret_cast<const uint32x4_t&>(byteIndex), 2);
	mPermute = mPermute | sOffset | vcgtq_u32(index.u4, sMask.u4);
#endif
}

Simd4i Gather<Simd4i>::operator()(const Simd4i* ptr) const
{
#ifdef __arm64__
	PX_ALIGN(16, uint8x8x2_t) result = reinterpret_cast<const uint8x8x2_t&>(mPermute);
	const uint8x16x2_t* table = reinterpret_cast<const uint8x16x2_t*>(ptr);
	result.val[0] = vtbl2q_u8(*table, result.val[0]);
	result.val[1] = vtbl2q_u8(*table, result.val[1]);
	return reinterpret_cast<const Simd4i&>(result);
#else
	PX_ALIGN(16, uint8x8x2_t) result = reinterpret_cast<const uint8x8x2_t&>(mPermute);
	const uint8x8x4_t* table = reinterpret_cast<const uint8x8x4_t*>(ptr);
	result.val[0] = vtbl4_u8(*table, result.val[0]);
	result.val[1] = vtbl4_u8(*table, result.val[1]);
	return reinterpret_cast<const Simd4i&>(result);
#endif
}

} // namespace cloth
} // namespace physx
