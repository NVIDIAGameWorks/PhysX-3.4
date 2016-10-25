/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


APEX_CUDA_STORAGE(remapStorage)


APEX_CUDA_TEXTURE_1D(texRefRemapPositions,      float4)
APEX_CUDA_TEXTURE_1D(texRefRemapActorIDs,       unsigned int)
APEX_CUDA_TEXTURE_1D(texRefRemapInStateToInput, unsigned int)


APEX_CUDA_BOUND_KERNEL((), makeSortKeys,
                       ((const uint32_t*, inStateToInput))((uint32_t, maxInputID))
                       ((uint32_t, numActorsPerVolume))((uint32_t, numActorIDs))
                       ((InplaceHandle<ActorIDRemapArray>, actorIDRemapArrayHandle))
                       ((const float4*, positionMass))((bool, outputDensityKeys))
                       ((PxVec3, eyePos))((PxVec3, eyeDir))((float, zNear))
                       ((uint32_t*, sortKey))((uint32_t*, sortValue))
                      )

APEX_CUDA_BOUND_KERNEL((), remapKernel,
                       ((const uint32_t*, inStateToInput))((uint32_t, maxInputID))
                       ((uint32_t, numActorsPerVolume))((uint32_t, numActorIDs))
                       ((InplaceHandle<ActorIDRemapArray>, actorIDRemapArrayHandle))
                       ((const unsigned int*, inSortedValue))((unsigned int*, outSortKey))
                      )
