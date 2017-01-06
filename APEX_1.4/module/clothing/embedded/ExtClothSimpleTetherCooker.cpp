/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

#include "ExtClothConfig.h"
#if APEX_USE_CLOTH_API

#include "ExtClothTetherCooker.h"
#include "PxStrideIterator.h"
#include "PxVec4.h"
#include "PsIntrinsics.h"
#include "PsArray.h"

using namespace nvidia;
using namespace physx;

struct nvidia::PxClothSimpleTetherCookerImpl
{
	PxClothSimpleTetherCookerImpl(const PxClothMeshDesc& desc);

	uint32_t	getCookerStatus() const;
	void	getTetherData(uint32_t* userTetherAnchors, float* userTetherLengths) const;

public:
	// output
	shdfnd::Array<uint32_t>	mTetherAnchors;
	shdfnd::Array<float>	mTetherLengths;

protected:
	void	createTetherData(const PxClothMeshDesc &desc);

	uint32_t	mCookerStatus;
};

PxClothSimpleTetherCooker::PxClothSimpleTetherCooker(const PxClothMeshDesc& desc)
: mImpl(new PxClothSimpleTetherCookerImpl(desc))
{
}

PxClothSimpleTetherCooker::~PxClothSimpleTetherCooker()
{
	delete mImpl;
}

uint32_t PxClothSimpleTetherCooker::getCookerStatus() const
{
	return mImpl->getCookerStatus();
}

void PxClothSimpleTetherCooker::getTetherData(uint32_t* userTetherAnchors, float* userTetherLengths) const
{
	mImpl->getTetherData(userTetherAnchors, userTetherLengths);
}

///////////////////////////////////////////////////////////////////////////////
PxClothSimpleTetherCookerImpl::PxClothSimpleTetherCookerImpl(const PxClothMeshDesc &desc) : mCookerStatus(1)
{
	createTetherData(desc);
}

///////////////////////////////////////////////////////////////////////////////
void PxClothSimpleTetherCookerImpl::createTetherData(const PxClothMeshDesc &desc)
{
	uint32_t numParticles = desc.points.count;

	if (!desc.invMasses.data)
		return;

	// assemble points
	shdfnd::Array<PxVec4> particles;
	particles.reserve(numParticles);
	PxStrideIterator<const PxVec3> pIt((const PxVec3*)desc.points.data, desc.points.stride);
	PxStrideIterator<const float> wIt((const float*)desc.invMasses.data, desc.invMasses.stride);
	for(uint32_t i=0; i<numParticles; ++i)
		particles.pushBack(PxVec4(*pIt++, wIt.ptr() ? *wIt++ : 1.0f));

	// compute tether data
	shdfnd::Array<uint32_t> attachedIndices;
	for(uint32_t i=0; i < numParticles; ++i)
		if(particles[i].w == 0.0f)
			attachedIndices.pushBack(i);

	uint32_t n = attachedIndices.empty() ? 0 : numParticles;
	for(uint32_t i=0; i < n; ++i)
	{
		mTetherAnchors.reserve(numParticles);
		mTetherLengths.reserve(numParticles);

		PxVec3 position = reinterpret_cast<const PxVec3&>(particles[i]);
		float minSqrDist = FLT_MAX;
		uint32_t minIndex = numParticles;
		const uint32_t *aIt, *aEnd = attachedIndices.end();
		for(aIt = attachedIndices.begin(); aIt != aEnd; ++aIt)
		{
			float sqrDist = (reinterpret_cast<const PxVec3&>(
				particles[*aIt]) - position).magnitudeSquared();
			if(minSqrDist > sqrDist)
				minSqrDist = sqrDist, minIndex = *aIt;
		}

		mTetherAnchors.pushBack(minIndex);
		mTetherLengths.pushBack(PxSqrt(minSqrDist));
	}

	PX_ASSERT(mTetherAnchors.size() == mTetherLengths.size());
	if (numParticles == mTetherAnchors.size() && numParticles == mTetherLengths.size())
	{
		mCookerStatus = 0;
	}
}

///////////////////////////////////////////////////////////////////////////////  
uint32_t PxClothSimpleTetherCookerImpl::getCookerStatus() const
{
	return mCookerStatus;
}

///////////////////////////////////////////////////////////////////////////////
void  
PxClothSimpleTetherCookerImpl::getTetherData(uint32_t* userTetherAnchors, float* userTetherLengths) const
{
	intrinsics::memCopy(userTetherAnchors, mTetherAnchors.begin(), mTetherAnchors.size() * sizeof(uint32_t));
	intrinsics::memCopy(userTetherLengths, mTetherLengths.begin(), mTetherLengths.size() * sizeof(float));
}


#endif //APEX_USE_CLOTH_API


