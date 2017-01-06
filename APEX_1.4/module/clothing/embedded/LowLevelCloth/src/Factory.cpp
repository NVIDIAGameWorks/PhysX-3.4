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

#include "SwFactory.h"

#if ENABLE_CUFACTORY
#include "CuFactory.h"
#endif

#if ENABLE_DXFACTORY
#include "windows/DxFactory.h"
//#include "PxGraphicsContextManager.h"
#pragma warning(disable : 4668 4917 4365 4061 4005)
#if PX_XBOXONE
#include <d3d11_x.h>
#else
#include <d3d11.h>
#endif
#endif

namespace nvidia
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

using namespace nvidia;

cloth::Factory* cloth::Factory::createFactory(Platform platform, void* contextManager)
{
	PX_UNUSED(contextManager);

	if(platform == Factory::CPU)
		return new SwFactory;

#if ENABLE_CUFACTORY
	if(platform == Factory::CUDA)
		return new CuFactory((PxCudaContextManager*)contextManager);
#endif

#if ENABLE_DXFACTORY
	if(platform == Factory::DirectCompute)
	{
		//physx::PxGraphicsContextManager* graphicsContextManager = (physx::PxGraphicsContextManager*)contextManager;
		//if(graphicsContextManager->getDevice()->GetFeatureLevel() >= D3D_FEATURE_LEVEL_11_0)
		//	return new DxFactory(graphicsContextManager);
	}
#endif

	return 0;
}
