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
// This file should only contain event definitions, using the
// DEFINE_EVENT macro.  E.g.:
//
//     DEFINE_EVENT(sample_name_1)
//     DEFINE_EVENT(sample_name_2)
//     DEFINE_EVENT(sample_name_3)

// Framework only event definitions

DEFINE_EVENT(ApexScene_simulate)
DEFINE_EVENT(ApexScene_fetchResults)
DEFINE_EVENT(ApexScene_checkResults)
DEFINE_EVENT(ApexSceneManualSubstep)
DEFINE_EVENT(ModuleSceneManualSubstep)
DEFINE_EVENT(ApexSceneBeforeStep)
DEFINE_EVENT(ApexSceneDuringStep)
DEFINE_EVENT(ApexSceneAfterStep)
DEFINE_EVENT(ApexScenePostFetchResults)
DEFINE_EVENT(ApexSceneLODUsedResource)
DEFINE_EVENT(ApexSceneLODSumBenefit)
DEFINE_EVENT(ApexRenderMeshUpdateRenderResources)
DEFINE_EVENT(ApexRenderMeshCreateRenderResources)
DEFINE_EVENT(ApexRenderMeshDispatchRenderResources)
DEFINE_EVENT(ApexRenderMeshUpdateInstances)
DEFINE_EVENT(ApexRenderMeshUpdateInstancesWritePoses)
DEFINE_EVENT(ApexRenderMeshUpdateInstancesWriteScales)