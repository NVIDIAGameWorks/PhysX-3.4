/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __NOISE_FS_COMMON_H__
#define __NOISE_FS_COMMON_H__

#include "../../fieldsampler/include/FieldSamplerCommon.h"
#include "SimplexNoise.h"

namespace nvidia
{
namespace basicfs
{

struct NoiseType
{
	enum Enum
	{
		SIMPLEX,
		CURL
	};
};

//struct NoiseFSParams
#define INPLACE_TYPE_STRUCT_NAME NoiseFSParams
#define INPLACE_TYPE_STRUCT_FIELDS \
	INPLACE_TYPE_FIELD(float,			noiseStrength) \
	INPLACE_TYPE_FIELD(PxVec3,			noiseSpaceFreq) \
	INPLACE_TYPE_FIELD(float,			noiseTimeFreq) \
	INPLACE_TYPE_FIELD(uint32_t,			noiseOctaves) \
	INPLACE_TYPE_FIELD(float,			noiseStrengthOctaveMultiplier) \
	INPLACE_TYPE_FIELD(PxVec3,			noiseSpaceFreqOctaveMultiplier) \
	INPLACE_TYPE_FIELD(float,			noiseTimeFreqOctaveMultiplier) \
	INPLACE_TYPE_FIELD(uint32_t,			noiseType) \
	INPLACE_TYPE_FIELD(uint32_t,			noiseSeed) \
	INPLACE_TYPE_FIELD(PxTransform,	worldToShape) \
	INPLACE_TYPE_FIELD(InplaceBool,				useLocalSpace)
#include INPLACE_TYPE_BUILD()


APEX_CUDA_CALLABLE PX_INLINE PxVec3 evalNoise(const NoiseFSParams& params, const PxVec3& pos, uint32_t totalElapsedMS)
{
	PxVec3 point;
	if (params.useLocalSpace)
	{
		const PxVec3 posInShape = params.worldToShape.transform(pos);
		point = PxVec3(params.noiseSpaceFreq.x * posInShape.x, params.noiseSpaceFreq.y * posInShape.y, params.noiseSpaceFreq.z * posInShape.z);
	}
	else
	{
		point = PxVec3(params.noiseSpaceFreq.x * pos.x, params.noiseSpaceFreq.y * pos.y, params.noiseSpaceFreq.z * pos.z);
	}
	float time = params.noiseTimeFreq * (totalElapsedMS * 1e-3f);

	PxVec3 result;
	if (params.noiseType == NoiseType::CURL)
	{
		PxVec4 dFx;
		dFx.setZero();
		PxVec4 dFy;
		dFy.setZero();
		PxVec4 dFz;
		dFz.setZero();
		float amp = 1.0f;
		int seed = (int)params.noiseSeed;
		for (uint32_t i = 0; i < params.noiseOctaves; ++i)
		{
			dFx += amp * SimplexNoise::eval4D(point.x, point.y, point.z, time, ++seed);
			dFy += amp * SimplexNoise::eval4D(point.x, point.y, point.z, time, ++seed);
			dFz += amp * SimplexNoise::eval4D(point.x, point.y, point.z, time, ++seed);

			amp *= params.noiseStrengthOctaveMultiplier;
			point.x *= params.noiseSpaceFreqOctaveMultiplier.x;
			point.y *= params.noiseSpaceFreqOctaveMultiplier.y;
			point.z *= params.noiseSpaceFreqOctaveMultiplier.z;
			time *= params.noiseTimeFreqOctaveMultiplier;
		}
		//build curl noise as a result
		result.x = dFz.y - dFy.z;
		result.y = dFx.z - dFz.x;
		result.z = dFy.x - dFx.y;
	}
	else
	{
		PxVec4 noise;
		noise.setZero();
		float amp = 1.0f;
		int seed = (int)params.noiseSeed;
		for (uint32_t i = 0; i < params.noiseOctaves; ++i)
		{
			noise += amp * SimplexNoise::eval4D(point.x, point.y, point.z, time, ++seed);

			amp *= params.noiseStrengthOctaveMultiplier;
			point.x *= params.noiseSpaceFreqOctaveMultiplier.x;
			point.y *= params.noiseSpaceFreqOctaveMultiplier.y;
			point.z *= params.noiseSpaceFreqOctaveMultiplier.z;
			time *= params.noiseTimeFreqOctaveMultiplier;
		}
		//get noise gradient as a result
		result = noise.getXYZ();
	}
	result *= params.noiseStrength;
	return result;
}

APEX_CUDA_CALLABLE PX_INLINE PxVec3 executeNoiseFS_GRID(const NoiseFSParams& params, const PxVec3& pos, uint32_t totalElapsedMS)
{
	return evalNoise(params, pos, totalElapsedMS);
}


APEX_CUDA_CALLABLE PX_INLINE PxVec3 executeNoiseFS(const NoiseFSParams& params, const PxVec3& pos, uint32_t totalElapsedMS)
{
	return evalNoise(params, pos, totalElapsedMS);
}

}
} // namespace nvidia

#endif
