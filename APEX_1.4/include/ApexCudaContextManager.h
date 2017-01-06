/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef APEX_CUDA_CONTEXT_MANAGER_H
#define APEX_CUDA_CONTEXT_MANAGER_H

/**
\file
\brief The file contains API for PxCudaConextManager public functions
*/

#include "foundation/Px.h"
#include "ApexSDK.h"

namespace physx
{
	class PxCudaContextManager;
	class PxCudaContextManagerDesc;
}

namespace nvidia
{
namespace apex
{

/**
 * \brief Ask the NVIDIA control panel which GPU has been selected for use by
 * PhysX.  Returns -1 if no PhysX capable GPU is found or GPU PhysX has
 * been disabled.
 */
APEX_API int CALL_CONV GetSuggestedCudaDeviceOrdinal(PxErrorCallback& errc);

/**
 * \brief Allocate a CUDA Context manager, complete with heaps and task dispatcher.
 * You only need one CUDA context manager per GPU device you intend to use for
 * CUDA tasks.  If mgr is NULL, no profiling of CUDA code will be possible.
 */
APEX_API PxCudaContextManager*	CALL_CONV CreateCudaContextManager(
	const PxCudaContextManagerDesc& desc, PxErrorCallback& errorCallback);
}
}

#endif // APEX_CUDA_CONTEXT_MANAGER_H
