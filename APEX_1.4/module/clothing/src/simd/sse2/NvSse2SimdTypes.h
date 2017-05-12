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

// SSE + SSE2 (don't include intrin.h!)
#include <emmintrin.h>

#if defined _MSC_VER && !(defined NV_SIMD_USE_NAMESPACE && NV_SIMD_USE_NAMESPACE)

// SIMD libarary lives in global namespace and Simd4f is
// typedef'd  to __m128 so it can be passed by value on MSVC.

typedef __m128 Simd4f;
typedef __m128i Simd4i;

#else

NV_SIMD_NAMESPACE_BEGIN

/** \brief SIMD type containing 4 floats */
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

/** \brief SIMD type containing 4 integers */
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

NV_SIMD_NAMESPACE_END

#endif
