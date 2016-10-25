/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


APEX_CUDA_TEXTURE_1D(texRefCompactScanSum, unsigned int)

APEX_CUDA_BOUND_KERNEL(COMPACT_KERNEL_CONFIG, compactKernel,
                       ((unsigned int, targetCount))((unsigned int, totalCount))((unsigned int, injectorCount))
                       ((APEX_MEM_BLOCK(unsigned int), g_outIndices))((APEX_MEM_BLOCK(unsigned int), g_outCount))((APEX_MEM_BLOCK(unsigned int), g_injCounters))
                      )
