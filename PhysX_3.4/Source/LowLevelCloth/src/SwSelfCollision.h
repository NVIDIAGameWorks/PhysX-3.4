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
// Copyright (c) 2008-2018 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.

#pragma once

#include "Types.h"
#include "StackAllocator.h"
#include "Simd.h"

namespace physx
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

#if PX_SUPPORT_EXTERN_TEMPLATE
//explicit template instantiation declaration
#if NV_SIMD_SIMD
extern template class SwSelfCollision<Simd4f>;
#endif
#if NV_SIMD_SCALAR
extern template class SwSelfCollision<Scalar4f>;
#endif
#endif

} // namespace cloth

} // namespace physx
