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

#include "Factory.h"
#include "Allocator.h"

#if PX_WINDOWS_FAMILY

/**
Create CuFactory interface class.  This is defined so the CUDA cloth solver can be isolated in its own DLL
*/
PX_C_EXPORT __declspec(dllexport) nvidia::cloth::CuFactory* PX_CALL_CONV PxCreateCuFactory(physx::PxCudaContextManager* contextManager);

#endif //PX_WINDOWS_FAMILY
