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

#ifndef __ARM_NEON__
#error This file needs to be compiled with NEON support!
#endif

#include "SwSolverKernel.cpp"

#include <cpu-features.h>

namespace nvidia
{
namespace cloth
{
bool neonSolverKernel(SwCloth const& cloth, SwClothData& data, SwKernelAllocator& allocator,
                      IterationStateFactory& factory, PxProfileZone* profileZone)
{
	return ANDROID_CPU_ARM_FEATURE_NEON & android_getCpuFeatures() &&
	       (SwSolverKernel<Simd4f>(cloth, data, allocator, factory, profileZone)(), true);
}
}
}
