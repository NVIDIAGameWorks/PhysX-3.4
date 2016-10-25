/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


APEX_CUDA_BOUND_KERNEL(NEW_SORT_KERNEL_CONFIG, newRadixSortBlockKernel,
                       ((unsigned int, count))((unsigned int, bitCount))((unsigned int, startBit))
                       ((unsigned int*, inpKeys))((unsigned int*, inpValues))
                      )

APEX_CUDA_BOUND_KERNEL(NEW_SORT_KERNEL_CONFIG, newRadixSortStepKernel,
                       ((unsigned int, count))((unsigned int, startBit))
                       ((unsigned int*, inpKeys))((unsigned int*, inpValues))
                       ((unsigned int*, outKeys))((unsigned int*, outValues))
                       ((unsigned int*, tempScan))
					   ((unsigned int, phase))((unsigned int, gridSize))
                      )
