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

#ifndef PXFOUNDATION_PXUNIXINTRINSICS_H
#define PXFOUNDATION_PXUNIXINTRINSICS_H

#include "foundation/Px.h"
#include "foundation/PxAssert.h"

#if !(PX_LINUX || PX_ANDROID || PX_PS4 || PX_APPLE_FAMILY)
#error "This file should only be included by Unix builds!!"
#endif

#if (PX_LINUX || PX_ANDROID) && !defined(__CUDACC__) && !PX_EMSCRIPTEN
    // Linux/android and CUDA compilation does not work with std::isfnite, as it is not marked as CUDA callable
    #include <cmath>
    #ifndef isfinite
        using std::isfinite;
    #endif
#endif

#include <math.h>
#include <float.h>

namespace physx
{
namespace intrinsics
{
//! \brief platform-specific absolute value
PX_CUDA_CALLABLE PX_FORCE_INLINE float abs(float a)
{
	return ::fabsf(a);
}

//! \brief platform-specific select float
PX_CUDA_CALLABLE PX_FORCE_INLINE float fsel(float a, float b, float c)
{
	return (a >= 0.0f) ? b : c;
}

//! \brief platform-specific sign
PX_CUDA_CALLABLE PX_FORCE_INLINE float sign(float a)
{
	return (a >= 0.0f) ? 1.0f : -1.0f;
}

//! \brief platform-specific reciprocal
PX_CUDA_CALLABLE PX_FORCE_INLINE float recip(float a)
{
	return 1.0f / a;
}

//! \brief platform-specific reciprocal estimate
PX_CUDA_CALLABLE PX_FORCE_INLINE float recipFast(float a)
{
	return 1.0f / a;
}

//! \brief platform-specific square root
PX_CUDA_CALLABLE PX_FORCE_INLINE float sqrt(float a)
{
	return ::sqrtf(a);
}

//! \brief platform-specific reciprocal square root
PX_CUDA_CALLABLE PX_FORCE_INLINE float recipSqrt(float a)
{
	return 1.0f / ::sqrtf(a);
}

PX_CUDA_CALLABLE PX_FORCE_INLINE float recipSqrtFast(float a)
{
	return 1.0f / ::sqrtf(a);
}

//! \brief platform-specific sine
PX_CUDA_CALLABLE PX_FORCE_INLINE float sin(float a)
{
	return ::sinf(a);
}

//! \brief platform-specific cosine
PX_CUDA_CALLABLE PX_FORCE_INLINE float cos(float a)
{
	return ::cosf(a);
}

//! \brief platform-specific minimum
PX_CUDA_CALLABLE PX_FORCE_INLINE float selectMin(float a, float b)
{
	return a < b ? a : b;
}

//! \brief platform-specific maximum
PX_CUDA_CALLABLE PX_FORCE_INLINE float selectMax(float a, float b)
{
	return a > b ? a : b;
}

//! \brief platform-specific finiteness check (not INF or NAN)
PX_CUDA_CALLABLE PX_FORCE_INLINE bool isFinite(float a)
{
	//std::isfinite not recommended as of Feb 2017, since it doesn't work with g++/clang's floating point optimization.
    union localU { PxU32 i; float f; } floatUnion;
    floatUnion.f = a;
    return !((floatUnion.i & 0x7fffffff) >= 0x7f800000);
}

//! \brief platform-specific finiteness check (not INF or NAN)
PX_CUDA_CALLABLE PX_FORCE_INLINE bool isFinite(double a)
{
	return !!isfinite(a);
}

/*!
Sets \c count bytes starting at \c dst to zero.
*/
PX_FORCE_INLINE void* memZero(void* dest, uint32_t count)
{
	return memset(dest, 0, count);
}

/*!
Sets \c count bytes starting at \c dst to \c c.
*/
PX_FORCE_INLINE void* memSet(void* dest, int32_t c, uint32_t count)
{
	return memset(dest, c, count);
}

/*!
Copies \c count bytes from \c src to \c dst. User memMove if regions overlap.
*/
PX_FORCE_INLINE void* memCopy(void* dest, const void* src, uint32_t count)
{
	return memcpy(dest, src, count);
}

/*!
Copies \c count bytes from \c src to \c dst. Supports overlapping regions.
*/
PX_FORCE_INLINE void* memMove(void* dest, const void* src, uint32_t count)
{
	return memmove(dest, src, count);
}

/*!
Set 128B to zero starting at \c dst+offset. Must be aligned.
*/
PX_FORCE_INLINE void memZero128(void* dest, uint32_t offset = 0)
{
	PX_ASSERT(((size_t(dest) + offset) & 0x7f) == 0);
	memSet(reinterpret_cast<char*>(dest) + offset, 0, 128);
}

} // namespace intrinsics
} // namespace physx

#endif // #ifndef PXFOUNDATION_PXUNIXINTRINSICS_H
