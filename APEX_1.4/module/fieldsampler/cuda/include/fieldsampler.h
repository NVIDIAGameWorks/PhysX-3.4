/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


APEX_CUDA_SURFACE_3D(surfRefGridAccum)


APEX_CUDA_BOUND_KERNEL(APEX_CUDA_KERNEL_DEFAULT_CONFIG, clearKernel,
                       ((APEX_MEM_BLOCK(float4), g_accumField))((APEX_MEM_BLOCK(float4), g_accumVelocity))
                      )

APEX_CUDA_BOUND_KERNEL(APEX_CUDA_KERNEL_DEFAULT_CONFIG, composeKernel,
                       ((APEX_MEM_BLOCK(float4), g_accumField))((APEX_MEM_BLOCK(const float4), g_accumVelocity))((APEX_MEM_BLOCK(const float4), g_velocity))((float, timestep))
                      )

APEX_CUDA_FREE_KERNEL_3D(APEX_CUDA_KERNEL_DEFAULT_CONFIG, clearGridKernel,
                       ((uint32_t, numX))((uint32_t, numY))((uint32_t, numZ))
                      )

APEX_CUDA_BOUND_KERNEL(APEX_CUDA_KERNEL_DEFAULT_CONFIG, applyParticlesKernel,
                       ((float4*, g_velocity))((const float4*, g_outField))
                      )

#ifdef APEX_TEST

APEX_CUDA_BOUND_KERNEL((), testParticleKernel,
                       ((float4*, g_position))((float4*, g_velocity))
					   ((uint32_t*, g_flag))
					   ((const float4*, g_initPosition))((const float4*, g_initVelocity))
                      )

#endif