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

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// factory implementation
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <>
inline Simd4iFactory<const int&>::operator Simd4i() const
{
	return _mm_set1_epi32(v);
}

inline Simd4iFactory<detail::FourTuple>::operator Simd4i() const
{
	return reinterpret_cast<const Simd4i&>(v);
}

template <int i>
inline Simd4iFactory<detail::IntType<i> >::operator Simd4i() const
{
	return _mm_set1_epi32(i);
}

template <>
inline Simd4iFactory<detail::IntType<0> >::operator Simd4i() const
{
	return _mm_setzero_si128();
}

template <>
inline Simd4iFactory<const int*>::operator Simd4i() const
{
	return _mm_loadu_si128(reinterpret_cast<const __m128i*>(v));
}

template <>
inline Simd4iFactory<detail::AlignedPointer<int> >::operator Simd4i() const
{
	return _mm_load_si128(reinterpret_cast<const __m128i*>(v.ptr));
}

template <>
inline Simd4iFactory<detail::OffsetPointer<int> >::operator Simd4i() const
{
	return _mm_load_si128(reinterpret_cast<const __m128i*>(reinterpret_cast<const char*>(v.ptr) + v.offset));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// expression template
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <>
inline ComplementExpr<Simd4i>::operator Simd4i() const
{
	return _mm_andnot_si128(v, _mm_set1_epi32(0xffffffff));
}

Simd4i operator&(const ComplementExpr<Simd4i>& complement, const Simd4i& v)
{
	return _mm_andnot_si128(complement.v, v);
}

Simd4i operator&(const Simd4i& v, const ComplementExpr<Simd4i>& complement)
{
	return _mm_andnot_si128(complement.v, v);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// operator implementations
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

Simd4i simdi::operator==(const Simd4i& v0, const Simd4i& v1)
{
	return _mm_cmpeq_epi32(v0, v1);
}

Simd4i simdi::operator<(const Simd4i& v0, const Simd4i& v1)
{
	return _mm_cmplt_epi32(v0, v1);
}

Simd4i simdi::operator>(const Simd4i& v0, const Simd4i& v1)
{
	return _mm_cmpgt_epi32(v0, v1);
}

ComplementExpr<Simd4i> operator~(const Simd4i& v)
{
	return ComplementExpr<Simd4i>(v);
}

Simd4i operator&(const Simd4i& v0, const Simd4i& v1)
{
	return _mm_and_si128(v0, v1);
}

Simd4i operator|(const Simd4i& v0, const Simd4i& v1)
{
	return _mm_or_si128(v0, v1);
}

Simd4i operator^(const Simd4i& v0, const Simd4i& v1)
{
	return _mm_xor_si128(v0, v1);
}

Simd4i operator<<(const Simd4i& v, int shift)
{
	return _mm_slli_epi32(v, shift);
}

Simd4i operator>>(const Simd4i& v, int shift)
{
	return _mm_srli_epi32(v, shift);
}

Simd4i simdi::operator+(const Simd4i& v0, const Simd4i& v1)
{
	return _mm_add_epi32(v0, v1);
}

Simd4i simdi::operator-(const Simd4i& v)
{
	return _mm_sub_epi32(_mm_setzero_si128(), v);
}

Simd4i simdi::operator-(const Simd4i& v0, const Simd4i& v1)
{
	return _mm_sub_epi32(v0, v1);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// function implementations
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

Simd4i simd4i(const Simd4f& v)
{
	return _mm_castps_si128(v);
}

int (&simdi::array(Simd4i& v))[4]
{
	return reinterpret_cast<int(&)[4]>(v);
}

const int (&simdi::array(const Simd4i& v))[4]
{
	return reinterpret_cast<const int(&)[4]>(v);
}

void store(int* ptr, const Simd4i& v)
{
	_mm_storeu_si128(reinterpret_cast<__m128i*>(ptr), v);
}

void storeAligned(int* ptr, const Simd4i& v)
{
	_mm_store_si128(reinterpret_cast<__m128i*>(ptr), v);
}

void storeAligned(int* ptr, unsigned int offset, const Simd4i& v)
{
	_mm_store_si128(reinterpret_cast<__m128i*>(reinterpret_cast<char*>(ptr) + offset), v);
}

template <size_t i>
Simd4i splat(const Simd4i& v)
{
	return _mm_shuffle_epi32(v, _MM_SHUFFLE(i, i, i, i));
}

Simd4i select(const Simd4i& mask, const Simd4i& v0, const Simd4i& v1)
{
	return _mm_xor_si128(v1, _mm_and_si128(mask, _mm_xor_si128(v1, v0)));
}

int simdi::allEqual(const Simd4i& v0, const Simd4i& v1)
{
	return allTrue(simdi::operator==(v0, v1));
}

int simdi::allEqual(const Simd4i& v0, const Simd4i& v1, Simd4i& outMask)
{
	return allTrue(outMask = simdi::operator==(v0, v1));
}

int simdi::anyEqual(const Simd4i& v0, const Simd4i& v1)
{
	return anyTrue(simdi::operator==(v0, v1));
}

int simdi::anyEqual(const Simd4i& v0, const Simd4i& v1, Simd4i& outMask)
{
	return anyTrue(outMask = simdi::operator==(v0, v1));
}

int simdi::allGreater(const Simd4i& v0, const Simd4i& v1)
{
	return allTrue(simdi::operator>(v0, v1));
}

int simdi::allGreater(const Simd4i& v0, const Simd4i& v1, Simd4i& outMask)
{
	return allTrue(outMask = simdi::operator>(v0, v1));
}

int simdi::anyGreater(const Simd4i& v0, const Simd4i& v1)
{
	return anyTrue(simdi::operator>(v0, v1));
}

int simdi::anyGreater(const Simd4i& v0, const Simd4i& v1, Simd4i& outMask)
{
	return anyTrue(outMask = simdi::operator>(v0, v1));
}

int allTrue(const Simd4i& v)
{
	return _mm_movemask_ps(_mm_castsi128_ps(v)) == 0xf;
}

int anyTrue(const Simd4i& v)
{
	return _mm_movemask_ps(_mm_castsi128_ps(v));
}
