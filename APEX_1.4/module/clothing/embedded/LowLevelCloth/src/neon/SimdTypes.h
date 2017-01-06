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

#include <arm_neon.h>

union Simd4f
{
	Simd4f()
	{
	}
	Simd4f(const float32x4_t& v) : f4(v)
	{
	}
#ifndef _M_ARM // all *32x4_t map to the same type
	Simd4f(const uint32x4_t& v) : u4(v)
	{
	}
#endif
	float32x4_t f4;
	uint32x4_t u4;
	int32x4_t i4;
};

union Simd4i
{
	Simd4i()
	{
	}
	Simd4i(const uint32x4_t& v) : u4(v)
	{
	}
#ifndef _M_ARM // all *32x4_t map to the same type
	Simd4i(const int32x4_t& v) : i4(v)
	{
	}
#endif
	uint32x4_t u4;
	int32x4_t i4;
};
