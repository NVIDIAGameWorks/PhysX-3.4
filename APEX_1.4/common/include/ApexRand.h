/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef APEX_RAND_H
#define APEX_RAND_H

#include "PxMath.h"
#include "PxVec3.h"
#include "ApexUsingNamespace.h"

namespace nvidia
{
namespace apex
{

// "Quick and Dirty Symmetric Random number generator"	- returns a uniform deviate in [-1.0,1.0)
class QDSRand
{
	uint32_t mSeed;

public:

	PX_CUDA_CALLABLE PX_INLINE QDSRand(uint32_t seed = 0) : mSeed(seed) {}

	PX_CUDA_CALLABLE PX_INLINE void setSeed(uint32_t seed = 0)
	{
		mSeed = seed;
	}

	PX_CUDA_CALLABLE PX_INLINE uint32_t seed() const
	{
		return mSeed;
	}

	PX_CUDA_CALLABLE PX_INLINE uint32_t nextSeed()
	{
		mSeed = mSeed * 1664525L + 1013904223L;
		return mSeed;
	}

	PX_CUDA_CALLABLE PX_INLINE float getNext()
	{
		union U32F32
		{
			uint32_t   u;
			float   f;
		} r;
		r.u = 0x40000000 | (nextSeed() >> 9);
		return r.f - 3.0f;
	}

	PX_CUDA_CALLABLE PX_INLINE float getScaled(const float min, const float max)
	{
		const float scale = (max - min) / 2.0f;
		return ((getNext() + 1.0f) * scale) + min;
	}

	PX_CUDA_CALLABLE PX_INLINE PxVec3 getScaled(const PxVec3& min, const PxVec3& max)
	{
		return PxVec3(getScaled(min.x, max.x), getScaled(min.y, max.y), getScaled(min.z, max.z));
	}

	PX_CUDA_CALLABLE PX_INLINE float getUnit()
	{
		union U32F32
		{
			uint32_t   u;
			float   f;
		} r;
		r.u = 0x3F800000 | (nextSeed() >> 9);
		return r.f - 1.0f;
	}

};

// "Quick and Dirty Normal Random number generator"	- returns normally-distributed values
class QDNormRand
{
	QDSRand mBase;

public:

	PX_CUDA_CALLABLE PX_INLINE QDNormRand(uint32_t seed = 0) : mBase(seed) {}

	PX_CUDA_CALLABLE PX_INLINE void setSeed(uint32_t seed = 0)
	{
		mBase.setSeed(seed);
	}

	PX_CUDA_CALLABLE PX_INLINE uint32_t setSeed() const
	{
		return mBase.seed();
	}

	PX_CUDA_CALLABLE PX_INLINE uint32_t nextSeed()
	{
		return mBase.nextSeed();
	}

	PX_CUDA_CALLABLE PX_INLINE float getNext()
	{
		//Using Box-Muller transform (see http://en.wikipedia.org/wiki/Box_Muller_transform)

		float u, v, s;
		do
		{
			u = mBase.getNext();
			v = mBase.getNext();
			s = u * u + v * v;
		}
		while (s >= 1.0);

		return u * PxSqrt(-2.0f * PxLog(s) / s);
	}

	PX_CUDA_CALLABLE PX_INLINE float getScaled(const float m, const float s)
	{
		return m + s * getNext();
	}
};

}
} // end namespace nvidia::apex

#endif
