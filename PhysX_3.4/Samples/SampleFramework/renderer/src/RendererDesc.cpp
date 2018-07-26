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
// Copyright (c) 2008-2018 NVIDIA Corporation. All rights reserved.
#include <RendererDesc.h>

using namespace SampleRenderer;

RendererDesc::RendererDesc(void)
{
#if defined(RENDERER_ENABLE_OPENGL)
	driver = Renderer::DRIVER_OPENGL;
#elif defined(RENDERER_ENABLE_GLES2)
	driver = Renderer::DRIVER_GLES2;
#elif defined(RENDERER_ENABLE_DIRECT3D9)
	driver = Renderer::DRIVER_DIRECT3D9;
#elif defined(RENDERER_ENABLE_DIRECT3D11)
	driver = Renderer::DRIVER_DIRECT3D11;
#elif defined(RENDERER_ENABLE_LIBGCM)
	driver = Renderer::DRIVER_LIBGCM;
#elif defined(RENDERER_ENABLE_LIBGNM)
	driver = Renderer::DRIVER_LIBGNM;
#elif defined(RENDERER_ENABLE_GX2)
	driver = Renderer::DRIVER_NULL;
#else
#error "No Renderer Drivers support!"
#endif
	windowHandle   = 0;

	errorCallback = 0;

	vsync = false;

	multipassDepthBias = false;
}

bool RendererDesc::isValid(void) const
{
	bool ok = true;
#if defined(RENDERER_WINDOWS)
	if(!windowHandle) ok = false;
#endif
	return ok;
}
