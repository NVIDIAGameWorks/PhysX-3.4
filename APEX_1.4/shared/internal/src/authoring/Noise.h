/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NOISE_H
#define NOISE_H

#include "authoring/ApexCSGMath.h"

#include "NoiseUtils.h"

#ifndef WITHOUT_APEX_AUTHORING
 
namespace ApexCSG 
{

class UserRandom;

/**
	Provides Perlin noise sampling across multiple dimensions and for different data types
*/
template<typename T, int SampleSize = 1024, int D = 3, class VecType = Vec<T, D> >
class PerlinNoise
{
public:
	PerlinNoise(UserRandom& rnd, int octaves = 1, T frequency = 1., T amplitude = 1.)
	  : mRnd(rnd),
		mOctaves(octaves),
		mFrequency(frequency),
		mAmplitude(amplitude),
		mbInit(false)
	{

	}

	void reset(int octaves = 1, T frequency = (T)1., T amplitude = (T)1.)
	{
		mOctaves   = octaves;
		mFrequency = frequency;
		mAmplitude = amplitude;
		init();
	}

	T sample(const VecType& point) 
	{
		return perlinNoise(point);
	}

private:
	PerlinNoise& operator=(const PerlinNoise&);

	T perlinNoise(VecType point)
	{
		if (!mbInit)
			init();

		const int octaves  = mOctaves;
		const T frequency  = mFrequency;
		T amplitude        = mAmplitude;
		T result           = (T)0;

		point *= frequency;

		for (int i = 0; i < octaves; ++i)
		{
			result    += noiseSample<T, SampleSize>(point, p, g) * amplitude;
			point     *= (T)2.0;
			amplitude *= (T)0.5;
		}

		return result;
	}

	void init(void)
	{
		mbInit = true;

		unsigned  i, j;
		int k;

		for (i = 0 ; i < (unsigned)SampleSize; i++)
		{
			p[i]  = (int)i;
			for (j = 0; j < D; ++j)
				g[i][j] = (T)((mRnd.getInt() % (SampleSize + SampleSize)) - SampleSize) / SampleSize;
			g[i].normalize();
		}

		while (--i)
		{
			k    = p[i];
			j = (unsigned)mRnd.getInt() % SampleSize;
			p[i] = p[j];
			p[j] = k;
		}

		for (i = 0 ; i < SampleSize + 2; ++i)
		{
			p [(unsigned)SampleSize + i] =  p[i];
			for (j = 0; j < D; ++j)
				g[(unsigned)SampleSize + i][j] = g[i][j];
		}

	}

	UserRandom& mRnd;
	int   mOctaves;
	T     mFrequency;
	T     mAmplitude;

	// Permutation vector
	int p[(unsigned)(SampleSize + SampleSize + 2)];
	// Gradient vector
	VecType g[(unsigned)(SampleSize + SampleSize + 2)];

	bool  mbInit;
};

}

#endif /* #ifndef WITHOUT_APEX_AUTHORING */

#endif /* #ifndef NOISE_H */

