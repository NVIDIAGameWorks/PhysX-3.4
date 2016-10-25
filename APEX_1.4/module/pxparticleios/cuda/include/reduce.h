/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


APEX_CUDA_SYNC_KERNEL(REDUCE_KERNEL_CONFIG, reduceSyncKernel, ((unsigned int, count))
                      ((float*, g_benefit))((float4*, g_output))((unsigned int*, g_tmp))
                     )

APEX_CUDA_BOUND_KERNEL(REDUCE_KERNEL_CONFIG, reduceKernel,
                       ((APEX_MEM_BLOCK(float), g_benefit))((APEX_MEM_BLOCK(float4), g_output))((APEX_MEM_BLOCK(unsigned int), g_tmp))
					   ((unsigned int, phase))((unsigned int, gridSize))
                      )
