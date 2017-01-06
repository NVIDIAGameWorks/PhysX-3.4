/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef APEX_DEFS_H
#define APEX_DEFS_H

/*!
\file
\brief Version identifiers and other macro definitions

	This file is intended to be usable without picking up the entire
	public APEX API, so it explicitly does not include Apex.h
*/

#include "PhysXSDKVersion.h"

/*!
 \def APEX_SDK_VERSION
 \brief APEX Framework API version

 Used for making sure you are linking to the same version of the SDK files
 that you have included.  Should be incremented with every API change.

 \def APEX_SDK_RELEASE
 \brief APEX SDK Release version

 Used for conditionally compiling user code based on the APEX SDK release version.

 \def DYNAMIC_CAST
 \brief Determines use of dynamic_cast<> by APEX modules

 \def APEX_USE_PARTICLES
 \brief Determines use of particle-related APEX modules

 \def APEX_DEFAULT_NO_INTEROP_IMPLEMENTATION
 \brief Provide API stubs with no CUDA interop support

 Use this to add default implementations of interop-related interfaces for UserRenderer.
*/

#include "foundation/PxPreprocessor.h"

#define APEX_SDK_VERSION 1
#define APEX_SDK_RELEASE 0x01040000

#if USE_RTTI
#define DYNAMIC_CAST(type) dynamic_cast<type>
#else
#define DYNAMIC_CAST(type) static_cast<type>
#endif

/// Enables CUDA code
#if defined(EXCLUDE_CUDA) && (EXCLUDE_CUDA > 0)
#define APEX_CUDA_SUPPORT 0
#else
#define APEX_CUDA_SUPPORT (PX_SUPPORT_GPU_PHYSX) && !(PX_LINUX)
#endif

/// Enables particles related code
#if !defined(EXCLUDE_PARTICLES) && PX_WINDOWS
#define APEX_USE_PARTICLES 1
#else
#define APEX_USE_PARTICLES 0
#endif

/// Enables code specific for UE4
#ifndef APEX_UE4
#define APEX_UE4 0
#endif


#define APEX_DEFAULT_NO_INTEROP_IMPLEMENTATION 1


#endif // APEX_DEFS_H
