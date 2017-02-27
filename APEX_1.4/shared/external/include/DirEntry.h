/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef DIR_ENTRY_INCLUDE_H
#define DIR_ENTRY_INCLUDE_H

#include "PxPreprocessor.h"

#if PX_WINDOWS_FAMILY
	#include "windows/DirEntry.h"
#elif PX_XBOXONE
	#include "xboxone/DirEntry.h"
#elif (PX_LINUX_FAMILY || PX_APPLE_FAMILY || PX_ANDROID)
	#include "linux/DirEntry.h"
#elif PX_PS4
	#include "ps4/DirEntry.h"
#elif PX_NX
	#include "nx/DirEntry.h" 
#else
	#error "Platform not supported!"
#endif

#endif // DIR_ENTRY_INCLUDE_H
