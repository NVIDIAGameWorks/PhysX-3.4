/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


APEX_CUDA_STORAGE(simulateStorage)


APEX_CUDA_TEXTURE_1D(texRefPositionMass,      float4)
APEX_CUDA_TEXTURE_1D(texRefVelocityLife,      float4)
APEX_CUDA_TEXTURE_1D(texRefIofxActorIDs,      unsigned int)
APEX_CUDA_TEXTURE_1D(texRefLifeSpan,          float)
APEX_CUDA_TEXTURE_1D(texRefLifeTime,          float)
APEX_CUDA_TEXTURE_1D(texRefInjector,          unsigned int)

APEX_CUDA_TEXTURE_1D(texRefUserData,		  unsigned int)

APEX_CUDA_TEXTURE_1D(texRefHoleScanSum,       unsigned int)
APEX_CUDA_TEXTURE_1D(texRefMoveIndices,       unsigned int)

APEX_CUDA_TEXTURE_1D(texRefField,             float4)

APEX_CUDA_TEXTURE_1D(texRefPxPosition,        float4)
APEX_CUDA_TEXTURE_1D(texRefPxVelocity,        float4)
APEX_CUDA_TEXTURE_1D(texRefPxCollision,       float4)
APEX_CUDA_TEXTURE_1D(texRefNvFlags,           unsigned int)
APEX_CUDA_TEXTURE_1D(texRefPxDensity,         float)


APEX_CUDA_BOUND_KERNEL(SIMULATE_KERNEL_CONFIG, simulateKernel,
                       ((unsigned int, lastCount))((float, deltaTime))((physx::PxVec3, eyePos))
                       ((InplaceHandle<InjectorParamsArray>, injectorParamsArrayHandle))((unsigned int, injectorCount))
                       ((unsigned int*, g_holeScanSum))((unsigned int*, g_inputIdToParticleIndex))((unsigned int*, g_moveCount))((unsigned int*, g_tmpHistogram))((unsigned int*, g_InjectorsCounters))
                       ((float4*, g_positionMass))((float4*, g_velocityLife))((float4*, g_collisionNormalFlags))((unsigned int*, g_userData))
                       ((float*, g_lifeSpan))((float*, g_lifeTime))((float*, g_density))((unsigned int*, g_injector))((IofxActorIDIntl*, g_iofxActorIDs))
                       ((float*, g_benefit))((float4*, g_pxPosition))((float4*, g_pxVelocity))((float4*, pxCollisionNormals))((float*, pxDensity))((unsigned int*, g_NvFlags))
					   ((GridDensityParams, params))
                      )

APEX_CUDA_BOUND_KERNEL(SIMULATE_KERNEL_CONFIG, simulateApplyFieldKernel,
                       ((unsigned int, lastCount))((float, deltaTime))((physx::PxVec3, eyePos))
                       ((InplaceHandle<InjectorParamsArray>, injectorParamsArrayHandle))((unsigned int, injectorCount))
                       ((APEX_MEM_BLOCK(unsigned int), g_holeScanSum))((APEX_MEM_BLOCK(unsigned int), g_inputIdToParticleIndex))
					   ((APEX_MEM_BLOCK(unsigned int), g_moveCount))
					   ((APEX_MEM_BLOCK(unsigned int), g_tmpHistogram))
					   ((APEX_MEM_BLOCK(unsigned int), g_InjectorsCounters))
					   ((APEX_MEM_BLOCK(float4), g_positionMass))((APEX_MEM_BLOCK(float4), g_velocityLife))
					   ((APEX_MEM_BLOCK(float4), g_collisionNormalFlags))((APEX_MEM_BLOCK(unsigned int),g_userData))((APEX_MEM_BLOCK(float), g_lifeSpan))((APEX_MEM_BLOCK(float), g_lifeTime))
					   ((APEX_MEM_BLOCK(float), g_density))((APEX_MEM_BLOCK(unsigned int), g_injector))((APEX_MEM_BLOCK(IofxActorIDIntl), g_iofxActorIDs))
                       ((APEX_MEM_BLOCK(float), g_benefit))((APEX_MEM_BLOCK(float4), g_pxPosition))((APEX_MEM_BLOCK(float4), g_pxVelocity))
					   ((APEX_MEM_BLOCK(float4), pxCollisionNormals))((APEX_MEM_BLOCK(float), pxDensity))((APEX_MEM_BLOCK(unsigned int), g_NvFlags))
					   ((GridDensityParams, params))
                      )

APEX_CUDA_BOUND_KERNEL((), mergeHistogramKernel,
						((unsigned int*, g_InjectorsCounters))((unsigned int*, g_tmpHistograms))((unsigned int, gridSize))((unsigned int, injectorCount))
						)


APEX_CUDA_BOUND_KERNEL((), stateKernel,
                       ((unsigned int, lastCount))((unsigned int, targetCount))
                       ((APEX_MEM_BLOCK(unsigned int), g_moveCount))
                       ((APEX_MEM_BLOCK(unsigned int), g_inStateToInput))((APEX_MEM_BLOCK(unsigned int), g_outStateToInput))
                      )

APEX_CUDA_BOUND_KERNEL((), testKernel,
                       ((unsigned int, scalarVar))((APEX_MEM_BLOCK(unsigned int), vectorVar))
					   ((InplaceHandle<int>, multHandle))
                      )

APEX_CUDA_BOUND_KERNEL((), gridDensityGridClearKernel,
						((float*, gridDensityGrid))((GridDensityParams, params))
						)

APEX_CUDA_BOUND_KERNEL((), gridDensityGridFillKernel,
						((float4*, positionMass))((float*, gridDensityGrid))((GridDensityParams, params))
						)

APEX_CUDA_BOUND_KERNEL((), gridDensityGridApplyKernel,
						((float*, density))((float4*, positionMass))((float*, gridDensityGrid))((GridDensityParams, params))
						)

APEX_CUDA_BOUND_KERNEL((), gridDensityGridFillFrustumKernel,
						((float4*, positionMass))((float*, gridDensityGrid))((GridDensityParams, params))((::physx::PxMat44,mat))((GridDensityFrustumParams,frustum))
						)

APEX_CUDA_BOUND_KERNEL((), gridDensityGridApplyFrustumKernel,
						((float*, density))((float4*, positionMass))((float*, gridDensityGrid))((GridDensityParams, params))((::physx::PxMat44,mat))((GridDensityFrustumParams,frustum))
						)

APEX_CUDA_FREE_KERNEL((), gridDensityGridLowPassKernel,
						((float*, gridDensityGridIn))((float*, gridDensityGridOut))((GridDensityParams, params))
						)
