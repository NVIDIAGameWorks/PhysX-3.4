/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __COMMON_H__
#define __COMMON_H__

#define APEX_CUDA_MODULE_PREFIX BasicIOS_

#include "ApexCuda.h"
#include "InplaceTypes.h"
#include "IofxManagerIntl.h"
#include <float.h>

#if PX_WINDOWS_FAMILY
#pragma warning(push)
#pragma warning(disable:4201)
#pragma warning(disable:4408)
#endif

#include <vector_types.h>

#if PX_WINDOWS_FAMILY
#pragma warning(pop)
#endif

const unsigned int HISTOGRAM_BIN_COUNT = 256;
const unsigned int HISTOGRAM_SIMULATE_BIN_COUNT = 512;	


#define COMPACT_KERNEL_CONFIG (0, WARP_SIZE * 3)
#define HISTOGRAM_KERNEL_CONFIG (0, HISTOGRAM_BIN_COUNT)
#define REDUCE_KERNEL_CONFIG (0, WARP_SIZE * 4)
#define SCAN_KERNEL_CONFIG (0, WARP_SIZE * 4)
#define SIMULATE_KERNEL_CONFIG (0, HISTOGRAM_SIMULATE_BIN_COUNT)


const unsigned int HOLE_SCAN_FLAG_BIT = 31;
const unsigned int HOLE_SCAN_FLAG = (1U << HOLE_SCAN_FLAG_BIT);
const unsigned int HOLE_SCAN_MASK = (HOLE_SCAN_FLAG - 1);

// mTmpOutput
const unsigned int STATUS_LAST_ACTIVE_COUNT		= 0;
const unsigned int STATUS_LAST_BENEFIT_SUM		= 1;
const unsigned int STATUS_LAST_BENEFIT_MIN		= 2;
const unsigned int STATUS_LAST_BENEFIT_MAX		= 3;

namespace nvidia
{
namespace basicios
{

}
} // namespace nvidia

#endif
