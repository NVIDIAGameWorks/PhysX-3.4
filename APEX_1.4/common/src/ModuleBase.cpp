/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "Apex.h"
#include "ApexSharedUtils.h"
#include "PsUserAllocated.h"
#include "ProfilerCallback.h"
#include "ModuleBase.h"

#ifdef PHYSX_PROFILE_SDK

#if PX_WINDOWS_FAMILY
nvidia::profile::PxProfileZone *gProfileZone=NULL;
#endif

#endif

namespace nvidia
{
namespace apex
{

ModuleBase::ModuleBase() :
	mSdk(0),
	mApiProxy(0)
{
}

const char* ModuleBase::getName() const
{
	return mName.c_str();
}

void ModuleBase::release()
{
	GetApexSDK()->releaseModule(mApiProxy);
}

void ModuleBase::destroy()
{
}

}
} // end namespace nvidia::apex
