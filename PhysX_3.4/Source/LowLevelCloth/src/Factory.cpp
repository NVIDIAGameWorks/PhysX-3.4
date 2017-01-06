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

#include "SwFactory.h"
#include "PxPhysXConfig.h"

// Factory.cpp gets included in both PhysXGPU and LowLevelCloth projects
// CuFactory can only be created in PhysXGPU project
#if defined(PX_PHYSX_GPU_EXPORTS) || PX_XBOXONE
#define ENABLE_CUFACTORY PX_SUPPORT_GPU_PHYSX
#else
#define ENABLE_CUFACTORY 0
#endif

#if ENABLE_CUFACTORY
#include "CuFactory.h"
#endif

namespace physx
{
namespace cloth
{
uint32_t getNextFabricId()
{
	static uint32_t sNextFabricId = 0;
	return sNextFabricId++;
}
}
}

using namespace physx;

cloth::Factory* cloth::Factory::createFactory(Platform platform, void* contextManager)
{
	PX_UNUSED(contextManager);

	if(platform == Factory::CPU)
		return new SwFactory;

#if ENABLE_CUFACTORY
	if(platform == Factory::CUDA)
		return new CuFactory((physx::PxCudaContextManager*)contextManager);
#endif
	return 0;
}
