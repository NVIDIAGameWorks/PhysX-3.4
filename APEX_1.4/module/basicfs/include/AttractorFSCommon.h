/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __ATTRACTOR_FS_COMMON_SRC_H__
#define __ATTRACTOR_FS_COMMON_SRC_H__

#include "../../fieldsampler/include/FieldSamplerCommon.h"

namespace nvidia
{
namespace basicfs
{

//struct AttractorFSParams
#define INPLACE_TYPE_STRUCT_NAME AttractorFSParams
#define INPLACE_TYPE_STRUCT_FIELDS \
	INPLACE_TYPE_FIELD(PxVec3,	origin) \
	INPLACE_TYPE_FIELD(float,	radius) \
	INPLACE_TYPE_FIELD(float,	constFieldStrength) \
	INPLACE_TYPE_FIELD(float,	variableFieldStrength)
#include INPLACE_TYPE_BUILD()


PX_CUDA_CALLABLE PX_INLINE PxVec3 commonAttractorFSKernel(const AttractorFSParams& params, const PxVec3& pos)
{
	PxVec3 dir = params.origin - pos;
	PX_ASSERT(params.radius);
	float dist = dir.magnitude() / params.radius;

	float result = params.constFieldStrength;
	if (dist >= 0.4)
	{
		result += params.variableFieldStrength / dist;
	}

	return result * dir.getNormalized();
}

PX_CUDA_CALLABLE PX_INLINE PxVec3 executeAttractorFS(const AttractorFSParams& params, const PxVec3& pos/*, uint32_t totalElapsedMS*/)
{
	PxVec3 dir = params.origin - pos;
	PX_ASSERT(params.radius);
	float dist = dir.magnitude() / params.radius;

	float result = params.constFieldStrength;
	if (dist >= 0.4)
	{
		result += params.variableFieldStrength / dist;
	}

	return result * dir.getNormalized();
}

}
} // namespace nvidia

#endif
