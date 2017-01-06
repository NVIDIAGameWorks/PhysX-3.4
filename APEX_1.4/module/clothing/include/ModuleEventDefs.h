/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


// This file is used to define a list of AgPerfMon events.
//
// This file is included exclusively by AgPerfMonEventSrcAPI.h
// and by AgPerfMonEventSrcAPI.cpp, for the purpose of building
// an enumeration (enum xx) and an array of strings ()
// that contain the list of events.
//
// This file should only contain event definitions, using the
// DEFINE_EVENT macro.  E.g.:
//
//     DEFINE_EVENT(sample_name_1)
//     DEFINE_EVENT(sample_name_2)
//     DEFINE_EVENT(sample_name_3)


DEFINE_EVENT(ClothingActorCPUSkinningGraphics)
DEFINE_EVENT(ClothingActorCreatePhysX)
DEFINE_EVENT(ClothingActorFallbackSkinning)

DEFINE_EVENT(ClothingActorUpdateStateInternal)
DEFINE_EVENT(ClothingActorInterpolateMatrices)
DEFINE_EVENT(ClothingActorUpdateCollision)
DEFINE_EVENT(ClothingActorApplyVelocityChanges)

DEFINE_EVENT(ClothingActorLodTick)
DEFINE_EVENT(ClothingActorMeshMeshSkinning)
DEFINE_EVENT(ClothingActorMorphTarget)
DEFINE_EVENT(ClothingActorRecomputeTangentSpace)
DEFINE_EVENT(ClothingActorSDKCreateClothSoftbody)
DEFINE_EVENT(ClothingActorUpdateBounds)
DEFINE_EVENT(ClothingActorUpdateCompressedSkinning)
DEFINE_EVENT(ClothingActorUpdateRenderResources)
DEFINE_EVENT(ClothingActorUpdateVertexBuffer)
DEFINE_EVENT(ClothingActorVelocityShader)
DEFINE_EVENT(ClothingActorWind)
DEFINE_EVENT(ClothingActorTeleport)
DEFINE_EVENT(ClothingActorTransformGraphicalMeshes)

DEFINE_EVENT(ClothingAssetDeserialize)
DEFINE_EVENT(ClothingAssetLoad)

DEFINE_EVENT(ClothingSceneDistributeSolverIterations)
