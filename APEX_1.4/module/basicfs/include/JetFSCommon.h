/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __JET_FS_COMMON_SRC_H__
#define __JET_FS_COMMON_SRC_H__

#include "../../fieldsampler/include/FieldSamplerCommon.h"
#include "SimplexNoise.h"

namespace nvidia
{
namespace basicfs
{

//struct JetFSParams
#define INPLACE_TYPE_STRUCT_NAME JetFSParams
#define INPLACE_TYPE_STRUCT_FIELDS \
	INPLACE_TYPE_FIELD(float,					strength) \
	INPLACE_TYPE_FIELD(float,					instStrength) \
	INPLACE_TYPE_FIELD(PxTransform,			worldToDir) \
	INPLACE_TYPE_FIELD(PxTransform,			worldToInstDir) \
	INPLACE_TYPE_FIELD(fieldsampler::FieldShapeParams,	gridIncludeShape) \
	INPLACE_TYPE_FIELD(float,					nearRadius) \
	INPLACE_TYPE_FIELD(float,					pivotRadius) \
	INPLACE_TYPE_FIELD(float,					farRadius) \
	INPLACE_TYPE_FIELD(float,					directionalStretch) \
	INPLACE_TYPE_FIELD(float,					averageStartDistance) \
	INPLACE_TYPE_FIELD(float,					averageEndDistance) \
	INPLACE_TYPE_FIELD(float,					pivotRatio) \
	INPLACE_TYPE_FIELD(float,					noiseStrength) \
	INPLACE_TYPE_FIELD(float,					noiseSpaceScale) \
	INPLACE_TYPE_FIELD(float,					noiseTimeScale) \
	INPLACE_TYPE_FIELD(uint32_t,					noiseOctaves)
#include INPLACE_TYPE_BUILD()


PX_CUDA_CALLABLE PX_INLINE float smoothstep(float x, float edge0, float edge1)
{
	//x should be >= 0
	x = (PxClamp(x, edge0, edge1) - edge0) / (edge1 - edge0);
	// Evaluate polynomial
	return x * x * (3 - 2 * x);
}

PX_CUDA_CALLABLE PX_INLINE float smoothstep1(float x, float edge)
{
	//x should be >= 0
	x = PxMin(x, edge) / edge;
	// Evaluate polynomial
	return x * x * (3 - 2 * x);
}

PX_CUDA_CALLABLE PX_INLINE PxVec3 executeJetFS_GRID(const JetFSParams& params)
{
	return params.worldToDir.q.rotate(PxVec3(0, params.strength, 0));
}

PX_CUDA_CALLABLE PX_INLINE PxVec3 evalToroidalField(const JetFSParams& params, const PxVec3& pos, const PxTransform& worldToDir, float strength0)
{
	PxVec3 point = worldToDir.transform(pos);

	float r = PxSqrt(point.x * point.x + point.z * point.z);
	float h = point.y / params.directionalStretch;

	float t;
	{
		const float r1 = r - params.pivotRadius;
		const float a = params.pivotRatio;
		const float b = (params.pivotRatio - 1) * r1;
		const float c = r1 * r1 + h * h;

		t = (PxSqrt(b * b + 4 * a * c) - b) / (2 * a);
	}

	const float r0 = params.pivotRadius + t * ((params.pivotRatio - 1) / 2);

	const float d = r0 - r;
	const float cosAngle = d / PxSqrt(d * d + h * h);
	const float angleLerp = (cosAngle + 1) * 0.5f;

	float rr = (r > 1e-10f) ? (1 / r) : 0;

	float xRatio = point.x * rr;
	float zRatio = point.z * rr;

	PxVec3 dir;
	dir.x = xRatio * h;
	dir.y = d * params.directionalStretch;
	dir.z = zRatio * h;

	dir.normalize();

	float strength = 0.0f;
	if (t <= params.pivotRadius)
	{
		strength = strength0 * smoothstep1(t, params.pivotRadius - params.nearRadius);

		strength *= (params.pivotRadius - t) * rr;
	}
	strength /= (angleLerp + params.pivotRatio * (1 - angleLerp));

	return strength * worldToDir.q.rotate(dir);
}

PX_CUDA_CALLABLE PX_INLINE PxVec3 executeJetFS(const JetFSParams& params, const PxVec3& pos, uint32_t totalElapsedMS)
{
	PxVec3 avgField = evalToroidalField(params, pos, params.worldToDir, params.strength);
	PxVec3 instField = evalToroidalField(params, pos, params.worldToInstDir, params.instStrength);

	float distance = (pos - params.worldToDir.p).magnitude();
	float lerpFactor = smoothstep(distance, params.averageStartDistance, params.averageEndDistance);
	PxVec3 result = lerpFactor * avgField + (1 - lerpFactor) * instField;

	if (params.noiseStrength > 0)
	{
		//add some noise
		PxVec3 point = params.noiseSpaceScale * (params.worldToDir.transform(pos));
		float time = (params.noiseTimeScale * 1e-3f) * totalElapsedMS;

		PxVec4 dFx;
		dFx.setZero();
		PxVec4 dFy;
		dFy.setZero();
		PxVec4 dFz;
		dFz.setZero();
		int seed = 0;
		float amp = 1.0f;
		for (uint32_t i = 0; i < params.noiseOctaves; ++i)
		{
			dFx += amp * SimplexNoise::eval4D(point.x, point.y, point.z, time, ++seed);
			dFy += amp * SimplexNoise::eval4D(point.x, point.y, point.z, time, ++seed);
			dFz += amp * SimplexNoise::eval4D(point.x, point.y, point.z, time, ++seed);

			point *= 2;
			time *= 2;
			amp *= 0.5f;
		}
		//get rotor
		PxVec3 rot;
		rot.x = dFz.y - dFy.z;
		rot.y = dFx.z - dFz.x;
		rot.z = dFy.x - dFx.y;

		result += params.noiseStrength * params.worldToDir.q.rotate(rot);
	}
	return result;
}

}
} // namespace nvidia

#endif
