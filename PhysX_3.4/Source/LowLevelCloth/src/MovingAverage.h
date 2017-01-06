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

#include "Allocator.h"

namespace physx
{
namespace cloth
{

struct MovingAverage
{
	struct Element
	{
		uint32_t mCount;
		float mValue;
	};

  public:
	MovingAverage(uint32_t n = 1) : mCount(0), mSize(n)
	{
	}

	bool empty() const
	{
		return mData.empty();
	}

	uint32_t size() const
	{
		return mSize;
	}

	void resize(uint32_t n)
	{
		PX_ASSERT(n);
		mSize = n;
		trim();
	}

	void reset()
	{
		mData.resize(0);
		mCount = 0;
	}

	void push(uint32_t n, float value)
	{
		n = PxMin(n, mSize);

		if(mData.empty() || mData.back().mValue != value)
		{
			Element element = { n, value };
			mData.pushBack(element);
		}
		else
		{
			mData.back().mCount += n;
		}

		mCount += n;
		trim();
	}

	float average() const
	{
		PX_ASSERT(!mData.empty());

		float sum = 0.0f;
		Vector<Element>::Type::ConstIterator it = mData.begin(), end = mData.end();
		for(; it != end; ++it)
			sum += it->mCount * it->mValue;

		// linear weight ramps at both ends for smoother average
		uint32_t n = mCount / 8;
		float ramp = 0.0f, temp = 0.0f;
		uint32_t countLo = (it = mData.begin())->mCount;
		uint32_t countHi = (--end)->mCount;
		for(uint32_t i = 0; i < n; ++i)
		{
			if(i == countLo)
				countLo += (++it)->mCount;
			if(i == countHi)
				countHi += (--end)->mCount;

			temp += it->mValue + end->mValue;
			ramp += temp;
		}

		uint32_t num = (mCount - n) * (n + 1);
		return (sum * (n + 1) - ramp) / num;
	}

  private:
	// remove oldest (front) values until mCount<=mSize
	void trim()
	{
		Vector<Element>::Type::Iterator it = mData.begin();
		for(uint32_t k = mSize; k < mCount; it += k <= mCount)
		{
			k += it->mCount;
			it->mCount = k - mCount;
		}

		if(it != mData.begin())
			mData.assign(it, mData.end());

		mCount = PxMin(mCount, mSize);
	}

	Vector<Element>::Type mData;

	uint32_t mCount;
	uint32_t mSize;
};
}
}
