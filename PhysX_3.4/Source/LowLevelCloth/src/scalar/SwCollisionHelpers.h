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

namespace physx
{
namespace cloth
{

#if !NV_SIMD_SIMD
uint32_t findBitSet(uint32_t mask)
{
	uint32_t result = 0;
	while(mask >>= 1)
		++result;
	return result;
}
#endif

inline Scalar4i intFloor(const Scalar4f& v)
{
	return Scalar4i(int(floor(v.f4[0])), int(floor(v.f4[1])), int(floor(v.f4[2])), int(floor(v.f4[3])));
}

inline Scalar4i horizontalOr(const Scalar4i& mask)
{
	return simd4i(mask.i4[0] | mask.i4[1] | mask.i4[2] | mask.i4[3]);
}

template <>
struct Gather<Scalar4i>
{
	inline Gather(const Scalar4i& index);
	inline Scalar4i operator()(const Scalar4i*) const;

	Scalar4i mIndex;
	Scalar4i mOutOfRange;
};

Gather<Scalar4i>::Gather(const Scalar4i& index)
{
	uint32_t mask = /* sGridSize */ 8 - 1;

	mIndex.u4[0] = index.u4[0] & mask;
	mIndex.u4[1] = index.u4[1] & mask;
	mIndex.u4[2] = index.u4[2] & mask;
	mIndex.u4[3] = index.u4[3] & mask;

	mOutOfRange.i4[0] = index.u4[0] & ~mask ? 0 : -1;
	mOutOfRange.i4[1] = index.u4[1] & ~mask ? 0 : -1;
	mOutOfRange.i4[2] = index.u4[2] & ~mask ? 0 : -1;
	mOutOfRange.i4[3] = index.u4[3] & ~mask ? 0 : -1;
}

Scalar4i Gather<Scalar4i>::operator()(const Scalar4i* ptr) const
{
	const int32_t* base = ptr->i4;
	const int32_t* index = mIndex.i4;
	const int32_t* mask = mOutOfRange.i4;
	return Scalar4i(base[index[0]] & mask[0], base[index[1]] & mask[1], base[index[2]] & mask[2],
	                base[index[3]] & mask[3]);
}

} // namespace cloth
} // namespace physx
