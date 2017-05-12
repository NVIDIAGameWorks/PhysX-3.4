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

NV_SIMD_NAMESPACE_BEGIN

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// factory implementation
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

Simd4iZeroFactory::operator Simd4i() const
{
	return vdupq_n_s32(0);
}

Simd4iScalarFactory::operator Simd4i() const
{
	return vdupq_n_s32(value);
}

Simd4iTupleFactory::operator Simd4i() const
{
	return reinterpret_cast<const Simd4i&>(tuple);
}

Simd4iLoadFactory::operator Simd4i() const
{
	return vld1q_s32(ptr);
}

Simd4iAlignedLoadFactory::operator Simd4i() const
{
	return vld1q_s32(ptr);
}

Simd4iOffsetLoadFactory::operator Simd4i() const
{
	return vld1q_s32(reinterpret_cast<const int*>(reinterpret_cast<const char*>(ptr) + offset));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// expression template
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <>
inline ComplementExpr<Simd4i>::operator Simd4i() const
{
	return vbicq_u32(vdupq_n_u32(0xffffffff), v.u4);
}

template <>
inline Simd4i operator&(const ComplementExpr<Simd4i>& complement, const Simd4i& v)
{
	return vbicq_u32(v.u4, complement.v.u4);
}

template <>
inline Simd4i operator&(const Simd4i& v, const ComplementExpr<Simd4i>& complement)
{
	return vbicq_u32(v.u4, complement.v.u4);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// operator implementations
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

Simd4i operator==(const Simd4i& v0, const Simd4i& v1)
{
	return vceqq_u32(v0.u4, v1.u4);
}

Simd4i operator<(const Simd4i& v0, const Simd4i& v1)
{
	return vcltq_s32(v0.i4, v1.i4);
}

Simd4i operator>(const Simd4i& v0, const Simd4i& v1)
{
	return vcgtq_s32(v0.i4, v1.i4);
}

ComplementExpr<Simd4i> operator~(const Simd4i& v)
{
	return ComplementExpr<Simd4i>(v);
}

Simd4i operator&(const Simd4i& v0, const Simd4i& v1)
{
	return vandq_u32(v0.u4, v1.u4);
}

Simd4i operator|(const Simd4i& v0, const Simd4i& v1)
{
	return vorrq_u32(v0.u4, v1.u4);
}

Simd4i operator^(const Simd4i& v0, const Simd4i& v1)
{
	return veorq_u32(v0.u4, v1.u4);
}

Simd4i operator<<(const Simd4i& v, int shift)
{
	return vshlq_u32(v.u4, vdupq_n_s32(shift));
}

Simd4i operator>>(const Simd4i& v, int shift)
{
	return vshlq_u32(v.u4, vdupq_n_s32(-shift));
}

Simd4i operator<<(const Simd4i& v, const Simd4i& shift)
{
	return vshlq_u32(v.u4, shift.i4);
}

Simd4i operator>>(const Simd4i& v, const Simd4i& shift)
{
	return vshlq_u32(v.u4, vnegq_s32(shift.i4));
}

Simd4i operator+(const Simd4i& v)
{
	return v;
}

Simd4i operator+(const Simd4i& v0, const Simd4i& v1)
{
	return vaddq_u32(v0.u4, v1.u4);
}

Simd4i operator-(const Simd4i& v)
{
	return vnegq_s32(v.i4);
}

Simd4i operator-(const Simd4i& v0, const Simd4i& v1)
{
	return vsubq_u32(v0.u4, v1.u4);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// function implementations
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

Simd4i simd4i(const Simd4f& v)
{
	return v.u4;
}

Simd4i truncate(const Simd4f& v)
{
	return vcvtq_s32_f32(v.f4);
}

int (&array(Simd4i& v))[4]
{
	return reinterpret_cast<int(&)[4]>(v);
}

const int (&array(const Simd4i& v))[4]
{
	return reinterpret_cast<const int(&)[4]>(v);
}

void store(int* ptr, const Simd4i& v)
{
	return vst1q_s32(ptr, v.i4);
}

void storeAligned(int* ptr, const Simd4i& v)
{
	vst1q_s32(ptr, v.i4);
}

void storeAligned(int* ptr, unsigned int offset, const Simd4i& v)
{
	return storeAligned(reinterpret_cast<int*>(reinterpret_cast<char*>(ptr) + offset), v);
}

template <size_t i>
Simd4i splat(Simd4i const& v)
{
	return vdupq_n_s32(array(v)[i]);
}

Simd4i select(Simd4i const& mask, Simd4i const& v0, Simd4i const& v1)
{
	return vbslq_u32(mask.u4, v0.u4, v1.u4);
}

int allEqual(const Simd4i& v0, const Simd4i& v1)
{
	return allTrue(operator==(v0, v1));
}

int allEqual(const Simd4i& v0, const Simd4i& v1, Simd4i& outMask)
{
	return allTrue(outMask = operator==(v0, v1));
}

int anyEqual(const Simd4i& v0, const Simd4i& v1)
{
	return anyTrue(operator==(v0, v1));
}

int anyEqual(const Simd4i& v0, const Simd4i& v1, Simd4i& outMask)
{
	return anyTrue(outMask = operator==(v0, v1));
}

int allGreater(const Simd4i& v0, const Simd4i& v1)
{
	return allTrue(operator>(v0, v1));
}

int allGreater(const Simd4i& v0, const Simd4i& v1, Simd4i& outMask)
{
	return allTrue(outMask = operator>(v0, v1));
}

int anyGreater(const Simd4i& v0, const Simd4i& v1)
{
	return anyTrue(operator>(v0, v1));
}

int anyGreater(const Simd4i& v0, const Simd4i& v1, Simd4i& outMask)
{
	return anyTrue(outMask = operator>(v0, v1));
}

int allTrue(const Simd4i& v)
{
#if NV_SIMD_INLINE_ASSEMBLER
	int result;
	asm volatile("vmovq q0, %q1 \n\t"
	             "vand.u32 d0, d0, d1 \n\t"
	             "vpmin.u32 d0, d0, d0 \n\t"
	             "vcmp.f32 s0, #0 \n\t"
	             "fmrx %0, fpscr"
	             : "=r"(result)
	             : "w"(v.u4)
	             : "q0");
	return result >> 28 & 0x1;
#else
	uint16x4_t hi = vget_high_u16(vreinterpretq_u16_u32(v.u4));
	uint16x4_t lo = vmovn_u32(v.u4);
	uint16x8_t combined = vcombine_u16(lo, hi);
	uint32x2_t reduced = vreinterpret_u32_u8(vmovn_u16(combined));
	return vget_lane_u32(reduced, 0) == 0xffffffff;
#endif
}

int anyTrue(const Simd4i& v)
{
#if NV_SIMD_INLINE_ASSEMBLER
	int result;
	asm volatile("vmovq q0, %q1 \n\t"
	             "vorr.u32 d0, d0, d1 \n\t"
	             "vpmax.u32 d0, d0, d0 \n\t"
	             "vcmp.f32 s0, #0 \n\t"
	             "fmrx %0, fpscr"
	             : "=r"(result)
	             : "w"(v.u4)
	             : "q0");
	return result >> 28 & 0x1;
#else
	uint16x4_t hi = vget_high_u16(vreinterpretq_u16_u32(v.u4));
	uint16x4_t lo = vmovn_u32(v.u4);
	uint16x8_t combined = vcombine_u16(lo, hi);
	uint32x2_t reduced = vreinterpret_u32_u8(vmovn_u16(combined));
	return vget_lane_u32(reduced, 0) != 0x0;
#endif
}

NV_SIMD_NAMESPACE_END
