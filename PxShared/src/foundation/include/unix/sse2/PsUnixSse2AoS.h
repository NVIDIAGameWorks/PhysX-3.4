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
// Copyright (c) 2008-2018 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.

#ifndef PSFOUNDATION_PSUNIXSSE2AOS_H
#define PSFOUNDATION_PSUNIXSSE2AOS_H

// no includes here! this file should be included from PxcVecMath.h only!!!

#if !COMPILE_VECTOR_INTRINSICS
#error Vector intrinsics should not be included when using scalar implementation.
#endif
#if PX_EMSCRIPTEN
typedef int8_t   __int8_t;
typedef int16_t  __int16_t;
typedef int32_t  __int32_t;
typedef int64_t  __int64_t;
typedef uint16_t __uint16_t;
typedef uint32_t __uint32_t;
typedef uint64_t __uint64_t;
#endif

typedef union UnionM128
{
	UnionM128()
	{
	}
	UnionM128(__m128 in)
	{
		m128 = in;
	}

	UnionM128(__m128i in)
	{
		m128i = in;
	}

	operator __m128()
	{
		return m128;
	}

	operator const __m128() const
	{
		return m128;
	}

	float m128_f32[4];
	__int8_t m128_i8[16];
	__int16_t m128_i16[8];
	__int32_t m128_i32[4];
	__int64_t m128_i64[2];
	__uint16_t m128_u16[8];
	__uint32_t m128_u32[4];
	__uint64_t m128_u64[2];
	__m128 m128;
	__m128i m128i;
} UnionM128;

typedef __m128 FloatV;
typedef __m128 Vec3V;
typedef __m128 Vec4V;
typedef __m128 BoolV;
typedef __m128 QuatV;
typedef __m128i VecI32V;
typedef UnionM128 VecU32V;
typedef UnionM128 VecU16V;
typedef UnionM128 VecI16V;
typedef UnionM128 VecU8V;

#define FloatVArg FloatV &
#define Vec3VArg Vec3V &
#define Vec4VArg Vec4V &
#define BoolVArg BoolV &
#define VecU32VArg VecU32V &
#define VecI32VArg VecI32V &
#define VecU16VArg VecU16V &
#define VecI16VArg VecI16V &
#define VecU8VArg VecU8V &
#define QuatVArg QuatV &

// Optimization for situations in which you cross product multiple vectors with the same vector.
// Avoids 2X shuffles per product
struct VecCrossV
{
	Vec3V mL1;
	Vec3V mR1;
};

struct VecShiftV
{
	VecI32V shift;
};
#define VecShiftVArg VecShiftV &

PX_ALIGN_PREFIX(16)
struct Mat33V
{
	Mat33V()
	{
	}
	Mat33V(const Vec3V& c0, const Vec3V& c1, const Vec3V& c2) : col0(c0), col1(c1), col2(c2)
	{
	}
	Vec3V PX_ALIGN(16, col0);
	Vec3V PX_ALIGN(16, col1);
	Vec3V PX_ALIGN(16, col2);
} PX_ALIGN_SUFFIX(16);

PX_ALIGN_PREFIX(16)
struct Mat34V
{
	Mat34V()
	{
	}
	Mat34V(const Vec3V& c0, const Vec3V& c1, const Vec3V& c2, const Vec3V& c3) : col0(c0), col1(c1), col2(c2), col3(c3)
	{
	}
	Vec3V PX_ALIGN(16, col0);
	Vec3V PX_ALIGN(16, col1);
	Vec3V PX_ALIGN(16, col2);
	Vec3V PX_ALIGN(16, col3);
} PX_ALIGN_SUFFIX(16);

PX_ALIGN_PREFIX(16)
struct Mat43V
{
	Mat43V()
	{
	}
	Mat43V(const Vec4V& c0, const Vec4V& c1, const Vec4V& c2) : col0(c0), col1(c1), col2(c2)
	{
	}
	Vec4V PX_ALIGN(16, col0);
	Vec4V PX_ALIGN(16, col1);
	Vec4V PX_ALIGN(16, col2);
} PX_ALIGN_SUFFIX(16);

PX_ALIGN_PREFIX(16)
struct Mat44V
{
	Mat44V()
	{
	}
	Mat44V(const Vec4V& c0, const Vec4V& c1, const Vec4V& c2, const Vec4V& c3) : col0(c0), col1(c1), col2(c2), col3(c3)
	{
	}
	Vec4V PX_ALIGN(16, col0);
	Vec4V PX_ALIGN(16, col1);
	Vec4V PX_ALIGN(16, col2);
	Vec4V PX_ALIGN(16, col3);
} PX_ALIGN_SUFFIX(16);

#endif // PSFOUNDATION_PSUNIXSSE2AOS_H
