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

#ifndef PSFOUNDATION_PSUNIXFPU_H
#define PSFOUNDATION_PSUNIXFPU_H

#include "foundation/PxPreprocessor.h"

#if PX_LINUX || PX_PS4 || PX_OSX

#if PX_X86 || PX_X64
#if PX_EMSCRIPTEN
#include <emmintrin.h>
#endif
#include <xmmintrin.h>
#elif PX_NEON
#include <arm_neon.h>
#endif


PX_INLINE physx::shdfnd::SIMDGuard::SIMDGuard()
{
#if !PX_EMSCRIPTEN && (PX_X86 || PX_X64)
	mControlWord = _mm_getcsr();
	// set default (disable exceptions: _MM_MASK_MASK) and FTZ (_MM_FLUSH_ZERO_ON), DAZ (_MM_DENORMALS_ZERO_ON: (1<<6))
	_mm_setcsr(_MM_MASK_MASK | _MM_FLUSH_ZERO_ON | (1 << 6));
#endif
}

PX_INLINE physx::shdfnd::SIMDGuard::~SIMDGuard()
{
#if !PX_EMSCRIPTEN && (PX_X86 || PX_X64)
	// restore control word and clear exception flags
	// (setting exception state flags cause exceptions on the first following fp operation)
	_mm_setcsr(mControlWord & ~_MM_EXCEPT_MASK);
#endif
}

#else
#error No SIMD implementation for this unix platform.
#endif // PX_LINUX || PX_PS4 || PX_OSX

#endif // #ifndef PSFOUNDATION_PSUNIXFPU_H
