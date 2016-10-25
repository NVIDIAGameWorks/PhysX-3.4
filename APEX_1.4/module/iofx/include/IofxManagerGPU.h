/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __IOFX_MANAGER_GPU_H__
#define __IOFX_MANAGER_GPU_H__

#include "Apex.h"
#include "IofxManager.h"
#include "IofxSceneGPU.h"

#if APEX_CUDA_SUPPORT
#include "ApexCudaWrapper.h"
#include "PxCudaContextManager.h"
#endif

#include "ModifierData.h"
#include "IosObjectData.h"

namespace nvidia
{
namespace apex
{
class SceneIntl;
class Modifier;
}
namespace iofx
{

class IofxAssetImpl;
class IofxScene;
class ModifierParamsMapperGPU;

/* Class which manages a per-IOS CUDA IOFX pipeline */
class IofxManagerGPU : public CudaPipeline, public UserAllocated
{
public:
	IofxManagerGPU(SceneIntl& scene, const IofxManagerDescIntl& desc, IofxManager&, const ApexMirroredPlace::Enum defaultPlace = ApexMirroredPlace::GPU);
	~IofxManagerGPU();

	void release();
	virtual void submitTasks();
	virtual void fetchResults();
	PxTaskID IofxManagerGPU::launchGpuTasks();
	void launchPrep();

	IofxManagerClient* createClient(IofxAssetSceneInst* assetSceneInst, uint32_t actorClassID, const IofxManagerClientIntl::Params& params);
	IofxAssetSceneInst* createAssetSceneInst(IofxAssetImpl* asset, uint32_t semantics);


	IofxManager&        mManager;
	IofxScene&          mIofxScene;

	bool				cudaLaunch(CUstream stream, int kernelIndex);
	void				cudaLaunchRadixSort(CUstream stream, unsigned int numElements, unsigned int keyBits, unsigned int startBit, bool useSyncKernels);

	uint32_t				mCurSeed;
	uint32_t*				mTargetBufDevPtr;
	uint32_t               mCountActorIDs;
	uint32_t               mNumberVolumes;
	uint32_t				mNumberActorClasses;
	uint32_t				mOutputDWords;
	bool                mEmptySimulation;
	PxTask*		mTaskLaunch;

#if APEX_CUDA_SUPPORT
	ApexCudaConstMemGroup					mVolumeConstMemGroup;
	InplaceHandle<VolumeParamsArray>		mVolumeParamsArrayHandle;
	InplaceHandle<ActorClassIDBitmapArray>	mActorClassIDBitmapArrayHandle;

	ApexCudaConstMemGroup					mRemapConstMemGroup;
	InplaceHandle<ActorIDRemapArray>		mActorIDRemapArrayHandle;

	ApexCudaConstMemGroup					mModifierConstMemGroup;
	InplaceHandle<ClientParamsHandleArray>	mClientParamsHandleArrayHandle;
	InplaceHandle<SpriteOutputLayout>		mSpriteOutputLayoutHandle;
	InplaceHandle<MeshOutputLayout>			mMeshOutputLayoutHandle;

	PxGpuCopyDescQueue mCopyQueue;
#endif

	const ApexMirroredPlace::Enum mDefaultPlace;

	ApexMirroredArray<float>	mCuSpawnScale;
	ApexMirroredArray<uint32_t>	mCuSpawnSeed;

	ApexMirroredArray<LCG_PRNG>		mCuBlockPRNGs;

	// sprite sorting, then actor ID sorting
	ApexMirroredArray<uint32_t>	mCuSortedActorIDs;
	ApexMirroredArray<uint32_t>	mCuSortedStateIDs;
	ApexMirroredArray<uint32_t>	mCuSortTempKeys;
	ApexMirroredArray<uint32_t>	mCuSortTempValues;
	ApexMirroredArray<uint32_t>	mCuSortTemp;

	ApexMirroredArray<uint32_t>	mCuActorStart;
	ApexMirroredArray<uint32_t>	mCuActorEnd;
	ApexMirroredArray<uint32_t>	mCuActorVisibleEnd;

	ApexMirroredArray<PxVec4>		mCuMinBounds;
	ApexMirroredArray<PxVec4>		mCuMaxBounds;
	ApexMirroredArray<PxVec4>		mCuTempMinBounds;
	ApexMirroredArray<PxVec4>		mCuTempMaxBounds;
	ApexMirroredArray<uint32_t>		mCuTempActorIDs;
	LCG_PRNG						mRandThreadLeap;
	LCG_PRNG						mRandGridLeap;


	class OutputBuffer
	{
		PxCudaBuffer* mGpuBuffer;

	public:
		OutputBuffer()
		{
			mGpuBuffer = 0;
		}
		~OutputBuffer()
		{
			release();
		}

		PX_INLINE bool isValid() const
		{
			return (mGpuBuffer != 0);
		}

		PX_INLINE void* getGpuPtr() const
		{
			return (mGpuBuffer != 0) ? reinterpret_cast<void*>(mGpuBuffer->getPtr()) : 0;
		}

		void release()
		{
			if (mGpuBuffer != 0)
			{
				mGpuBuffer->free();
				mGpuBuffer = 0;
			}
		}

		void realloc(size_t capacity, PxCudaContextManager* ctx)
		{
			if (mGpuBuffer != 0 && mGpuBuffer->getSize() >= capacity) 
			{
				return;
			}
			if (capacity > 0)
			{
				release();
				mGpuBuffer = ctx->getMemoryManager()->alloc(
								PxCudaBufferType(PxCudaBufferMemorySpace::T_GPU, PxCudaBufferFlags::F_READ_WRITE),
								capacity);
				PX_ASSERT(mGpuBuffer != 0);
			}
		}

		void copyToHost(CUstream stream, void* hostPtr, size_t size)
		{
			if (isValid())
			{
				CUT_SAFE_CALL(cuMemcpyDtoHAsync(hostPtr, CUdeviceptr(mGpuBuffer->getPtr()), size, stream));
			}
		}
	};

	OutputBuffer				mTargetOutputBuffer;
	uint32_t					mTargetTextureCount;
	ApexCudaArray				mTargetCudaArrayList[IofxSpriteRenderLayout::MAX_SURFACE_COUNT];
};

}
} // namespace nvidia

#endif // __IOFX_ACTOR_GPU_H__
