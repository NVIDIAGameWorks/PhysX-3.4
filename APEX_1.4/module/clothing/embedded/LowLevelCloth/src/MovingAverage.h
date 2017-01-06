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

#include "Allocator.h"

namespace nvidia
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
