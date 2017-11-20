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

#include "PxPhysXConfig.h"

#if PX_SUPPORT_GPU_PHYSX

#include "foundation/Px.h"
#include "PsFoundation.h"
#include "PxPhysics.h"
#include "PxGpu.h"

#include "cudamanager/PxCudaContextManager.h"

namespace physx
{
	// alias shared foundation to something usable
	namespace Ps = shdfnd;
}

#define STRINGIFY(x) #x
#define GETSTRING(x) STRINGIFY(x)

#if PX_X86
#define PLATFORM_SUB_STR "x86"
#elif PX_X64
#define PLATFORM_SUB_STR "x64"
#endif

#if defined(PX_PHYSX_DLL_NAME_POSTFIX)
#define CONFIG_SUB_STR GETSTRING(PX_PHYSX_DLL_NAME_POSTFIX)
#else
#define CONFIG_SUB_STR
#endif

#if PX_WINDOWS

#include "windows/PsWindowsInclude.h"
#include "windows/CmWindowsModuleUpdateLoader.h"
static const char*	gPhysXGpuLibraryName = "PhysX3Gpu" CONFIG_SUB_STR "_" PLATFORM_SUB_STR ".dll";

#elif PX_LINUX

#include <dlfcn.h>
static const char*	gPhysXGpuLibraryName = "./libPhysX3Gpu" CONFIG_SUB_STR "_" PLATFORM_SUB_STR ".so";

#endif // PX_LINUX

#undef GETSTRING
#undef STRINGIFY

void PxSetPhysXGpuLoadHook(const PxGpuLoadHook* hook)
{
	if(strstr(gPhysXGpuLibraryName, "DEBUG"))
	{
		gPhysXGpuLibraryName = hook->getPhysXGpuDEBUGDllName();
		return;
	}

	if(strstr(gPhysXGpuLibraryName, "CHECKED"))
	{
		gPhysXGpuLibraryName = hook->getPhysXGpuCHECKEDDllName();
		return;
	}

	if(strstr(gPhysXGpuLibraryName, "PROFILE"))
	{
		gPhysXGpuLibraryName = hook->getPhysXGpuPROFILEDllName();
		return;
	}

	gPhysXGpuLibraryName = hook->getPhysXGpuDllName();
}

namespace physx
{
#if PX_VC
#pragma warning(disable: 4191)	//'operator/operation' : unsafe conversion from 'type of expression' to 'type required'
#endif

	class PxFoundation;
	class PxPhysXGpu;

	typedef physx::PxPhysXGpu* (PxCreatePhysXGpu_FUNC)();
	typedef physx::PxCudaContextManager* (PxCreateCudaContextManager_FUNC)(physx::PxFoundation& foundation, const physx::PxCudaContextManagerDesc& desc);
	typedef int (PxGetSuggestedCudaDeviceOrdinal_FUNC)(physx::PxErrorCallback& errc);

	PxCreatePhysXGpu_FUNC* g_PxCreatePhysXGpu_Func = NULL;
	PxCreateCudaContextManager_FUNC* g_PxCreateCudaContextManager_Func = NULL;
	PxGetSuggestedCudaDeviceOrdinal_FUNC* g_PxGetSuggestedCudaDeviceOrdinal_Func = NULL;

#if PX_WINDOWS

#define DEFAULT_PHYSX_GPU_GUID    "D79FA4BF-177C-4841-8091-4375D311D6A3"

	void PxLoadPhysxGPUModule(const char* appGUID)
	{
		static HMODULE s_library;

		if (s_library == NULL)
			s_library = GetModuleHandle(gPhysXGpuLibraryName);

		if (s_library == NULL)
		{
			Cm::CmModuleUpdateLoader moduleLoader(UPDATE_LOADER_DLL_NAME);
			s_library = moduleLoader.LoadModule(gPhysXGpuLibraryName, appGUID == NULL ? DEFAULT_PHYSX_GPU_GUID : appGUID);
		}

		if (s_library)
		{
			g_PxCreatePhysXGpu_Func = (PxCreatePhysXGpu_FUNC*)GetProcAddress(s_library, "PxCreatePhysXGpu");
			g_PxCreateCudaContextManager_Func = (PxCreateCudaContextManager_FUNC*)GetProcAddress(s_library, "PxCreateCudaContextManager");
			g_PxGetSuggestedCudaDeviceOrdinal_Func = (PxGetSuggestedCudaDeviceOrdinal_FUNC*)GetProcAddress(s_library, "PxGetSuggestedCudaDeviceOrdinal");
		}

		// Check for errors
		if (s_library == NULL)
			Ps::getFoundation().error(PxErrorCode::eINTERNAL_ERROR, __FILE__, __LINE__, "Failed to load PhysXGpu dll!");
		if (g_PxCreatePhysXGpu_Func == NULL || g_PxCreateCudaContextManager_Func == NULL || g_PxGetSuggestedCudaDeviceOrdinal_Func == NULL)
			Ps::getFoundation().error(PxErrorCode::eINTERNAL_ERROR, __FILE__, __LINE__, "PhysXGpu dll is incompatible with this version of PhysX!");
	}

#elif PX_LINUX

	void PxLoadPhysxGPUModule(const char*)
	{
		static void* s_library;

		if (s_library == NULL)
		{
			// load libcuda.so here since gcc configured with --as-needed won't link to it
			// if there is no call from the binary to it.
			void* hLibCuda = dlopen("libcuda.so", RTLD_NOW | RTLD_GLOBAL);
			if (hLibCuda)
			{
				s_library = dlopen(gPhysXGpuLibraryName, RTLD_NOW);
			}
		}

		// no UpdateLoader
		if (s_library)
		{
			*reinterpret_cast<void**>(&g_PxCreatePhysXGpu_Func) = dlsym(s_library, "PxCreatePhysXGpu");
			*reinterpret_cast<void**>(&g_PxCreateCudaContextManager_Func) = dlsym(s_library, "PxCreateCudaContextManager");
			*reinterpret_cast<void**>(&g_PxGetSuggestedCudaDeviceOrdinal_Func) = dlsym(s_library, "PxGetSuggestedCudaDeviceOrdinal");
		}

		// Check for errors
		if (s_library == NULL)
			Ps::getFoundation().error(PxErrorCode::eINTERNAL_ERROR, __FILE__, __LINE__, "Failed to load PhysXGpu so!");
		if (g_PxCreatePhysXGpu_Func == NULL || g_PxCreateCudaContextManager_Func == NULL || g_PxGetSuggestedCudaDeviceOrdinal_Func == NULL)
			Ps::getFoundation().error(PxErrorCode::eINTERNAL_ERROR, __FILE__, __LINE__, "PhysXGpu so is incompatible with this version of PhysX!");
	}

#endif // PX_LINUX

} // end physx namespace

#endif // PX_SUPPORT_GPU_PHYSX