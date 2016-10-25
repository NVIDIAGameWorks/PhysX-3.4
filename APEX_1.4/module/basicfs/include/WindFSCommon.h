/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __WIND_FS_COMMON_H__
#define __WIND_FS_COMMON_H__

#include "../../fieldsampler/include/FieldSamplerCommon.h"
#include "SimplexNoise.h"

namespace nvidia
{
namespace basicfs
{

//struct WindFSParams
#define INPLACE_TYPE_STRUCT_NAME WindFSParams
#define INPLACE_TYPE_STRUCT_FIELDS \
	INPLACE_TYPE_FIELD(PxVec3, fieldValue)
#include INPLACE_TYPE_BUILD()


PX_CUDA_CALLABLE PX_INLINE PxVec3 evalWind(const WindFSParams& params)
{
	return params.fieldValue;
}

PX_CUDA_CALLABLE PX_INLINE PxVec3 executeWindFS(const WindFSParams& params, const PxVec3& )
{
	return evalWind(params);
}

}
} // namespace nvidia

#endif
