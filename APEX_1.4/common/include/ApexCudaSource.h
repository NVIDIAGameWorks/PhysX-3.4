/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __APEX_CUDA_SOURCE_H__
#define __APEX_CUDA_SOURCE_H__


#undef APEX_CUDA_TEXTURE_1D
#undef APEX_CUDA_TEXTURE_2D
#undef APEX_CUDA_TEXTURE_3D
#undef APEX_CUDA_TEXTURE_3D_FILTER
#undef APEX_CUDA_SURFACE_1D
#undef APEX_CUDA_SURFACE_2D
#undef APEX_CUDA_SURFACE_3D
#undef APEX_CUDA_STORAGE_SIZE
#undef APEX_CUDA_FREE_KERNEL
#undef APEX_CUDA_FREE_KERNEL_2D
#undef APEX_CUDA_FREE_KERNEL_3D
#undef APEX_CUDA_SYNC_KERNEL
#undef APEX_CUDA_BOUND_KERNEL

#define __APEX_CUDA_OBJ(name) initCudaObj( APEX_CUDA_OBJ_NAME(name) );

#define APEX_CUDA_TEXTURE_1D(name, format) __APEX_CUDA_OBJ(name)
#define APEX_CUDA_TEXTURE_2D(name, format) __APEX_CUDA_OBJ(name)
#define APEX_CUDA_TEXTURE_3D(name, format) __APEX_CUDA_OBJ(name)
#define APEX_CUDA_TEXTURE_3D_FILTER(name, format, filter) __APEX_CUDA_OBJ(name)

#define APEX_CUDA_SURFACE_1D(name) __APEX_CUDA_OBJ(name)
#define APEX_CUDA_SURFACE_2D(name) __APEX_CUDA_OBJ(name)
#define APEX_CUDA_SURFACE_3D(name) __APEX_CUDA_OBJ(name)

#define APEX_CUDA_STORAGE_SIZE(name, size) __APEX_CUDA_OBJ(name)

#define APEX_CUDA_FREE_KERNEL(warps, name, argseq) __APEX_CUDA_OBJ(name)
#define APEX_CUDA_FREE_KERNEL_2D(warps, name, argseq) __APEX_CUDA_OBJ(name)
#define APEX_CUDA_FREE_KERNEL_3D(warps, name, argseq) __APEX_CUDA_OBJ(name)
#define APEX_CUDA_SYNC_KERNEL(warps, name, argseq) __APEX_CUDA_OBJ(name)
#define APEX_CUDA_BOUND_KERNEL(warps, name, argseq) __APEX_CUDA_OBJ(name)


#endif //__APEX_CUDA_SOURCE_H__
