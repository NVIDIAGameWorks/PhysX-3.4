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

#include "Types.h"
#include "Range.h"
#include "Allocator.h"
#include "Vec4T.h"

namespace nvidia
{

namespace cloth
{

struct TripletScheduler
{
	typedef Vector<Vec4u>::Type::ConstIterator ConstTripletIter;
	typedef Vector<Vec4u>::Type::Iterator TripletIter;

	TripletScheduler(Range<const uint32_t[4]>);
	void simd(uint32_t numParticles, uint32_t simdWidth);
	void warp(uint32_t numParticles, uint32_t warpWidth);

	Vector<Vec4u>::Type mTriplets;
	Vector<uint32_t>::Type mSetSizes;
};
}
}
