/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


// This file exists primarily for APEX integration

#ifndef RT_DEF_H
#define RT_DEF_H

#include "ApexDefs.h"
#include "ModuleDestructible.h"

#if APEX_RUNTIME_FRACTURE
#define RT_COMPILE 1
#else
#define RT_COMPILE 0
#endif

#endif