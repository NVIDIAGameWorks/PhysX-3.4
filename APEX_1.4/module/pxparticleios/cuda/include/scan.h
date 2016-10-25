/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


APEX_CUDA_SYNC_KERNEL(SCAN_KERNEL_CONFIG, scanSyncKernel, ((unsigned int, count))
                      ((float, benefitMin))((float, benefitMax))
                      ((unsigned int*, g_indices))((const float*, g_benefits))
                      ((unsigned int*, g_boundParams))
                      ((unsigned int*, g_tmpCounts))((unsigned int*, g_tmpCounts1))
                     )

APEX_CUDA_BOUND_KERNEL(SCAN_KERNEL_CONFIG, scanKernel,
                       ((float, benefitMin))((float, benefitMax))
                       ((APEX_MEM_BLOCK(unsigned int), g_indices))((APEX_MEM_BLOCK(float), g_benefits))
                       ((APEX_MEM_BLOCK(unsigned int), g_boundParams))
                       ((APEX_MEM_BLOCK(unsigned int), g_tmpCounts))((APEX_MEM_BLOCK(unsigned int), g_tmpCounts1))
					   ((unsigned int, phase))((unsigned int, gridSize))
                      )
