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

#ifndef PHYS_XINDICATOR_H
#define PHYS_XINDICATOR_H

#include "AgMMFile.h"
#include "nvPhysXtoDrv.h"

namespace nvidia
{
	class PhysXCpuIndicator
	{
	public:
		PhysXCpuIndicator();
		~PhysXCpuIndicator();

		static void* createIndicatorBlock(AgMMFile &mmfile, bool &alreadyExists);

	private:
		AgMMFile mSharedMemConfig;
		physx::NvPhysXToDrv_Data_V1* mPhysXDataPtr;
	};

	class PhysXGpuIndicator
	{
	public:
		PhysXGpuIndicator();
		~PhysXGpuIndicator();

		void gpuOn();
		void gpuOff();

	private:
		AgMMFile mSharedMemConfig;
		physx::NvPhysXToDrv_Data_V1* mPhysXDataPtr;
		bool mAlreadyExists;
		bool mGpuTrigger;

	};
}

#endif	// __PHYSXINDICATOR_H__
