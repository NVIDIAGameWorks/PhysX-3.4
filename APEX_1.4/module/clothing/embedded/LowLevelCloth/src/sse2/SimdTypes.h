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

// SSE + SSE2 (don't include intrin.h!)
#include <emmintrin.h>

#if defined(_MSC_VER)

typedef __m128 Simd4f;
typedef __m128i Simd4i;

#else

struct Simd4f
{
	Simd4f()
	{
	}
	Simd4f(__m128 x) : m128(x)
	{
	}

	operator __m128&()
	{
		return m128;
	}
	operator const __m128&() const
	{
		return m128;
	}

  private:
	__m128 m128;
};

struct Simd4i
{
	Simd4i()
	{
	}
	Simd4i(__m128i x) : m128i(x)
	{
	}

	operator __m128i&()
	{
		return m128i;
	}
	operator const __m128i&() const
	{
		return m128i;
	}

  private:
	__m128i m128i;
};

#endif
