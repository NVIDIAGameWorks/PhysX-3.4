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

#include "foundation/PxAssert.h"
#include "Types.h"

namespace physx
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
} // namespace physx
