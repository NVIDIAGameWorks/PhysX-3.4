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

DEFINE_EVENT(EmitterSceneBeforeTick)
DEFINE_EVENT(EmitterSceneFetchResults)

DEFINE_EVENT(EmitterActorOverlapAABBSceneQuery)
DEFINE_EVENT(EmitterActorOverlapSphereSceneQuery)

DEFINE_EVENT(GroundParticlesEmitterTick)
DEFINE_EVENT(GroundParticlesEmitterInjection)
DEFINE_EVENT(GroundParticlesEmitterRefreshCircle)
DEFINE_EVENT(GroundParticlesEmitterGridInspect)
DEFINE_EVENT(GroundParticlesEmitterRaycasts)
DEFINE_EVENT(GroundParticlesEmitterFetchResults)
DEFINE_EVENT(GroundParticlesEmitterOnRaycastQuery)

