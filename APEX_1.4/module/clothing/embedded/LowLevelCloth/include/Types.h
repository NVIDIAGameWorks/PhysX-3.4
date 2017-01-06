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

#ifndef __CUDACC__
#include "ApexUsingNamespace.h"
#include "Px.h"
#include "PxVec3.h"
#include "PxVec4.h"
#include "PxQuat.h"
#endif

// Factory.cpp gets included in both PhysXGPU and LowLevelCloth projects
// CuFactory can only be created in PhysXGPU project
// DxFactory can only be created in PhysXGPU (win) or LowLevelCloth (xbox1)
#if defined(PX_PHYSX_GPU_EXPORTS) || PX_XBOXONE
#define ENABLE_CUFACTORY ((PX_WINDOWS_FAMILY && (PX_WINRT==0)) || PX_LINUX)

//TEMPORARY DISABLE DXFACTORY
#define ENABLE_DXFACTORY 0
//#define ENABLE_DXFACTORY ((PX_WINDOWS_FAMILY && (PX_WINRT==0)) || PX_XBOXONE)
#else
#define ENABLE_CUFACTORY 0
#define ENABLE_DXFACTORY 0
#endif

#ifndef _MSC_VER
#include <stdint.h>
#else
// typedef standard integer types
typedef unsigned __int8 uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;
typedef __int16 int16_t;
typedef __int32 int32_t;
#if _MSC_VER < 1600
#define nullptr NULL
#endif
#endif
