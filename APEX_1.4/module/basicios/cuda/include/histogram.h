/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


APEX_CUDA_SYNC_KERNEL(HISTOGRAM_KERNEL_CONFIG, histogramSyncKernel, ((unsigned int, count))
                      ((const float*, g_data))((unsigned int, bound))
                      ((float, dataMin))((float, dataMax))((unsigned int*, g_boundParams))
                      ((unsigned int*, g_tmpHistograms))
                     )

APEX_CUDA_BOUND_KERNEL(HISTOGRAM_KERNEL_CONFIG, histogramKernel,
                       ((APEX_MEM_BLOCK(float), g_data))((unsigned int, bound))
                       ((float, dataMin))((float, dataMax))((APEX_MEM_BLOCK(unsigned int), g_boundParams))
                       ((APEX_MEM_BLOCK(unsigned int), g_tmpHistograms))
					   ((unsigned int, phase))((unsigned int, gridSize))
                      )
