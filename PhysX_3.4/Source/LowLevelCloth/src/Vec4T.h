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

#include "Types.h"

namespace physx
{

namespace cloth
{

template <typename T>
struct Vec4T
{
	Vec4T()
	{
	}

	Vec4T(T a, T b, T c, T d) : x(a), y(b), z(c), w(d)
	{
	}

	template <typename S>
	Vec4T(const Vec4T<S>& other)
	{
		x = T(other.x);
		y = T(other.y);
		z = T(other.z);
		w = T(other.w);
	}

	template <typename Index>
	T& operator[](Index i)
	{
		return reinterpret_cast<T*>(this)[i];
	}

	template <typename Index>
	const T& operator[](Index i) const
	{
		return reinterpret_cast<const T*>(this)[i];
	}

	T x, y, z, w;
};

template <typename T>
Vec4T<T> operator*(const Vec4T<T>& vec, T scalar)
{
	return Vec4T<T>(vec.x * scalar, vec.y * scalar, vec.z * scalar, vec.w * scalar);
}

template <typename T>
Vec4T<T> operator/(const Vec4T<T>& vec, T scalar)
{
	return Vec4T<T>(vec.x / scalar, vec.y / scalar, vec.z / scalar, vec.w / scalar);
}

template <typename T>
T (&array(Vec4T<T>& vec))[4]
{
	return reinterpret_cast<T(&)[4]>(vec);
}

template <typename T>
const T (&array(const Vec4T<T>& vec))[4]
{
	return reinterpret_cast<const T(&)[4]>(vec);
}

typedef Vec4T<uint32_t> Vec4u;
typedef Vec4T<uint16_t> Vec4us;

} // namespace cloth

} // namespace physx
