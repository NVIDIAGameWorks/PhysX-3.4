/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

// Copyright (C) 2002-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (C) 2001-2006 NovodeX. All rights reserved.

#ifndef __MODULE_PROFILE_COMMON_H___
#define __MODULE_PROFILE_COMMON_H___

#ifndef MODULE_NAMESPACE
#error "MODULE_NAMESPACE must be defined before including ModuleProfileCommon.h"
#endif

#ifndef PHYSX_PROFILE_SDK
#define PX_DISABLE_USER_PROFILER_CALLBACK
#endif

#include "ProfilerCallback.h"

namespace nvidia
{
namespace apex
{
class ApexSDKIntl;
}
}

namespace MODULE_NAMESPACE
{

}

#endif
