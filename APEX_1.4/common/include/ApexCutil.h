/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef APEX_CUTIL_H
#define APEX_CUTIL_H

#if defined(__CUDACC__)
# define CUT_SAFE_CALL(call)  call
# define CUT_CHECK_ERROR(errorMessage)
#else
# include "ApexSDKIntl.h"

# define CUT_SAFE_CALL(call)  { CUresult ret = call;                         \
		if( CUDA_SUCCESS != ret ) {                                              \
			APEX_INTERNAL_ERROR("Cuda Error %d", ret);                         \
			PX_ASSERT(!ret); } }

# if _DEBUG
#  define CUT_CHECK_ERROR(errorMessage)		                                 \
	if( CUDA_SUCCESS != cuCtxSynchronize() ) {                                \
		APEX_INTERNAL_ERROR(errorMessage);									 \
		PX_ASSERT(0); }
# else
#  define CUT_CHECK_ERROR(errorMessage)
# endif

#endif

#define CUT_TODEVICE(gpuptr) (CUdeviceptr)(size_t)(gpuptr)

#endif
