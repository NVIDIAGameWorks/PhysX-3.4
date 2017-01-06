// This code contains NVIDIA Confidential Information and is disclosed to you
// under a form of NVIDIA software license agreement provided separately to you.
//
// Notice
// NVIDIA Corporation and its licensors retain all intellectual property and
// proprietary rights in and to this software and related documentation and
// any modifications thereto. Any use, reproduction, disclosure, or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA Corporation is strictly prohibited.
//
// ALL NVIDIA DESIGN SPECIFICATIONS, CODE ARE PROVIDED "AS IS.". NVIDIA MAKES
// NO WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES OF NONINFRINGEMENT,
// MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Information and code furnished is believed to be accurate and reliable.
// However, NVIDIA Corporation assumes no responsibility for the consequences of use of such
// information or for any infringement of patents or other rights of third parties that may
// result from its use. No license is granted by implication or otherwise under any patent
// or patent rights of NVIDIA Corporation. Details are subject to change without notice.
// This code supersedes and replaces all information previously supplied.
// NVIDIA Corporation products are not authorized for use as critical
// components in life support devices or systems without express written approval of
// NVIDIA Corporation.
//
// Copyright (c) 2008-2017 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  


#ifndef PX_PHYSX_INDICATOR_DEVICE_EXCLUSIVE_H
#define PX_PHYSX_INDICATOR_DEVICE_EXCLUSIVE_H

#include "PxPhysXCommonConfig.h"

namespace physx
{

/**
\brief API for gpu specific PhysX Indicator functionality.
*/
class PxPhysXIndicatorDeviceExclusive
{
public:

	/**
	\brief Register external Gpu client of PhysX Indicator.
	
	By calling this method, the PhysX Indicator will increment the number of external Cpu clients by one.

	\param[in] physics PxPhysics to register the client in.

	@see PxPhysXIndicatorDeviceExclusive.unregisterPhysXIndicatorGpuClient
	*/
	PX_PHYSX_CORE_API static void registerPhysXIndicatorGpuClient(class PxPhysics& physics);

	/**
	\brief Unregister external Gpu client of PhysX Indicator.
	
	By calling this method, the PhysX Indicator will decrement the number of external Cpu clients by one.

	\param[in] physics PxPhysics to unregister the client in.

	@see PxPhysXIndicatorDeviceExclusive.registerPhysXIndicatorGpuClient
	*/
	PX_PHYSX_CORE_API static void unregisterPhysXIndicatorGpuClient(class PxPhysics& physics);
};

}

#endif // PX_PHYSX_INDICATOR_DEVICE_EXCLUSIVE_H
