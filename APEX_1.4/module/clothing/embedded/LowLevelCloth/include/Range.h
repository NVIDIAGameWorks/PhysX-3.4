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

#include "PxAssert.h"
#include "Types.h"

namespace nvidia
{
namespace cloth
{

template <class T>
struct Range
{
	Range();

	Range(T* first, T* last);

	template <typename S>
	Range(const Range<S>& other);

	uint32_t size() const;
	bool empty() const;

	void popFront();
	void popBack();

	T* begin() const;
	T* end() const;

	T& front() const;
	T& back() const;

	T& operator[](uint32_t i) const;

  private:
	T* mFirst;
	T* mLast; // past last element
};

template <typename T>
Range<T>::Range()
: mFirst(0), mLast(0)
{
}

template <typename T>
Range<T>::Range(T* first, T* last)
: mFirst(first), mLast(last)
{
}

template <typename T>
template <typename S>
Range<T>::Range(const Range<S>& other)
: mFirst(other.begin()), mLast(other.end())
{
}

template <typename T>
uint32_t Range<T>::size() const
{
	return uint32_t(mLast - mFirst);
}

template <typename T>
bool Range<T>::empty() const
{
	return mFirst >= mLast;
}

template <typename T>
void Range<T>::popFront()
{
	PX_ASSERT(mFirst < mLast);
	++mFirst;
}

template <typename T>
void Range<T>::popBack()
{
	PX_ASSERT(mFirst < mLast);
	--mLast;
}

template <typename T>
T* Range<T>::begin() const
{
	return mFirst;
}

template <typename T>
T* Range<T>::end() const
{
	return mLast;
}

template <typename T>
T& Range<T>::front() const
{
	PX_ASSERT(mFirst < mLast);
	return *mFirst;
}

template <typename T>
T& Range<T>::back() const
{
	PX_ASSERT(mFirst < mLast);
	return mLast[-1];
}

template <typename T>
T& Range<T>::operator[](uint32_t i) const
{
	PX_ASSERT(mFirst + i < mLast);
	return mFirst[i];
}

} // namespace cloth
} // namespace nvidia
