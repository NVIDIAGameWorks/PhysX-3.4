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

#include <cmath>

// ps4 compiler defines _M_X64 without value
#if !defined(PX_SIMD_DISABLED)
#if((defined _M_IX86) || (defined _M_X64) || (defined __i386__) || (defined __x86_64__) || (defined __EMSCRIPTEN__ && defined __SSE2__))
#define NVMATH_SSE2 1
#else
#define NVMATH_SSE2 0
#endif
#if((defined _M_ARM) || (defined __ARM_NEON__) || (defined __ARM_NEON_FP))
#define NVMATH_NEON 1
#else
#define NVMATH_NEON 0
#endif
#else
#define NVMATH_SSE2 0
#define NVMATH_NEON 0
#endif

// which simd types are implemented (one or both are all valid options)
#define NVMATH_SIMD (NVMATH_SSE2 || NVMATH_NEON)
#define NVMATH_SCALAR !NVMATH_SIMD
// #define NVMATH_SCALAR 1

// use template expression to fuse multiply-adds into a single instruction
#define NVMATH_FUSE_MULTIPLY_ADD (NVMATH_NEON)
// support shift by vector operarations
#define NVMATH_SHIFT_BY_VECTOR (NVMATH_NEON)
// Simd4f and Simd4i map to different types
#define NVMATH_DISTINCT_TYPES (NVMATH_SSE2 || NVMATH_NEON)
// support inline assembler
#if !((defined _M_ARM) || (defined SN_TARGET_PSP2) || (defined __arm64__) || (defined __aarch64__))
#define NVMATH_INLINE_ASSEMBLER 1
#else
#define NVMATH_INLINE_ASSEMBLER 0
#endif

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// expression template
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/*! \brief Expression template to fuse and-not. */
template <typename T>
struct ComplementExpr
{
	inline ComplementExpr(T const& v_) : v(v_)
	{
	}
	inline operator T() const;
	const T v;

  private:
	ComplementExpr& operator=(const ComplementExpr&); // not implemented
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// helper functions
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename T>
T sqr(const T& x)
{
	return x * x;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// details
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

namespace detail
{
template <typename T>
struct AlignedPointer
{
	AlignedPointer(const T* p) : ptr(p)
	{
	}
	const T* ptr;
};

template <typename T>
struct OffsetPointer
{
	OffsetPointer(const T* p, unsigned int off) : ptr(p), offset(off)
	{
	}
	const T* ptr;
	unsigned int offset;
};

struct FourTuple
{
};

// zero and one literals
template <int i>
struct IntType
{
};
}

// Supress warnings
#if defined(__GNUC__) || defined(__SNC__)
#define NVMATH_UNUSED __attribute__((unused))
#else
#define NVMATH_UNUSED
#endif

static detail::IntType<0> _0 NVMATH_UNUSED;
static detail::IntType<1> _1 NVMATH_UNUSED;
static detail::IntType<int(0x80000000)> _sign NVMATH_UNUSED;
static detail::IntType<-1> _true NVMATH_UNUSED;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// platform specific includes
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#if NVMATH_SSE2
#include "sse2/SimdTypes.h"
#elif NVMATH_NEON
#include "neon/SimdTypes.h"
#else
struct Simd4f;
struct Simd4i;
#endif

#if NVMATH_SCALAR
#include "scalar/SimdTypes.h"
#else
struct Scalar4f;
struct Scalar4i;
#endif
