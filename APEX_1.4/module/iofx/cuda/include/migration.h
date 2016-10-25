/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


APEX_CUDA_STORAGE(migrationStorage)


APEX_CUDA_BOUND_KERNEL((), volumeMigrationKernel,
                       ((InplaceHandle<VolumeParamsArray>, volumeParamsArrayHandle))
                       ((InplaceHandle<ActorClassIDBitmapArray>, actorClassIDBitmapArrayHandle))
                       ((uint32_t, numActorClasses))((uint32_t, numVolumes))((uint32_t, numActorIDValues))
                       ((IofxActorIDIntl*, actorID))((uint32_t, maxInputID))
                       ((const float4*, positionMass))
                       ((uint32_t*, actorStart))((uint32_t*, actorEnd))((uint32_t*, actorVisibleEnd))
                      )
