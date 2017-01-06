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
#include "Simd4f.h"

namespace nvidia
{

namespace cloth
{

// acts as a poor mans random access iterator
template <typename Simd4f, typename BaseIterator>
class LerpIterator
{

	LerpIterator& operator=(const LerpIterator&); // not implemented

  public:
	LerpIterator(BaseIterator start, BaseIterator target, float alpha)
	: mAlpha(simd4f(alpha)), mStart(start), mTarget(target)
	{
	}

	// return the interpolated point at a given index
	inline Simd4f operator[](size_t index) const
	{
		return mStart[index] + (mTarget[index] - mStart[index]) * mAlpha;
	}

	inline Simd4f operator*() const
	{
		return (*this)[0];
	}

	// prefix increment only
	inline LerpIterator& operator++()
	{
		++mStart;
		++mTarget;
		return *this;
	}

  private:
	// interpolation parameter
	const Simd4f mAlpha;

	BaseIterator mStart;
	BaseIterator mTarget;
};

template <typename Simd4f, size_t Stride>
class UnalignedIterator
{

	UnalignedIterator& operator=(const UnalignedIterator&); // not implemented

  public:
	UnalignedIterator(const float* pointer) : mPointer(pointer)
	{
	}

	inline Simd4f operator[](size_t index) const
	{
		return load(mPointer + index * Stride);
	}

	inline Simd4f operator*() const
	{
		return (*this)[0];
	}

	// prefix increment only
	inline UnalignedIterator& operator++()
	{
		mPointer += Stride;
		return *this;
	}

  private:
	const float* mPointer;
};

// acts as an iterator but returns a constant
template <typename Simd4f>
class ConstantIterator
{
  public:
	ConstantIterator(const Simd4f& value) : mValue(value)
	{
	}

	inline Simd4f operator*() const
	{
		return mValue;
	}

	inline ConstantIterator& operator++()
	{
		return *this;
	}

  private:
	ConstantIterator& operator=(const ConstantIterator&);
	const Simd4f mValue;
};

// wraps an iterator with constant scale and bias
template <typename Simd4f, typename BaseIterator>
class ScaleBiasIterator
{
  public:
	ScaleBiasIterator(BaseIterator base, const Simd4f& scale, const Simd4f& bias)
	: mScale(scale), mBias(bias), mBaseIterator(base)
	{
	}

	inline Simd4f operator*() const
	{
		return (*mBaseIterator) * mScale + mBias;
	}

	inline ScaleBiasIterator& operator++()
	{
		++mBaseIterator;
		return *this;
	}

  private:
	ScaleBiasIterator& operator=(const ScaleBiasIterator&);

	const Simd4f mScale;
	const Simd4f mBias;

	BaseIterator mBaseIterator;
};

} // namespace cloth

} // namespace nvidia
