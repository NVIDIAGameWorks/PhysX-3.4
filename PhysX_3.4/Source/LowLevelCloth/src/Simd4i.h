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

#include "SimdTypes.h"

NV_SIMD_NAMESPACE_BEGIN

/*! \brief Creates Simd4i with all components set to zero.
* \relates Simd4i */
struct Simd4iZeroFactory
{
	inline operator Simd4i() const;
	inline operator Scalar4i() const;
};

/*! \brief Replicates int into all four Simd4i components.
* \relates Simd4i */
struct Simd4iScalarFactory
{
	explicit Simd4iScalarFactory(const int& s) : value(s)
	{
	}
	Simd4iScalarFactory& operator=(const Simd4iScalarFactory&); // not implemented
	inline operator Simd4i() const;
	inline operator Scalar4i() const;
	const int value;
};

/*! \brief Creates Simd4i from four ints.
* \relates Simd4i */
struct Simd4iTupleFactory
{
	Simd4iTupleFactory(int x, int y, int z, int w)
	// c++11: : tuple{ x, y, z, w }
	{
		tuple[0] = x;
		tuple[1] = y;
		tuple[2] = z;
		tuple[3] = w;
	}
	Simd4iTupleFactory& operator=(const Simd4iTupleFactory&); // not implemented
	inline operator Simd4i() const;
	inline operator Scalar4i() const;
	NV_SIMD_ALIGN(16, int) tuple[4];
};

/*! \brief Loads Simd4i from (unaligned) pointer.
* \relates Simd4i */
struct Simd4iLoadFactory
{
	explicit Simd4iLoadFactory(const int* p) : ptr(p)
	{
	}
	Simd4iLoadFactory& operator=(const Simd4iLoadFactory&); // not implemented
	inline operator Simd4i() const;
	inline operator Scalar4i() const;
	const int* const ptr;
};

/*! \brief Loads Simd4i from (aligned) pointer.
* \relates Simd4i */
struct Simd4iAlignedLoadFactory
{
	explicit Simd4iAlignedLoadFactory(const int* p) : ptr(p)
	{
	}
	Simd4iAlignedLoadFactory& operator=(const Simd4iAlignedLoadFactory&); // not implemented
	inline operator Simd4i() const;
	inline operator Scalar4i() const;
	const int* const ptr;
};

/*! \brief Loads Simd4i from (aligned) pointer with offset.
* \relates Simd4i */
struct Simd4iOffsetLoadFactory
{
	Simd4iOffsetLoadFactory(const int* p, unsigned int off) : ptr(p), offset(off)
	{
	}
	Simd4iOffsetLoadFactory& operator=(const Simd4iOffsetLoadFactory&); // not implemented
	inline operator Simd4i() const;
	inline operator Scalar4i() const;
	const int* const ptr;
	const unsigned int offset;
};

// map Simd4f/Scalar4f to Simd4i/Scalar4i
template <typename>
struct Simd4fToSimd4i;
template <>
struct Simd4fToSimd4i<Simd4f>
{
	typedef Simd4i Type;
};
template <>
struct Simd4fToSimd4i<Scalar4f>
{
	typedef Scalar4i Type;
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// operators
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/*! \brief Vector bit-wise NOT operator
* \return A vector holding the bit-negate of \a v.
* \relates Simd4i */
inline ComplementExpr<Simd4i> operator~(const Simd4i& v);

/*! \brief Vector bit-wise AND operator
* \return A vector holding the bit-wise AND of \a v0 and \a v1.
* \relates Simd4i */
inline Simd4i operator&(const Simd4i& v0, const Simd4i& v1);

/*! \brief Vector bit-wise OR operator
* \return A vector holding the bit-wise OR of \a v0 and \a v1.
* \relates Simd4i */
inline Simd4i operator|(const Simd4i& v0, const Simd4i& v1);

/*! \brief Vector bit-wise XOR operator
* \return A vector holding the bit-wise XOR of \a v0 and \a v1.
* \relates Simd4i */
inline Simd4i operator^(const Simd4i& v0, const Simd4i& v1);

/*! \brief Vector logical left shift.
* \return A vector with 4 elements of \a v0, each shifted left by \a shift bits.
* \relates Simd4i */
inline Simd4i operator<<(const Simd4i& v, int shift);

/*! \brief Vector logical right shift.
* \return A vector with 4 elements of \a v0, each shifted right by \a shift bits.
* \relates Simd4i */
inline Simd4i operator>>(const Simd4i& v, int shift);

#if NV_SIMD_SHIFT_BY_VECTOR

/*! \brief Vector logical left shift.
* \return A vector with 4 elements of \a v0, each shifted left by \a shift bits.
* \relates Simd4i */
inline Simd4i operator<<(const Simd4i& v, const Simd4i& shift);

/*! \brief Vector logical right shift.
* \return A vector with 4 elements of \a v0, each shifted right by \a shift bits.
* \relates Simd4i */
inline Simd4i operator>>(const Simd4i& v, const Simd4i& shift);

#endif // NV_SIMD_SHIFT_BY_VECTOR

// note: operator?= missing because they don't have corresponding intrinsics.

/*! \brief Test for equality of two vectors.
* \return Vector of per element result mask (all bits set for 'true', none set for 'false').
* \relates Simd4i */
inline Simd4i operator==(const Simd4i& v0, const Simd4i& v1);

// no !=, <=, >= because VMX128/SSE don't support it, use ~operator== etc.

/*! \brief Less-compare all elements of two *signed* vectors.
* \return Vector of per element result mask (all bits set for 'true', none set for 'false').
* \relates Simd4i */
inline Simd4i operator<(const Simd4i& v0, const Simd4i& v1);

/*! \brief Greater-compare all elements of two *signed* vectors.
* \return Vector of per element result mask (all bits set for 'true', none set for 'false').
* \relates Simd4i */
inline Simd4i operator>(const Simd4i& v0, const Simd4i& v1);

/*! \brief Unary vector addition operator.
* \return A vector holding the component-wise copy of \a v.
* \relates Simd4i */
inline Simd4i operator+(const Simd4i& v);

/*! \brief Vector addition operator
* \return A vector holding the component-wise sum of \a v0 and \a v1.
* \relates Simd4i */
inline Simd4i operator+(const Simd4i& v0, const Simd4i& v1);

/*! \brief Unary vector negation operator.
* \return A vector holding the component-wise negation of \a v.
* \relates Simd4i */
inline Simd4i operator-(const Simd4i& v);

/*! \brief Vector subtraction operator.
* \return A vector holding the component-wise difference of \a v0 and \a v1.
* \relates Simd4i */
inline Simd4i operator-(const Simd4i& v0, const Simd4i& v1);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// functions
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/*! \brief Load int value into all vector components.
* \relates Simd4i */
inline Simd4iScalarFactory simd4i(const int& s)
{
	return Simd4iScalarFactory(s);
}

/*! \brief Load 4 int values into vector.
* \relates Simd4i */
inline Simd4iTupleFactory simd4i(int x, int y, int z, int w)
{
	return Simd4iTupleFactory(x, y, z, w);
}

/*! \brief Reinterpret Simd4f as Simd4i.
* \return A copy of \a v, but reinterpreted as Simd4i.
* \relates Simd4i */
inline Simd4i simd4i(const Simd4f& v);

/*! \brief Truncate Simd4f to Simd4i.
* \relates Simd4i */
inline Simd4i truncate(const Simd4f& v);

/*! \brief return reference to contiguous array of vector elements
* \relates Simd4i */
inline int (&array(Simd4i& v))[4];

/*! \brief return constant reference to contiguous array of vector elements
* \relates Simd4i */
inline const int (&array(const Simd4i& v))[4];

/*! \brief Create vector from int array.
* \relates Simd4i */
inline Simd4iLoadFactory load(const int* ptr)
{
	return Simd4iLoadFactory(ptr);
}

/*! \brief Create vector from aligned int array.
* \note \a ptr needs to be 16 byte aligned.
* \relates Simd4i */
inline Simd4iAlignedLoadFactory loadAligned(const int* ptr)
{
	return Simd4iAlignedLoadFactory(ptr);
}

/*! \brief Create vector from aligned float array.
* \param offset pointer offset in bytes.
* \note \a ptr+offset needs to be 16 byte aligned.
* \relates Simd4i */
inline Simd4iOffsetLoadFactory loadAligned(const int* ptr, unsigned int offset)
{
	return Simd4iOffsetLoadFactory(ptr, offset);
}

/*! \brief Store vector \a v to int array \a ptr.
* \relates Simd4i */
inline void store(int* ptr, const Simd4i& v);

/*! \brief Store vector \a v to aligned int array \a ptr.
* \note \a ptr needs to be 16 byte aligned.
* \relates Simd4i */
inline void storeAligned(int* ptr, const Simd4i& v);

/*! \brief Store vector \a v to aligned int array \a ptr.
* \param offset pointer offset in bytes.
* \note \a ptr+offset needs to be 16 byte aligned.
* \relates Simd4i */
inline void storeAligned(int* ptr, unsigned int offset, const Simd4i& v);

/*! \brief replicate i-th component into all vector components.
* \return Vector with all elements set to \a v[i].
* \relates Simd4i */
template <size_t i>
inline Simd4i splat(const Simd4i& v);

/*! \brief Select \a v0 or \a v1 based on \a mask.
* \return mask ? v0 : v1
* \relates Simd4i */
inline Simd4i select(const Simd4i& mask, const Simd4i& v0, const Simd4i& v1);

/*! \brief returns non-zero if all elements or \a v0 and \a v1 are equal
* \relates Simd4i */
inline int allEqual(const Simd4i& v0, const Simd4i& v1);

/*! \brief returns non-zero if all elements or \a v0 and \a v1 are equal
* \param outMask holds the result of \a v0 == \a v1.
* \relates Simd4i */
inline int allEqual(const Simd4i& v0, const Simd4i& v1, Simd4i& outMask);

/*! \brief returns non-zero if any elements or \a v0 and \a v1 are equal
* \relates Simd4i */
inline int anyEqual(const Simd4i& v0, const Simd4i& v1);

/*! \brief returns non-zero if any elements or \a v0 and \a v1 are equal
* \param outMask holds the result of \a v0 == \a v1.
* \relates Simd4i */
inline int anyEqual(const Simd4i& v0, const Simd4i& v1, Simd4i& outMask);

/*! \brief returns non-zero if all *signed* elements or \a v0 and \a v1 are greater
* \relates Simd4i */
inline int allGreater(const Simd4i& v0, const Simd4i& v1);

/*! \brief returns non-zero if all *signed* elements or \a v0 and \a v1 are greater
* \param outMask holds the result of \a v0 == \a v1.
* \relates Simd4i */
inline int allGreater(const Simd4i& v0, const Simd4i& v1, Simd4i& outMask);

/*! \brief returns non-zero if any elements or \a v0 and \a v1 are greater
* \relates Simd4i */
inline int anyGreater(const Simd4i& v0, const Simd4i& v1);

/*! \brief returns non-zero if any elements or \a v0 and \a v1 are greater
* \param outMask holds the result of \a v0 == \a v1.
* \relates Simd4i */
inline int anyGreater(const Simd4i& v0, const Simd4i& v1, Simd4i& outMask);

/*! \brief returns non-zero if all elements are true
* \note undefined if parameter is not result of a comparison.
* \relates Simd4i */
inline int allTrue(const Simd4i& v);

/*! \brief returns non-zero if any element is true
* \note undefined if parameter is not result of a comparison.
* \relates Simd4i */
inline int anyTrue(const Simd4i& v);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// constants
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

NV_SIMD_GLOBAL_CONSTANT Simd4iZeroFactory gSimd4iZero = Simd4iZeroFactory();
NV_SIMD_GLOBAL_CONSTANT Simd4iScalarFactory gSimd4iOne = simd4i(1);

NV_SIMD_NAMESPACE_END

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// platform specific includes
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#if NV_SIMD_SSE2
#include "sse2/Simd4i.h"
#elif NV_SIMD_NEON
#include "neon/Simd4i.h"
#endif

#if NV_SIMD_SCALAR
#include "scalar/Simd4i.h"
#endif
