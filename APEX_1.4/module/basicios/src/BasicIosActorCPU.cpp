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
#include "SceneIntl.h"
#include "ApexSDKIntl.h"

#include "BasicIosActor.h"
#include "BasicIosActorCPU.h"
#include "BasicIosAssetImpl.h"
#include "IofxAsset.h"
#include "IofxActor.h"
#include "ModuleBasicIosImpl.h"
#include "BasicIosSceneCPU.h"
#include "RenderDebugInterface.h"
#include "AuthorableObjectIntl.h"
#include "FieldSamplerQueryIntl.h"
#include "PxMath.h"
#include "ApexMirroredArray.h"

namespace nvidia
{
namespace basicios
{

#pragma warning(disable: 4355) // 'this' : used in base member initializer list

BasicIosActorCPU::BasicIosActorCPU(
    ResourceList& list,
    BasicIosAssetImpl& asset,
    BasicIosScene& scene,
    nvidia::apex::IofxAsset& iofxAsset)
	: BASIC_IOS_ACTOR(list, asset, scene, iofxAsset, false)
	, mSimulateTask(*this)
{
	initStorageGroups(mSimulationStorage);

	mLifeTime.setSize(mMaxParticleCount);
	mLifeSpan.setSize(mMaxTotalParticleCount);
	mInjector.setSize(mMaxTotalParticleCount);
	mBenefit.setSize(mMaxTotalParticleCount);

	if (mAsset->mParams->collisionWithConvex)
	{
		mConvexPlanes.reserve(MAX_CONVEX_PLANES_COUNT);
		mConvexVerts.reserve(MAX_CONVEX_VERTS_COUNT);
		mConvexPolygonsData.reserve(MAX_CONVEX_POLYGONS_DATA_SIZE);
	}
	if (mAsset->mParams->collisionWithTriangleMesh)
	{
		mTrimeshVerts.reserve(MAX_TRIMESH_VERTS_COUNT);
		mTrimeshIndices.reserve(MAX_TRIMESH_INDICES_COUNT);
	}

	mNewIndices.resize(mMaxParticleCount);
}
BasicIosActorCPU::~BasicIosActorCPU()
{
}

void BasicIosActorCPU::submitTasks()
{
	BasicIosActorImpl::submitTasks();

	mInjectorsCounters.setSize(mInjectorList.getSize(), ApexMirroredPlace::CPU); 
	PxTaskManager* tm = mBasicIosScene->getApexScene().getTaskManager();
	tm->submitUnnamedTask(mSimulateTask);
}

void BasicIosActorCPU::setTaskDependencies()
{
	BasicIosActorImpl::setTaskDependencies(&mSimulateTask, false);
}

void BasicIosActorCPU::fetchResults()
{
	BASIC_IOS_ACTOR::fetchResults();
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

void BasicIosActorCPU::simulateParticles()
{
	float deltaTime = mBasicIosScene->getApexScene().getPhysXSimulateTime();
	const PxVec3& eyePos = mBasicIosScene->getApexScene().getEyePosition();

	mTotalElapsedTime += deltaTime;

	PxVec3 gravity = -mUp;

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

		uint32_t boundBin = computeHistogram(totalCount, benefitMin, benefitMax, boundCount);
		for (uint32_t i = 0, boundIndex = 0; i < totalCount; ++i)
		{
			float benefit = mBenefit[i];
			if (benefit > -FLT_MAX)
			{
				PX_ASSERT(benefit >= benefitMin && benefit < benefitMax);

				uint32_t bin = uint32_t((benefit - benefitMin) * HISTOGRAM_BIN_COUNT / (benefitMax - benefitMin));
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

		checkBenefit(totalCount);
		checkHoles(totalCount);
	}
	mLastActiveCount = 0;
	mLastBenefitSum  = 0.0f;
	mLastBenefitMin  = +FLT_MAX;
	mLastBenefitMax  = -FLT_MAX;

	if (targetCount > 0)
	{
		const InjectorParams* injectorParamsList = DYNAMIC_CAST(BasicIosSceneCPU*)(mBasicIosScene)->mInjectorParamsArray.begin();

		FieldAccessor fieldAccessor(mFieldSamplerQuery ? mField.getPtr() : 0);

		SimulationParams simParams;
		mSimulationParamsHandle.fetch(mSimulationStorage, simParams);

		for (uint32_t dest = 0, srcHole = targetCount; dest < targetCount; ++dest)
		{
			uint32_t src = dest;
			//do we have a hole in dest region?
			if (!(mBenefit[dest] > -FLT_MAX))
			{
				//skip holes in src region
				while (!(mBenefit[srcHole] > -FLT_MAX))
				{
					++srcHole;
				}
				PX_ASSERT(srcHole < totalCount);
				src = srcHole++;
			}
			//do we have a new particle?
			bool isNewParticle = (src >= mParticleCount);

			unsigned int injIndex;
			float benefit = simulateParticle(
			                    &simParams, mSimulationStorage, injectorParamsList,
			                    deltaTime, gravity, eyePos,
			                    isNewParticle, src, dest,
			                    mBufDesc.pmaPositionMass->getPtr(), mBufDesc.pmaVelocityLife->getPtr(), mBufDesc.pmaActorIdentifiers->getPtr(),
								mLifeSpan.getPtr(), mLifeTime.getPtr(), mInjector.getPtr(), mBufDesc.pmaCollisionNormalFlags->getPtr(), mBufDesc.pmaUserData->getPtr(),
			                    fieldAccessor, injIndex
			                );

			if (injIndex < mInjectorsCounters.getSize())
			{
				++mInjectorsCounters[injIndex]; 
			}
			
			if (!isNewParticle)
			{
				mNewIndices[src] = dest;
			}
			else
			{
				mBufDesc.pmaInStateToInput->get(maxStateID) = dest | IosBufferDescIntl::NEW_PARTICLE_FLAG;
				++maxStateID;
			}

			mBenefit[dest] = benefit;
			if (benefit > -FLT_MAX)
			{
				mLastBenefitSum += benefit;
				mLastBenefitMin = PxMin(mLastBenefitMin, benefit);
				mLastBenefitMax = PxMax(mLastBenefitMax, benefit);
				++mLastActiveCount;
			}
		}

		//update stateToInput
		for (uint32_t i = 0; i < mParticleCount; ++i)
		{
			uint32_t src = mBufDesc.pmaOutStateToInput->get(i);
			PX_ASSERT( src < mParticleCount );
			mBufDesc.pmaInStateToInput->get(i) = mNewIndices[src];
		}
	}
	checkInState(totalCount);

	mParticleCount = targetCount;

	/* Oh! Manager of the IOFX! do your thing */
	mIofxMgr->updateEffectsData(deltaTime, mParticleCount, mParticleCount, maxStateID);
}

uint32_t BasicIosActorCPU::computeHistogram(uint32_t dataCount, float dataMin, float dataMax, uint32_t& bound)
{
	const float* dataArray = mBenefit.getPtr();

	uint32_t histogram[HISTOGRAM_BIN_COUNT];

	//clear Histogram
	for (uint32_t i = 0; i < HISTOGRAM_BIN_COUNT; ++i)
	{
		histogram[i] = 0;
	}
	//accum Histogram
	for (uint32_t i = 0; i < dataCount; ++i)
	{
		float data = dataArray[i];
		if (data >= dataMin && data < dataMax)
		{
			uint32_t bin = uint32_t((data - dataMin) * HISTOGRAM_BIN_COUNT / (dataMax - dataMin));
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

	checkHistogram(bound, histogram[beg], histogram[HISTOGRAM_BIN_COUNT - 1]);

	if (beg > 0)
	{
		bound -= histogram[beg - 1];
	}

	return beg;
}

}
} // namespace nvidia
