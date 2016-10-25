/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


APEX_CUDA_SYNC_KERNEL(SORT_KERNEL_CONFIG, radixSortSyncKernel, ((unsigned int, numElements))
                      ((unsigned int*, keys))((unsigned int*, values))
                      ((unsigned int*, tempKeys))((unsigned int*, tempValues))
                      ((unsigned int*, g_temp))((unsigned int, keyBits))((unsigned int, startbit))
                     )

APEX_CUDA_BOUND_KERNEL(SORT_KERNEL_CONFIG, radixSortStepKernel,
                       ((unsigned int*, keys))((unsigned int*, values))
                       ((unsigned int*, tempKeys))((unsigned int*, tempValues))
                       ((unsigned int*, g_temp))((unsigned int, startbit))
					   ((unsigned int, phase))((unsigned int, gridSize))
                      )
