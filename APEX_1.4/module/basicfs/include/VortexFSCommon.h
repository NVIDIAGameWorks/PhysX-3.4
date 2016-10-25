/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __VORTEX_FS_COMMON_SRC_H__
#define __VORTEX_FS_COMMON_SRC_H__

#include "../../fieldsampler/include/FieldSamplerCommon.h"

namespace nvidia
{
namespace basicfs
{

//struct VortexFSParams
#define INPLACE_TYPE_STRUCT_NAME VortexFSParams
#define INPLACE_TYPE_STRUCT_FIELDS \
	INPLACE_TYPE_FIELD(PxTransform,			worldToDir) \
	INPLACE_TYPE_FIELD(InplaceBool,						bottomSphericalForce) \
	INPLACE_TYPE_FIELD(InplaceBool,						topSphericalForce) \
	INPLACE_TYPE_FIELD(float,					height) \
	INPLACE_TYPE_FIELD(float,					bottomRadius) \
	INPLACE_TYPE_FIELD(float,					topRadius) \
	INPLACE_TYPE_FIELD(float,					rotationalStrength) \
	INPLACE_TYPE_FIELD(float,					radialStrength) \
	INPLACE_TYPE_FIELD(float,					liftStrength)
#include INPLACE_TYPE_BUILD()


PX_CUDA_CALLABLE PX_INLINE float sqr(float x)
{
	return x * x;
}

/*
PX_CUDA_CALLABLE PX_INLINE PxVec3 executeVortexFS_GRID(const VortexFSParams& params)
{
	return params.worldToDir.M.multiplyByTranspose(PxVec3(0, params.strength, 0));
}*/

APEX_CUDA_CALLABLE PX_INLINE PxVec3 executeVortexFS(const VortexFSParams& params, const PxVec3& pos/*, uint32_t totalElapsedMS*/)
{
	PX_ASSERT(params.bottomRadius);
	PX_ASSERT(params.topRadius);
	
	PxVec3 result(PxZero);
	PxVec3 point = params.worldToDir.transform(pos);
	float R = PxSqrt(point.x * point.x + point.z * point.z);
	float invR = 1.f / R;
	float invRS = invR;
	float curR = 0;
	float h = params.height, r1 = params.bottomRadius, r2 = params.topRadius, y = point.y;

	if (y < h/2 && y > -h/2)
	{
		curR = r1 + (r2-r1) * (y / h + 0.5f);
	}
	else if (y <= -h/2 && y >= -h/2-r1)
	{
		curR = PxSqrt(r1*r1 - sqr(y+h/2));
		if (params.bottomSphericalForce)
		{
			float y = point.y + h/2;
			invRS = 1.f / PxSqrt(point.x * point.x + y * y + point.z * point.z);
			result.y = params.radialStrength * y;
		}
	}
	else if (y >= h/2 && y <= h/2+r2)
	{
		curR = PxSqrt(r2*r2 - sqr(y-h/2));
		if (params.topSphericalForce)
		{
			float y = point.y - h/2;
			invRS = 1.f / PxSqrt(point.x * point.x + y * y + point.z * point.z);
			result.y = params.radialStrength * y;
		}
	}

	if (curR > 0.f && R <= curR)
	{
		result.x += params.radialStrength * point.x * invRS - params.rotationalStrength * R / curR * point.z * invR;
		result.y += params.liftStrength;
		result.z += params.radialStrength * point.z * invRS + params.rotationalStrength * R / curR * point.x * invR;
	}

	return params.worldToDir.q.rotate(result);
}

}
} // namespace nvidia

#endif
