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

namespace nvidia
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

} // namespace nvidia
