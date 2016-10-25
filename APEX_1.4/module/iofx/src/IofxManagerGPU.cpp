/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "Apex.h"
#include "ApexDefs.h"

#if APEX_CUDA_SUPPORT

#include "ApexSDKIntl.h"
#include "SceneIntl.h"
#include "ModifierImpl.h"
#include "IofxActor.h"
#include "IofxManagerGPU.h"
#include "IofxAssetImpl.h"
#include "IofxSceneGPU.h"

#include "ModuleIofxImpl.h"
#include "IofxActorGPU.h"

#include "PxGpuTask.h"
#include "ApexCutil.h"

#include "RandStateHelpers.h"

#include "IofxRenderData.h"

#define CUDA_OBJ(name) SCENE_CUDA_OBJ(mIofxScene, name)

namespace nvidia
{
namespace iofx
{

class IofxAssetSceneInstGPU : public IofxAssetSceneInst
{
public:
	IofxAssetSceneInstGPU(IofxAssetImpl* asset, uint32_t semantics, IofxScene* scene)
		: IofxAssetSceneInst(asset, semantics)
		, _constMemGroup(SCENE_CUDA_OBJ(*scene, modifierStorage))
	{
		_totalRandomCount = 0;

		APEX_CUDA_CONST_MEM_GROUP_SCOPE(_constMemGroup)

		_storage_.alloc(_assetParamsHandle);
		AssetParams assetParams;
		buildModifierList(assetParams.spawnModifierList, _asset->mSpawnModifierStack);
		buildModifierList(assetParams.continuousModifierList, _asset->mContinuousModifierStack);
		_storage_.update(_assetParamsHandle, assetParams);
	}
	virtual ~IofxAssetSceneInstGPU() {}

	InplaceHandle<AssetParams> getAssetParamsHandle() const
	{
		return _assetParamsHandle;
	}

private:

	void buildModifierList(ModifierList& list, const ModifierStack& stack)
	{
		InplaceStorage& _storage_ = _constMemGroup.getStorage();

		class Mapper : public ModifierParamsMapperGPU
		{
		public:
			InplaceStorage* storage;

			InplaceHandleBase paramsHandle;
			uint32_t paramsRandomCount;

			virtual InplaceStorage& getStorage()
			{
				return *storage;
			}

			virtual void  onParams(InplaceHandleBase handle, uint32_t randomCount)
			{
				paramsHandle = handle;
				paramsRandomCount = randomCount;
			}

		} mapper;
		mapper.storage = &_storage_;

		list.resize(_storage_, stack.size());

		uint32_t index = 0;
		for (ModifierStack::ConstIterator it = stack.begin(); it != stack.end(); ++it)
		{
			uint32_t type = (*it)->getModifierType();
			//NxU32 usage = (*it)->getModifierUsage();
			//if ((usage & usageStage) == usageStage && (usage & usageClass) == usageClass)
			{
				const ModifierImpl* modifier = ModifierImpl::castFrom(*it);
				modifier->mapParamsGPU(mapper);

				ModifierListElem listElem;
				listElem.type = type;
				listElem.paramsHandle = mapper.paramsHandle;
				list.updateElem(_storage_, listElem, index);

				_totalRandomCount += mapper.paramsRandomCount;
			}
			++index;
		}
	}

	ApexCudaConstMemGroup		_constMemGroup;
	InplaceHandle<AssetParams>	_assetParamsHandle;
	uint32_t				_totalRandomCount;
};

class IofxManagerClientGPU : public IofxManagerClient
{
public:
	IofxManagerClientGPU(IofxAssetSceneInst* assetSceneInst, uint32_t actorClassID, const IofxManagerClientIntl::Params& params, IofxScene* scene)
		: IofxManagerClient(assetSceneInst, actorClassID, params)
		, _constMemGroup(SCENE_CUDA_OBJ(*scene, modifierStorage))
	{
		setParamsGPU();
	}

	InplaceHandle<ClientParams> getClientParamsHandle() const
	{
		return _clientParamsHandle;
	}

	// IofxManagerClientIntl interface
	virtual void setParams(const IofxManagerClientIntl::Params& params)
	{
		IofxManagerClient::setParams(params);
		setParamsGPU();
	}

private:
	void setParamsGPU()
	{
		APEX_CUDA_CONST_MEM_GROUP_SCOPE(_constMemGroup)

		ClientParams clientParams;
		if (_clientParamsHandle.allocOrFetch(_storage_, clientParams))
		{
			clientParams.assetParamsHandle = static_cast<IofxAssetSceneInstGPU*>(_assetSceneInst)->getAssetParamsHandle();
		}
		clientParams.objectScale = _params.objectScale;
		_clientParamsHandle.update(_storage_, clientParams);
	}

	ApexCudaConstMemGroup		_constMemGroup;
	InplaceHandle<ClientParams>	_clientParamsHandle;
};


IofxManagerClient* IofxManagerGPU::createClient(IofxAssetSceneInst* assetSceneInst, uint32_t actorClassID, const IofxManagerClientIntl::Params& params)
{
	return PX_NEW(IofxManagerClientGPU)(assetSceneInst, actorClassID, params, &mIofxScene);
}

IofxAssetSceneInst* IofxManagerGPU::createAssetSceneInst(IofxAssetImpl* asset,uint32_t semantics)
{
	return PX_NEW(IofxAssetSceneInstGPU)(asset, semantics, &mIofxScene);
}

class IofxManagerLaunchTask : public PxGpuTask, public UserAllocated
{
public:
	IofxManagerLaunchTask(IofxManagerGPU* actor) : mActor(actor) {}
	const char* getName() const
	{
		return "IofxManagerLaunchTask";
	}
	void         run()
	{
		PX_ALWAYS_ASSERT();
	}
	bool         launchInstance(CUstream stream, int kernelIndex)
	{
		return mActor->cudaLaunch(stream, kernelIndex);
	}
	PxGpuTaskHint::Enum getTaskHint() const
	{
		return PxGpuTaskHint::Kernel;
	}

protected:
	IofxManagerGPU* mActor;
};

IofxManagerGPU::IofxManagerGPU(SceneIntl& scene, const IofxManagerDescIntl& desc, IofxManager& mgr, const ApexMirroredPlace::Enum defaultPlace)
	: mManager(mgr)
	, mIofxScene(*mgr.mIofxScene)
	, mCopyQueue(*scene.getTaskManager()->getGpuDispatcher())
	, mDefaultPlace(defaultPlace)
	, mCuSpawnScale(scene)
	, mCuSpawnSeed(scene)
	, mCuBlockPRNGs(scene)
	, mCuSortedActorIDs(scene)
	, mCuSortedStateIDs(scene)
	, mCuSortTempKeys(scene)
	, mCuSortTempValues(scene)
	, mCuSortTemp(scene)
	, mCuMinBounds(scene)
	, mCuMaxBounds(scene)
	, mCuTempMinBounds(scene)
	, mCuTempMaxBounds(scene)
	, mCuTempActorIDs(scene)
	, mCuActorStart(scene)
	, mCuActorEnd(scene)
	, mCuActorVisibleEnd(scene)
	, mCurSeed(0)
	, mTargetBufDevPtr(NULL)
	, mCountActorIDs(0)
	, mNumberVolumes(0)
	, mNumberActorClasses(0)
	, mEmptySimulation(false)
	, mVolumeConstMemGroup(CUDA_OBJ(migrationStorage))
	, mRemapConstMemGroup(CUDA_OBJ(remapStorage))
	, mModifierConstMemGroup(CUDA_OBJ(modifierStorage))
{
	mTaskLaunch = PX_NEW(IofxManagerLaunchTask)(this);

	const uint32_t maxObjectCount = desc.maxObjectCount;
	const uint32_t maxInStateCount = desc.maxInStateCount;
	uint32_t usageClass = 0;
	uint32_t blockSize = MAX_THREADS_PER_BLOCK;

	if (mManager.mIsMesh)
	{
		usageClass = ModifierUsage_Mesh;
		//blockSize = CUDA_OBJ(meshModifiersKernel).getBlockDim().x;
	}
	else
	{
		usageClass = ModifierUsage_Sprite;
		//blockSize = CUDA_OBJ(spriteModifiersKernel).getBlockDim().x;
	}

	mCuSpawnScale.reserve(mManager.mOutStateOffset + maxObjectCount, ApexMirroredPlace::GPU);
	mCuSpawnSeed.reserve(mManager.mOutStateOffset + maxObjectCount, ApexMirroredPlace::GPU);

	mCuSortedActorIDs.reserve(maxInStateCount, defaultPlace);
	mCuSortedStateIDs.reserve(maxInStateCount, defaultPlace);

	mCuSortTempKeys.reserve(maxInStateCount, ApexMirroredPlace::GPU);
	mCuSortTempValues.reserve(maxInStateCount, ApexMirroredPlace::GPU);
	mCuSortTemp.reserve(MAX_BOUND_BLOCKS * NEW_SORT_KEY_DIGITS, ApexMirroredPlace::GPU);

	mCuTempMinBounds.reserve(WARP_SIZE * 2, ApexMirroredPlace::GPU);
	mCuTempMaxBounds.reserve(WARP_SIZE * 2, ApexMirroredPlace::GPU);
	mCuTempActorIDs.reserve(WARP_SIZE * 2, ApexMirroredPlace::GPU);

	// alloc volumeConstMem
	{
		APEX_CUDA_CONST_MEM_GROUP_SCOPE(mVolumeConstMemGroup)

		mVolumeParamsArrayHandle.alloc(_storage_);
		mActorClassIDBitmapArrayHandle.alloc(_storage_);
	}

	// alloc remapConstMem
	{
		APEX_CUDA_CONST_MEM_GROUP_SCOPE(mRemapConstMemGroup)

		mActorIDRemapArrayHandle.alloc(_storage_);
	}

	// alloc modifierConstMem
	{
		APEX_CUDA_CONST_MEM_GROUP_SCOPE(mModifierConstMemGroup)

		mClientParamsHandleArrayHandle.alloc(_storage_);

		if (mManager.mIsMesh)
		{
			mMeshOutputLayoutHandle.alloc(_storage_);
		}
		else
		{
			mSpriteOutputLayoutHandle.alloc(_storage_);
		}
	}

	InitDevicePRNGs(scene, blockSize, mRandThreadLeap, mRandGridLeap, mCuBlockPRNGs);
}

void IofxManagerGPU::release()
{
	delete this;
}

IofxManagerGPU::~IofxManagerGPU()
{
	delete mTaskLaunch;
}


void IofxManagerGPU::submitTasks()
{
	mNumberActorClasses = mManager.mActorClassTable.size();
	mNumberVolumes = mManager.mVolumeTable.size();
	mCountActorIDs = mManager.mActorTable.size() * mNumberVolumes;

	// update volumeConstMem
	if (mNumberVolumes)
	{
		APEX_CUDA_CONST_MEM_GROUP_SCOPE(mVolumeConstMemGroup)

		VolumeParamsArray volumeParamsArray;
		_storage_.fetch(mVolumeParamsArrayHandle, volumeParamsArray);
		volumeParamsArray.resize(_storage_, mNumberVolumes);
		_storage_.update(mVolumeParamsArrayHandle, volumeParamsArray);


		ActorClassIDBitmapArray actorClassIDBitmapArray;
		_storage_.fetch(mActorClassIDBitmapArrayHandle, actorClassIDBitmapArray);
		actorClassIDBitmapArray.resize(_storage_, mManager.mVolumeActorClassBitmap.size());
		_storage_.update(mActorClassIDBitmapArrayHandle, actorClassIDBitmapArray);

		actorClassIDBitmapArray.updateRange(_storage_, &mManager.mVolumeActorClassBitmap.front(), actorClassIDBitmapArray.getSize());

		for (uint32_t i = 0 ; i < mNumberVolumes ; i++)
		{
			VolumeParams volumeParams;
			IofxManager::VolumeData& vd = mManager.mVolumeTable[ i ];
			if (vd.vol)
			{
				volumeParams.bounds = vd.mBounds;
				volumeParams.priority = vd.mPri;
			}
			else
			{
				volumeParams.bounds.setEmpty();
				volumeParams.priority = 0;
			}
			volumeParamsArray.updateElem(_storage_, volumeParams, i);
		}
	}
	else
	{
		APEX_DEBUG_WARNING("IofxManager: There is no render volume!");
	}

	// update remapConstMem
	{
		APEX_CUDA_CONST_MEM_GROUP_SCOPE(mRemapConstMemGroup)

		ActorIDRemapArray actorIDRemapArray;
		_storage_.fetch(mActorIDRemapArrayHandle, actorIDRemapArray);
		actorIDRemapArray.resize(_storage_, mNumberActorClasses);
		for (uint32_t i = 0 ; i < mNumberActorClasses ; ++i)
		{
			actorIDRemapArray.updateElem(_storage_, mManager.mActorClassTable[i].actorID, i);
		}
		_storage_.update(mActorIDRemapArrayHandle, actorIDRemapArray);
	}

	// update modifierConstMem
	{
		APEX_CUDA_CONST_MEM_GROUP_SCOPE(mModifierConstMemGroup)

		ClientParamsHandleArray clientParamsHandleArray;
		_storage_.fetch(mClientParamsHandleArrayHandle, clientParamsHandleArray);
		clientParamsHandleArray.resize(_storage_, mNumberActorClasses);
		for (uint32_t i = 0 ; i < mNumberActorClasses ; ++i)
		{
			InplaceHandle<ClientParams> clientParamsHandle;
			IofxManagerClientGPU* clientGPU = static_cast<IofxManagerClientGPU*>(mManager.mActorClassTable[i].client);
			if (clientGPU != NULL)
			{
				clientParamsHandle = clientGPU->getClientParamsHandle();
			}
			clientParamsHandleArray.updateElem(_storage_, clientParamsHandle, i);
		}
		_storage_.update(mClientParamsHandleArrayHandle, clientParamsHandleArray);

		if (mManager.mIsMesh)
		{
			MeshOutputLayout meshOutputLayout;

			IosObjectGpuData* mWorkingData = DYNAMIC_CAST(IosObjectGpuData*)(mManager.mWorkingIosData);
			IofxSharedRenderDataMeshImpl* meshRenderData = DYNAMIC_CAST(IofxSharedRenderDataMeshImpl*)(mWorkingData->renderData);
			const IofxMeshRenderLayout& meshRenderLayout = meshRenderData->getRenderLayout();

			mOutputDWords = meshRenderLayout.stride >> 2;
			meshOutputLayout.stride = meshRenderLayout.stride;
			::memcpy(meshOutputLayout.offsets, meshRenderLayout.offsets, sizeof(meshOutputLayout.offsets));

			_storage_.update(mMeshOutputLayoutHandle, meshOutputLayout);
		}
		else
		{
			SpriteOutputLayout spriteOutputLayout;

			IosObjectGpuData* mWorkingData = DYNAMIC_CAST(IosObjectGpuData*)(mManager.mWorkingIosData);
			IofxSharedRenderDataSpriteImpl* spriteRenderData = DYNAMIC_CAST(IofxSharedRenderDataSpriteImpl*)(mWorkingData->renderData);
			const IofxSpriteRenderLayout& spriteRenderLayout = spriteRenderData->getRenderLayout();
			
			mOutputDWords = spriteRenderLayout.stride >> 2;
			spriteOutputLayout.stride = spriteRenderLayout.stride;
			::memcpy(spriteOutputLayout.offsets, spriteRenderLayout.offsets, sizeof(spriteOutputLayout.offsets));

			_storage_.update(mSpriteOutputLayoutHandle, spriteOutputLayout);
		}
	}

}


#pragma warning(push)
#pragma warning(disable:4312) // conversion from 'CUdeviceptr' to 'uint32_t *' of greater size

PxTaskID IofxManagerGPU::launchGpuTasks()
{
	PxTaskManager* tm = mIofxScene.mApexScene->getTaskManager();
	tm->submitUnnamedTask(*mTaskLaunch, PxTaskType::TT_GPU);
	mTaskLaunch->finishBefore(mManager.mPostUpdateTaskID);
	return mTaskLaunch->getTaskID();
}

void IofxManagerGPU::launchPrep()
{
	IosObjectGpuData* mWorkingData = DYNAMIC_CAST(IosObjectGpuData*)(mManager.mWorkingIosData);

	if (!mWorkingData->numParticles)
	{
		mEmptySimulation = true;
		return;
	}

	mCurSeed = static_cast<uint32_t>(mIofxScene.mApexScene->getSeed());

	PxTaskManager* tm = mIofxScene.mApexScene->getTaskManager();
	PxCudaContextManager* ctx = tm->getGpuDispatcher()->getCudaContextManager();
	{
		PxScopedCudaLock s(*ctx);

		mTargetTextureCount = 0;
		mTargetBufDevPtr = 0;
		if (!mManager.mIsMesh)
		{
			IofxSharedRenderDataSpriteImpl* spriteRenderData = DYNAMIC_CAST(IofxSharedRenderDataSpriteImpl*)(mWorkingData->renderData);
			const IofxSpriteRenderLayout& spriteRenderLayout = spriteRenderData->getRenderLayout();

			mTargetTextureCount = spriteRenderLayout.surfaceCount;
			for( uint32_t i = 0; i < mTargetTextureCount; ++i )
			{
				const CUarray cuArray = spriteRenderData->getSurfaceMappedCudaArray(i);
				if (cuArray != NULL)
				{
					mTargetCudaArrayList[i].assign(cuArray, false);
				}
				else
				{
					CUarray_format format = CUarray_format(0);
					uint32_t numChannels = 0;
					switch (spriteRenderLayout.surfaceElements[i])
					{
					case IofxSpriteRenderLayoutSurfaceElement::POSITION_FLOAT4:
					case IofxSpriteRenderLayoutSurfaceElement::SCALE_ORIENT_SUBTEX_FLOAT4:
					case IofxSpriteRenderLayoutSurfaceElement::COLOR_FLOAT4:
						format = CU_AD_FORMAT_FLOAT;
						numChannels = 4;
						break;
					case IofxSpriteRenderLayoutSurfaceElement::COLOR_RGBA8:
					case IofxSpriteRenderLayoutSurfaceElement::COLOR_BGRA8:
						format = CU_AD_FORMAT_UNSIGNED_INT32;
						numChannels = 1;
					default:
						PX_ALWAYS_ASSERT();
						break;
					}
					const UserRenderSurfaceDesc& desc = spriteRenderLayout.surfaceDescs[i];
					mTargetCudaArrayList[i].create(format, numChannels, uint32_t(desc.width), uint32_t(desc.height), 0, true);
				}
			}
			for( uint32_t i = mTargetTextureCount; i < IofxSpriteRenderLayout::MAX_SURFACE_COUNT; ++i ) {
				mTargetCudaArrayList[i].release();
			}
		}

		if (mTargetTextureCount == 0)
		{
			const CUdeviceptr cudaPtr = mWorkingData->renderData->getBufferMappedCudaPtr();
			if (cudaPtr != 0)
			{
				mTargetOutputBuffer.release();
				mTargetBufDevPtr = reinterpret_cast<uint32_t*>(cudaPtr);
			}
			else
			{
				const size_t size = mWorkingData->renderData->getRenderBufferSize();
				if (size > 0)
				{
					mTargetOutputBuffer.realloc(size, ctx);
					mTargetBufDevPtr = static_cast<uint32_t*>( mTargetOutputBuffer.getGpuPtr() );
				}
			}
		}
	}

	const uint32_t numActorIDValues = mCountActorIDs + 2;
	mCuActorStart.setSize(numActorIDValues, ApexMirroredPlace::CPU_GPU);
	mCuActorEnd.setSize(numActorIDValues, ApexMirroredPlace::CPU_GPU);
	mCuActorVisibleEnd.setSize(numActorIDValues, ApexMirroredPlace::CPU_GPU);
	mCuMinBounds.setSize(numActorIDValues, ApexMirroredPlace::CPU_GPU);
	mCuMaxBounds.setSize(numActorIDValues, ApexMirroredPlace::CPU_GPU);

	mCuSortedActorIDs.setSize(mWorkingData->maxStateID, mDefaultPlace);
	mCuSortedStateIDs.setSize(mWorkingData->maxStateID, mDefaultPlace);

	mManager.positionMass.setSize(mWorkingData->maxInputID, ApexMirroredPlace::CPU_GPU);
	mManager.velocityLife.setSize(mWorkingData->maxInputID, ApexMirroredPlace::CPU_GPU);
	mManager.actorIdentifiers.setSize(mWorkingData->maxInputID, ApexMirroredPlace::CPU_GPU);
	mManager.inStateToInput.setSize(mWorkingData->maxStateID, ApexMirroredPlace::CPU_GPU);
	mManager.outStateToInput.setSize(mWorkingData->numParticles, ApexMirroredPlace::CPU_GPU);
	if (mWorkingData->iosSupportsCollision)
	{
		mManager.collisionNormalFlags.setSize(mWorkingData->maxInputID, ApexMirroredPlace::CPU_GPU);
	}
	if (mWorkingData->iosSupportsDensity)
	{
		mManager.density.setSize(mWorkingData->maxInputID, ApexMirroredPlace::CPU_GPU);
	}
	if (mWorkingData->iosSupportsUserData)
	{
		mManager.userData.setSize(mWorkingData->maxInputID, ApexMirroredPlace::CPU_GPU);
	}

	mEmptySimulation = false;
}

#pragma warning(pop)


///
PX_INLINE uint32_t getHighestBitShift(uint32_t x)
{
	PX_ASSERT(isPowerOfTwo(x));
	return highestSetBit(x);
}

void IofxManagerGPU::cudaLaunchRadixSort(CUstream stream, unsigned int numElements, unsigned int keyBits, unsigned int startBit, bool useSyncKernels)
{
	if (useSyncKernels)
	{
		//we use OLD Radix Sort on Tesla (SM < 2), because it is faster
		CUDA_OBJ(radixSortSyncKernel)(
			stream, numElements,
			mCuSortedActorIDs.getGpuPtr(), mCuSortedStateIDs.getGpuPtr(),
			mCuSortTempKeys.getGpuPtr(), mCuSortTempValues.getGpuPtr(),
			mCuSortTemp.getGpuPtr(), keyBits, startBit
		);
	}
	else
	{
#if 1
		//NEW Radix Sort
		unsigned int totalThreads = (numElements + NEW_SORT_VECTOR_SIZE - 1) / NEW_SORT_VECTOR_SIZE;
		if (CUDA_OBJ(newRadixSortBlockKernel).isSingleBlock(totalThreads))
		{
			//launch just a single block for small sizes
			CUDA_OBJ(newRadixSortBlockKernel)(
				stream, APEX_CUDA_SINGLE_BLOCK_LAUNCH,
				numElements, keyBits, startBit,
				mCuSortedActorIDs.getGpuPtr(), mCuSortedStateIDs.getGpuPtr()
			);
		}
		else
		{
			for (unsigned int bit = startBit; bit < startBit + keyBits; bit += RADIX_SORT_NBITS)
			{
				uint32_t gridSize = 
					CUDA_OBJ(newRadixSortStepKernel)(
						stream, totalThreads,
						numElements, bit,
						mCuSortedActorIDs.getGpuPtr(), mCuSortedStateIDs.getGpuPtr(),
						mCuSortTempKeys.getGpuPtr(), mCuSortTempValues.getGpuPtr(),
						mCuSortTemp.getGpuPtr(),
						1, 0
					);

				//launch just a single block
				CUDA_OBJ(newRadixSortStepKernel)(
					stream, APEX_CUDA_SINGLE_BLOCK_LAUNCH,
					numElements, bit,
					mCuSortedActorIDs.getGpuPtr(), mCuSortedStateIDs.getGpuPtr(),
					mCuSortTempKeys.getGpuPtr(), mCuSortTempValues.getGpuPtr(),
					mCuSortTemp.getGpuPtr(),
					2, gridSize
				);

				CUDA_OBJ(newRadixSortStepKernel)(
					stream, totalThreads,
					numElements, bit,
					mCuSortedActorIDs.getGpuPtr(), mCuSortedStateIDs.getGpuPtr(),
					mCuSortTempKeys.getGpuPtr(), mCuSortTempValues.getGpuPtr(),
					mCuSortTemp.getGpuPtr(),
					3, 0
				);

				mCuSortedActorIDs.swapGpuPtr(mCuSortTempKeys);
				mCuSortedStateIDs.swapGpuPtr(mCuSortTempValues);
			}
		}
#else
		//OLD Radix Sort
		for (unsigned int startBit = 0; startBit < keyBits; startBit += RADIX_SORT_NBITS)
		{
			int gridSize = 
				CUDA_OBJ(radixSortStep1Kernel)(
					stream, numElements,
					mCuSortedActorIDs.getGpuPtr(), mCuSortedStateIDs.getGpuPtr(),
					mCuSortTempKeys.getGpuPtr(), mCuSortTempValues.getGpuPtr(),
					mCuSortTemp.getGpuPtr(), startBit
				);

			//launch just 1 block
			CUDA_OBJ(radixSortStep2Kernel)(
				stream, CUDA_OBJ(radixSortStep2Kernel).getBlockDim().x,
				mCuSortTemp.getGpuPtr(), gridSize
			);

			CUDA_OBJ(radixSortStep3Kernel)(
				stream, numElements,
				mCuSortedActorIDs.getGpuPtr(), mCuSortedStateIDs.getGpuPtr(),
				mCuSortTempKeys.getGpuPtr(), mCuSortTempValues.getGpuPtr(),
				mCuSortTemp.getGpuPtr(), startBit
			);
		}
#endif
	}
}

bool IofxManagerGPU::cudaLaunch(CUstream stream, int kernelIndex)
{
	PxTaskManager* tm = mIofxScene.mApexScene->getTaskManager();

	if (mEmptySimulation)
	{
		return false;
	}

	const uint32_t numActorIDValues = mCountActorIDs + 2;
	//value <  mCountActorIDs     - valid particle with volume
	//value == mCountActorIDs     - homeless particle (no volume or invalid actor class)
	//value == mCountActorIDs + 1 - NOT_A_PARTICLE


	IofxSceneGPU* sceneGPU = static_cast<IofxSceneGPU*>(&mIofxScene);
	bool useSyncKernels = !sceneGPU->getGpuDispatcher()->getCudaContextManager()->supportsArchSM20();

	IosObjectGpuData* mWorkingData = DYNAMIC_CAST(IosObjectGpuData*)(mManager.mWorkingIosData);

	switch (kernelIndex)
	{
	case 0:
		if (mManager.mOnStartCallback)
		{
			(*mManager.mOnStartCallback)(stream);
		}
		mCopyQueue.reset(stream, 24);
		if (!mManager.mCudaIos && mWorkingData->maxInputID > 0)
		{
			mManager.positionMass.copyHostToDeviceQ(mCopyQueue);
			mManager.velocityLife.copyHostToDeviceQ(mCopyQueue);
			mManager.actorIdentifiers.copyHostToDeviceQ(mCopyQueue);
			mManager.inStateToInput.copyHostToDeviceQ(mCopyQueue);
			if (mWorkingData->iosSupportsCollision)
			{
				mManager.collisionNormalFlags.copyHostToDeviceQ(mCopyQueue);
			}
			if (mWorkingData->iosSupportsDensity)
			{
				mManager.density.copyHostToDeviceQ(mCopyQueue);
			}
			if (mWorkingData->iosSupportsUserData)
			{
				mManager.userData.copyHostToDeviceQ(mCopyQueue);
			}
			mCopyQueue.flushEnqueued();
		}
		break;

	case 1:
		/* Volume Migration (input space) */
		CUDA_OBJ(volumeMigrationKernel)(stream,
		                                PxMax(mWorkingData->maxInputID, numActorIDValues),
										mVolumeConstMemGroup.getStorage().mappedHandle(mVolumeParamsArrayHandle),
		                                mVolumeConstMemGroup.getStorage().mappedHandle(mActorClassIDBitmapArrayHandle),
		                                mNumberActorClasses, mNumberVolumes, numActorIDValues,
		                                mManager.actorIdentifiers.getGpuPtr(), mWorkingData->maxInputID,
		                                (const float4*)mManager.positionMass.getGpuPtr(),
		                                mCuActorStart.getGpuPtr(), mCuActorEnd.getGpuPtr(), mCuActorVisibleEnd.getGpuPtr()
		                               );
		break;

	case 2:
		{
			APEX_CUDA_TEXTURE_SCOPE_BIND(texRefRemapPositions,      mManager.positionMass)
			APEX_CUDA_TEXTURE_SCOPE_BIND(texRefRemapActorIDs,       mManager.actorIdentifiers)
			APEX_CUDA_TEXTURE_SCOPE_BIND(texRefRemapInStateToInput, mManager.inStateToInput)

			/* if mDistanceSortingEnabled, sort on camera distance first, else directly make ActorID keys */
			CUDA_OBJ(makeSortKeys)(stream, mWorkingData->maxStateID,
								   mManager.inStateToInput.getGpuPtr(), mWorkingData->maxInputID,
								   mManager.mActorTable.size(), mCountActorIDs,
								   mRemapConstMemGroup.getStorage().mappedHandle(mActorIDRemapArrayHandle),
								   (const float4*)mManager.positionMass.getGpuPtr(), mManager.mDistanceSortingEnabled,
								   mWorkingData->eyePosition, mWorkingData->eyeDirection, mWorkingData->zNear,
								   mCuSortedActorIDs.getGpuPtr(), mCuSortedStateIDs.getGpuPtr());

			if (mManager.mDistanceSortingEnabled)
			{
				cudaLaunchRadixSort(stream, mWorkingData->maxStateID, 32, 0, useSyncKernels);

				/* Generate ActorID sort keys, using distance sorted stateID values */
				CUDA_OBJ(remapKernel)(stream, mWorkingData->maxStateID,
									  mManager.inStateToInput.getGpuPtr(), mWorkingData->maxInputID,
									  mManager.mActorTable.size(), mCountActorIDs,
									  mRemapConstMemGroup.getStorage().mappedHandle(mActorIDRemapArrayHandle),
									  mCuSortedStateIDs.getGpuPtr(), mCuSortedActorIDs.getGpuPtr());
			}
		}
		break;

	case 3:
		/* ActorID Sort (output state space) */
		// input: mCuSortedActorIDs == actorIDs, in distance sorted order
		// input: mCuSortedStateIDs == stateIDs, in distance sorted order

		// output: mCuSortedActorIDs == sorted ActorIDs
		// output: mCuSortedStateIDs == output-to-input state
		{
			//SortedActorIDs could contain values from 0 to mCountActorIDs + 1 (included),
			//so keybits should cover at least mCountActorIDs + 2 numbers
			uint32_t keybits = 0;
			while ((1U << keybits) < numActorIDValues)
			{
				++keybits;
			}

			cudaLaunchRadixSort(stream, mWorkingData->maxStateID, keybits, 0, useSyncKernels);
		}
		break;

	case 4:
		/* Per-IOFX actor particle range detection */
		CUDA_OBJ(actorRangeKernel)(stream, mWorkingData->maxStateID,
		                           mCuSortedActorIDs.getGpuPtr(), mCountActorIDs,
		                           mCuActorStart.getGpuPtr(), mCuActorEnd.getGpuPtr(), mCuActorVisibleEnd.getGpuPtr(),
								   mCuSortedStateIDs.getGpuPtr()
		                          );
		break;

	case 5:
		/* Modifiers (output state space) */
		{
			PX_PROFILE_ZONE("IofxManagerGPUModifiers", GetInternalApexSDK()->getContextId());
			ModifierCommonParams commonParams = mWorkingData->getCommonParams();

			APEX_CUDA_TEXTURE_SCOPE_BIND(texRefPositionMass,     mManager.positionMass)
			APEX_CUDA_TEXTURE_SCOPE_BIND(texRefVelocityLife,     mManager.velocityLife)
			APEX_CUDA_TEXTURE_SCOPE_BIND(texRefInStateToInput,   mManager.inStateToInput)
			APEX_CUDA_TEXTURE_SCOPE_BIND(texRefStateSpawnSeed,   mCuSpawnSeed)
			APEX_CUDA_TEXTURE_SCOPE_BIND(texRefStateSpawnScale,  mCuSpawnScale)

			APEX_CUDA_TEXTURE_SCOPE_BIND(texRefActorIDs,         mManager.actorIdentifiers)

			if (mWorkingData->iosSupportsCollision)
			{
				CUDA_OBJ(texRefCollisionNormalFlags).bindTo(mManager.collisionNormalFlags);
			}
			if (mWorkingData->iosSupportsDensity)
			{
				CUDA_OBJ(texRefDensity).bindTo(mManager.density);
			}
			if (mWorkingData->iosSupportsUserData)
			{
				CUDA_OBJ(texRefUserData).bindTo(mManager.userData);
			}

			PRNGInfo rand;
			rand.g_stateSpawnSeed = mCuSpawnSeed.getGpuPtr();
			rand.g_randBlock = mCuBlockPRNGs.getGpuPtr();
			rand.randGrid = mRandGridLeap;
			rand.randThread = mRandThreadLeap;
			rand.seed = mCurSeed;

			if (mManager.mIsMesh)
			{
				// 3x3 matrix => 9 float scalars => 3 slices

				APEX_CUDA_TEXTURE_SCOPE_BIND(texRefMeshPrivState0, *mManager.privState.slices[0]);
				APEX_CUDA_TEXTURE_SCOPE_BIND(texRefMeshPrivState1, *mManager.privState.slices[1]);
				APEX_CUDA_TEXTURE_SCOPE_BIND(texRefMeshPrivState2, *mManager.privState.slices[2]);

				MeshPrivateStateArgs meshPrivStateArgs;
				meshPrivStateArgs.g_state[0] = mManager.privState.a[0];
				meshPrivStateArgs.g_state[1] = mManager.privState.a[1];
				meshPrivStateArgs.g_state[2] = mManager.privState.a[2];

				CUDA_OBJ(meshModifiersKernel)(ApexKernelConfig(MAX_SMEM_BANKS * mOutputDWords, WARP_SIZE * PxMax<uint32_t>(mOutputDWords, 4)), 
											  stream, mWorkingData->numParticles,
											  mManager.mInStateOffset, mManager.mOutStateOffset,
											  mModifierConstMemGroup.getStorage().mappedHandle(mClientParamsHandleArrayHandle),
											  commonParams,
											  mCuSortedActorIDs.getGpuPtr(), mCuSortedStateIDs.getGpuPtr(),
											  mManager.outStateToInput.getGpuPtr(),
											  meshPrivStateArgs, mCuSpawnScale.getGpuPtr(),
											  rand, mTargetBufDevPtr,
											  mModifierConstMemGroup.getStorage().mappedHandle(mMeshOutputLayoutHandle)
											 );
			}
			else
			{
				// 1 float scalar => 1 slice

				APEX_CUDA_TEXTURE_SCOPE_BIND(texRefSpritePrivState0, *mManager.privState.slices[0]);

				SpritePrivateStateArgs spritePrivStateArgs;
				spritePrivStateArgs.g_state[0] = mManager.privState.a[0];

				IofxSharedRenderDataSpriteImpl* renderDataSprite = static_cast<IofxSharedRenderDataSpriteImpl*>(mWorkingData->renderData);
				const IofxSpriteRenderLayout& spriteRenderLayout = renderDataSprite->getRenderLayout();

				if (mTargetTextureCount > 0)
				{
					SpriteTextureOutputLayout outputLayout;
					outputLayout.textureCount = mTargetTextureCount;
					for (uint32_t i = 0; i < outputLayout.textureCount; ++i)
					{
						outputLayout.textureData[i].layout = static_cast<uint16_t>(spriteRenderLayout.surfaceElements[i]);

						uint32_t width = mTargetCudaArrayList[i].getWidth();
						//width should be a power of 2 and a multiply of WARP_SIZE
						PX_ASSERT(isPowerOfTwo(width));
						PX_ASSERT((width & (WARP_SIZE - 1)) == 0);
						outputLayout.textureData[i].widthShift = static_cast<uint8_t>(highestSetBit(width));

						outputLayout.textureData[i].pitchShift = 0; //unused in GPU mode!
						outputLayout.texturePtr[i] = NULL; //unused in GPU mode!
					}

					if (0 < outputLayout.textureCount) APEX_CUDA_SURFACE_BIND(surfRefOutput0, mTargetCudaArrayList[0], ApexCudaMemFlags::OUT);
					if (1 < outputLayout.textureCount) APEX_CUDA_SURFACE_BIND(surfRefOutput1, mTargetCudaArrayList[1], ApexCudaMemFlags::OUT);
					if (2 < outputLayout.textureCount) APEX_CUDA_SURFACE_BIND(surfRefOutput2, mTargetCudaArrayList[2], ApexCudaMemFlags::OUT);
					if (3 < outputLayout.textureCount) APEX_CUDA_SURFACE_BIND(surfRefOutput3, mTargetCudaArrayList[3], ApexCudaMemFlags::OUT);

					CUDA_OBJ(spriteTextureModifiersKernel)(stream, mWorkingData->numParticles,
														   mManager.mInStateOffset, mManager.mOutStateOffset,
														   mModifierConstMemGroup.getStorage().mappedHandle(mClientParamsHandleArrayHandle),
														   commonParams,
														   mCuSortedActorIDs.getGpuPtr(), mCuSortedStateIDs.getGpuPtr(),
														   mManager.outStateToInput.getGpuPtr(),
														   spritePrivStateArgs, mCuSpawnScale.getGpuPtr(),
														   rand, outputLayout
														  );

					if (0 < outputLayout.textureCount) APEX_CUDA_SURFACE_UNBIND(surfRefOutput0);
					if (1 < outputLayout.textureCount) APEX_CUDA_SURFACE_UNBIND(surfRefOutput1);
					if (2 < outputLayout.textureCount) APEX_CUDA_SURFACE_UNBIND(surfRefOutput2);
					if (3 < outputLayout.textureCount) APEX_CUDA_SURFACE_UNBIND(surfRefOutput3);
				}
				else
				{
					CUDA_OBJ(spriteModifiersKernel)(ApexKernelConfig(MAX_SMEM_BANKS * mOutputDWords, WARP_SIZE * PxMax<uint32_t>(mOutputDWords, 4)),
													stream, mWorkingData->numParticles,
													mManager.mInStateOffset, mManager.mOutStateOffset,
													mModifierConstMemGroup.getStorage().mappedHandle(mClientParamsHandleArrayHandle),
													commonParams,
													mCuSortedActorIDs.getGpuPtr(), mCuSortedStateIDs.getGpuPtr(),
													mManager.outStateToInput.getGpuPtr(),
													spritePrivStateArgs, mCuSpawnScale.getGpuPtr(),
													rand, mTargetBufDevPtr,
													mModifierConstMemGroup.getStorage().mappedHandle(mSpriteOutputLayoutHandle)
												   );
				}
			}

			if (mWorkingData->iosSupportsCollision)
			{
				CUDA_OBJ(texRefCollisionNormalFlags).unbind();
			}
			if (mWorkingData->iosSupportsDensity)
			{
				CUDA_OBJ(texRefDensity).unbind();
			}
			if (mWorkingData->iosSupportsUserData)
			{
				CUDA_OBJ(texRefUserData).unbind();
			}
		}
		break;

	case 6:
		if (mCountActorIDs > 0)
		{
			/* Per-IOFX actor BBox generation */
			APEX_CUDA_TEXTURE_SCOPE_BIND(texRefBBoxPositions, mManager.positionMass)

			if (useSyncKernels)
			{
				CUDA_OBJ(bboxSyncKernel)(
					stream, mWorkingData->numParticles,
					mCuSortedActorIDs.getGpuPtr(),
					mManager.outStateToInput.getGpuPtr(),
					(const float4*)mManager.positionMass.getGpuPtr(),
					(float4*)mCuMinBounds.getGpuPtr(), (float4*)mCuMaxBounds.getGpuPtr(),
					mCuTempActorIDs.getGpuPtr(),
					(float4*)mCuTempMinBounds.getGpuPtr(), (float4*)mCuTempMaxBounds.getGpuPtr()
				);
			}
			else
			{
				uint32_t bboxGridSize =
					CUDA_OBJ(bboxKernel)(
						stream, mWorkingData->numParticles,
						mCuSortedActorIDs.getGpuPtr(),
						mManager.outStateToInput.getGpuPtr(),
						(const float4*)mManager.positionMass.getGpuPtr(),
						(float4*)mCuMinBounds.getGpuPtr(), (float4*)mCuMaxBounds.getGpuPtr(),
						mCuTempActorIDs.getGpuPtr(),
						(float4*)mCuTempMinBounds.getGpuPtr(), (float4*)mCuTempMaxBounds.getGpuPtr(),
						1, 0
					);

				CUDA_OBJ(bboxKernel)(
					stream, APEX_CUDA_SINGLE_BLOCK_LAUNCH,
					mCuSortedActorIDs.getGpuPtr(),
					mManager.outStateToInput.getGpuPtr(),
					(const float4*)mManager.positionMass.getGpuPtr(),
					(float4*)mCuMinBounds.getGpuPtr(), (float4*)mCuMaxBounds.getGpuPtr(),
					mCuTempActorIDs.getGpuPtr(),
					(float4*)mCuTempMinBounds.getGpuPtr(), (float4*)mCuTempMaxBounds.getGpuPtr(),
					2, bboxGridSize
				);
			}
		}
		break;

	case 7:
		if (mTargetTextureCount > 0)
		{
			IofxSharedRenderDataSpriteImpl* spriteRenderData = DYNAMIC_CAST(IofxSharedRenderDataSpriteImpl*)(mWorkingData->renderData);
			PX_ASSERT(spriteRenderData->getRenderLayout().surfaceCount == mTargetTextureCount);

			for (uint32_t i = 0; i < mTargetTextureCount; ++i)
			{
				UserRenderSurface::MappedInfo mappedInfo;
				if (spriteRenderData->getSurfaceMappedInfo(i, mappedInfo))
				{
					const size_t surfaceWidth = spriteRenderData->getRenderLayout().surfaceDescs[i].width;
					size_t copyHeight = (mWorkingData->numParticles + surfaceWidth - 1) / surfaceWidth;
					mTargetCudaArrayList[i].copyToHost(stream, mappedInfo.pData, mappedInfo.rowPitch, 0, 0, copyHeight);
				}
			}
		}
		else
		{
			void* mappedPtr = mWorkingData->renderData->getBufferMappedPtr();
			if (mappedPtr)
			{
				size_t size = (mOutputDWords << 2) * mWorkingData->numParticles;
				mTargetOutputBuffer.copyToHost(stream, mappedPtr, size);
			}
		}
		if (mCountActorIDs > 0)
		{
			mCuMinBounds.copyDeviceToHostQ(mCopyQueue);
			mCuMaxBounds.copyDeviceToHostQ(mCopyQueue);
		}
		mCuActorStart.copyDeviceToHostQ(mCopyQueue);
		mCuActorEnd.copyDeviceToHostQ(mCopyQueue);
		mCuActorVisibleEnd.copyDeviceToHostQ(mCopyQueue);


		if (mCuSortedActorIDs.cpuPtrIsValid())
		{
			mManager.inStateToInput.copyDeviceToHostQ(mCopyQueue);
			mManager.actorIdentifiers.copyDeviceToHostQ(mCopyQueue);
			mManager.outStateToInput.copyDeviceToHostQ(mCopyQueue);
			mManager.positionMass.copyDeviceToHostQ(mCopyQueue);

			mCuSortedActorIDs.copyDeviceToHostQ(mCopyQueue);
			mCuSortedStateIDs.copyDeviceToHostQ(mCopyQueue);
		}
		else if (!mManager.mCudaIos)
		{
			mManager.actorIdentifiers.copyDeviceToHostQ(mCopyQueue);
			mManager.outStateToInput.copyDeviceToHostQ(mCopyQueue);
		}

		mCopyQueue.flushEnqueued();

		if (mManager.mOnFinishCallback)
		{
			(*mManager.mOnFinishCallback)(stream);
		}

		tm->getGpuDispatcher()->addCompletionPrereq(*tm->getTaskFromID(mManager.mPostUpdateTaskID));
		return false;

	default:
		PX_ALWAYS_ASSERT();
		return false;
	}

	return true;
}

void IofxManagerGPU::fetchResults()
{
	IosObjectGpuData* mWorkingData = DYNAMIC_CAST(IosObjectGpuData*)(mManager.mWorkingIosData);
	PX_UNUSED(mWorkingData);

#if 0
	{
		ApexMirroredArray<uint32_t> actorID(*mIofxScene.mApexScene);
		ApexMirroredArray<PxVec4> outMinBounds(*mIofxScene.mApexScene);
		ApexMirroredArray<PxVec4> outMaxBounds(*mIofxScene.mApexScene);
		ApexMirroredArray<PxVec4> outDebugInfo(*mIofxScene.mApexScene);
		ApexMirroredArray<uint32_t> tmpLastActorID(*mIofxScene.mApexScene);
		tmpLastActorID.setSize(64, ApexMirroredPlace::CPU_GPU);

		const uint32_t NE = 2000;
		actorID.setSize(NE, ApexMirroredPlace::CPU_GPU);

		Array<uint32_t> actorCounts;
		actorCounts.reserve(1000);

		uint32_t NA = 0;
		for (uint32_t ie = 0; ie < NE; ++NA)
		{
			uint32_t num_ie = rand(1, 100); // We need to use QDSRand here s.t. seed could be preset during tests!
			uint32_t next_ie = PxMin(ie + num_ie, NE);

			actorCounts.pushBack(next_ie - ie);

			for (; ie < next_ie; ++ie)
			{
				actorID[ie] = NA;
			}
		}
		outMinBounds.setSize(NA, ApexMirroredPlace::CPU_GPU);
		outMaxBounds.setSize(NA, ApexMirroredPlace::CPU_GPU);
		outDebugInfo.setSize(NA, ApexMirroredPlace::CPU_GPU);

		for (uint32_t ia = 0; ia < NA; ++ia)
		{
			outMinBounds[ia].setZero();
			outMaxBounds[ia].setZero();
		}

		PxTaskManager* tm = mIofxScene.mApexScene->getTaskManager();
		PxCudaContextManager* ctx = tm->getGpuDispatcher()->getCudaContextManager();
		PxScopedCudaLock s(*ctx);

		mCopyQueue.reset(0, 4);

		actorID.copyHostToDeviceQ(mCopyQueue);
		outMinBounds.copyHostToDeviceQ(mCopyQueue);
		outMaxBounds.copyHostToDeviceQ(mCopyQueue);
		mCopyQueue.flushEnqueued();

		CUDA_OBJ(bboxKernel2)(0, NE, actorID.getGpuPtr(), NULL, 0, (float4*)outDebugInfo.getGpuPtr(), (float4*)outMinBounds.getGpuPtr(), (float4*)outMaxBounds.getGpuPtr()/*, tmpLastActorID.getGpuPtr()*/);

		outMinBounds.copyDeviceToHostQ(mCopyQueue);
		outMaxBounds.copyDeviceToHostQ(mCopyQueue);
		outDebugInfo.copyDeviceToHostQ(mCopyQueue);
		tmpLastActorID.copyDeviceToHostQ(mCopyQueue);
		mCopyQueue.flushEnqueued();

		CUT_SAFE_CALL(cuCtxSynchronize());

		uint32_t errors = 0;
		float totCount = 0;
		for (uint32_t ie = 0; ie < NE; ++ie)
		{
			uint32_t id = actorID[ie];
			if (ie == 0 || actorID[ie - 1] != id)
			{
				uint32_t count = actorCounts[id];
				const PxVec4& bounds = outMinBounds[id];
				if (bounds.x != count)
				{
					++errors;
				}
				if (bounds.y != count * 2)
				{
					++errors;
				}
				if (bounds.z != count * 3)
				{
					++errors;
				}
				totCount += count;
			}
		}

	}
#endif

#if 0
	{
		PxTaskManager* tm = mIofxScene.mApexScene->getTaskManager();
		PxCudaContextManager* ctx = tm->getGpuDispatcher()->getCudaContextManager();

		PxScopedCudaLock s(*ctx);

		CUT_SAFE_CALL(cuCtxSynchronize());
	}
#endif
#if DEBUG_GPU
	{
		nvidia::Array<int> valuesCounters(mWorkingData->maxStateID, 0);
		uint32_t lastKey = uint32_t(-1);
		for (uint32_t i = 0; i < mWorkingData->maxStateID; ++i)
		{
			uint32_t currKey = mCuSortedActorIDs.get(i);
			PX_ASSERT(currKey < mCountActorIDs + 2);
			if (lastKey != uint32_t(-1))
			{
				PX_ASSERT(lastKey <= currKey);
			}
			if (lastKey != currKey)
			{
				if (mCuActorStart[currKey] != i)
				{
					int temp = 0;
					temp++;
				}
				PX_ASSERT(mCuActorStart[currKey] == i);
				if (lastKey != uint32_t(-1))
				{
					if (mCuActorEnd[lastKey] != i)
					{
						int temp = 0;
						temp++;
					}
					PX_ASSERT(mCuActorEnd[lastKey] == i);
				}
			}
			lastKey = currKey;

			uint32_t currValue = (mCuSortedStateIDs.get(i) & STATE_ID_MASK);
			PX_ASSERT(currValue < mWorkingData->maxStateID);
			if (currValue < mWorkingData->maxStateID)
			{
				valuesCounters[currValue] += 1;
			}
		}
		if (lastKey != uint32_t(-1))
		{
			PX_ASSERT(mCuActorEnd[lastKey] == mWorkingData->maxStateID);
		}
		for (uint32_t i = 0; i < mWorkingData->maxStateID; ++i)
		{
			PX_ASSERT(valuesCounters[i] == 1);
		}
	}
#endif

	/* Swap input/output state offsets */
	mManager.swapStates();

	if (mEmptySimulation)
	{
		for (uint32_t i = 0 ; i < mNumberVolumes ; i++)
		{
			IofxManager::VolumeData& d = mManager.mVolumeTable[ i ];
			if (d.vol == 0)
			{
				continue;
			}

			for (uint32_t j = 0 ; j < mManager.mActorTable.size() ; j++)
			{
				IofxActorImpl* iofx = d.mActors[ j ];
				if (iofx && iofx != DEFERRED_IOFX_ACTOR)
				{
					iofx->mResultBounds.setEmpty();
					iofx->mResultRange.startIndex = 0;
					iofx->mResultRange.objectCount = 0;
					iofx->mResultVisibleCount = 0;
				}
			}
		}
	}
	else
	{
		PX_ASSERT(mCuActorStart.cpuPtrIsValid() && mCuActorEnd.cpuPtrIsValid());
		if (!mCuActorStart.cpuPtrIsValid() || !mCuActorEnd.cpuPtrIsValid())
		{
			// Workaround for issue seen by a customer
			APEX_INTERNAL_ERROR("Bad cpuPtr in IofxManagerGPU::fetchResults");
			return;
		}
#ifndef NDEBUG
		//check Actor Ranges
		{
			uint32_t totalCount = 0;
			//range with the last index (= mCountActorIDs) contains homeless particles!
			for (uint32_t i = 0 ; i <= mCountActorIDs ; i++)
			{
				const uint32_t rangeStart = mCuActorStart[ i ];
				const uint32_t rangeEnd = mCuActorEnd[ i ];
				const uint32_t rangeVisibleEnd = mCuActorVisibleEnd[ i ];

				PX_ASSERT(rangeStart < mWorkingData->numParticles);
				PX_ASSERT(rangeEnd <= mWorkingData->numParticles);
				PX_ASSERT(rangeStart <= rangeEnd);
				PX_ASSERT(rangeStart <= rangeVisibleEnd && rangeVisibleEnd <= rangeEnd);
				PX_UNUSED(rangeVisibleEnd);

				const uint32_t rangeCount = rangeEnd - rangeStart;
				totalCount += rangeCount;
			}
			PX_ASSERT(totalCount == mWorkingData->numParticles);
		}
#endif

		uint32_t aid = 0;
		for (uint32_t i = 0 ; i < mNumberVolumes ; i++)
		{
			IofxManager::VolumeData& d = mManager.mVolumeTable[ i ];
			if (d.vol == 0)
			{
				aid += mManager.mActorTable.size();
				continue;
			}

			for (uint32_t j = 0 ; j < mManager.mActorTable.size() ; j++)
			{
				const uint32_t rangeStart = mCuActorStart[ aid ];
				const uint32_t rangeEnd = mCuActorEnd[ aid ];
				const uint32_t rangeVisibleEnd = mCuActorVisibleEnd[ aid ];

				const uint32_t rangeCount = rangeEnd - rangeStart;
				const uint32_t visibleCount = rangeVisibleEnd - rangeStart;

				if (d.mActors[ j ] == DEFERRED_IOFX_ACTOR && mManager.mActorTable[ j ] != NULL &&
				        (mIofxScene.mModule->mDeferredDisabled || rangeCount))
				{
					IofxActorImpl* iofxActor = PX_NEW(IofxActorGPU)(mManager.mActorTable[j]->getRenderResID(), &mIofxScene, mManager);
					if (d.vol->addIofxActor(*iofxActor))
					{
						d.mActors[ j ] = iofxActor;

						mManager.initIofxActor(iofxActor, j, d.vol);

						// lock this renderable because the APEX scene will unlock it after this method is called
						iofxActor->renderDataLock();
					}
					else
					{
						iofxActor->release();
					}
				}

				IofxActorImpl* iofxActor = d.mActors[ j ];
				if (iofxActor && iofxActor != DEFERRED_IOFX_ACTOR)
				{
					iofxActor->mResultBounds.setEmpty();
					if (rangeCount > 0)
					{
						iofxActor->mResultBounds.minimum = mCuMinBounds[ aid ].getXYZ();
						iofxActor->mResultBounds.maximum = mCuMaxBounds[ aid ].getXYZ();
					}
					PX_ASSERT(iofxActor->mRenderBounds.isFinite());
					iofxActor->mResultRange.startIndex = rangeStart;
					iofxActor->mResultRange.objectCount = rangeCount;
					iofxActor->mResultVisibleCount = visibleCount;
				}

				aid++;
			}
		}
	}

}


/**
 * Called from render thread context, just before renderer calls update/dispatch on any IOFX
 * actors.  Map/Unmap render resources as required.  "Mapped" means the graphics buffer has been
 * mapped into our CUDA context where our kernels can write directly into it.
 */
void IofxManager::fillMapUnmapArraysForInterop(nvidia::Array<CUgraphicsResource> &toMapArray, nvidia::Array<CUgraphicsResource> &toUnmapArray)
{
	if (mInteropFlags == RenderInteropFlags::CUDA_INTEROP)
	{
		mResultIosData->renderData->fillMapUnmapArraysForInterop(toMapArray, toUnmapArray);
		mStagingIosData->renderData->fillMapUnmapArraysForInterop(toMapArray, toUnmapArray);
	}
}


void IofxManager::mapBufferResultsForInterop(bool mapSuccess, bool unmapSuccess)
{
	if (mInteropFlags == RenderInteropFlags::CUDA_INTEROP)
	{
		mResultIosData->renderData->mapBufferResultsForInterop(mapSuccess, unmapSuccess);
		mStagingIosData->renderData->mapBufferResultsForInterop(mapSuccess, unmapSuccess);
	}
}

}
} // namespace nvidia

#endif
