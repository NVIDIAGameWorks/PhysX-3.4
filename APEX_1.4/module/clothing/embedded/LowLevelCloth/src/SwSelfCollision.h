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
#include "StackAllocator.h"
#include "Simd4i.h"

namespace nvidia
{

namespace cloth
{

class SwCloth;
struct SwClothData;

typedef StackAllocator<16> SwKernelAllocator;

template <typename Simd4f>
class SwSelfCollision
{
	typedef typename Simd4fToSimd4i<Simd4f>::Type Simd4i;

  public:
	SwSelfCollision(SwClothData& clothData, SwKernelAllocator& alloc);
	~SwSelfCollision();

	void operator()();

	static size_t estimateTemporaryMemory(const SwCloth&);

  private:
	SwSelfCollision& operator=(const SwSelfCollision&); // not implemented
	static size_t getBufferSize(uint32_t);

	template <bool useRestParticles>
	void collideParticles(Simd4f&, Simd4f&, const Simd4f&, const Simd4f&);

	template <bool useRestParticles>
	void collideParticles(const uint32_t*, uint16_t, const uint16_t*, uint32_t);

	Simd4f mCollisionDistance;
	Simd4f mCollisionSquareDistance;
	Simd4f mStiffness;

	SwClothData& mClothData;
	SwKernelAllocator& mAllocator;

  public:
	mutable uint32_t mNumTests;
	mutable uint32_t mNumCollisions;
};

} // namespace cloth

} // namespace nvidia
