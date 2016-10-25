/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


APEX_CUDA_TEXTURE_1D(texRefBBoxPositions, float4)

APEX_CUDA_SYNC_KERNEL(BBOX_KERNEL_CONFIG, bboxSyncKernel, ((unsigned int, count))
                      ((unsigned int*, g_actorID))
                      ((unsigned int*, stateToInput))
                      ((const float4*, g_positionMass))
                      ((float4*, g_outMinBounds))((float4*, g_outMaxBounds))
                      ((unsigned int*, g_tmpActorID))
                      ((float4*, g_tmpMinBounds))((float4*, g_tmpMaxBounds))
                     )

APEX_CUDA_BOUND_KERNEL(BBOX_KERNEL_CONFIG, bboxKernel,
                       ((unsigned int*, g_actorID))
                       ((unsigned int*, stateToInput))
                       ((const float4*, g_positionMass))
                       ((float4*, g_outMinBounds))((float4*, g_outMaxBounds))
                       ((unsigned int*, g_tmpActorID))
                       ((float4*, g_tmpMinBounds))((float4*, g_tmpMaxBounds))
					   ((unsigned int, phase))((unsigned int, gridSize))
                     )
