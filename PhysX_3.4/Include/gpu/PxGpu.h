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

#ifndef PX_GPU_H
#define PX_GPU_H

#include "PxPhysXConfig.h"


#if PX_SUPPORT_GPU_PHYSX

#include "cudamanager/PxCudaContextManager.h"
#include "cudamanager/PxCudaMemoryManager.h"
#include "cudamanager/PxGpuCopyDesc.h"
#include "foundation/Px.h"
#include "foundation/PxPreprocessor.h"
#include "common/PxPhysXCommonConfig.h"
#include "foundation/PxFoundation.h"

/**
\brief PxGpuLoadHook

This is a helper class for loading the PhysXGpu dll. 
If a PhysXGpu dll with a non-default file name needs to be loaded, 
PxGpuLoadHook can be sub-classed to provide the custom filenames.

Once the names are set, the instance must be set for use by PhysX.dll using PxSetPhysXGpuLoadHook(), 

@see PxSetPhysXGpuLoadHook()
*/
class PxGpuLoadHook
{
public:
	PxGpuLoadHook() {}
	virtual ~PxGpuLoadHook() {}

	virtual const char* getPhysXGpuDEBUGDllName() const = 0;
	virtual const char* getPhysXGpuCHECKEDDllName() const = 0;
	virtual const char* getPhysXGpuPROFILEDllName() const = 0;
	virtual const char* getPhysXGpuDllName() const = 0;

protected:
private:
};

/**
\brief Sets GPU load hook instance for PhysX dll.

\param[in] hook GPU load hook.

@see PxGpuLoadHook
*/
PX_C_EXPORT PX_PHYSX_CORE_API void PX_CALL_CONV PxSetPhysXGpuLoadHook(const PxGpuLoadHook* hook);

/**
 * \brief Ask the NVIDIA control panel which GPU has been selected for use by
 * PhysX.  Returns -1 if no PhysX capable GPU is found or GPU PhysX has
 * been disabled.
 */
PX_C_EXPORT PX_PHYSX_CORE_API int PX_CALL_CONV PxGetSuggestedCudaDeviceOrdinal(physx::PxErrorCallback& errc);

/**
 * \brief Allocate a CUDA Context manager, complete with heaps and task dispatcher.
 * You only need one CUDA context manager per GPU device you intend to use for
 * CUDA tasks. 
 */
PX_C_EXPORT PX_PHYSX_CORE_API physx::PxCudaContextManager* PX_CALL_CONV PxCreateCudaContextManager(physx::PxFoundation& foundation, const physx::PxCudaContextManagerDesc& desc);

#endif // PX_SUPPORT_GPU_PHYSX

#endif // PX_GPU_H
