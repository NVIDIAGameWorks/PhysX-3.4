/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


APEX_CUDA_STORAGE_SIZE(modifierStorage, MAX_CONST_MEM_SIZE - 32)


APEX_CUDA_TEXTURE_1D(texRefPositionMass,         float4)
APEX_CUDA_TEXTURE_1D(texRefVelocityLife,         float4)
APEX_CUDA_TEXTURE_1D(texRefCollisionNormalFlags, float4)
APEX_CUDA_TEXTURE_1D(texRefDensity,              float)
APEX_CUDA_TEXTURE_1D(texRefActorIDs,             unsigned int)
APEX_CUDA_TEXTURE_1D(texRefUserData,             unsigned int)

APEX_CUDA_TEXTURE_1D(texRefSpritePrivState0,     uint4)

APEX_CUDA_TEXTURE_1D(texRefMeshPrivState0,       uint4)
APEX_CUDA_TEXTURE_1D(texRefMeshPrivState1,       uint4)
APEX_CUDA_TEXTURE_1D(texRefMeshPrivState2,       uint4)

APEX_CUDA_TEXTURE_1D(texRefInStateToInput,       unsigned int)
APEX_CUDA_TEXTURE_1D(texRefStateSpawnSeed,       unsigned int)
APEX_CUDA_TEXTURE_1D(texRefStateSpawnScale,      float)

APEX_CUDA_TEXTURE_2D(texRefCurveSamples,         float)

#ifndef SKIP_OUTPUT_SURFACES_DEF
APEX_CUDA_SURFACE_2D(surfRefOutput0)
APEX_CUDA_SURFACE_2D(surfRefOutput1)
APEX_CUDA_SURFACE_2D(surfRefOutput2)
APEX_CUDA_SURFACE_2D(surfRefOutput3)
#else
#undef SKIP_OUTPUT_SURFACES_DEF
#endif

APEX_CUDA_BOUND_KERNEL((), spriteModifiersKernel,
                       ((unsigned int, inStateOffset))((unsigned int, outStateOffset))
                       ((InplaceHandle<ClientParamsHandleArray>, clientParamsHandleArrayHandle))
                       ((ModifierCommonParams, commonParams))
                       ((unsigned int*, g_sortedActorIDs))((unsigned int*, g_sortedStateIDs))((unsigned int*, g_outStateToInput))
                       ((SpritePrivateStateArgs, privStateArgs))((float*, g_stateSpawnScale))
                       ((PRNGInfo, rand))((unsigned int*, g_outputBuffer))
					   ((InplaceHandle<SpriteOutputLayout>, outputLayoutHandle))
                      )

APEX_CUDA_BOUND_KERNEL(SPRITE_TEXTURE_MODIFIER_KERNEL_CONFIG, spriteTextureModifiersKernel,
                       ((unsigned int, inStateOffset))((unsigned int, outStateOffset))
                       ((InplaceHandle<ClientParamsHandleArray>, clientParamsHandleArrayHandle))
                       ((ModifierCommonParams, commonParams))
                       ((unsigned int*, g_sortedActorIDs))((unsigned int*, g_sortedStateIDs))((unsigned int*, g_outStateToInput))
                       ((SpritePrivateStateArgs, privStateArgs))((float*, g_stateSpawnScale))
                       ((PRNGInfo, rand))((SpriteTextureOutputLayout, outputLayout))
                      )

APEX_CUDA_BOUND_KERNEL((), meshModifiersKernel,
                       ((unsigned int, inStateOffset))((unsigned int, outStateOffset))
                       ((InplaceHandle<ClientParamsHandleArray>, clientParamsHandleArrayHandle))
                       ((ModifierCommonParams, commonParams))
                       ((unsigned int*, g_sortedActorIDs))((unsigned int*, g_sortedStateIDs))((unsigned int*, g_outStateToInput))
                       ((MeshPrivateStateArgs, privStateArgs))((float*, g_stateSpawnScale))
                       ((PRNGInfo, rand))((unsigned int*, g_outputBuffer))
					   ((InplaceHandle<MeshOutputLayout>, outputLayoutHandle))
                      )
