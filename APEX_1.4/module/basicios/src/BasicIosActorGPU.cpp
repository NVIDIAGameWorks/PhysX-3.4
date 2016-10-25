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

#include "BasicIosActor.h"
#include "BasicIosActorGPU.h"
#include "BasicIosAssetImpl.h"
#include "IofxAsset.h"
#include "IofxActor.h"
#include "ModuleBasicIosImpl.h"
#include "BasicIosSceneGPU.h"
#include "RenderDebugInterface.h"
#include "AuthorableObjectIntl.h"

#include "PxMath.h"

//CUDA
#include "PxGpuTask.h"
#include "ApexCutil.h"

#define CUDA_OBJ(name) SCENE_CUDA_OBJ(*mBasicIosScene, name)

namespace nvidia
{
namespace basicios
{

#pragma warning(disable: 4355) // 'this' : used in base member initializer list

BasicIosActorGPU::BasicIosActorGPU(
    ResourceList& list,
    BasicIosAssetImpl& asset,
    BasicIosScene& scene,
    nvidia::apex::IofxAsset& iofxAsset,
	const ApexMirroredPlace::Enum defaultPlace)
	: BASIC_IOS_ACTOR(list, asset, scene, iofxAsset, true)
	, mCopyQueue(*scene.getApexScene().getTaskManager()->getGpuDispatcher())
	, mHoleScanSum(scene.getApexScene())
	, mMoveIndices(scene.getApexScene())
	, mTmpReduce(scene.getApexScene())
	, mTmpHistogram(scene.getApexScene())
	, mTmpScan(scene.getApexScene())
	, mTmpScan1(scene.getApexScene())
	, mTmpOutput(scene.getApexScene())
	, mTmpOutput1(scene.getApexScene())
	, mLaunchTask(*this)
{
	initStorageGroups(CUDA_OBJ(simulateStorage));

	//CUDA
	mTmpOutput.setSize(4, ApexMirroredPlace::CPU_GPU);
	mTmpOutput1.setSize(2, ApexMirroredPlace::CPU_GPU);

	const unsigned int ScanWarpsPerBlock = MAX_WARPS_PER_BLOCK; //CUDA_OBJ(scanKernel).getBlockDim().x / WARP_SIZE;
	mTmpReduce.reserve(MAX_BOUND_BLOCKS * 4, defaultPlace);
	mTmpHistogram.reserve(MAX_BOUND_BLOCKS * HISTOGRAM_SIMULATE_BIN_COUNT, defaultPlace);
	mTmpScan.reserve(MAX_BOUND_BLOCKS * ScanWarpsPerBlock, defaultPlace);
	mTmpScan1.reserve(MAX_BOUND_BLOCKS * ScanWarpsPerBlock, defaultPlace);

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

	if (mAsset->mParams->collisionWithConvex)
	{
		mConvexPlanes.reserve(MAX_CONVEX_PLANES_COUNT, ApexMirroredPlace::CPU_GPU);
		mConvexVerts.reserve(MAX_CONVEX_VERTS_COUNT, ApexMirroredPlace::CPU_GPU);
		mConvexPolygonsData.reserve(MAX_CONVEX_POLYGONS_DATA_SIZE, ApexMirroredPlace::CPU_GPU);
	}
	if (mAsset->mParams->collisionWithTriangleMesh)
	{
		mTrimeshVerts.reserve(MAX_TRIMESH_VERTS_COUNT, ApexMirroredPlace::CPU_GPU);
		mTrimeshIndices.reserve(MAX_TRIMESH_INDICES_COUNT, ApexMirroredPlace::CPU_GPU);
	}

	mHoleScanSum.reserve(mMaxTotalParticleCount, defaultPlace);
	mMoveIndices.reserve(mMaxTotalParticleCount, defaultPlace);
}

BasicIosActorGPU::~BasicIosActorGPU()
{
}

void BasicIosActorGPU::submitTasks()
{
	BasicIosActorImpl::submitTasks();

	mInjectorsCounters.setSize(mInjectorList.getSize(), ApexMirroredPlace::CPU_GPU); 
	PxTaskManager* tm = mBasicIosScene->getApexScene().getTaskManager();
	tm->submitUnnamedTask(mLaunchTask, PxTaskType::TT_GPU);
}

void BasicIosActorGPU::setTaskDependencies()
{
	BasicIosActorImpl::setTaskDependencies(&mLaunchTask, true);

	PxTaskManager* tm = mBasicIosScene->getApexScene().getTaskManager();
	if (tm->getGpuDispatcher()->getCudaContextManager()->supportsArchSM20())
	{
		/* For Fermi devices, it pays to launch all IOS together.  This also forces
		 * The IOFX managers to step at the same time.
		 */
		PxTaskID interlock = tm->getNamedTask("IOS::StepInterlock");
		mLaunchTask.startAfter(interlock);
	}
}

bool BasicIosActorGPU::launch(CUstream stream, int kernelIndex)
{
	float deltaTime = mBasicIosScene->getApexScene().getPhysXSimulateTime();

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

	BasicIosSceneGPU* sceneGPU = static_cast<BasicIosSceneGPU*>(mBasicIosScene);
	bool useSyncKernels = !sceneGPU->getGpuDispatcher()->getCudaContextManager()->supportsArchSM20();

	switch (kernelIndex)
	{
	case 0:
		if (!mFieldSamplerQuery && mOnStartCallback)
		{
			(*mOnStartCallback)(stream);
		}

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
			mBufDesc.pmaUserData->copyHostToDeviceQ(mCopyQueue, mInjectedCount, mParticleCount);
		}
		if (mAsset->mParams->collisionWithConvex)
		{
			mConvexPlanes.copyHostToDeviceQ(mCopyQueue);
			mConvexVerts.copyHostToDeviceQ(mCopyQueue);
			mConvexPolygonsData.copyHostToDeviceQ(mCopyQueue);
		}
		if (mAsset->mParams->collisionWithTriangleMesh)
		{
			mTrimeshVerts.copyHostToDeviceQ(mCopyQueue);
			mTrimeshIndices.copyHostToDeviceQ(mCopyQueue);
		}
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
					mTmpOutput1.getGpuPtr(),
					mTmpHistogram.getGpuPtr()
				);
			}
			else
			{
				uint32_t histogramGridSize =
					CUDA_OBJ(histogramKernel)(
						stream, totalCount,
						createApexCudaMemRef(mBenefit, totalCount, ApexCudaMemFlags::IN), 
						boundCount,	benefitMin, benefitMax,
						createApexCudaMemRef(mTmpOutput1, ApexCudaMemFlags::IN),						
						createApexCudaMemRef(mTmpHistogram, ApexCudaMemFlags::OUT),
						1, 0
					);

				//launch just 1 block
				CUDA_OBJ(histogramKernel)(
					stream, APEX_CUDA_SINGLE_BLOCK_LAUNCH,
					createApexCudaMemRef(mBenefit, totalCount, ApexCudaMemFlags::IN),
					boundCount, benefitMin, benefitMax,
					createApexCudaMemRef(mTmpOutput1, ApexCudaMemFlags::OUT),
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
					mTmpOutput1.getGpuPtr(),
					mTmpScan.getGpuPtr(), mTmpScan1.getGpuPtr()
				);
			}
			else
			{
				uint32_t scanGridSize = 
					CUDA_OBJ(scanKernel)(
						stream, totalCount,
						benefitMin, benefitMax,
						createApexCudaMemRef(mHoleScanSum, totalCount, ApexCudaMemFlags::IN), 
						createApexCudaMemRef(mBenefit, totalCount, ApexCudaMemFlags::IN),
						createApexCudaMemRef(mTmpOutput1, ApexCudaMemFlags::IN),
						createApexCudaMemRef(mTmpScan, ApexCudaMemFlags::OUT),
						createApexCudaMemRef(mTmpScan1, ApexCudaMemFlags::OUT),
						1, 0
					);

				//launch just 1 block
				CUDA_OBJ(scanKernel)(
					stream, APEX_CUDA_SINGLE_BLOCK_LAUNCH,
					benefitMin, benefitMax,
					createApexCudaMemRef(mHoleScanSum, totalCount, ApexCudaMemFlags::IN), 
					createApexCudaMemRef(mBenefit, totalCount, ApexCudaMemFlags::IN),
					createApexCudaMemRef(mTmpOutput1, ApexCudaMemFlags::IN),
					createApexCudaMemRef(mTmpScan, ApexCudaMemFlags::IN_OUT),
					createApexCudaMemRef(mTmpScan1, ApexCudaMemFlags::IN_OUT),
					2, scanGridSize
				);

				CUDA_OBJ(scanKernel)(
					stream, totalCount,
					benefitMin, benefitMax,
					createApexCudaMemRef(mHoleScanSum, totalCount, ApexCudaMemFlags::OUT),
					createApexCudaMemRef(mBenefit, totalCount, ApexCudaMemFlags::IN),
					createApexCudaMemRef(mTmpOutput1, ApexCudaMemFlags::IN),
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
				createApexCudaMemRef(mMoveIndices, totalCount, ApexCudaMemFlags::OUT),
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
				APEX_CUDA_TEXTURE_SCOPE_BIND(texRefMoveIndices, mMoveIndices);

				APEX_CUDA_TEXTURE_SCOPE_BIND(texRefPositionMass, *mBufDesc.pmaPositionMass);
				APEX_CUDA_TEXTURE_SCOPE_BIND(texRefVelocityLife, *mBufDesc.pmaVelocityLife);
				APEX_CUDA_TEXTURE_SCOPE_BIND(texRefIofxActorIDs, *mBufDesc.pmaActorIdentifiers);
				APEX_CUDA_TEXTURE_SCOPE_BIND(texRefLifeSpan, mLifeSpan);
				APEX_CUDA_TEXTURE_SCOPE_BIND(texRefLifeTime, mLifeTime);
				APEX_CUDA_TEXTURE_SCOPE_BIND(texRefInjector, mInjector);

				APEX_CUDA_TEXTURE_SCOPE_BIND(texRefUserData, *mBufDesc.pmaUserData);

				APEX_CUDA_TEXTURE_SCOPE_BIND(texRefConvexPlanes, mConvexPlanes);
				APEX_CUDA_TEXTURE_SCOPE_BIND(texRefConvexVerts, mConvexVerts);
				APEX_CUDA_TEXTURE_SCOPE_BIND(texRefConvexPolygonsData, mConvexPolygonsData);

				APEX_CUDA_TEXTURE_SCOPE_BIND(texRefTrimeshVerts, mTrimeshVerts);
				APEX_CUDA_TEXTURE_SCOPE_BIND(texRefTrimeshIndices, mTrimeshIndices);

				PxVec3 gravity = -mUp;
				const PxVec3& eyePos = mBasicIosScene->getApexScene().getEyePosition();
			
				if (mFieldSamplerQuery != NULL)
				{
					APEX_CUDA_TEXTURE_SCOPE_BIND(texRefField, mField);

					histogramGridSize = CUDA_OBJ(simulateApplyFieldKernel)(stream, 
						targetCount,
						lastCount,
						deltaTime,
						gravity,
						eyePos,
						sceneGPU->mInjectorConstMemGroup.getStorage().mappedHandle(sceneGPU->mInjectorParamsArrayHandle), mInjectorsCounters.getSize(),
						createApexCudaMemRef(mHoleScanSum, targetCount, ApexCudaMemFlags::IN),
						createApexCudaMemRef(mTmpScan, 1, ApexCudaMemFlags::IN),
						createApexCudaMemRef(mTmpHistogram, ApexCudaMemFlags::OUT),
						createApexCudaMemRef(mInjectorsCounters, mInjectorsCounters.getSize(), ApexCudaMemFlags::OUT),
						createApexCudaMemRef((float4*)mBufDesc.pmaPositionMass->getGpuPtr(), targetCount, ApexCudaMemFlags::OUT),
						createApexCudaMemRef((float4*)mBufDesc.pmaVelocityLife->getGpuPtr(), targetCount, ApexCudaMemFlags::OUT),
						createApexCudaMemRef((float4*)mBufDesc.pmaCollisionNormalFlags->getGpuPtr(), targetCount, ApexCudaMemFlags::OUT),
						createApexCudaMemRef(mBufDesc.pmaUserData->getGpuPtr(), targetCount, ApexCudaMemFlags::OUT),
						createApexCudaMemRef(mLifeSpan, targetCount, ApexCudaMemFlags::OUT),
						createApexCudaMemRef(mLifeTime, targetCount, ApexCudaMemFlags::OUT),
						createApexCudaMemRef(mInjector, targetCount, ApexCudaMemFlags::OUT),
						createApexCudaMemRef(*(mBufDesc.pmaActorIdentifiers),  targetCount, ApexCudaMemFlags::OUT),
						createApexCudaMemRef(mBenefit, targetCount, ApexCudaMemFlags::OUT),
						mSimulationStorageGroup.getStorage().mappedHandle(mSimulationParamsHandle)
						);
				}
				else
				{
					histogramGridSize = CUDA_OBJ(simulateKernel)(stream, targetCount,
											 lastCount, deltaTime, gravity, eyePos,
											 sceneGPU->mInjectorConstMemGroup.getStorage().mappedHandle(sceneGPU->mInjectorParamsArrayHandle), mInjectorsCounters.getSize(),
											 mHoleScanSum.getGpuPtr(), mTmpScan.getGpuPtr(), mTmpHistogram.getGpuPtr(), mInjectorsCounters.getGpuPtr(),
											 (float4*)mBufDesc.pmaPositionMass->getGpuPtr(),
											 (float4*)mBufDesc.pmaVelocityLife->getGpuPtr(),
											 (float4*)mBufDesc.pmaCollisionNormalFlags->getGpuPtr(),
											 mBufDesc.pmaUserData->getGpuPtr(),
											 mLifeSpan.getGpuPtr(), mLifeTime.getGpuPtr(), mInjector.getGpuPtr(), mBufDesc.pmaActorIdentifiers->getGpuPtr(),
											 mBenefit.getGpuPtr(), mSimulationStorageGroup.getStorage().mappedHandle(mSimulationParamsHandle)
											);
				}
			}
			//new kernel invocation - to merge temp histograms 
			{
				if(mInjectorsCounters.getSize() <= HISTOGRAM_SIMULATE_BIN_COUNT)
				{
					CUDA_OBJ(mergeHistogramKernel)(stream, APEX_CUDA_SINGLE_BLOCK_LAUNCH,
						createApexCudaMemRef(mInjectorsCounters, mInjectorsCounters.getSize(), ApexCudaMemFlags::IN),
						createApexCudaMemRef(mTmpHistogram, ApexCudaMemFlags::IN),
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
					BasicIOSAssetParam* gridParams = (BasicIOSAssetParam*)(mAsset->getAssetNvParameterized());
					mGridDensityParams.GridSize = gridParams->GridDensity.GridSize;
					mGridDensityParams.GridMaxCellCount = gridParams->GridDensity.MaxCellCount;
				}
				// extract frustum
				if (mBasicIosScene->getApexScene().getNumProjMatrices() > 0)
				{
					PxMat44 matDen = PxMat44(PxIdentity);
					GridDensityFrustumParams frustum;
					PxMat44 matModel = mBasicIosScene->getApexScene().getViewMatrix();
					PxMat44 matProj  = mBasicIosScene->getApexScene().getProjMatrix();
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
					// see how skewed the eye point is
					PxVec3 eye;
					{
						// find the eye point
						PxVec4 a4 = matInv.transform(PxVec4(1.f,1.f,0.00f,1.f));
						PxVec4 b4 = matInv.transform(PxVec4(1.f,1.f,0.01f,1.f));
						PxVec4 c4 = matInv.transform(PxVec4(-1.f,-1.f,0.00f,1.f));
						PxVec4 d4 = matInv.transform(PxVec4(-1.f,-1.f,0.01f,1.f));
						PxVec3 a3 = a4.getXYZ()/a4.w;
						PxVec3 b3 = b4.getXYZ()/b4.w;
						PxVec3 c3 = c4.getXYZ()/c4.w;
						PxVec3 d3 = d4.getXYZ()/d4.w;
						PxVec3 a = b3-a3;
						PxVec3 b = d3-c3;
						PxVec3 c = a.cross(b);
						PxVec3 d = a3-c3;
						PxMat33 m(a,b,c);
						PxMat33 mInv = m.getInverse();
						PxVec3 coord = mInv.transform(d);
						eye = c3 + (d3-c3)*coord.y;		
					}
					// build scale,rotation,translation matrix
					PxMat44 mat1Inv = PxMat44(PxIdentity);
					mat1Inv.column0 = PxVec4(basisX3,0.f);
					mat1Inv.column1 = PxVec4(basisY3,0.f);
					mat1Inv.column2 = PxVec4(basisZ3,0.f);
					mat1Inv.column3 = PxVec4(origin3,1.f);
					PxMat44 mat1 = inverse(mat1Inv);
					PxVec3 eyeOffset = mat1.transform(eye);
					// do perspective transform
					PxMat44 mat2 = PxMat44(PxIdentity);
					{
						float xshift = -2.f*(eyeOffset.x-0.5f);
						float yshift = -2.f*(eyeOffset.y-0.5f);
						float left		= -3.0f + xshift;
						float right	= 1.0f + xshift;
						float top		= 1.0f + yshift;
						float bottom	= -3.0f + yshift;
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
					float factor = PxMin((float)(mGridDensityParams.GridResolution-4) / (mGridDensityParams.GridResolution),0.75f);
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
						createApexCudaMemRef(mBenefit, targetCount, ApexCudaMemFlags::IN),
						createApexCudaMemRef((float4*)mTmpOutput.getGpuPtr(), 1, ApexCudaMemFlags::IN),
						createApexCudaMemRef(mTmpReduce, ApexCudaMemFlags::OUT),
						1, 0
					);

				//launch just 1 block
				CUDA_OBJ(reduceKernel)(
					stream, APEX_CUDA_SINGLE_BLOCK_LAUNCH,
					createApexCudaMemRef(mBenefit, targetCount, ApexCudaMemFlags::IN),
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
								  createApexCudaMemRef(mTmpScan, 1, ApexCudaMemFlags::IN),
								  createApexCudaMemRef(*mBufDesc.pmaInStateToInput, totalCount, ApexCudaMemFlags::OUT),
								  createApexCudaMemRef(*mBufDesc.pmaOutStateToInput, lastCount, ApexCudaMemFlags::IN)
			                     );

		}
		return true;

	case 7:
		mTmpOutput.copyDeviceToHostQ(mCopyQueue);
		mInjectorsCounters.copyDeviceToHostQ(mCopyQueue);

		if (mTmpScan.cpuPtrIsValid())
		{
			mTmpOutput1.copyDeviceToHostQ(mCopyQueue);
			mTmpScan.copyDeviceToHostQ(mCopyQueue, 1);
			mHoleScanSum.copyDeviceToHostQ(mCopyQueue, totalCount);
			mMoveIndices.copyDeviceToHostQ(mCopyQueue, totalCount);
			mTmpHistogram.copyDeviceToHostQ(mCopyQueue, HISTOGRAM_BIN_COUNT);
			mBenefit.copyDeviceToHostQ(mCopyQueue, totalCount);
			mBufDesc.pmaInStateToInput->copyDeviceToHostQ(mCopyQueue, totalCount);
			mBufDesc.pmaPositionMass->copyDeviceToHostQ(mCopyQueue, targetCount);
		}
		mCopyQueue.flushEnqueued();

		/* Oh! Manager of the IOFX! do your thing */
		mIofxMgr->updateEffectsData(deltaTime, targetCount, targetCount, totalCount, stream);
		return false;
	}
	return false;
}

void BasicIosActorGPU::fetchResults()
{
	BASIC_IOS_ACTOR::fetchResults();

	uint32_t totalCount = mParticleCount + mInjectedCount;
	checkBenefit(totalCount);
	checkHoles(totalCount);
	checkInState(totalCount);
	/*checkHistogram(PxMax<int32_t>(mLastActiveCount + mInjectedCount - mParticleBudget, 0), mTmpHistogram[ mTmpOutput1[1] ], mTmpHistogram[ HISTOGRAM_BIN_COUNT - 1 ]);*/
	//this can be done only after TaskLaunchAfterLod is finished!!!

	mParticleCount = mParticleBudget;

	float* pTmpOutput = (float*)mTmpOutput.getPtr();
	mLastActiveCount = mTmpOutput[STATUS_LAST_ACTIVE_COUNT];
	mLastBenefitSum  = pTmpOutput[STATUS_LAST_BENEFIT_SUM];
	mLastBenefitMin  = pTmpOutput[STATUS_LAST_BENEFIT_MIN];
	mLastBenefitMax  = pTmpOutput[STATUS_LAST_BENEFIT_MAX];
}


PxMat44 BasicIosActorGPU::inverse(const PxMat44& in)
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

float BasicIosActorGPU::distance(PxVec4 a, PxVec4 b)
{
	PxVec3 a3(a.x/a.w,a.y/a.w,a.z/a.w);
	PxVec3 b3(b.x/b.w,b.y/b.w,b.z/b.w);
	PxVec3 diff(b3-a3);
	return diff.magnitude();
}

}
} // namespace nvidia

#endif //APEX_CUDA_SUPPORT
