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
#include "Apex.h"
#include "SceneIntl.h"
#include "ApexSDKIntl.h"

#include "ParticleIosActor.h"
#include "ParticleIosActorCPU.h"
#include "ParticleIosAssetImpl.h"
#include "IofxAsset.h"
#include "IofxActor.h"
#include "ModuleParticleIosImpl.h"
#include "ParticleIosScene.h"
#include "RenderDebugInterface.h"
#include "AuthorableObjectIntl.h"
#include "FieldSamplerQueryIntl.h"
#include "PxMath.h"
#include "ApexMirroredArray.h"

#include "PxParticleSystem.h"
#include "PxParticleCreationData.h"
#include "PxParticleReadData.h"
#include "PxParticleDeviceExclusive.h"

namespace nvidia
{
namespace pxparticleios
{
using namespace physx;

#pragma warning(disable: 4355) // 'this' : used in base member initializer list

ParticleIosActorCPU::ParticleIosActorCPU(
    ResourceList& list,
    ParticleIosAssetImpl& asset,
    ParticleIosScene& scene,
	IofxAsset& iofxAsset)
	: ParticleIosActorImpl(list, asset, scene, iofxAsset, false)
	, mSimulateTask(*this)
{
	initStorageGroups(mSimulationStorage);

	mField.reserve(mMaxParticleCount);
	mLifeTime.setSize(mMaxParticleCount);
	mLifeSpan.setSize(mMaxTotalParticleCount);
	mInjector.setSize(mMaxTotalParticleCount);
	mBenefit.setSize(mMaxTotalParticleCount);

	mNewIndices.resize(mMaxParticleCount);
	mAddedParticleList.reserve(mMaxParticleCount);
	mRemovedParticleList.reserve(mMaxParticleCount);
	mInputIdToParticleIndex.setSize(mMaxParticleCount, ApexMirroredPlace::CPU);

	mIndexPool = PxParticleExt::createIndexPool(mMaxParticleCount);

	mUpdateIndexBuffer.reserve(mMaxParticleCount);
	mUpdateVelocityBuffer.reserve(mMaxParticleCount);
}
ParticleIosActorCPU::~ParticleIosActorCPU()
{
	if (mIndexPool)
	{
		mIndexPool->release();
		mIndexPool = NULL;
	}	

}

PxTaskID ParticleIosActorCPU::submitTasks(PxTaskManager* tm)
{
	ParticleIosActorImpl::submitTasks(tm);
	mInjectorsCounters.setSize(mInjectorList.getSize(), ApexMirroredPlace::CPU); 

	if (mAsset->getParticleDesc()->Enable == false)
	{
		return mInjectTask.getTaskID();
	}

	const PxTaskID taskID = tm->submitUnnamedTask(mSimulateTask);
	return taskID;
}

void ParticleIosActorCPU::setTaskDependencies(PxTaskID taskStartAfterID, PxTaskID taskFinishBeforeID)
{
	PxTask* iosTask = NULL;
	if (mAsset->getParticleDesc()->Enable)
	{
		iosTask = &mSimulateTask;
	}
	ParticleIosActorImpl::setTaskDependencies(taskStartAfterID, taskFinishBeforeID, iosTask, false);
}

namespace
{
class FieldAccessor
{
	const PxVec4* mField;
public:
	explicit FieldAccessor(const PxVec4* field)
	{
		mField = field;
	}

	PX_INLINE void operator()(unsigned int srcIdx, PxVec3& velocityDelta)
	{
		if (mField != NULL)
		{
			velocityDelta += mField[srcIdx].getXYZ();
		}
	}
};
}

void ParticleIosActorCPU::simulateParticles()
{
	float deltaTime = mParticleIosScene->getApexScene().getPhysXSimulateTime();
	const PxVec3& eyePos = mParticleIosScene->getApexScene().getEyePosition();

	SCOPED_PHYSX_LOCK_WRITE(&mParticleIosScene->getApexScene());

	mTotalElapsedTime += deltaTime;

	uint32_t totalCount = mParticleCount + mInjectedCount;
	uint32_t activeCount = mLastActiveCount + mInjectedCount;

	mParticleBudget = mMaxParticleCount;
	if (mParticleBudget > activeCount)
	{
		mParticleBudget = activeCount;
	}
	uint32_t targetCount = mParticleBudget;

	uint32_t maxStateID = 0; //we could drop state in case targetCount = 0

	for(uint32_t i = 0; i < mInjectorList.getSize(); ++i)
	{
		mInjectorsCounters[i] = 0; 
	}

	if (targetCount > 0)
	{
		maxStateID = mParticleCount;
		for (uint32_t i = 0; i < maxStateID; ++i)
		{
			mNewIndices[i] = IosBufferDescIntl::NOT_A_PARTICLE;
		}

		uint32_t boundCount = 0;
		if (activeCount > targetCount)
		{
			boundCount = activeCount - targetCount;
		}

		float benefitMin = PxMin(mLastBenefitMin, mInjectedBenefitMin);
		float benefitMax = PxMax(mLastBenefitMax, mInjectedBenefitMax);
		PX_ASSERT(benefitMin <= benefitMax);
		benefitMax *= 1.00001f;

		/*
			boundBin - the highest benefit bin that should be culled
			boundCount - before computeHistogram it's the total culled particles.
					   - after computeHistogram it's the count of culled particles in boundBin
			boundIndex - count of culled particles in boundBin (0..boundCount-1)
		 */
		int32_t boundBin = (int32_t)computeHistogram(totalCount, benefitMin, benefitMax, boundCount);
		float	factor = HISTOGRAM_BIN_COUNT / (benefitMax - benefitMin);
		for (uint32_t i = 0, boundIndex = 0; i < totalCount; ++i)
		{
			float benefit = mBenefit[i];
			if (benefit > -FLT_MAX)
			{
				PX_ASSERT(benefit >= benefitMin && benefit < benefitMax);

				int32_t bin = int32_t((benefit - benefitMin) * factor);
				if (bin < boundBin)
				{
					mBenefit[i] = -FLT_MAX;
					continue;
				}
				if (bin == boundBin && boundIndex < boundCount)
				{
					mBenefit[i] = -FLT_MAX;
					++boundIndex;
				}
			}
		}
	}

	if (mParticleCount > 0)
	{
		mRemovedParticleList.clear();
		for (uint32_t i = 0 ; i < mParticleCount; ++i)
		{
			if (!(mBenefit[i] > -FLT_MAX))
			{
				mRemovedParticleList.pushBack(mInputIdToParticleIndex[i]);
				mInputIdToParticleIndex[i] = INVALID_PARTICLE_INDEX;
			}
		}
		if (mRemovedParticleList.size())
		{
			PxStrideIterator<const uint32_t> indexData( &mRemovedParticleList[0] );
			((PxParticleBase*)mParticleActor)->releaseParticles(mRemovedParticleList.size(), indexData);
			mIndexPool->freeIndices(mRemovedParticleList.size(), indexData);
			mRemovedParticleList.clear();
		}
	}

	mLastActiveCount = 0;
	mLastBenefitSum  = 0.0f;
	mLastBenefitMin  = +FLT_MAX;
	mLastBenefitMax  = -FLT_MAX;

	if (targetCount > 0)
	{
		const Px3InjectorParams* injectorParamsList = DYNAMIC_CAST(ParticleIosSceneCPU*)(mParticleIosScene)->mInjectorParamsArray.begin();

		FieldAccessor fieldAccessor(mFieldSamplerQuery ? mField.getPtr() : 0);

		mAddedParticleList.clear();
		mUpdateIndexBuffer.clear();
		mUpdateVelocityBuffer.clear();
		physx::PxParticleReadData* readData = ((PxParticleBase*)mParticleActor)->lockParticleReadData();

		bool isDensityValid = false;
		if (!mIsParticleSystem)
		{
			PxParticleFluidReadData* fluidReadData = static_cast<PxParticleFluidReadData*>(readData);
			isDensityValid = (fluidReadData->densityBuffer.ptr() != 0);
		}

		for (uint32_t dstIdx = 0, srcHole = targetCount; dstIdx < targetCount; ++dstIdx)
		{
			uint32_t srcIdx = dstIdx;
			//do we have a hole in dstIdx region?
			if (!(mBenefit[dstIdx] > -FLT_MAX))
			{
				//skip holes in srcIdx region
				while (!(mBenefit[srcHole] > -FLT_MAX))
				{
					++srcHole;
				}
				PX_ASSERT(srcHole < totalCount);
				srcIdx = srcHole++;
			}
			//do we have a new particle?
			bool isNewParticle = (srcIdx >= mParticleCount);

			uint32_t  pxIdx;
			PxVec3 position;
			PxVec3 velocity;
			PxVec3 collisionNormal;
			uint32_t  particleFlags;
			float  density;

			if (isNewParticle)
			{
				PxStrideIterator<uint32_t> indexBuffer(&pxIdx);
				if (mIndexPool->allocateIndices(1, indexBuffer) != 1)
				{
					PX_ALWAYS_ASSERT();
					continue;
				}
				mInputIdToParticleIndex[dstIdx]	= pxIdx;
			}
			else
			{
				pxIdx = mInputIdToParticleIndex[srcIdx];
				PX_ASSERT((readData->flagsBuffer[pxIdx] & PxParticleFlag::eVALID));
				if (dstIdx != srcIdx)
				{
					PX_ASSERT(dstIdx < mParticleCount || !(readData->flagsBuffer[mInputIdToParticleIndex[dstIdx]] & PxParticleFlag::eVALID));
					mInputIdToParticleIndex[dstIdx]	= pxIdx;
				}

				position = readData->positionBuffer[pxIdx],
				velocity = readData->velocityBuffer[pxIdx],
				collisionNormal = readData->collisionNormalBuffer[pxIdx],
				particleFlags = readData->flagsBuffer[pxIdx],
				density = isDensityValid ? static_cast<PxParticleFluidReadData*>(readData)->densityBuffer[pxIdx] : 0.0f;
			}

			unsigned int injIndex;
			float benefit = simulateParticle(
			                    NULL, injectorParamsList,
			                    deltaTime, eyePos,
			                    isNewParticle, srcIdx, dstIdx,
			                    mBufDesc.pmaPositionMass->getPtr(), mBufDesc.pmaVelocityLife->getPtr(), 
								mBufDesc.pmaCollisionNormalFlags->getPtr(), mBufDesc.pmaUserData->getPtr(), mBufDesc.pmaActorIdentifiers->getPtr(),
								mLifeSpan.getPtr(), mLifeTime.getPtr(), mBufDesc.pmaDensity ? mBufDesc.pmaDensity->getPtr() : NULL, mInjector.getPtr(),
			                    fieldAccessor, injIndex,
								mGridDensityParams,
								position,
								velocity,
								collisionNormal,
								particleFlags,
								density
			                );

			if (injIndex < mInjectorsCounters.getSize())
			{
				++mInjectorsCounters[injIndex]; 
			}

			if (isNewParticle)
			{
				NewParticleData data;
				data.destIndex	= pxIdx;
				data.position	= position;
				data.velocity	= velocity;
				mAddedParticleList.pushBack(data);

				mBufDesc.pmaInStateToInput->get(maxStateID) = dstIdx | IosBufferDescIntl::NEW_PARTICLE_FLAG;
				++maxStateID;
			}
			else
			{
				mUpdateIndexBuffer.pushBack(pxIdx);
				mUpdateVelocityBuffer.pushBack(velocity);

				mNewIndices[srcIdx] = dstIdx;
			}

			mBenefit[dstIdx] = benefit;
			if (benefit > -FLT_MAX)
			{
				mLastBenefitSum += benefit;
				mLastBenefitMin = PxMin(mLastBenefitMin, benefit);
				mLastBenefitMax = PxMax(mLastBenefitMax, benefit);
				++mLastActiveCount;
			}
		}

		if (readData)
		{
			readData->unlock();
		}

		if (mUpdateIndexBuffer.size())
		{
			((PxParticleBase*)mParticleActor)->setVelocities(mUpdateIndexBuffer.size(), PxStrideIterator<const uint32_t>(&mUpdateIndexBuffer[0]), PxStrideIterator<const PxVec3>(&mUpdateVelocityBuffer[0]));
		}

		if (mAddedParticleList.size())
		{
			PxParticleCreationData createData;
			createData.numParticles = mAddedParticleList.size();
			createData.positionBuffer = PxStrideIterator<const PxVec3>(&mAddedParticleList[0].position, sizeof(NewParticleData));
			createData.velocityBuffer = PxStrideIterator<const PxVec3>(&mAddedParticleList[0].velocity, sizeof(NewParticleData));
			createData.indexBuffer = PxStrideIterator<const uint32_t>(&mAddedParticleList[0].destIndex, sizeof(NewParticleData));
			bool ok = ((PxParticleBase*)mParticleActor)->createParticles(createData);
			PX_ASSERT(ok);
			PX_UNUSED(ok);
		}

		//update stateToInput
		for (uint32_t i = 0; i < mParticleCount; ++i)
		{
			uint32_t srcIdx = mBufDesc.pmaOutStateToInput->get(i);
			PX_ASSERT(srcIdx < mParticleCount);
			mBufDesc.pmaInStateToInput->get(i) = mNewIndices[srcIdx];
		}
	}
	mParticleCount = targetCount;

	/* Oh! Manager of the IOFX! do your thing */
	mIofxMgr->updateEffectsData(deltaTime, mParticleCount, mParticleCount, maxStateID);
}

uint32_t ParticleIosActorCPU::computeHistogram(uint32_t dataCount, float dataMin, float dataMax, uint32_t& bound)
{
	const float* dataArray = mBenefit.getPtr();

	uint32_t histogram[HISTOGRAM_BIN_COUNT];

	//clear Histogram
	for (uint32_t i = 0; i < HISTOGRAM_BIN_COUNT; ++i)
	{
		histogram[i] = 0;
	}

	float	factor = HISTOGRAM_BIN_COUNT / (dataMax - dataMin);
	//accum Histogram
	for (uint32_t i = 0; i < dataCount; ++i)
	{
		float data = dataArray[i];
		if (data >= dataMin && data < dataMax)
		{
			int32_t bin = int32_t((data - dataMin) * factor);
			++histogram[bin];
		}
	}
	//compute CDF from Histogram
	uint32_t countSum = 0;
	for (uint32_t i = 0; i < HISTOGRAM_BIN_COUNT; ++i)
	{
		uint32_t count = histogram[i];
		countSum += count;
		histogram[i] = countSum;
	}

	PX_ASSERT(countSum == mLastActiveCount + mInjectedCount);

	//binary search in CDF
	uint32_t beg = 0;
	uint32_t end = HISTOGRAM_BIN_COUNT;
	while (beg < end)
	{
		uint32_t mid = beg + ((end - beg) >> 1);
		if (bound > histogram[mid])
		{
			beg = mid + 1;
		}
		else
		{
			end = mid;
		}
	}

	PX_ASSERT(histogram[beg] >= bound);
	if (beg > 0)
	{
		bound -= histogram[beg - 1];
	}

	return beg;
}

}
} // end namespace nvidia


