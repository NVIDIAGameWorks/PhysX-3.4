/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef PHYSX_SDK_VERSION_H
#define PHYSX_SDK_VERSION_H

/*!
\file
\brief Version identifiers for PhysX major release 3

	This file is intended to be included by ApexDefs.h
*/

/// helper  Version number, this number should not be trusted for differentiating between different PhysX 3 versions
#define SDK_VERSION_NUMBER 300

/// Major version of PhysX
#define PX_PHYSICS_VERSION_MAJOR 3
/// Minor version of PhysX
#define PX_PHYSICS_VERSION_MINOR 4
/// Bugfix version of PhysX
#define PX_PHYSICS_VERSION_BUGFIX 0

/**
The constant PX_PHYSICS_VERSION is used when creating certain PhysX module objects.
This is to ensure that the application is using the same header version as the library was built with.
*/
#define PX_PHYSICS_VERSION ((PX_PHYSICS_VERSION_MAJOR<<24) + (PX_PHYSICS_VERSION_MINOR<<16) + (PX_PHYSICS_VERSION_BUGFIX<<8) + 0)

#endif // PHYSX_SDK_VERSION_H
