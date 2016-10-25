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

#define APEX_CUDA_MODULE_PREFIX BasicFS_

#include "ApexCuda.h"
#include "../include/JetFSCommon.h"
#include "../include/AttractorFSCommon.h"
#include "../include/VortexFSCommon.h"
#include "../include/NoiseFSCommon.h"
#include "../include/WindFSCommon.h"

#define FIELD_SAMPLER_POINTS_KERNEL_CONFIG ()
#define FIELD_SAMPLER_GRID_KERNEL_CONFIG ()

namespace nvidia
{
namespace apex
{
namespace basicfs
{


}
}
} // namespace nvidia

#endif
