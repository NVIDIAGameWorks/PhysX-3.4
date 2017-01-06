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

#include "PxAssert.h"
#include "SwFabric.h"
#include "SwFactory.h"
#include "PsSort.h"
#include "limits.h" // for USHRT_MAX

#include "PsUtilities.h"

using namespace nvidia;
using namespace physx::shdfnd;

cloth::SwTether::SwTether(uint16_t anchor, float length) : mAnchor(anchor), mLength(length)
{
}

cloth::SwFabric::SwFabric(SwFactory& factory, uint32_t numParticles, Range<const uint32_t> phases,
                          Range<const uint32_t> sets, Range<const float> restvalues, Range<const uint32_t> indices,
                          Range<const uint32_t> anchors, Range<const float> tetherLengths, uint32_t id)
: mFactory(factory), mNumParticles(numParticles), mTetherLengthScale(1.0f), mId(id)
{
	// should no longer be prefixed with 0
	PX_ASSERT(sets.front() != 0);

#if PX_WINDOWS_FAMILY
	const uint32_t kSimdWidth = 8; // avx
#else
	const uint32_t kSimdWidth = 4;
#endif

	// consistency check
	PX_ASSERT(sets.back() == restvalues.size());
	PX_ASSERT(restvalues.size() * 2 == indices.size());
	PX_ASSERT(mNumParticles > *maxElement(indices.begin(), indices.end()));
	PX_ASSERT(mNumParticles + kSimdWidth - 1 <= USHRT_MAX);

	mPhases.assign(phases.begin(), phases.end());
	mSets.reserve(sets.size() + 1);
	mSets.pushBack(0); // prefix with 0

	mOriginalNumRestvalues = uint32_t(restvalues.size());

	// padd indices for SIMD
	const uint32_t* iBegin = indices.begin(), *iIt = iBegin;
	const float* rBegin = restvalues.begin(), *rIt = rBegin;
	const uint32_t* sIt, *sEnd = sets.end();
	for(sIt = sets.begin(); sIt != sEnd; ++sIt)
	{
		const float* rEnd = rBegin + *sIt;
		const uint32_t* iEnd = iBegin + *sIt * 2;
		uint32_t numConstraints = uint32_t(rEnd - rIt);

		for(; rIt != rEnd; ++rIt)
			mRestvalues.pushBack(*rIt);

		for(; iIt != iEnd; ++iIt)
			mIndices.pushBack(uint16_t(*iIt));

		// add dummy indices to make multiple of 4
		for(; numConstraints &= kSimdWidth - 1; ++numConstraints)
		{
			mRestvalues.pushBack(-FLT_MAX);
			uint32_t index = mNumParticles + numConstraints - 1;
			mIndices.pushBack(uint16_t(index));
			mIndices.pushBack(uint16_t(index));
		}

		mSets.pushBack(uint32_t(mRestvalues.size()));
	}

	// trim overallocations
	RestvalueContainer(mRestvalues.begin(), mRestvalues.end()).swap(mRestvalues);
	Vector<uint16_t>::Type(mIndices.begin(), mIndices.end()).swap(mIndices);

	// tethers
	PX_ASSERT(anchors.size() == tetherLengths.size());

	// pad to allow for direct 16 byte (unaligned) loads
	mTethers.reserve(anchors.size() + 2);
	for(; !anchors.empty(); anchors.popFront(), tetherLengths.popFront())
		mTethers.pushBack(SwTether(uint16_t(anchors.front()), tetherLengths.front()));

	mFactory.mFabrics.pushBack(this);
}

cloth::SwFabric::~SwFabric()
{
	Vector<SwFabric*>::Type::Iterator fIt = mFactory.mFabrics.find(this);
	PX_ASSERT(fIt != mFactory.mFabrics.end());
	mFactory.mFabrics.replaceWithLast(fIt);
}

cloth::Factory& cloth::SwFabric::getFactory() const
{
	return mFactory;
}

uint32_t cloth::SwFabric::getNumPhases() const
{
	return uint32_t(mPhases.size());
}

uint32_t cloth::SwFabric::getNumRestvalues() const
{
	return mOriginalNumRestvalues;
}

uint32_t cloth::SwFabric::getNumSets() const
{
	return uint32_t(mSets.size() - 1);
}

uint32_t cloth::SwFabric::getNumIndices() const
{
	return 2 * mOriginalNumRestvalues;
}

uint32_t cloth::SwFabric::getNumParticles() const
{
	return mNumParticles;
}

uint32_t cloth::SwFabric::getNumTethers() const
{
	return uint32_t(mTethers.size());
}

void cloth::SwFabric::scaleRestvalues(float scale)
{
	RestvalueContainer::Iterator rIt, rEnd = mRestvalues.end();
	for(rIt = mRestvalues.begin(); rIt != rEnd; ++rIt)
		*rIt *= scale;
}

void cloth::SwFabric::scaleTetherLengths(float scale)
{
	mTetherLengthScale *= scale;
}
