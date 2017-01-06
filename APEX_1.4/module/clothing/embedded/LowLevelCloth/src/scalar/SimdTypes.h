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

#ifdef PX_WIIU
#pragma ghs nowarning 193 // warning #193-D: zero used for undefined preprocessing identifier
#endif

#include <algorithm>

#ifdef PX_WIIU
#pragma ghs endnowarning
#endif

union Scalar4f
{
	Scalar4f()
	{
	}

	Scalar4f(float x, float y, float z, float w)
	{
		f4[0] = x;
		f4[1] = y;
		f4[2] = z;
		f4[3] = w;
	}

	Scalar4f(int32_t x, int32_t y, int32_t z, int32_t w)
	{
		i4[0] = x;
		i4[1] = y;
		i4[2] = z;
		i4[3] = w;
	}

	Scalar4f(uint32_t x, uint32_t y, uint32_t z, uint32_t w)
	{
		u4[0] = x;
		u4[1] = y;
		u4[2] = z;
		u4[3] = w;
	}

	Scalar4f(bool x, bool y, bool z, bool w)
	{
		u4[0] = ~(uint32_t(x) - 1);
		u4[1] = ~(uint32_t(y) - 1);
		u4[2] = ~(uint32_t(z) - 1);
		u4[3] = ~(uint32_t(w) - 1);
	}

	Scalar4f(const Scalar4f& other)
	{
		u4[0] = other.u4[0];
		u4[1] = other.u4[1];
		u4[2] = other.u4[2];
		u4[3] = other.u4[3];
	}

	Scalar4f& operator=(const Scalar4f& other)
	{
		u4[0] = other.u4[0];
		u4[1] = other.u4[1];
		u4[2] = other.u4[2];
		u4[3] = other.u4[3];
		return *this;
	}

	float f4[4];
	int32_t i4[4];
	uint32_t u4[4];
};

typedef Scalar4f Scalar4i;
