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

#include "PhysXIndicator.h"
#include "AgMMFile.h"

#include <windows.h>
#if _MSC_VER >= 1800
#include <VersionHelpers.h>	// for IsWindowsVistaOrGreater
#endif
#include <stdio.h>

using namespace nvidia;
using namespace physx;

// Scope-based to indicate to NV driver that CPU PhysX is happening
PhysXCpuIndicator::PhysXCpuIndicator() :
	mPhysXDataPtr(NULL)
{
	bool alreadyExists = false;
	
	mPhysXDataPtr = (physx::NvPhysXToDrv_Data_V1*)PhysXCpuIndicator::createIndicatorBlock(mSharedMemConfig, alreadyExists);
	
	if (!mPhysXDataPtr)
	{
		return;
	}
	
	if (!alreadyExists)
	{
		mPhysXDataPtr->bGpuPhysicsPresent = 0;
		mPhysXDataPtr->bCpuPhysicsPresent = 1;
	}
	else
	{
		mPhysXDataPtr->bCpuPhysicsPresent++;
	}

	// init header last to prevent race conditions
	// this must be done because the driver may have already created the shared memory block,
	// thus alreadyExists may be true, even if PhysX hasn't been initialized
	NvPhysXToDrv_Header_Init(mPhysXDataPtr->header);
}

PhysXCpuIndicator::~PhysXCpuIndicator()
{
	if (!mPhysXDataPtr)
	{
		return;
	}
	
	mPhysXDataPtr->bCpuPhysicsPresent--;
	
	mPhysXDataPtr = NULL;
	mSharedMemConfig.destroy();
}

void* PhysXCpuIndicator::createIndicatorBlock(AgMMFile &mmfile, bool &alreadyExists)
{
	char configName[128];

    // Get the windows version (we can only create Global\\ namespace objects in XP)
#if _MSC_VER >= 1800
	// Windows 8.1 SDK, which comes with VS2013, deprecated the GetVersionEx function
	// Windows 8.1 SDK added the IsWindowsVistaOrGreater helper function which we use instead
	BOOL bIsVistaOrGreater = IsWindowsVistaOrGreater();
#else
	OSVERSIONINFOEX windowsVersionInfo;

	/**
		Operating system		Version number
		----------------		--------------
		Windows 7				6.1
		Windows Server 2008 R2	6.1
		Windows Server 2008		6.0
		Windows Vista			6.0
		Windows Server 2003 R2	5.2
		Windows Server 2003		5.2
		Windows XP				5.1
		Windows 2000			5.0
	**/
	windowsVersionInfo.dwOSVersionInfoSize = sizeof(windowsVersionInfo);
	GetVersionEx((LPOSVERSIONINFO)&windowsVersionInfo);

	bool bIsVistaOrGreater = (windowsVersionInfo.dwMajorVersion >= 6);
#endif

	if (bIsVistaOrGreater)
	{
		NvPhysXToDrv_Build_SectionName((uint32_t)GetCurrentProcessId(), configName);
	}
	else
	{
		NvPhysXToDrv_Build_SectionNameXP((uint32_t)GetCurrentProcessId(), configName);
	}

	mmfile.create(configName, sizeof(physx::NvPhysXToDrv_Data_V1), alreadyExists);

	return mmfile.getAddr();
}

//-----------------------------------------------------------------------------------------------------------

PhysXGpuIndicator::PhysXGpuIndicator() :
	mPhysXDataPtr(NULL),
	mAlreadyExists(false),
	mGpuTrigger(false)
{
	mPhysXDataPtr = (physx::NvPhysXToDrv_Data_V1*)PhysXCpuIndicator::createIndicatorBlock(mSharedMemConfig, mAlreadyExists);
	
	// init header last to prevent race conditions
	// this must be done because the driver may have already created the shared memory block,
	// thus alreadyExists may be true, even if PhysX hasn't been initialized
	NvPhysXToDrv_Header_Init(mPhysXDataPtr->header);
}

PhysXGpuIndicator::~PhysXGpuIndicator()
{
	gpuOff();
	
	mPhysXDataPtr = NULL;
	mSharedMemConfig.destroy();
}

// Explicit set functions to indicate to NV driver that GPU PhysX is happening
void PhysXGpuIndicator::gpuOn()
{
	if (!mPhysXDataPtr || mGpuTrigger)
	{
		return;
	}

	if (!mAlreadyExists)
	{
		mPhysXDataPtr->bGpuPhysicsPresent = 1;
		mPhysXDataPtr->bCpuPhysicsPresent = 0;
	}
	else
	{
		mPhysXDataPtr->bGpuPhysicsPresent++;
	}

	mGpuTrigger = true;
}

void PhysXGpuIndicator::gpuOff()
{
	if (!mPhysXDataPtr || !mGpuTrigger)
	{
		return;
	}

	mPhysXDataPtr->bGpuPhysicsPresent--;

	mGpuTrigger = false;
}
