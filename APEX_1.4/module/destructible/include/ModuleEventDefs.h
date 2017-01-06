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

DEFINE_EVENT(DestructibleFetchResults)
DEFINE_EVENT(DestructibleUpdateRenderResources)
DEFINE_EVENT(DestructibleDispatchRenderResources)
DEFINE_EVENT(DestructibleSeparateUnsupportedIslands)
DEFINE_EVENT(DestructibleCreateDynamicIsland)
DEFINE_EVENT(DestructibleCookChunkCollisionMeshes)
DEFINE_EVENT(DestructibleAddActors)
DEFINE_EVENT(DestructibleCreateRoot)
DEFINE_EVENT(DestructibleAppendShape)
DEFINE_EVENT(DestructibleStructureTick)
DEFINE_EVENT(DestructiblePhysBasedStressSolver)
DEFINE_EVENT(DestructiblePhysBasedStressSolverInitial)
DEFINE_EVENT(DestructiblePhysBasedStressSolverCalculateForce)
DEFINE_EVENT(DestructiblePhysBasedStressSolverRemoveChunk)
DEFINE_EVENT(DestructibleStructureBoundsCalculateOverlaps)
DEFINE_EVENT(DestructibleStructureDetailedOverlapTest)
DEFINE_EVENT(DestructibleCreateActor)
DEFINE_EVENT(DestructibleCacheChunkCookedCollisionMeshes)
DEFINE_EVENT(DestructibleFractureChunk)
DEFINE_EVENT(DestructibleChunkReport)
DEFINE_EVENT(DestructibleRayCastFindVisibleChunk)
DEFINE_EVENT(DestructibleRayCastFindDeepestChunk)
DEFINE_EVENT(DestructibleRayCastProcessChunk)
DEFINE_EVENT(DestructibleRemoveChunksForBudget)
DEFINE_EVENT(DestructibleProcessFractureBuffer)
DEFINE_EVENT(DestructibleKillStructures)
DEFINE_EVENT(DestructibleOnContactConstraint)
DEFINE_EVENT(DestructibleOnContactNotify)
DEFINE_EVENT(DestructibleBeforeTickLockRenderables)
DEFINE_EVENT(DestructibleUpdateRenderMeshBonePoses)
DEFINE_EVENT(DestructibleStructureBuildSupportGraph)
DEFINE_EVENT(DestructibleSimulate)
DEFINE_EVENT(DestructibleCalcStaticChunkBenefit)
DEFINE_EVENT(DestructibleGatherNonessentialChunks)
DEFINE_EVENT(DestructibleCalculateBenefit)
DEFINE_EVENT(DestructibleGetChunkGlobalPose)
