/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "ApexDefs.h"
#if APEX_CUDA_SUPPORT

#include "Apex.h"
#include "SceneIntl.h"
#include "ApexSDKIntl.h"
#include "ParticleIosActor.h"
#include "ParticleIosActorGPU.h"
#include "ParticleIosAssetImpl.h"

#include "iofx/IofxAsset.h"
#include "iofx/IofxActor.h"

#include "ModuleParticleIosImpl.h"
#include "ParticleIosScene.h"
#include "RenderDebugInterface.h"
#include "AuthorableObjectIntl.h"

#include "PxMath.h"

#define DEBUG_GPU 0
#define USE_PHYSX_TASK_SYNC 1

//CUDA
#include "PxGpuTask.h"
#include "ApexCutil.h"

#define CUDA_OBJ(name) SCENE_CUDA_OBJ(*mParticleIosScene, name)

#include "PxParticleBase.h"
#include "PxParticleSystem.h"
#include "PxParticleDeviceExclusive.h"

namespace nvidia
{
namespace pxparticleios
{

using namespace physx;

#pragma warning(disable: 4355) // 'this' : used in base member initializer list

ParticleIosActorGPU::ParticleIosActorGPU(
    ResourceList& list,
    ParticleIosAssetImpl& asset,
    ParticleIosScene& scene,
	IofxAsset& iofxAsset)
	: ParticleIosActorImpl(list, asset, scene, iofxAsset, true)
	, mCopyQueue(*scene.getApexScene().getTaskManager()->getGpuDispatcher())
	, mHoleScanSum(scene.getApexScene(), PX_ALLOC_INFO("mHoleScanSum", PARTICLES))
	, mMoveIndices(scene.getApexScene(), PX_ALLOC_INFO("mMoveIndices", PARTICLES))
	, mTmpReduce(scene.getApexScene(), PX_ALLOC_INFO("mTmpReduce", PARTICLES))
	, mTmpHistogram(scene.getApexScene(), PX_ALLOC_INFO("mTmpHistogram", PARTICLES))
	, mTmpScan(scene.getApexScene(), PX_ALLOC_INFO("mTmpScan", PARTICLES))
	, mTmpScan1(scene.getApexScene(), PX_ALLOC_INFO("mTmpScan1", PARTICLES))
	, mTmpOutput(scene.getApexScene(), PX_ALLOC_INFO("mTmpOutput", PARTICLES))
	, mTmpBoundParams(scene.getApexScene(), PX_ALLOC_INFO("mTmpBoundParams", PARTICLES))
	, mLaunchTask(*this)
	, mTriggerTask(*this)
	, mCuSyncEvent(0)
{
	initStorageGroups(CUDA_OBJ(simulateStorage));

#if DEBUG_GPU
	const ApexMirroredPlace::Enum defaultPlace = ApexMirroredPlace::CPU_GPU;
#else
	const ApexMirroredPlace::Enum defaultPlace = ApexMirroredPlace::GPU;
#endif
	
	mTmpOutput.setSize(4, ApexMirroredPlace::CPU_GPU);
	mTmpBoundParams.setSize(2, defaultPlace);

	const unsigned int ScanWarpsPerBlock = MAX_WARPS_PER_BLOCK; //CUDA_OBJ(scanKernel).getBlockDim().x / WARP_SIZE;

	mTmpReduce.reserve(MAX_BOUND_BLOCKS * 4, defaultPlace);
	mTmpHistogram.reserve(MAX_BOUND_BLOCKS * HISTOGRAM_SIMULATE_BIN_COUNT, defaultPlace);
	mTmpScan.reserve(MAX_BOUND_BLOCKS * ScanWarpsPerBlock, defaultPlace);
	mTmpScan1.reserve(MAX_BOUND_BLOCKS * ScanWarpsPerBlock, defaultPlace);
	
	mField.reserve(mMaxParticleCount, defaultPlace);
	mLifeTime.reserve(mMaxParticleCount, defaultPlace);

	mLifeSpan.reserve(mMaxTotalParticleCount, ApexMirroredPlace::CPU_GPU);
	mInjector.reserve(mMaxTotalParticleCount, ApexMirroredPlace::CPU_GPU);
	mBenefit.reserve(mMaxTotalParticleCount, ApexMirroredPlace::CPU_GPU);

	{
		uint32_t size = mGridDensityParams.GridResolution;
		if(size > 0)
		{
			mGridDensityGrid.setSize(size*size*size,ApexMirroredPlace::GPU);
			mGridDensityGridLowPass.setSize(size*size*size,ApexMirroredPlace::GPU);
		}
	}

	mHoleScanSum.reserve(mMaxTotalParticleCount, defaultPlace);
	mMoveIndices.reserve(mMaxTotalParticleCount, defaultPlace);

#if USE_PHYSX_TASK_SYNC
	{
		PxCudaContextManager* ctxMgr = mParticleIosScene->getApexScene().getTaskManager()->getGpuDispatcher()->getCudaContextManager();
		PxScopedCudaLock _lock_(*ctxMgr);

		CUT_SAFE_CALL(cuEventCreate(&mCuSyncEvent, CU_EVENT_DISABLE_TIMING));
	}
#endif
}

ParticleIosActorGPU::~ParticleIosActorGPU()
{
#if USE_PHYSX_TASK_SYNC
	{
		PxCudaContextManager* ctxMgr = mParticleIosScene->getApexScene().getTaskManager()->getGpuDispatcher()->getCudaContextManager();
		PxScopedCudaLock _lock_(*ctxMgr);

		CUT_SAFE_CALL(cuEventDestroy(mCuSyncEvent));
	}
#endif
}

PxTaskID ParticleIosActorGPU::submitTasks(PxTaskManager* tm)
{
	ParticleIosActorImpl::submitTasks(tm);
	mInjectorsCounters.setSize(mInjectorList.getSize(), ApexMirroredPlace::CPU_GPU); 

	if (mAsset->getParticleDesc()->Enable == false)
	{
		return mInjectTask.getTaskID();
	}

	tm->submitUnnamedTask(mTriggerTask);
	PxTaskID taskID = tm->submitUnnamedTask(mLaunchTask, PxTaskType::TT_GPU);

	SCOPED_PHYSX_LOCK_WRITE(mParticleIosScene->getModulePhysXScene());

	if (!PxParticleDeviceExclusive::isEnabled(*mParticleActor->is<PxParticleBase>()))
	{
		PxParticleDeviceExclusive::enable(*mParticleActor->is<PxParticleBase>());
	}
	PxParticleDeviceExclusive::setValidParticleRange(*mParticleActor->is<PxParticleBase>(), mParticleCount);

#if USE_PHYSX_TASK_SYNC
	PxParticleDeviceExclusive::addLaunchTaskDependent(*mParticleActor->is<PxParticleBase>(), mLaunchTask);

	PxBaseTask* physxTask = PxParticleDeviceExclusive::getLaunchTask(*mParticleActor->is<PxParticleBase>());
	static_cast<ParticleIosSceneGPU*>(mParticleIosScene)->getGpuDispatcher()->addPreLaunchDependent(*physxTask);
	physxTask->removeReference();
#endif
	return taskID;
}

void ParticleIosActorGPU::setTaskDependencies(PxTaskID taskStartAfterID, PxTaskID taskFinishBeforeID)
{
	if (mAsset->getParticleDesc()->Enable == false)
	{
		ParticleIosActorImpl::setTaskDependencies(taskStartAfterID, taskFinishBeforeID, NULL, true);
		return;
	}

#if USE_PHYSX_TASK_SYNC
	PX_UNUSED(taskStartAfterID);
	PX_UNUSED(taskFinishBeforeID);
	ParticleIosActorImpl::setTaskDependencies(PxTaskID(0xFFFFFFFF), PxTaskID(0xFFFFFFFF), &mLaunchTask, true);

	mTriggerTask.startAfter(mParticleIosScene->getApexScene().getTaskManager()->getNamedTask(AST_PHYSX_SIMULATE));
	mTriggerTask.finishBefore(mLaunchTask.getTaskID());
#else
	ParticleIosActorImpl::setTaskDependencies(taskStartAfterID, taskFinishBeforeID, &mLaunchTask, true);
#endif

#if 0
	if (tm->getGpuDispatcher()->getCudaContextManager()->supportsArchSM20())
	{
		/* For Fermi devices, it pays to launch all IOS together.  This also forces
		 * The IOFX managers to step at the same time.
		 */
		PxTaskID interlock = tm->getNamedTask("IOS::StepInterlock");
		mLaunchTask.startAfter(interlock);
	}
#endif
}

void ParticleIosActorGPU::trigger()
{
#if USE_PHYSX_TASK_SYNC
	static_cast<ParticleIosSceneGPU*>(mParticleIosScene)->getGpuDispatcher()->getPreLaunchTask().removeReference();
#endif
}

bool ParticleIosActorGPU::launch(CUstream stream, int kernelIndex)
{
	float deltaTime = mParticleIosScene->getApexScene().getPhysXSimulateTime();

	uint32_t activeCount = mLastActiveCount + mInjectedCount;
	mParticleBudget = mMaxParticleCount;
	if (mParticleBudget > activeCount)
	{
		mParticleBudget = activeCount;
	}
	uint32_t targetCount = mParticleBudget;
	if (targetCount == 0)
	{
		//reset output
		float* pTmpOutput = (float*)mTmpOutput.getPtr();
		mTmpOutput[STATUS_LAST_ACTIVE_COUNT] = 0;
		pTmpOutput[STATUS_LAST_BENEFIT_SUM]  = 0.0f;
		pTmpOutput[STATUS_LAST_BENEFIT_MIN]  = +FLT_MAX;
		pTmpOutput[STATUS_LAST_BENEFIT_MAX]  = -FLT_MAX;

		for(uint32_t i = 0; i < mInjectorsCounters.getSize(); ++i)
		{
			mInjectorsCounters[i] = 0; 
		}

		//skip simulation & just call IofxManager
		mIofxMgr->updateEffectsData(deltaTime, 0, 0, 0, stream);
		return false;
	}

	uint32_t lastCount = mParticleCount;
	uint32_t injectCount = mInjectedCount;
	uint32_t totalCount = lastCount + injectCount;
	PX_ASSERT(targetCount <= totalCount);

	uint32_t boundCount = 0;
	if (activeCount > targetCount)
	{
		boundCount = activeCount - targetCount;
	}

	ParticleIosSceneGPU* sceneGPU = static_cast<ParticleIosSceneGPU*>(mParticleIosScene);
	bool useSyncKernels = !sceneGPU->getGpuDispatcher()->getCudaContextManager()->supportsArchSM20();

	switch (kernelIndex)
	{
	case 0:
		if (!mFieldSamplerQuery && mOnStartCallback)
		{
			(*mOnStartCallback)(stream);
		}

		// Copy particle data for newly injected particles
		mCopyQueue.reset(stream, 24);
		mIofxMgr->outputHostToDevice(mCopyQueue);
		if (mInjectedCount > 0)
		{
			mBufDesc.pmaPositionMass->copyHostToDeviceQ(mCopyQueue, mInjectedCount, mParticleCount);
			mBufDesc.pmaVelocityLife->copyHostToDeviceQ(mCopyQueue, mInjectedCount, mParticleCount);
			mBufDesc.pmaActorIdentifiers->copyHostToDeviceQ(mCopyQueue, mInjectedCount, mParticleCount);
			mLifeSpan.copyHostToDeviceQ(mCopyQueue, mInjectedCount, mParticleCount);
			mInjector.copyHostToDeviceQ(mCopyQueue, mInjectedCount, mParticleCount);
			mBenefit.copyHostToDeviceQ(mCopyQueue, mInjectedCount, mParticleCount);
			mBufDesc.pmaUserData->copyHostToDeviceQ(mCopyQueue,mInjectedCount,mParticleCount);
		}
#if DEBUG_GPU
		mBenefit.copyDeviceToHostQ(mCopyQueue, mParticleCount);
#endif
		mCopyQueue.flushEnqueued();
		return true;

	case 1:
		if (totalCount > 0)
		{
			float benefitMin = PxMin(mLastBenefitMin, mInjectedBenefitMin);
			float benefitMax = PxMax(mLastBenefitMax, mInjectedBenefitMax);
			PX_ASSERT(benefitMin <= benefitMax);
			benefitMax *= 1.00001f;

			if (useSyncKernels)
			{
				CUDA_OBJ(histogramSyncKernel)(
					stream, totalCount,
					mBenefit.getGpuPtr(), boundCount,
					benefitMin, benefitMax,
					mTmpBoundParams.getGpuPtr(),
					mTmpHistogram.getGpuPtr()
				);
			}
			else
			{
				uint32_t histogramGridSize =
					CUDA_OBJ(histogramKernel)(
						stream, totalCount,
						createApexCudaMemRef(mBenefit, ApexCudaMemFlags::IN), 
						boundCount,	benefitMin, benefitMax,
						createApexCudaMemRef(mTmpBoundParams, ApexCudaMemFlags::IN),						
						createApexCudaMemRef(mTmpHistogram, ApexCudaMemFlags::OUT),
						1, 0
					);

				//launch just 1 block
				CUDA_OBJ(histogramKernel)(
					stream, APEX_CUDA_SINGLE_BLOCK_LAUNCH,
					createApexCudaMemRef(mBenefit, ApexCudaMemFlags::IN), boundCount,
					benefitMin, benefitMax,
					createApexCudaMemRef(mTmpBoundParams, ApexCudaMemFlags::OUT),
					createApexCudaMemRef(mTmpHistogram, ApexCudaMemFlags::IN_OUT),
					2, histogramGridSize
				);
			}
		}
		return true;

	case 2:
		if (totalCount > 0)
		{
			float benefitMin = PxMin(mLastBenefitMin, mInjectedBenefitMin);
			float benefitMax = PxMax(mLastBenefitMax, mInjectedBenefitMax);
			PX_ASSERT(benefitMin <= benefitMax);
			benefitMax *= 1.00001f;

			if (useSyncKernels)
			{
				CUDA_OBJ(scanSyncKernel)(
					stream, totalCount,
					benefitMin, benefitMax,
					mHoleScanSum.getGpuPtr(), mBenefit.getGpuPtr(),
					mTmpBoundParams.getGpuPtr(),
					mTmpScan.getGpuPtr(), mTmpScan1.getGpuPtr()
				);
			}
			else
			{
				uint32_t scanGridSize = 
					CUDA_OBJ(scanKernel)(
						stream, totalCount,
						benefitMin, benefitMax,
						createApexCudaMemRef(mHoleScanSum, ApexCudaMemFlags::IN), 
						createApexCudaMemRef(mBenefit, ApexCudaMemFlags::IN),
						createApexCudaMemRef(mTmpBoundParams, ApexCudaMemFlags::IN),
						createApexCudaMemRef(mTmpScan, ApexCudaMemFlags::OUT),
						createApexCudaMemRef(mTmpScan1, ApexCudaMemFlags::OUT),
						1, 0
					);

				//launch just 1 block
				CUDA_OBJ(scanKernel)(
					stream, APEX_CUDA_SINGLE_BLOCK_LAUNCH,
					benefitMin, benefitMax,
					createApexCudaMemRef(mHoleScanSum, ApexCudaMemFlags::IN), 
					createApexCudaMemRef(mBenefit, ApexCudaMemFlags::IN),
					createApexCudaMemRef(mTmpBoundParams, ApexCudaMemFlags::IN),
					createApexCudaMemRef(mTmpScan, ApexCudaMemFlags::IN_OUT),
					createApexCudaMemRef(mTmpScan1, ApexCudaMemFlags::IN_OUT),
					2, scanGridSize
				);

				CUDA_OBJ(scanKernel)(
					stream, totalCount,
					benefitMin, benefitMax,
					createApexCudaMemRef(mHoleScanSum, ApexCudaMemFlags::OUT),
					createApexCudaMemRef(mBenefit, ApexCudaMemFlags::IN),
					createApexCudaMemRef(mTmpBoundParams, ApexCudaMemFlags::IN),
					createApexCudaMemRef(mTmpScan, ApexCudaMemFlags::IN), 
					createApexCudaMemRef(mTmpScan1, ApexCudaMemFlags::IN),
					3, 0
				);
			}
		}
		return true;

	case 3:
	{
		if (totalCount > 0)
		{
			APEX_CUDA_TEXTURE_SCOPE_BIND(texRefCompactScanSum, mHoleScanSum);
			const uint32_t injectorCount = mInjectorList.getSize();

			CUDA_OBJ(compactKernel)(
				stream,
				PxMax(totalCount, injectorCount),
				targetCount,
				totalCount,
				injectorCount,
				createApexCudaMemRef(mMoveIndices, ApexCudaMemFlags::OUT),
				createApexCudaMemRef(mTmpScan, ApexCudaMemFlags::OUT),
				createApexCudaMemRef(mInjectorsCounters, ApexCudaMemFlags::OUT)
			);
		}
		return true;
	}

	case 4:
		if (targetCount > 0)
		{
			uint32_t histogramGridSize = 0;
			{
				PxCudaReadWriteParticleBuffers buffers;
				memset(&buffers, 0, sizeof(buffers));

				CUstream physxCuStream = 0;
				{
					SCOPED_PHYSX_LOCK_READ(&mParticleIosScene->getApexScene());

					PxParticleDeviceExclusive::getReadWriteCudaBuffers(*mParticleActor->is<PxParticleBase>(), buffers);
					PX_ASSERT( buffers.positions && buffers.velocities && buffers.collisionNormals && buffers.flags);

#if USE_PHYSX_TASK_SYNC
					physxCuStream = PxParticleDeviceExclusive::getCudaStream(*mParticleActor->is<PxParticleBase>());
#endif
				}
				PX_UNUSED(physxCuStream);
#if USE_PHYSX_TASK_SYNC
				//sync physx & apex cuda streams!
				if (stream != 0 && physxCuStream != 0)
				{
					CUT_SAFE_CALL(cuEventRecord(mCuSyncEvent, physxCuStream));
					CUT_SAFE_CALL(cuStreamWaitEvent(stream, mCuSyncEvent, 0));
				}
#endif

				APEX_CUDA_TEXTURE_SCOPE_BIND_SIZE(texRefMoveIndices, mMoveIndices, totalCount);

				APEX_CUDA_TEXTURE_SCOPE_BIND_SIZE(texRefPositionMass, *mBufDesc.pmaPositionMass, totalCount);
				APEX_CUDA_TEXTURE_SCOPE_BIND_SIZE(texRefVelocityLife, *mBufDesc.pmaVelocityLife, totalCount);
				APEX_CUDA_TEXTURE_SCOPE_BIND_SIZE(texRefIofxActorIDs, *mBufDesc.pmaActorIdentifiers, totalCount);
				APEX_CUDA_TEXTURE_SCOPE_BIND_SIZE(texRefLifeSpan, mLifeSpan, totalCount);
				APEX_CUDA_TEXTURE_SCOPE_BIND_SIZE(texRefLifeTime, mLifeTime, totalCount);
				APEX_CUDA_TEXTURE_SCOPE_BIND_SIZE(texRefInjector, mInjector, totalCount);

				APEX_CUDA_TEXTURE_SCOPE_BIND_SIZE(texRefUserData,*mBufDesc.pmaUserData, totalCount);

		
				APEX_CUDA_TEXTURE_SCOPE_BIND_PTR(texRefPxPosition,  (float4*)buffers.positions,        lastCount);
				APEX_CUDA_TEXTURE_SCOPE_BIND_PTR(texRefPxVelocity,  (float4*)buffers.velocities,       lastCount);
				APEX_CUDA_TEXTURE_SCOPE_BIND_PTR(texRefPxCollision, (float4*)buffers.collisionNormals, lastCount);
				if(buffers.densities)
				{
					CUDA_OBJ(texRefPxDensity).bindTo(buffers.densities, lastCount);
				}
				APEX_CUDA_TEXTURE_SCOPE_BIND_PTR(texRefNvFlags,     (unsigned int*)buffers.flags,      lastCount);

				const PxVec3& eyePos = mParticleIosScene->getApexScene().getEyePosition();
				ParticleIosSceneGPU* sceneGPU = static_cast<ParticleIosSceneGPU*>(mParticleIosScene);

				if (mFieldSamplerQuery != NULL)
				{
					APEX_CUDA_TEXTURE_SCOPE_BIND_SIZE(texRefField, mField, totalCount);

					histogramGridSize = CUDA_OBJ(simulateApplyFieldKernel)(stream,
						targetCount,
						lastCount,
						deltaTime,
						eyePos,
						sceneGPU->mInjectorConstMemGroup.getStorage().mappedHandle(sceneGPU->mInjectorParamsArrayHandle),
						mInjectorsCounters.getSize(),
						createApexCudaMemRef(mHoleScanSum, targetCount, ApexCudaMemFlags::IN),
						createApexCudaMemRef(mInputIdToParticleIndex, ApexCudaMemFlags::IN),
						createApexCudaMemRef(mTmpScan, 1, ApexCudaMemFlags::IN), //g_moveCount
						createApexCudaMemRef(mTmpHistogram, targetCount, ApexCudaMemFlags::OUT),   //targetCount ????
						createApexCudaMemRef(mInjectorsCounters, mInjectorsCounters.getSize(), ApexCudaMemFlags::OUT),
						createApexCudaMemRef((float4*)mBufDesc.pmaPositionMass->getGpuPtr(), targetCount, ApexCudaMemFlags::OUT),
						createApexCudaMemRef((float4*)mBufDesc.pmaVelocityLife->getGpuPtr(), targetCount, ApexCudaMemFlags::OUT),
						createApexCudaMemRef((float4*)mBufDesc.pmaCollisionNormalFlags->getGpuPtr(), targetCount, ApexCudaMemFlags::OUT),
						createApexCudaMemRef((unsigned int*)mBufDesc.pmaUserData->getGpuPtr(), targetCount, ApexCudaMemFlags::OUT),
						createApexCudaMemRef(mLifeSpan, targetCount, ApexCudaMemFlags::OUT),
						createApexCudaMemRef(mLifeTime, targetCount, ApexCudaMemFlags::OUT),
						mBufDesc.pmaDensity != NULL ? createApexCudaMemRef((float*)mBufDesc.pmaDensity->getGpuPtr(), targetCount, ApexCudaMemFlags::OUT) : ApexCudaMemRef<float>(NULL, 0),
						createApexCudaMemRef(mInjector, targetCount, ApexCudaMemFlags::OUT),
						createApexCudaMemRef(*(mBufDesc.pmaActorIdentifiers), targetCount, ApexCudaMemFlags::OUT),
						createApexCudaMemRef(mBenefit, targetCount, ApexCudaMemFlags::OUT),
						createApexCudaMemRef((float4*)buffers.positions, targetCount, ApexCudaMemFlags::OUT),
						createApexCudaMemRef((float4*)buffers.velocities, targetCount, ApexCudaMemFlags::OUT),
						createApexCudaMemRef((float4*)buffers.collisionNormals, targetCount, ApexCudaMemFlags::IN),
						buffers.densities != NULL ? createApexCudaMemRef((float*)buffers.densities, targetCount, ApexCudaMemFlags::OUT) : ApexCudaMemRef<float>(NULL, 0),
						createApexCudaMemRef((unsigned int*)buffers.flags, targetCount, ApexCudaMemFlags::OUT),
						mGridDensityParams
						);
				}
				else
				{
					histogramGridSize = CUDA_OBJ(simulateKernel)(stream,
						targetCount,
						lastCount,
						deltaTime,
						eyePos,
						sceneGPU->mInjectorConstMemGroup.getStorage().mappedHandle(sceneGPU->mInjectorParamsArrayHandle),
						mInjectorsCounters.getSize(),
						mHoleScanSum.getGpuPtr(),
						mInputIdToParticleIndex.getGpuPtr(),
						mTmpScan.getGpuPtr(),
						mTmpHistogram.getGpuPtr(),
						mInjectorsCounters.getGpuPtr(),
						(float4*)mBufDesc.pmaPositionMass->getGpuPtr(),
						(float4*)mBufDesc.pmaVelocityLife->getGpuPtr(),
						(float4*)mBufDesc.pmaCollisionNormalFlags->getGpuPtr(),
						(unsigned int*)mBufDesc.pmaUserData->getGpuPtr(),
						mLifeSpan.getGpuPtr(),
						mLifeTime.getGpuPtr(),
						mBufDesc.pmaDensity != NULL ? (float*)mBufDesc.pmaDensity->getGpuPtr() : NULL,
						mInjector.getGpuPtr(),
						mBufDesc.pmaActorIdentifiers->getGpuPtr(),
						mBenefit.getGpuPtr(),
						(float4*)buffers.positions,
						(float4*)buffers.velocities,
						(float4*)buffers.collisionNormals,
						buffers.densities != NULL ? (float*)buffers.densities : NULL,
						(unsigned int*) buffers.flags,
						mGridDensityParams
						);
				}
				if(buffers.densities)
				{
					CUDA_OBJ(texRefPxDensity).unbind();
				}
			}
			//new kernel invocation - to merge temp histograms 
			{
				if(mInjectorsCounters.getSize() <= HISTOGRAM_SIMULATE_BIN_COUNT)
				{
					CUDA_OBJ(mergeHistogramKernel)(stream, APEX_CUDA_SINGLE_BLOCK_LAUNCH,
						mInjectorsCounters.getGpuPtr(),
						mTmpHistogram.getGpuPtr(),
						histogramGridSize,
						mInjectorsCounters.getSize()
						);
				}

			}
			// calculate grid grid density
			if (mGridDensityParams.Enabled)
			{
				mGridDensityParams.DensityOrigin = mDensityOrigin;
				const unsigned int dim = mGridDensityParams.GridResolution;
				// refreshed non-shared params
				{
					ParticleIosAssetParam* params = (ParticleIosAssetParam*)(mAsset->getAssetNvParameterized());
					const SimpleParticleSystemParams* gridParams = static_cast<SimpleParticleSystemParams*>(params->particleType);
					mGridDensityParams.GridSize = gridParams->GridDensity.GridSize;
					mGridDensityParams.GridMaxCellCount = gridParams->GridDensity.MaxCellCount;
				}
				// extract frustum
				if(mParticleIosScene->getApexScene().getNumProjMatrices() > 0)
				{
					PxMat44 matDen = PxMat44(PxIdentity);
					GridDensityFrustumParams frustum;
					PxMat44 matModel = mParticleIosScene->getApexScene().getViewMatrix();
					PxMat44 matProj  = mParticleIosScene->getApexScene().getProjMatrix();
					PxMat44 mat = matProj*matModel;
					PxMat44 matInv = inverse(mat);
					const float targetDepth = mGridDensityParams.GridSize;
					// for debug vis
					mDensityDebugMatInv = matInv;
					// to calculate w transform
					float nearDimX = distance(matInv.transform(PxVec4(-1.f,0.f,0.f,1.f)),matInv.transform(PxVec4(1.f,0.f,0.f,1.f)));
					float farDimX	= distance(matInv.transform(PxVec4(-1.f,0.f,1.f,1.f)),matInv.transform(PxVec4(1.f,0.f,1.f,1.f)));
					float nearDimY	= distance(matInv.transform(PxVec4(0.f,-1.f,0.f,1.f)),matInv.transform(PxVec4(0.f,1.f,0.f,1.f)));
					float farDimY	= distance(matInv.transform(PxVec4(0.f,-1.f,1.f,1.f)),matInv.transform(PxVec4(0.f,1.f,1.f,1.f)));
					float dimZ		= distance(matInv.transform(PxVec4(0.f, 0.f,0.f,1.f)),matInv.transform(PxVec4(0.f,0.f,1.f,1.f)));
					float myFarDimX = nearDimX*(1.f-targetDepth/dimZ) + farDimX*(targetDepth/dimZ);
					float myFarDimY = nearDimY*(1.f-targetDepth/dimZ) + farDimY*(targetDepth/dimZ);
					// grab necessary frustum coordinates
					PxVec4 origin4 = matInv.transform(PxVec4(-1.f, 1.f,0.f,1.f));
					PxVec4 basisX4 = matInv.transform(PxVec4( 1.f, 1.f,0.f,1.f));
					PxVec4 basisY4 = matInv.transform(PxVec4(-1.f,-1.f,0.f,1.f));
					PxVec4 zDepth4 = matInv.transform(PxVec4(-1.f, 1.f,1.f,1.f));
					// create vec3 versions
					PxVec3 origin3(origin4.x/origin4.w,origin4.y/origin4.w,origin4.z/origin4.w);
					PxVec3 basisX3(basisX4.x/basisX4.w,basisX4.y/basisX4.w,basisX4.z/basisX4.w);
					PxVec3 basisY3(basisY4.x/basisY4.w,basisY4.y/basisY4.w,basisY4.z/basisY4.w);
					PxVec3 zDepth3(zDepth4.x/zDepth4.w,zDepth4.y/zDepth4.w,zDepth4.z/zDepth4.w);
					// make everthing relative to origin
					basisX3 -= origin3;
					basisY3 -= origin3;
					zDepth3 -= origin3;
					// find third basis
					PxVec3 basisZ3(basisX3.cross(basisY3));
					basisZ3.normalize();
					basisZ3*= targetDepth;
					// build scale,rotation,translation matrix
					PxMat44 mat1Inv = PxMat44(PxIdentity);
					mat1Inv.column0 = PxVec4(basisX3,0.f);
					mat1Inv.column1 = PxVec4(basisY3,0.f);
					mat1Inv.column2 = PxVec4(basisZ3,0.f);
					mat1Inv.column3 = PxVec4(origin3,1.f);
					PxMat44 mat1 = inverse(mat1Inv);
					// do perspective transform
					PxMat44 mat2 = PxMat44(PxIdentity);
					{
						float left		= -3.0f;
						float right	= 1.0f;
						float top		= 1.0f;
						float bottom	= -3.0f;
						float nearVal	= nearDimX/(0.5f*(myFarDimX-nearDimX));
						//float farVal	= nearVal + 1.f;
						// build matrix
						mat2.column0.x = -2.f*nearVal/(right-left);
						mat2.column1.y = -2.f*nearVal/(top-bottom);
						mat2.column2.x = (right+left)/(right-left);
						mat2.column2.y = (top+bottom)/(top-bottom);
						//mat2.column2.z = -(farVal+nearVal)/(farVal-nearVal);
						mat2.column2.w = -1.f;
						//mat2.column3.z = -(2.f*farVal*nearVal)/(farVal-nearVal);
						mat2.column3.w = 0.f;
					}
					// shrink to calculate density just outside of frustum
					PxMat44 mat3 = PxMat44(PxIdentity);
					float factor = (float)(mGridDensityParams.GridResolution-4) / (mGridDensityParams.GridResolution);
					{			
						mat3.column0.x = factor;
						mat3.column1.y = factor;
						mat3.column2.z = factor;
						mat3.column3.x = (1.0f-factor)/2.0f;
						mat3.column3.y = (1.0f-factor)/2.0f;
						mat3.column3.z = (1.0f-factor)/2.0f;
					}
					// create final matrix
					matDen = mat3*mat2*mat1;
					// create frustum info
					frustum.nearDimX = factor*nearDimX;
					frustum.farDimX  = factor*myFarDimX;
					frustum.nearDimY = factor*nearDimY;
					frustum.farDimY	 = factor*myFarDimY;
					frustum.dimZ     = factor*targetDepth;
					// launch frustum kernels
					CUDA_OBJ(gridDensityGridClearKernel)(stream, dim*dim*dim,
						mGridDensityGrid.getGpuPtr(),
						mGridDensityParams
					);
					CUDA_OBJ(gridDensityGridFillFrustumKernel)(stream, targetCount,
						(float4*)mBufDesc.pmaPositionMass->getGpuPtr(),
						mGridDensityGrid.getGpuPtr(),
						mGridDensityParams,
						matDen,
						frustum
						);
					CUDA_OBJ(gridDensityGridLowPassKernel)(stream, dim*dim*dim,
						mGridDensityGrid.getGpuPtr(),
						mGridDensityGridLowPass.getGpuPtr(),
						mGridDensityParams
						);
					CUDA_OBJ(gridDensityGridApplyFrustumKernel)(stream, targetCount,
						mBufDesc.pmaDensity != NULL ? (float*)mBufDesc.pmaDensity->getGpuPtr() : NULL,
						(float4*)mBufDesc.pmaPositionMass->getGpuPtr(),
						mGridDensityGridLowPass.getGpuPtr(),
						mGridDensityParams,
						matDen,
						frustum
						);
				}
			}
		}
		return true;

	case 5:
		if (targetCount > 0)
		{
			if (useSyncKernels)
			{
				CUDA_OBJ(reduceSyncKernel)(
					stream, targetCount,
					mBenefit.getGpuPtr(), (float4*)mTmpOutput.getGpuPtr(), mTmpReduce.getGpuPtr()
				);
			}
			else
			{
				uint32_t reduceGridSize =
					CUDA_OBJ(reduceKernel)(
						stream, targetCount,
						createApexCudaMemRef(mBenefit, ApexCudaMemFlags::IN),
						createApexCudaMemRef((float4*)mTmpOutput.getGpuPtr(), 1, ApexCudaMemFlags::IN),
						createApexCudaMemRef(mTmpReduce, ApexCudaMemFlags::OUT),
						1, 0
					);

				//launch just 1 block
				CUDA_OBJ(reduceKernel)(
					stream, APEX_CUDA_SINGLE_BLOCK_LAUNCH,
					createApexCudaMemRef(mBenefit, ApexCudaMemFlags::IN),
					createApexCudaMemRef((float4*)mTmpOutput.getGpuPtr(), 1, ApexCudaMemFlags::OUT),
					createApexCudaMemRef(mTmpReduce, ApexCudaMemFlags::IN),
					2, reduceGridSize
				);
			}
		}
		return true;

	case 6:
		if (totalCount > 0)
		{
			APEX_CUDA_TEXTURE_SCOPE_BIND(texRefHoleScanSum, mHoleScanSum);
			APEX_CUDA_TEXTURE_SCOPE_BIND(texRefMoveIndices, mMoveIndices);

			CUDA_OBJ(stateKernel)(stream, totalCount,
			                      lastCount, targetCount,
								  createApexCudaMemRef(mTmpScan, ApexCudaMemFlags::IN),
								  createApexCudaMemRef(*mBufDesc.pmaInStateToInput, ApexCudaMemFlags::OUT),
								  createApexCudaMemRef(*mBufDesc.pmaOutStateToInput, ApexCudaMemFlags::IN)
			                     );
		}
		return true;

	case 7:
		mTmpOutput.copyDeviceToHostQ(mCopyQueue);
		mInjectorsCounters.copyDeviceToHostQ(mCopyQueue);
#if DEBUG_GPU
		mHoleScanSum.copyDeviceToHostQ(mCopyQueue, totalCount);
		mMoveIndices.copyDeviceToHostQ(mCopyQueue, totalCount);
		mTmpScan.copyDeviceToHostQ(mCopyQueue, 1);
		mTmpBoundParams.copyDeviceToHostQ(mCopyQueue, 2);
		//mTmpHistogram.copyDeviceToHostQ(mCopyQueue, HISTOGRAM_BIN_COUNT);
		mBufDesc.pmaInStateToInput->copyDeviceToHostQ(mCopyQueue, totalCount);
#endif
		mCopyQueue.flushEnqueued();
		
		/* Oh! Manager of the IOFX! do your thing */
		mIofxMgr->updateEffectsData(deltaTime, targetCount, targetCount, totalCount, stream);
		return false;
	}
	return false;
}


#if DEBUG_GPU
template<typename T, typename F>
void dumpArray(const char* name, ApexMirroredArray<T>& inpArray, uint32_t size, F func)
{
	char buf[256 * 1024];
	char* str = buf;
	str += sprintf(str, "%s[%d]=", name, size);
	for (uint32_t i = 0; i < PxMin<uint32_t>(size, 1024); ++i)
	{
		str += func(str, inpArray.get(i));
	}
	APEX_DEBUG_INFO(buf);
}
#endif

void ParticleIosActorGPU::fetchResults()
{
	ParticleIosActorImpl::fetchResults();
	if (mAsset->getParticleDesc()->Enable == false)
	{
		return;
	}
#if DEBUG_GPU
	if (mParticleBudget > 0)
	{
		uint32_t targetCount = mParticleBudget;
		uint32_t lastCount = mParticleCount;
		uint32_t totalCount = lastCount + mInjectedCount;
		uint32_t activeCount = mLastActiveCount + mInjectedCount;
		uint32_t boundCount = (activeCount > targetCount) ? (activeCount - targetCount) : 0;

		uint32_t lastActiveCount = 0;
		for (uint32_t i = 0; i < lastCount; ++i)
		{
			float benefit = mBenefit[i];
			if (benefit > -FLT_MAX)
			{
				++lastActiveCount;
			}
		}
		if (lastActiveCount != mLastActiveCount)
		{
			APEX_DEBUG_INFO("lastCount=%d, totalCount=%d, targetCount=%d", lastCount, totalCount, targetCount);
			struct FBenefit { PX_INLINE uint32_t operator () (char* str, float x) { return sprintf(str, "%f,", x); } };
			dumpArray("mBenefit", mBenefit, lastCount, FBenefit() );
			APEX_INTERNAL_ERROR("lastActiveCount(%d) != mLastActiveCount(%d)", lastActiveCount, mLastActiveCount);
		}
		PX_ASSERT(lastActiveCount == mLastActiveCount);

		//Test src hole count
		uint32_t moveCount = mTmpScan[0];

		uint32_t holeCount = 0;
		for (uint32_t i = 0; i < totalCount; ++i)
		{
			uint32_t holeScanSum = mHoleScanSum[i];
			if (holeScanSum & HOLE_SCAN_FLAG)
			{
				++holeCount;
			}
			PX_ASSERT(holeCount == (holeScanSum & ~HOLE_SCAN_FLAG));
		}
		if (totalCount != targetCount + holeCount)
		{
			APEX_DEBUG_INFO("lastCount=%d, lastActiveCount=%d, injectCount=%d, totalCount=%d, targetCount=%d, moveCount=%d, holeCount=%d", lastCount, mLastActiveCount, mInjectedCount, totalCount, targetCount, moveCount, holeCount);
			APEX_DEBUG_INFO("boundCount=%d, mTmpBoundParams[0]=%d, mTmpBoundParams[1]=%d", boundCount, mTmpBoundParams[0], mTmpBoundParams[1]);
			struct FBenefit { PX_INLINE uint32_t operator () (char* str, float x) { return sprintf(str, "%f,", x); } };
			dumpArray("mBenefit", mBenefit, totalCount, FBenefit() );
			struct FHoleScanSum { PX_INLINE uint32_t operator () (char* str, uint32_t x) { return sprintf(str, "%x,", x); } };
			dumpArray("mHoleScanSum", mHoleScanSum, totalCount, FHoleScanSum() );
			//struct FHistogram { PX_INLINE uint32_t operator () (char* str, uint32_t x) { return sprintf(str, "%d,", x); } };
			//dumpArray("mTmpHistogram", mTmpHistogram, HISTOGRAM_BIN_COUNT, FHistogram() );
			APEX_INTERNAL_ERROR("totalCount (%d) != targetCount + holeCount (%d)", totalCount, targetCount + holeCount);
		}
		PX_ASSERT(totalCount == targetCount + holeCount);

		PX_ASSERT(moveCount <= holeCount);
		for (uint32_t i = 0; i < moveCount; ++i)
		{
			uint32_t holeIndex = mMoveIndices[i];
			PX_ASSERT(holeIndex < targetCount);

			uint32_t holeScanSum = mHoleScanSum[holeIndex];
			PX_ASSERT((holeScanSum & HOLE_SCAN_FLAG) != 0);
			PX_ASSERT( i + 1 == (holeScanSum & HOLE_SCAN_MASK) );
		}
		for (uint32_t i = moveCount; i < moveCount*2; ++i)
		{
			uint32_t nonHoleIndex = mMoveIndices[i];
			PX_ASSERT(nonHoleIndex >= targetCount);
			PX_ASSERT(nonHoleIndex < totalCount);

			uint32_t nonHoleScanSum = mHoleScanSum[nonHoleIndex];
			PX_ASSERT((nonHoleScanSum & HOLE_SCAN_FLAG) == 0);
			PX_ASSERT( i + 1 == moveCount + (((nonHoleIndex + 1) - nonHoleScanSum) - (targetCount - moveCount)) );
		}

		uint32_t validInputCount = 0;
		for (uint32_t i = 0; i < totalCount; ++i)
		{
			uint32_t inputId = mBufDesc.pmaInStateToInput->get(i);
			if (inputId != IosBufferDescIntl::NOT_A_PARTICLE)
			{
				inputId &= ~IosBufferDescIntl::NEW_PARTICLE_FLAG;
				PX_ASSERT(inputId < targetCount);

				validInputCount++;
			}
		}
		PX_ASSERT(validInputCount == targetCount);
	}
#endif

	mParticleCount = mParticleBudget;

	float* pTmpOutput = (float*)mTmpOutput.getPtr();
	mLastActiveCount = mTmpOutput[STATUS_LAST_ACTIVE_COUNT];
	mLastBenefitSum  = pTmpOutput[STATUS_LAST_BENEFIT_SUM];
	mLastBenefitMin  = pTmpOutput[STATUS_LAST_BENEFIT_MIN];
	mLastBenefitMax  = pTmpOutput[STATUS_LAST_BENEFIT_MAX];
}


PxMat44 ParticleIosActorGPU::inverse(const PxMat44& in)
{
	PxMat44 ret;
	float inv[16];
	float* invOut = &ret.column0.x;
	const float* m = &in.column0.x;
	int i;

    inv[0] = m[5]  * m[10] * m[15] - 
             m[5]  * m[11] * m[14] - 
             m[9]  * m[6]  * m[15] + 
             m[9]  * m[7]  * m[14] +
             m[13] * m[6]  * m[11] - 
             m[13] * m[7]  * m[10];

    inv[4] = -m[4]  * m[10] * m[15] + 
              m[4]  * m[11] * m[14] + 
              m[8]  * m[6]  * m[15] - 
              m[8]  * m[7]  * m[14] - 
              m[12] * m[6]  * m[11] + 
              m[12] * m[7]  * m[10];

    inv[8] = m[4]  * m[9] * m[15] - 
             m[4]  * m[11] * m[13] - 
             m[8]  * m[5] * m[15] + 
             m[8]  * m[7] * m[13] + 
             m[12] * m[5] * m[11] - 
             m[12] * m[7] * m[9];

    inv[12] = -m[4]  * m[9] * m[14] + 
               m[4]  * m[10] * m[13] +
               m[8]  * m[5] * m[14] - 
               m[8]  * m[6] * m[13] - 
               m[12] * m[5] * m[10] + 
               m[12] * m[6] * m[9];

    inv[1] = -m[1]  * m[10] * m[15] + 
              m[1]  * m[11] * m[14] + 
              m[9]  * m[2] * m[15] - 
              m[9]  * m[3] * m[14] - 
              m[13] * m[2] * m[11] + 
              m[13] * m[3] * m[10];

    inv[5] = m[0]  * m[10] * m[15] - 
             m[0]  * m[11] * m[14] - 
             m[8]  * m[2] * m[15] + 
             m[8]  * m[3] * m[14] + 
             m[12] * m[2] * m[11] - 
             m[12] * m[3] * m[10];

    inv[9] = -m[0]  * m[9] * m[15] + 
              m[0]  * m[11] * m[13] + 
              m[8]  * m[1] * m[15] - 
              m[8]  * m[3] * m[13] - 
              m[12] * m[1] * m[11] + 
              m[12] * m[3] * m[9];

    inv[13] = m[0]  * m[9] * m[14] - 
              m[0]  * m[10] * m[13] - 
              m[8]  * m[1] * m[14] + 
              m[8]  * m[2] * m[13] + 
              m[12] * m[1] * m[10] - 
              m[12] * m[2] * m[9];

    inv[2] = m[1]  * m[6] * m[15] - 
             m[1]  * m[7] * m[14] - 
             m[5]  * m[2] * m[15] + 
             m[5]  * m[3] * m[14] + 
             m[13] * m[2] * m[7] - 
             m[13] * m[3] * m[6];

    inv[6] = -m[0]  * m[6] * m[15] + 
              m[0]  * m[7] * m[14] + 
              m[4]  * m[2] * m[15] - 
              m[4]  * m[3] * m[14] - 
              m[12] * m[2] * m[7] + 
              m[12] * m[3] * m[6];

    inv[10] = m[0]  * m[5] * m[15] - 
              m[0]  * m[7] * m[13] - 
              m[4]  * m[1] * m[15] + 
              m[4]  * m[3] * m[13] + 
              m[12] * m[1] * m[7] - 
              m[12] * m[3] * m[5];

    inv[14] = -m[0]  * m[5] * m[14] + 
               m[0]  * m[6] * m[13] + 
               m[4]  * m[1] * m[14] - 
               m[4]  * m[2] * m[13] - 
               m[12] * m[1] * m[6] + 
               m[12] * m[2] * m[5];

    inv[3] = -m[1] * m[6] * m[11] + 
              m[1] * m[7] * m[10] + 
              m[5] * m[2] * m[11] - 
              m[5] * m[3] * m[10] - 
              m[9] * m[2] * m[7] + 
              m[9] * m[3] * m[6];

    inv[7] = m[0] * m[6] * m[11] - 
             m[0] * m[7] * m[10] - 
             m[4] * m[2] * m[11] + 
             m[4] * m[3] * m[10] + 
             m[8] * m[2] * m[7] - 
             m[8] * m[3] * m[6];

    inv[11] = -m[0] * m[5] * m[11] + 
               m[0] * m[7] * m[9] + 
               m[4] * m[1] * m[11] - 
               m[4] * m[3] * m[9] - 
               m[8] * m[1] * m[7] + 
               m[8] * m[3] * m[5];

    inv[15] = m[0] * m[5] * m[10] - 
              m[0] * m[6] * m[9] - 
              m[4] * m[1] * m[10] + 
              m[4] * m[2] * m[9] + 
              m[8] * m[1] * m[6] - 
              m[8] * m[2] * m[5];

    float det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

    if (det == 0)
		return PxMat44(PxIdentity);

    det = 1.0f / det;

    for (i = 0; i < 16; i++)
        invOut[i] = inv[i] * det;

	return ret;
}

float ParticleIosActorGPU::distance(PxVec4 a, PxVec4 b)
{
	PxVec3 a3(a.x/a.w,a.y/a.w,a.z/a.w);
	PxVec3 b3(b.x/b.w,b.y/b.w,b.z/b.w);
	PxVec3 diff(b3-a3);
	return diff.magnitude();
}

}
} // namespace nvidia

#endif //APEX_CUDA_SUPPORT

