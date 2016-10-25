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
#include "PsArray.h"
#include "SceneIntl.h"
#include "ModuleIofxImpl.h"
#include "IofxManager.h"
#include "IofxSceneCPU.h"
#include "IofxAssetImpl.h"
#include "IosObjectData.h"
#include "IofxRenderData.h"

#include "IofxActorCPU.h"

#if APEX_CUDA_SUPPORT
#include "ApexCuda.h" // APEX_CUDA_MEM_ALIGN_UP_32BIT
#include "ApexMirroredArray.h"
#if ENABLE_TEST
#include "IofxTestManagerGPU.h"
#define IOFX_MANAGER_GPU IofxTestManagerGPU
#else
#include "IofxManagerGPU.h"
#define IOFX_MANAGER_GPU IofxManagerGPU
#endif
#endif

#define BASE_SPRITE_SEMANTICS (1<<IofxRenderSemantic::POSITION) | \
	(1<<IofxRenderSemantic::VELOCITY) | \
	(1<<IofxRenderSemantic::LIFE_REMAIN)

#define BASE_MESH_SEMANTICS   (1<<IofxRenderSemantic::POSITION) | \
	(1<<IofxRenderSemantic::SCALE) | \
	(1<<IofxRenderSemantic::ROTATION) | \
	(1<<IofxRenderSemantic::VELOCITY) | \
	(1<<IofxRenderSemantic::LIFE_REMAIN)

namespace nvidia
{
namespace iofx
{

#pragma warning(disable: 4355) // 'this' : used in base member initializer list


IofxManager::IofxManager(IofxScene& scene, const IofxManagerDescIntl& desc, bool isMesh)
	: mPostUpdateTaskID(0)
	, mIofxScene(&scene)
	, mIosAssetName(desc.iosAssetName)
	, mWorkingIosData(NULL)
	, mResultIosData(NULL)
	, mStagingIosData(NULL)
	, mTargetSemantics(0)
	, mRenderState(RENDER_WAIT_FOR_DATA_ALLOC)
	, mInteropFlags(RenderInteropFlags::NO_INTEROP)
	, positionMass(*scene.mApexScene, PX_ALLOC_INFO("positionMass", PARTICLES))
	, velocityLife(*scene.mApexScene, PX_ALLOC_INFO("velocityLife", PARTICLES))
	, collisionNormalFlags(*scene.mApexScene, PX_ALLOC_INFO("collisionNormalFlags", PARTICLES))
	, density(*scene.mApexScene, PX_ALLOC_INFO("density", PARTICLES))
	, actorIdentifiers(*scene.mApexScene, PX_ALLOC_INFO("actorIdentifiers", PARTICLES))
	, inStateToInput(*scene.mApexScene, PX_ALLOC_INFO("inStateToInput", PARTICLES))
	, outStateToInput(*scene.mApexScene, PX_ALLOC_INFO("outStateToInput", PARTICLES))
	, userData(*scene.mApexScene, PX_ALLOC_INFO("userData", PARTICLES))
	, pubStateSize(0)
	, privStateSize(0)
	, mStateSwap(false)
	, mTotalElapsedTime(0)
	, mIsMesh(isMesh)
	, mDistanceSortingEnabled(false)
	, mCudaIos(desc.iosOutputsOnDevice)
	, mCudaModifiers(false)
	, mCudaPipeline(NULL)
	, mSimulateTask(*this)
#if APEX_CUDA_SUPPORT
	, mCopyQueue(*scene.mApexScene->getTaskManager()->getGpuDispatcher())
#endif
	, mLastNumObjects(0)
	, mLastMaxInputID(0)
	, mOnStartCallback(NULL)
	, mOnFinishCallback(NULL)
{
	scene.mActorManagers.add(*this);

	mBounds.setEmpty();

	mInStateOffset = 0;
	mOutStateOffset = desc.maxObjectCount;

	// The decision whether to use GPU IOFX Modifiers is separate from whether the IOS
	// outputs will come from the GPU or not
#if APEX_CUDA_SUPPORT
	PxGpuDispatcher* gd = scene.mApexScene->getTaskManager()->getGpuDispatcher();
	if (gd && gd->getCudaContextManager()->contextIsValid() && !scene.mModule->mCudaDisabled)
	{
		mCudaModifiers = true;
		// detect interop
		if (gd->getCudaContextManager()->getInteropMode() != PxCudaInteropMode::NO_INTEROP && !scene.mModule->mInteropDisabled)
		{
			mInteropFlags = RenderInteropFlags::CUDA_INTEROP;
		}
		const uint32_t dataCount = 3u;
		for (uint32_t i = 0 ; i < dataCount ; i++)
		{
			IosObjectGpuData* gpuIosData = PX_NEW(IosObjectGpuData)(i);
			mObjData.pushBack(gpuIosData);
		}

		mOutStateOffset = APEX_CUDA_MEM_ALIGN_UP_32BIT(desc.maxObjectCount);
		mCudaPipeline = PX_NEW(IOFX_MANAGER_GPU)(*mIofxScene->mApexScene, desc, *this);
	}
	else
#endif
	{
		const uint32_t dataCount = 3u;
		for (uint32_t i = 0 ; i < dataCount ; i++)
		{
			IosObjectCpuData* cpuIosData = PX_NEW(IosObjectCpuData)(i);
			mObjData.pushBack(cpuIosData);
		}
	}

	mWorkingIosData = mObjData[0];
	mResultIosData = mObjData[1];
	mStagingIosData = mObjData[2];

	// Create & Assign Shared Render Data
	for (uint32_t i = 0 ; i < mObjData.size() ; i++)
	{
		if (mIsMesh)
		{
			mObjData[i]->renderData = PX_NEW(IofxSharedRenderDataMeshImpl)(i, mInteropFlags);
		}
		else
		{
			mObjData[i]->renderData = PX_NEW(IofxSharedRenderDataSpriteImpl)(i, mInteropFlags);
		}
	}

	ApexMirroredPlace::Enum place = ApexMirroredPlace::CPU;
#if APEX_CUDA_SUPPORT
	if (mCudaIos || mCudaModifiers)
	{
		place =  ApexMirroredPlace::CPU_GPU;
	}
#endif
	{
		positionMass.setSize(desc.maxInputCount, place);
		velocityLife.setSize(desc.maxInputCount, place);
		if (desc.iosSupportsCollision)
		{
			collisionNormalFlags.setSize(desc.maxInputCount, place);
		}
		if (desc.iosSupportsDensity)
		{
			density.setSize(desc.maxInputCount, place);
		}
		actorIdentifiers.setSize(desc.maxInputCount, place);
		inStateToInput.setSize(desc.maxInStateCount, place);
		outStateToInput.setSize(desc.maxObjectCount, place);

		if (desc.iosSupportsUserData)
		{
			userData.setSize(desc.maxInputCount, place);
		}

		mSimBuffers.pmaPositionMass = &positionMass;
		mSimBuffers.pmaVelocityLife = &velocityLife;
		mSimBuffers.pmaCollisionNormalFlags = desc.iosSupportsCollision ? &collisionNormalFlags : NULL;
		mSimBuffers.pmaDensity = desc.iosSupportsDensity ? &density : NULL;
		mSimBuffers.pmaActorIdentifiers = &actorIdentifiers;
		mSimBuffers.pmaInStateToInput = &inStateToInput;
		mSimBuffers.pmaOutStateToInput = &outStateToInput;
		mSimBuffers.pmaUserData = desc.iosSupportsUserData ? &userData : NULL;
	}

	if (!mCudaModifiers)
	{
		mOutputToState.resize(desc.maxObjectCount);
	}

	/* Initialize IOS object data structures */
	for (uint32_t i = 0 ; i < mObjData.size() ; i++)
	{
		mObjData[i]->pmaPositionMass = mSimBuffers.pmaPositionMass;
		mObjData[i]->pmaVelocityLife = mSimBuffers.pmaVelocityLife;
		mObjData[i]->pmaCollisionNormalFlags = mSimBuffers.pmaCollisionNormalFlags;
		mObjData[i]->pmaDensity = mSimBuffers.pmaDensity;
		mObjData[i]->pmaActorIdentifiers = mSimBuffers.pmaActorIdentifiers;
		mObjData[i]->pmaInStateToInput = mSimBuffers.pmaInStateToInput;
		mObjData[i]->pmaOutStateToInput = mSimBuffers.pmaOutStateToInput;
		mObjData[i]->pmaUserData = mSimBuffers.pmaUserData;

		mObjData[i]->iosAssetName = desc.iosAssetName;
		mObjData[i]->iosOutputsOnDevice = desc.iosOutputsOnDevice;
		mObjData[i]->iosSupportsDensity = desc.iosSupportsDensity;
		mObjData[i]->iosSupportsCollision = desc.iosSupportsCollision;
		mObjData[i]->iosSupportsUserData = desc.iosSupportsUserData;
		mObjData[i]->maxObjectCount = desc.maxObjectCount;
		mObjData[i]->maxInputCount = desc.maxInputCount;
		mObjData[i]->maxInStateCount = desc.maxInStateCount;
	}
}

IofxManager::~IofxManager()
{
	for (uint32_t i = 0; i < pubState.slices.size(); ++i)
	{
		delete pubState.slices[i];
	}

	for (uint32_t i = 0; i < privState.slices.size(); ++i)
	{
		delete privState.slices[i];
	}
}

void IofxManager::destroy()
{
#if APEX_CUDA_SUPPORT
	if (mCudaPipeline)
	{
		mCudaPipeline->release();
	}
#endif
	for (uint32_t i = 0 ; i < mObjData.size() ; i++)
	{
		PX_DELETE(mObjData[i]);
	}

	delete this;
}


void IofxManager::release()
{
	mIofxScene->releaseIofxManager(this);
}

#if !APEX_CUDA_SUPPORT
/* Stubs for console builds */
void IofxManager::fillMapUnmapArraysForInterop(nvidia::Array<CUgraphicsResource> &, nvidia::Array<CUgraphicsResource> &) {}
void IofxManager::mapBufferResultsForInterop(bool, bool) {}
#endif

void IofxManager::prepareRenderables()
{
	const uint32_t targetSemantics = mTargetSemantics;

	switch (mRenderState)
	{
	case RENDER_READY:
		//PX_ASSERT(mResultIosData->renderData->getBufferIsMapped() == true);
		mResultIosData->renderData->unmap();

		//PX_ASSERT(mStagingIosData->renderData->getBufferIsMapped() == false);
		mStagingIosData->updateSemantics(*mIofxScene, targetSemantics);
		mStagingIosData->renderData->map();

		nvidia::swap(mStagingIosData, mResultIosData);

		mRenderState = RENDER_WAIT_FOR_NEW;
		break;
	case RENDER_WAIT_FOR_DATA_ALLOC:
		if (targetSemantics != 0)
		{
			mResultIosData->updateSemantics(*mIofxScene, targetSemantics);
			mResultIosData->renderData->map();
			mStagingIosData->updateSemantics(*mIofxScene, targetSemantics);
			mStagingIosData->renderData->map();

			mRenderState = RENDER_WAIT_FOR_FETCH_RESULT;
		}
		//go to default case here!
	default:
		return;
	};

	// mLiveRenderVolumesLock is allready locked in IofxScene::prepareRenderables
	for (uint32_t i = 0 ; i < mIofxScene->mLiveRenderVolumes.size() ; i++)
	{
		RenderVolumeImpl* vol = mIofxScene->mLiveRenderVolumes[i];
		// all render volumes are allready locked in IofxScene::prepareRenderables

		uint32_t iofxActorCount;
		IofxActor* const* iofxActorList = vol->lockIofxActorList(iofxActorCount);
		for (uint32_t iofxActorIndex = 0; iofxActorIndex < iofxActorCount; ++iofxActorIndex)
		{
			IofxActorImpl* iofxActor = DYNAMIC_CAST(IofxActorImpl*)( iofxActorList[iofxActorIndex] );
			if (&iofxActor->mMgr == this)
			{
				iofxActor->prepareRenderables(mStagingIosData);
			}
		}
		vol->unlockIofxActorList();
	}
}

float IofxManager::getObjectRadius() const
{
	return mObjData[0] ? mObjData[0]->radius : 0.0f;
}

void IofxManager::setSimulationParameters(float radius, const PxVec3& up, float gravity, float restDensity)
{
	/* Initialize IOS object data structures */
	for (uint32_t i = 0 ; i < mObjData.size() ; i++)
	{
		mObjData[i]->radius = radius;
		mObjData[i]->upVector = up;
		mObjData[i]->gravity = gravity;
		mObjData[i]->restDensity = restDensity;
	}
}

void IofxManager::createSimulationBuffers(IosBufferDescIntl& outDesc)
{
	outDesc = mSimBuffers;
}

/* Called by owning IOS actor during simulation startup, only if
 * the IOS is going to simulate this frame, so it is safe to submit
 * tasks from here. postUpdateTaskID is the ID for an IOS task
 * that should run after IOFX modifiers.  If the IOFX Manager adds
 * no dependencies, postUpdateTaskID task will run right after
 * updateEffectsData() returns.  If updateEffectsData() will be completely
 * synchronous, it is safe to return (PxTaskID)0xFFFFFFFF here.
 */
PxTaskID IofxManager::getUpdateEffectsTaskID(PxTaskID postUpdateTaskID)
{
	PxTaskManager* tm = mIofxScene->mApexScene->getTaskManager();
	mPostUpdateTaskID = postUpdateTaskID;
	if (mCudaModifiers)
	{
		return mCudaPipeline->launchGpuTasks();
	}
	else
	{
		tm->submitUnnamedTask(mSimulateTask);
		mSimulateTask.finishBefore(tm->getNamedTask(AST_PHYSX_FETCH_RESULTS));
		return mSimulateTask.getTaskID();
	}
}


void TaskUpdateEffects::run()
{
	setProfileStat((uint16_t) mOwner.mWorkingIosData->numParticles);
	mOwner.cpuModifiers();
}

/// \brief Called by IOS actor before TaskUpdateEffects is scheduled to run
void IofxManager::updateEffectsData(float deltaTime, uint32_t numObjects, uint32_t maxInputID, uint32_t maxStateID, void* extraData)
{
	PX_ASSERT(maxStateID >= maxInputID && maxInputID >= numObjects);

	mLastNumObjects = numObjects;
	mLastMaxInputID = maxInputID;

	if (mCudaIos && !mCudaModifiers)
	{
#if APEX_CUDA_SUPPORT
		/* Presumably, updateEffectsData() is being called from a DtoH GPU task */
		mCopyQueue.reset(CUstream(extraData), 8);
		positionMass.copyDeviceToHostQ(mCopyQueue, maxInputID);
		velocityLife.copyDeviceToHostQ(mCopyQueue, maxInputID);
		if (mWorkingIosData->iosSupportsCollision)
		{
			collisionNormalFlags.copyDeviceToHostQ(mCopyQueue, maxInputID);
		}
		if (mWorkingIosData->iosSupportsDensity)
		{
			density.copyDeviceToHostQ(mCopyQueue, maxInputID);
		}
		if (mWorkingIosData->iosSupportsUserData)
		{
			userData.copyDeviceToHostQ(mCopyQueue, maxInputID);
		}
		actorIdentifiers.copyDeviceToHostQ(mCopyQueue, maxInputID);
		inStateToInput.copyDeviceToHostQ(mCopyQueue, maxStateID);
		mCopyQueue.flushEnqueued();

		mIofxScene->mApexScene->getTaskManager()->getGpuDispatcher()->addCompletionPrereq(mSimulateTask);
#else
		PX_UNUSED(extraData);
		PX_ALWAYS_ASSERT();
#endif
	}

	/* Data from the IOS */
	mWorkingIosData->maxInputID = maxInputID;
	mWorkingIosData->maxStateID = maxStateID;
	mWorkingIosData->numParticles = numObjects;

	/* Data from the scene */
	mWorkingIosData->eyePosition = mIofxScene->mApexScene->getEyePosition();
	mWorkingIosData->eyeDirection = mIofxScene->mApexScene->getEyeDirection();

	PxMat44 viewMtx = mIofxScene->mApexScene->getViewMatrix();
	PxMat44 projMtx = mIofxScene->mApexScene->getProjMatrix();
	mWorkingIosData->eyeAxisX = PxVec3(viewMtx.column0.x, viewMtx.column1.x, viewMtx.column2.x);
	mWorkingIosData->eyeAxisY = PxVec3(viewMtx.column0.y, viewMtx.column1.y, viewMtx.column2.y);
	mWorkingIosData->zNear = PxAbs(projMtx.column3.z / projMtx.column2.z);

	mWorkingIosData->deltaTime = deltaTime;
	// TODO: Convert into uint32_t elapsed milliseconds
	mTotalElapsedTime = numObjects ? mTotalElapsedTime + mWorkingIosData->deltaTime : 0;
	mWorkingIosData->elapsedTime = mTotalElapsedTime;

	/* IOFX data */
	if (mCudaModifiers)
	{
		mCudaPipeline->launchPrep(); // calls allocOutputs
	}
}

void IofxManager::cpuModifiers()
{
	if (!mCudaIos && mOnStartCallback)
	{
		(*mOnStartCallback)(NULL);
	}
	uint32_t maxInputID, maxStateID, numObjects;

	maxInputID = mWorkingIosData->maxInputID;
	maxStateID = mWorkingIosData->maxStateID;
	numObjects = mWorkingIosData->numParticles;

	PX_UNUSED(numObjects);

	/* Swap state buffer pointers */

	IosObjectCpuData* md = DYNAMIC_CAST(IosObjectCpuData*)(mWorkingIosData);

	md->inPubState = mStateSwap ? &pubState.a[0] : &pubState.b[0];
	md->outPubState = mStateSwap ? &pubState.b[0] : &pubState.a[0];

	md->inPrivState = mStateSwap ? &privState.a[0] : &privState.b[0];
	md->outPrivState = mStateSwap ? &privState.b[0] : &privState.a[0];

	swapStates();

	/* Sort sprites */

	if (!mIsMesh)
	{
		DYNAMIC_CAST(IosObjectCpuData*)(mWorkingIosData)->sortingKeys =
			mDistanceSortingEnabled ? &mSortingKeys.front() : NULL;
	}

	/* Volume Migration (1 pass) */

	mCountPerActor.clear();
	mCountPerActor.resize(mActorTable.size() * mVolumeTable.size(), 0);
	for (uint32_t input = 0 ; input < maxInputID ; input++)
	{
		IofxActorIDIntl& id = mWorkingIosData->pmaActorIdentifiers->get(input);
		if (id.getActorClassID() == IofxActorIDIntl::IPX_ACTOR || id.getActorClassID() >= mActorClassTable.size())
		{
			id.set(IofxActorIDIntl::NO_VOLUME, IofxActorIDIntl::IPX_ACTOR);
		}
		else
		{
			const PxVec3& pos = mWorkingIosData->pmaPositionMass->get(input).getXYZ();
			uint32_t curPri = 0;
			uint16_t curVID = IofxActorIDIntl::NO_VOLUME;

			for (uint16_t i = 0 ; i < mVolumeTable.size() ; i++)
			{
				VolumeData& vd = mVolumeTable[ i ];
				if (vd.vol == NULL)
				{
					continue;
				}

				const uint32_t bit = mActorClassTable.size() * i + id.getActorClassID();

				// This volume owns this particle if:
				//  1. The volume bounds contain the particle
				//  2. The volume affects the particle's IOFX Asset
				//  3. This volume has the highest priority or was the previous owner
				if (vd.mBounds.contains(pos) &&
				    (mVolumeActorClassBitmap[ bit >> 5 ] & (1u << (bit & 31))) &&
				    (curVID == IofxActorIDIntl::NO_VOLUME || vd.mPri > curPri || (vd.mPri == curPri && id.getVolumeID() == i)))
				{
					curVID = i;
					curPri = vd.mPri;
				}
			}

			id.setVolumeID(curVID);
		}

		// Count particles in each actor
		if (id.getVolumeID() != IofxActorIDIntl::NO_VOLUME)
		{
			const uint32_t actorID = mActorClassTable[ id.getActorClassID() ].actorID;
			PX_ASSERT(actorID < mActorTable.size());
			const uint32_t aid = id.getVolumeID() * mActorTable.size() + actorID;
			++mCountPerActor[aid];
		}
	}

	/* Prefix sum */
	mStartPerActor.clear();
	mStartPerActor.resize(mCountPerActor.size(), 0);
	uint32_t sum = 0;
	for (uint32_t i = 0 ; i < mStartPerActor.size() ; i++)
	{
		mStartPerActor[ i ] = sum;
		sum += mCountPerActor[ i ];
	}

	IosObjectCpuData* objData = DYNAMIC_CAST(IosObjectCpuData*)(mWorkingIosData);
	objData->outputToState = &mOutputToState.front();

	/* Generate outputToState (1 pass) */
	mBuildPerActor.clear();
	mBuildPerActor.resize(mStartPerActor.size(), 0);
	uint32_t homeless = 0;
	for (uint32_t state = 0 ; state < maxStateID ; state++)
	{
		uint32_t input = objData->pmaInStateToInput->get(state);
		if (input == IosBufferDescIntl::NOT_A_PARTICLE)
		{
			continue;
		}

		input &= ~IosBufferDescIntl::NEW_PARTICLE_FLAG;

		const IofxActorIDIntl id = objData->pmaActorIdentifiers->get(input);
		if (id.getVolumeID() == IofxActorIDIntl::NO_VOLUME)
		{
			objData->pmaOutStateToInput->get(sum + homeless) = input;
			++homeless;
		}
		else
		{
			PX_ASSERT(id.getActorClassID() != IofxActorIDIntl::IPX_ACTOR && id.getActorClassID() < mActorClassTable.size());
			const uint32_t actorID = mActorClassTable[ id.getActorClassID() ].actorID;
			PX_ASSERT(actorID < mActorTable.size());
			const uint32_t aid = id.getVolumeID() * mActorTable.size() + actorID;
			objData->outputToState[ mStartPerActor[aid] + mBuildPerActor[ aid ]++ ] = state;
		}
	}

	/* Step IOFX Actors */
	uint32_t aid = 0;
	PxTaskManager* tm = mIofxScene->mApexScene->getTaskManager();
	for (uint32_t i = 0 ; i < mVolumeTable.size() ; i++)
	{
		VolumeData& d = mVolumeTable[ i ];
		if (d.vol == 0)
		{
			aid += mActorTable.size();
			continue;
		}

		for (uint32_t j = 0 ; j < mActorTable.size(); j++)
		{
			if (d.mActors[ j ] == DEFERRED_IOFX_ACTOR && mActorTable[ j ] != NULL &&
			        (mIofxScene->mModule->mDeferredDisabled || mCountPerActor[ aid ]))
			{
				IofxActorImpl* iofxActor = PX_NEW(IofxActorCPU)(mActorTable[j]->getRenderResID(), mIofxScene, *this);
				if (d.vol->addIofxActor(*iofxActor))
				{
					d.mActors[ j ] = iofxActor;

					initIofxActor(iofxActor, j, d.vol);
				}
				else
				{
					iofxActor->release();
				}
			}

			IofxActorCPU* cpuIofx = DYNAMIC_CAST(IofxActorCPU*)(d.mActors[ j ]);
			if (cpuIofx && cpuIofx != DEFERRED_IOFX_ACTOR)
			{
				if (mCountPerActor[ aid ])
				{
					ObjectRange range;
					range.objectCount = mCountPerActor[ aid ];
					range.startIndex = mStartPerActor[ aid ];
					PX_ASSERT(range.startIndex + range.objectCount <= numObjects);

					cpuIofx->mWorkingRange = range;

					cpuIofx->mModifierTask.setContinuation(*tm, tm->getTaskFromID(mPostUpdateTaskID));
					cpuIofx->mModifierTask.removeReference();
				}
				else
				{
					cpuIofx->mWorkingVisibleCount = 0;
					cpuIofx->mWorkingRange.objectCount = 0;
					cpuIofx->mWorkingRange.startIndex = 0;
					cpuIofx->mWorkingBounds.setEmpty();
				}
			}

			aid++;
		}
	}

	if (!mCudaIos && mOnFinishCallback)
	{
		(*mOnFinishCallback)(NULL);
	}
}

void IofxManager::outputHostToDevice(PxGpuCopyDescQueue& copyQueue)
{
	if (mCudaIos && !mCudaModifiers)
	{
#if APEX_CUDA_SUPPORT
		actorIdentifiers.copyHostToDeviceQ(copyQueue, mLastMaxInputID);
		outStateToInput.copyHostToDeviceQ(copyQueue, mLastNumObjects);
#else
		PX_UNUSED(copyQueue);
		PX_ALWAYS_ASSERT();
#endif
	}
}


void IofxManager::submitTasks()
{
	/* Discover new volumes, removed volumes */
	for (uint32_t i = 0 ; i < mVolumeTable.size() ; i++)
	{
		mVolumeTable[ i ].mFlags = 0;
	}

	for (uint32_t i = 0 ; i < mIofxScene->mLiveRenderVolumes.size() ; i++)
	{
		getVolumeID(mIofxScene->mLiveRenderVolumes[ i ]);
	}

	for (uint32_t i = 0 ; i < mVolumeTable.size() ; i++)
	{
		if (mVolumeTable[ i ].mFlags == 0)
		{
			mVolumeTable[ i ].vol = 0;
		}
	}

	/* Trim Volume, ActorID and ActorClassID tables */
	while (mVolumeTable.size() && mVolumeTable.back().vol == 0)
	{
		mVolumeTable.popBack();
	}

	if (!mActorTable.empty())
	{
		int32_t lastValidID = -1;
		for (int32_t cur = (int32_t)mActorTable.size() - 1; cur >= 0; --cur)
		{
			if (mActorTable[(uint32_t)cur] != NULL)
			{
				lastValidID = cur;
				break;
			}
		}
		if (lastValidID == -1)
		{
			mActorTable.clear();
		}
		else
		{
			mActorTable.resize((uint32_t)lastValidID + 1);
		}
	}

	if (!mActorClassTable.empty())
	{
		int32_t lastValidID = -1;
		for (uint32_t cur = 0; cur < mActorClassTable.size(); cur += mActorClassTable[ cur ].count)
		{
			if (mActorClassTable[ cur ].client != NULL)
			{
				lastValidID = (int32_t)cur;
			}
		}
		if (lastValidID == -1)
		{
			mActorClassTable.clear();
		}
		else
		{
			mActorClassTable.resize((uint32_t)(lastValidID + mActorClassTable[ (uint32_t)lastValidID ].count));
		}
	}

	const uint32_t volumeActorClassBitmapSize = (mVolumeTable.size() * mActorClassTable.size() + 31) >> 5;
	mVolumeActorClassBitmap.resize(volumeActorClassBitmapSize);
	for (uint32_t i = 0 ; i < volumeActorClassBitmapSize ; i++)
	{
		mVolumeActorClassBitmap[ i ] = 0;
	}

	/* Add new IofxActors as necessary */
	for (uint32_t i = 0 ; i < mVolumeTable.size() ; i++)
	{
		VolumeData& d = mVolumeTable[ i ];

		// First, ensure per-volume actor array can hold all ClassIDs
		d.mActors.resize(mActorTable.size(), 0);

		if (d.vol == NULL)
		{
			continue;
		}

		d.mBounds = d.vol->getOwnershipBounds();
		d.mPri = d.vol->getPriority();

		for (uint32_t cur = 0; cur < mActorTable.size(); ++cur)
		{
			if (mActorTable[ cur ] != NULL)
			{
				if (!d.mActors[ cur ])
				{
					d.mActors[ cur ] = DEFERRED_IOFX_ACTOR;
				}
			}
		}

		d.vol->lockReader(); // for safety during affectsIofxAsset() calls
		for (uint32_t cur = 0; cur < mActorClassTable.size(); ++cur)
		{
			const ActorClassData& acd = mActorClassTable[ cur ];
			if (acd.client != NULL && acd.actorID < mActorTable.size())
			{
				IofxAssetImpl* iofxAsset = acd.client->getAssetSceneInst()->getAsset();
				if (iofxAsset && d.vol->affectsIofxAsset(*iofxAsset))
				{
					const uint32_t bit = mActorClassTable.size() * i + cur;
					mVolumeActorClassBitmap[ bit >> 5 ] |= (1u << (bit & 31));
				}
			}
		}
		d.vol->unlockReader(); // for safety during affectsIofxAsset() calls
	}

	uint32_t targetSemantics = 0;
	mDistanceSortingEnabled = false;
	{
		for (AssetHashMap_t::Iterator it = mAssetHashMap.getIterator(); !it.done(); ++it)
		{
			IofxAssetImpl* iofxAsset = it->first;
			IofxAssetSceneInst* iofxAssetSceneInst = it->second;

			targetSemantics |= iofxAssetSceneInst->getSemantics();
			if (!mDistanceSortingEnabled && iofxAsset->isSortingEnabled())
			{
				mDistanceSortingEnabled = true;
				if (!mCudaModifiers)
				{
					mSortingKeys.resize(mOutputToState.size());
				}
			}
		}
	}
	mTargetSemantics = targetSemantics;

	if (mCudaModifiers)
	{
		mCudaPipeline->submitTasks();
	}

	if (!addedAssets.empty())
	{
		/* Calculate state sizes required by new assets */
		uint32_t newPubStateSize = 0, newPrivStateSize = 0;
		for (uint32_t i = 0; i < addedAssets.size(); ++i)
		{
			newPubStateSize = PxMax(newPubStateSize, addedAssets[i]->getPubStateSize());
			newPrivStateSize = PxMax(newPrivStateSize, addedAssets[i]->getPrivStateSize());
		}

		uint32_t maxObjectCount = outStateToInput.getSize(),
			totalCount = mOutStateOffset + maxObjectCount;

		// Allocate data for pubstates
		while (newPubStateSize > pubStateSize)
		{
			pubStateSize += sizeof(IofxSlice);

			SliceArray* slice = new SliceArray(*mIofxScene->mApexScene, PX_ALLOC_INFO("slice", PARTICLES));

#if APEX_CUDA_SUPPORT
			if (mCudaModifiers)
			{
				//slice->reserve(totalCount, ApexMirroredPlace::GPU); Recalculated on GPU
			}
			else
#endif
			{
				slice->reserve(totalCount, ApexMirroredPlace::CPU);
			}

			pubState.slices.pushBack(slice);

			IofxSlice* p;
#if APEX_CUDA_SUPPORT
			p = mCudaModifiers
				? pubState.slices.back()->getGpuPtr()
				: pubState.slices.back()->getPtr();
#else
			p = pubState.slices.back()->getPtr();
#endif
			pubState.a.pushBack(p + mInStateOffset);
			pubState.b.pushBack(p + mOutStateOffset);
		}

		// Allocate data for privstates
		while (newPrivStateSize > privStateSize)
		{
			privStateSize += sizeof(IofxSlice);

			SliceArray* slice = new SliceArray(*mIofxScene->mApexScene, PX_ALLOC_INFO("slice", PARTICLES));

#if APEX_CUDA_SUPPORT
			if (mCudaModifiers)
			{
				slice->reserve(totalCount, ApexMirroredPlace::GPU);
			}
			else
#endif
			{
				slice->reserve(totalCount, ApexMirroredPlace::CPU);
			}

			privState.slices.pushBack(slice);

			IofxSlice* p;
#if APEX_CUDA_SUPPORT
			p = mCudaModifiers 
				? privState.slices.back()->getGpuPtr()
				: privState.slices.back()->getPtr();
#else
			p = privState.slices.back()->getPtr();
#endif
			privState.a.pushBack(p + mInStateOffset);
			privState.b.pushBack(p + mOutStateOffset);
		}

		addedAssets.clear();
	}
}

void IofxManager::swapStates()
{
	mStateSwap = !mStateSwap;
	swap(mInStateOffset, mOutStateOffset);
}

void IofxManager::fetchResults()
{
	if (!mPostUpdateTaskID)
	{
		return;
	}
	mPostUpdateTaskID = 0;

	if (mCudaModifiers)
	{
		mCudaPipeline->fetchResults();
	}
	else
	{
		for (uint32_t i = 0 ; i < mVolumeTable.size() ; i++)
		{
			VolumeData& d = mVolumeTable[ i ];
			for (uint32_t j = 0 ; j < mActorTable.size() ; j++)
			{
				IofxActorCPU* cpuIofx = DYNAMIC_CAST(IofxActorCPU*)(d.mActors[ j ]);
				if (cpuIofx && cpuIofx != DEFERRED_IOFX_ACTOR)
				{
					cpuIofx->mResultBounds = cpuIofx->mWorkingBounds;
					cpuIofx->mResultRange = cpuIofx->mWorkingRange;
					cpuIofx->mResultVisibleCount = cpuIofx->mWorkingVisibleCount;
				}
			}
		}
	}

	//build bounds
	{
		mBounds.setEmpty();
		for (uint32_t i = 0 ; i < mVolumeTable.size() ; i++)
		{
			VolumeData& d = mVolumeTable[ i ];
			for (uint32_t j = 0 ; j < mActorTable.size() ; j++)
			{
				IofxActorImpl* iofx = d.mActors[ j ];
				if (iofx && iofx != DEFERRED_IOFX_ACTOR)
				{
					mBounds.include(iofx->mResultBounds);
				}
			}
		}
	}

	{
		switch (mRenderState)
		{
		case RENDER_WAIT_FOR_DATA_ALLOC:
			//just do nothing
			break;
		case RENDER_WAIT_FOR_FETCH_RESULT:
			nvidia::swap(mStagingIosData, mWorkingIosData);
			mRenderState = RENDER_WAIT_FOR_NEW;
			break;
		case RENDER_WAIT_FOR_NEW:
		case RENDER_READY:
			nvidia::swap(mResultIosData, mWorkingIosData);
			mRenderState = RENDER_READY;
			break;
		default:
			PX_ALWAYS_ASSERT();
		};
	}
}

PxBounds3 IofxManager::getBounds() const
{
	return mBounds;
}

uint32_t IofxManager::getActorID(IofxAssetSceneInst* assetSceneInst, uint16_t meshID)
{
	IofxAssetImpl* iofxAsset = assetSceneInst->getAsset();

	ResID renderResID = INVALID_RESOURCE_ID;
	if (mIsMesh)
	{
		const char* rmName = iofxAsset->getMeshAssetName(meshID);
		bool isOpaqueMesh = iofxAsset->isOpaqueMesh(meshID);
		renderResID = iofxAsset->mRenderMeshAssetTracker.getResourceIdFromName(rmName, isOpaqueMesh);
		if (renderResID == INVALID_RESOURCE_ID)
		{
			APEX_INVALID_PARAMETER("IofxManager: Mesh Asset with name \"%s\" not found.", rmName);
		}
	}
	else
	{
		const char* mtlName = iofxAsset->getSpriteMaterialName();
		renderResID = iofxAsset->mSpriteMaterialAssetTracker.getResourceIdFromName(mtlName, false);
		if (renderResID == INVALID_RESOURCE_ID)
		{
			APEX_INVALID_PARAMETER("IofxManager: Sprite Material with name \"%s\" not found.", mtlName);
		}
	}
	uint32_t actorID = uint32_t(-1);
	if (renderResID != INVALID_RESOURCE_ID)
	{
		for (uint32_t id = 0 ; id < mActorTable.size() ; id++)
		{
			if (mActorTable[id] != NULL)
			{
				if (mActorTable[id]->getRenderResID() == renderResID)
				{
					actorID = id;
					break;
				}
			}
			else if (actorID == uint32_t(-1))
			{
				actorID = id;
			}
		}
		if (actorID == uint32_t(-1))
		{
			actorID = mActorTable.size();
			mActorTable.resize(actorID + 1, NULL);
		}

		IofxActorSceneInst* &actorSceneInst = mActorTable[actorID];
		if (actorSceneInst == NULL)
		{
			actorSceneInst = PX_NEW(IofxActorSceneInst)(renderResID);
		}
		actorSceneInst->addRef();

		// only add iofxAsset one time, check refCount
		if (assetSceneInst->getRefCount() == 1)
		{
			actorSceneInst->addAssetSceneInst(assetSceneInst);
		}
	}
	return actorID;
}
void IofxManager::releaseActorID(IofxAssetSceneInst* assetSceneInst, uint32_t actorID)
{
	PX_ASSERT(actorID < mActorTable.size());
	IofxActorSceneInst* &actorSceneInst = mActorTable[actorID];
	if (actorSceneInst != NULL)
	{
		PX_ASSERT(actorSceneInst->getRefCount() > 0);
		if (actorSceneInst->removeRef())
		{
			for (uint16_t j = 0 ; j < mVolumeTable.size() ; j++)
			{
				if (mVolumeTable[ j ].vol == NULL)
				{
					continue;
				}

				if (actorID < mVolumeTable[ j ].mActors.size())
				{
					IofxActorImpl* iofxActor = mVolumeTable[ j ].mActors[ actorID ];
					if (iofxActor && iofxActor != DEFERRED_IOFX_ACTOR)
					{
						iofxActor->release();
						//IofxManager::removeActorAtIndex should zero the actor in mActors
						PX_ASSERT(mVolumeTable[ j ].mActors[ actorID ] == 0);
					}
					mVolumeTable[ j ].mActors[ actorID ] = 0;
				}
			}

			PX_DELETE(actorSceneInst);
			actorSceneInst = NULL;
		}
		else
		{
			// only remove iofxAsset one time, check refCount
			if (assetSceneInst->getRefCount() == 1)
			{
				actorSceneInst->removeAssetSceneInst(assetSceneInst);
			}
		}
	}
}

uint16_t IofxManager::getActorClassID(IofxManagerClientIntl* iofxClient, uint16_t meshID)
{
	IofxManagerClient* client = static_cast<IofxManagerClient*>(iofxClient);
	if (client != 0)
	{
		const uint16_t actorClassID = uint16_t(client->getActorClassID() + meshID);
		PX_ASSERT(actorClassID < mActorClassTable.size());
		PX_ASSERT(mActorClassTable[actorClassID].client == client);
		PX_ASSERT(meshID < mActorClassTable[actorClassID].count);

		const uint32_t actorID = mActorClassTable[actorClassID].actorID;
		if (actorID != uint32_t(-1))
		{
			return actorClassID;
		}
		else
		{
			APEX_DEBUG_WARNING("IofxManager: getActorClassID returned invalid actor.");
			return IofxActorIDIntl::IPX_ACTOR;
		}
	}
	else
	{
		APEX_INVALID_PARAMETER("IofxManager: getActorClassID was called with invalid client.");
		return IofxActorIDIntl::IPX_ACTOR;
	}
}

IofxManagerClientIntl* IofxManager::createClient(nvidia::apex::IofxAsset* asset, const IofxManagerClientIntl::Params& params)
{
	IofxAssetImpl* iofxAsset = static_cast<IofxAssetImpl*>(asset);

	IofxAssetSceneInst* &assetSceneInst = mAssetHashMap[iofxAsset];
	if (assetSceneInst == NULL)
	{
		assetSceneInst = createAssetSceneInst(iofxAsset);
		// Update state sizes later in submitTasks
		addedAssets.pushBack(iofxAsset);
		// increase asset refCount
		//ResourceProvider* nrp = GetInternalApexSDK()->getNamedResourceProvider();
		//nrp->setResource(IOFX_AUTHORING_TYPE_NAME, asset->getName(), asset, true);
	}
	assetSceneInst->addRef();

	//allocate actorClasses
	uint16_t actorClassCount = uint16_t( PxMax(1u, iofxAsset->getMeshAssetCount()) );
	uint32_t actorClassID = 0;
	while (actorClassID < mActorClassTable.size())
	{
		ActorClassData& acd = mActorClassTable[ actorClassID ];

		if (acd.client == NULL && actorClassCount <= acd.count)
		{
			/* Make a shim to conver remaining hole */
			uint16_t remains = uint16_t(acd.count - actorClassCount);
			for (uint16_t i = 0 ; i < remains ; i++)
			{
				ActorClassData& acd1 = mActorClassTable[ actorClassID + actorClassCount + i ];
				acd1.client = 0;
				acd1.meshid = i;
				acd1.count = remains;
				acd1.actorID = uint32_t(-1);
			}
			break;
		}
		actorClassID = actorClassID + acd.count;
	}
	if (actorClassID >= mActorClassTable.size())
	{
		/* Asset is not in table, append it */
		actorClassID = mActorClassTable.size();
		mActorClassTable.resize(actorClassID + actorClassCount);
	}

	IofxManagerClient* client = NULL;
#if APEX_CUDA_SUPPORT
	if (mCudaModifiers)
	{
		client = mCudaPipeline->createClient(assetSceneInst, actorClassID, params);
	}
	else
#endif
	{
		client = PX_NEW(IofxManagerClient)(assetSceneInst, actorClassID, params);
	}
	PX_ASSERT(client != NULL);

	for (uint16_t i = 0 ; i < actorClassCount ; i++)
	{
		ActorClassData& acd = mActorClassTable[ actorClassID + i ];
		acd.client = client;
		acd.meshid = i;
		acd.count = actorClassCount;
		acd.actorID = getActorID(assetSceneInst, i);
	}

	return static_cast<IofxManagerClientIntl*>(client);
}

void IofxManager::releaseClient(IofxManagerClientIntl* iofxClient)
{
	// TODO: free unused memory in states

	IofxManagerClient* client = static_cast<IofxManagerClient*>(iofxClient);
	if (client != 0)
	{
		IofxAssetSceneInst* assetSceneInst = client->getAssetSceneInst();
		uint32_t actorClassID = uint16_t(client->getActorClassID());

		if (actorClassID < mActorClassTable.size())
		{
			for (uint16_t i = 0 ; i < mActorClassTable[ actorClassID ].count ; i++)
			{
				uint32_t actorID = mActorClassTable[ actorClassID + i ].actorID;
				if (actorID != uint32_t(-1))
				{
					releaseActorID(assetSceneInst, actorID);
				}
			}

			// TODO: merge backward hole also
			/* merge consecutive holes */
			uint32_t next = actorClassID + mActorClassTable[ actorClassID ].count;
			while (next < mActorClassTable.size() && mActorClassTable[ next ].client == NULL)
			{
				next = next + mActorClassTable[ next ].count;
			}

			uint16_t count = uint16_t(next - actorClassID);
			for (uint16_t i = 0 ; i < count ; i++)
			{
				ActorClassData& acd = mActorClassTable[ actorClassID + i ];
				acd.client = 0;
				acd.meshid = i;
				acd.count = count;
				acd.actorID = uint32_t(-1);
			}
		}

		PX_DELETE(client);

		if (assetSceneInst->removeRef())
		{
			// decrease asset refCount
			//ResourceProvider* nrp = GetInternalApexSDK()->getNamedResourceProvider();
			//nrp->releaseResource(IOFX_AUTHORING_TYPE_NAME, ad.asset->getName());

			IofxAssetImpl* iofxAsset = assetSceneInst->getAsset();
			PX_DELETE(assetSceneInst);
			assetSceneInst = NULL;

			mAssetHashMap.erase(iofxAsset);
		}
	}
}

uint16_t IofxManager::getVolumeID(RenderVolume* vol)
{
	int32_t hole = -1;
	for (uint16_t i = 0 ; i < mVolumeTable.size() ; i++)
	{
		if (vol == mVolumeTable[ i ].vol)
		{
			mVolumeTable[ i ].mFlags = 1;
			return i;
		}
		else if (hole == -1 && !mVolumeTable[ i ].vol)
		{
			hole = (int32_t) i;
		}
	}
	if (hole == -1)
	{
		mVolumeTable.insert();
		hole = (int32_t)mVolumeTable.size() - 1;
	}
	VolumeData& d = mVolumeTable[ (uint32_t)hole ];
	d.vol = DYNAMIC_CAST(RenderVolumeImpl*)(vol);
	d.mFlags = 1;
	d.mActors.clear(); //Iofx Actors are released in RenderVolumeImpl destructor!
	return (uint16_t) hole;
}


void IofxManager::removeActorAtIndex(uint32_t index)
{
	IofxActorImpl* iofx = DYNAMIC_CAST(IofxActorImpl*)(mActorArray[ index ]);

	for (uint32_t i = 0 ; i < mVolumeTable.size() ; i++)
	{
		if (mVolumeTable[ i ].vol == iofx->mRenderVolume)
		{
			PX_ASSERT(iofx == mVolumeTable[ i ].mActors[ iofx->mActorID ]);
			mVolumeTable[ i ].mActors[ iofx->mActorID ] = 0;
		}
	}

	ApexContext::removeActorAtIndex(index);
}

IofxAssetSceneInst* IofxManager::createAssetSceneInst(IofxAsset* nxAsset)
{
	IofxAssetImpl* asset = DYNAMIC_CAST(IofxAssetImpl*)( nxAsset );

	uint32_t semantics = uint32_t(mIsMesh ? BASE_MESH_SEMANTICS : BASE_SPRITE_SEMANTICS);
	if( mObjData[0]->iosSupportsDensity ) 
	{
		semantics |= (uint32_t)IofxRenderSemantic::DENSITY;
	}
	semantics |= mIsMesh ? asset->getMeshSemanticsBitmap() : asset->getSpriteSemanticsBitmap();

	IofxAssetSceneInst* assetSceneInst = 0;
#if APEX_CUDA_SUPPORT
	if (mCudaModifiers)
	{
		assetSceneInst = mCudaPipeline->createAssetSceneInst(asset, semantics);
	}
	else
#endif
	{
		assetSceneInst = PX_NEW(IofxAssetSceneInstCPU)(asset, semantics, mIofxScene);
	}
	PX_ASSERT(assetSceneInst != 0);
	return assetSceneInst;
}

void IofxManager::initIofxActor(IofxActorImpl* iofxActor, uint32_t actorID, RenderVolumeImpl* renderVolume)
{
	iofxActor->addSelfToContext(*this);
	iofxActor->mActorID = actorID;
	iofxActor->mRenderVolume = renderVolume;
	iofxActor->mSemantics = 0;
	iofxActor->mDistanceSortingEnabled = false;

	PX_ASSERT(mActorTable[actorID] != NULL);
	const nvidia::Array<IofxAssetSceneInst*>& iofxAssets = mActorTable[actorID]->getAssetSceneInstArray();
	for (uint32_t k = 0; k < iofxAssets.size(); ++k)
	{
		IofxAssetSceneInst* assetSceneInst = iofxAssets[k];

		iofxActor->mSemantics |= assetSceneInst->getSemantics();
		iofxActor->mDistanceSortingEnabled |= assetSceneInst->getAsset()->isSortingEnabled();
	}

}

}
} // end namespace nvidia::apex
