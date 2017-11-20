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
#include "PsFPU.h"

#if !(defined(__CYGWIN__) || PX_ANDROID || PX_PS4)
#include <fenv.h>
PX_COMPILE_TIME_ASSERT(8 * sizeof(uint32_t) >= sizeof(fenv_t));
#endif

#if PX_OSX
// osx defines SIMD as standard for floating point operations.
#include <xmmintrin.h>
#endif

physx::shdfnd::FPUGuard::FPUGuard()
{
#if defined(__CYGWIN__)
#pragma message "FPUGuard::FPUGuard() is not implemented"
#elif PX_ANDROID
// not supported unless ARM_HARD_FLOAT is enabled.
#elif PX_PS4
	// not supported
	PX_UNUSED(mControlWords);
#elif PX_OSX
	mControlWords[0] = _mm_getcsr();
	// set default (disable exceptions: _MM_MASK_MASK) and FTZ (_MM_FLUSH_ZERO_ON), DAZ (_MM_DENORMALS_ZERO_ON: (1<<6))
	_mm_setcsr(_MM_MASK_MASK | _MM_FLUSH_ZERO_ON | (1 << 6));
#elif PX_EMSCRIPTEN
// not supported
#else
	PX_COMPILE_TIME_ASSERT(sizeof(fenv_t) <= sizeof(mControlWords));

	fegetenv(reinterpret_cast<fenv_t*>(mControlWords));
	fesetenv(FE_DFL_ENV);

#if PX_LINUX
	// need to explicitly disable exceptions because fesetenv does not modify
	// the sse control word on 32bit linux (64bit is fine, but do it here just be sure)
	fedisableexcept(FE_ALL_EXCEPT);
#endif

#endif
}

physx::shdfnd::FPUGuard::~FPUGuard()
{
#if defined(__CYGWIN__)
#pragma message "FPUGuard::~FPUGuard() is not implemented"
#elif PX_ANDROID
// not supported unless ARM_HARD_FLOAT is enabled.
#elif PX_PS4
// not supported
#elif PX_OSX
	// restore control word and clear exception flags
	// (setting exception state flags cause exceptions on the first following fp operation)
	_mm_setcsr(mControlWords[0] & ~_MM_EXCEPT_MASK);
#elif PX_EMSCRIPTEN
// not supported
#else
	fesetenv(reinterpret_cast<fenv_t*>(mControlWords));
#endif
}

PX_FOUNDATION_API void physx::shdfnd::enableFPExceptions()
{
#if PX_LINUX && !PX_EMSCRIPTEN
	feclearexcept(FE_ALL_EXCEPT);
	feenableexcept(FE_INVALID | FE_DIVBYZERO | FE_OVERFLOW);
#elif PX_OSX
	// clear any pending exceptions
	// (setting exception state flags cause exceptions on the first following fp operation)
	uint32_t control = _mm_getcsr() & ~_MM_EXCEPT_MASK;

	// enable all fp exceptions except inexact and underflow (common, benign)
	// note: denorm has to be disabled as well because underflow can create denorms
	_mm_setcsr((control & ~_MM_MASK_MASK) | _MM_MASK_INEXACT | _MM_MASK_UNDERFLOW | _MM_MASK_DENORM);

#endif
}

PX_FOUNDATION_API void physx::shdfnd::disableFPExceptions()
{
#if PX_LINUX && !PX_EMSCRIPTEN
	fedisableexcept(FE_ALL_EXCEPT);
#elif PX_OSX
	// clear any pending exceptions
	// (setting exception state flags cause exceptions on the first following fp operation)
	uint32_t control = _mm_getcsr() & ~_MM_EXCEPT_MASK;
	_mm_setcsr(control | _MM_MASK_MASK);
#endif
}
