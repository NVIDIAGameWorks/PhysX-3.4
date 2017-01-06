// This code contains NVIDIA Confidential Information and is disclosed to you
// under a form of NVIDIA software license agreement provided separately to you.
//
// Notice
// NVIDIA Corporation and its licensors retain all intellectual property and
// proprietary rights in and to this software and related documentation and
// any modifications thereto. Any use, reproduction, disclosure, or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA Corporation is strictly prohibited.
//
// ALL NVIDIA DESIGN SPECIFICATIONS, CODE ARE PROVIDED "AS IS.". NVIDIA MAKES
// NO WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES OF NONINFRINGEMENT,
// MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Information and code furnished is believed to be accurate and reliable.
// However, NVIDIA Corporation assumes no responsibility for the consequences of use of such
// information or for any infringement of patents or other rights of third parties that may
// result from its use. No license is granted by implication or otherwise under any patent
// or patent rights of NVIDIA Corporation. Details are subject to change without notice.
// This code supersedes and replaces all information previously supplied.
// NVIDIA Corporation products are not authorized for use as critical
// components in life support devices or systems without express written approval of
// NVIDIA Corporation.
//
// Copyright (c) 2008-2017 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.

#include "foundation/PxMath.h"
#include "TripletScheduler.h"
#include "PsUtilities.h"
#include "PsFPU.h"

using namespace physx;
using namespace shdfnd;

cloth::TripletScheduler::TripletScheduler(Range<const uint32_t[4]> triplets)
: mTriplets(reinterpret_cast<const Vec4u*>(triplets.begin()), reinterpret_cast<const Vec4u*>(triplets.end()))
{
}

// SSE version
void cloth::TripletScheduler::simd(uint32_t numParticles, uint32_t simdWidth)
{
	if(mTriplets.empty())
		return;

	Vector<uint32_t>::Type mark(numParticles, uint32_t(-1));

	uint32_t setIndex = 0, setSize = 0;
	for(TripletIter tIt = mTriplets.begin(), tEnd = mTriplets.end(); tIt != tEnd; ++setIndex)
	{
		TripletIter tLast = tIt + PxMin(simdWidth, uint32_t(tEnd - tIt));
		TripletIter tSwap = tEnd;

		for(; tIt != tLast && tIt != tSwap; ++tIt, ++setSize)
		{
			// swap from tail until independent triplet found
			while((mark[tIt->x] == setIndex || mark[tIt->y] == setIndex || mark[tIt->z] == setIndex) && tIt != --tSwap)
				swap(*tIt, *tSwap);

			if(tIt == tSwap)
				break; // no independent triplet found

			// mark vertices to be used in simdIndex
			mark[tIt->x] = setIndex;
			mark[tIt->y] = setIndex;
			mark[tIt->z] = setIndex;
		}

		if(tIt == tSwap) // remaining triplets depend on current set
		{
			if(setSize > simdWidth) // trim set to multiple of simdWidth
			{
				uint32_t overflow = setSize % simdWidth;
				setSize -= overflow;
				tIt -= overflow;
			}
			mSetSizes.pushBack(setSize);
			setSize = 0;
		}
	}
}

namespace
{
struct TripletSet
{
	TripletSet() : mMark(0xFFFFFFFF)
	{
		mNumReplays[0] = mNumReplays[1] = mNumReplays[2] = 1;
		memset(mNumConflicts[0], 0, 32);
		memset(mNumConflicts[1], 0, 32);
		memset(mNumConflicts[2], 0, 32);
	}

	uint32_t mMark; // triplet index
	uint8_t mNumReplays[3];
	uint8_t mNumConflicts[3][32];
};

/*
struct GreaterSum
{
    typedef cloth::Vector<uint32_t>::Type Container;

    GreaterSum(const Container& cont)
        : mContainer(cont)
    {}

    bool operator()(const cloth::Vec4u& a, const cloth::Vec4u& b) const
    {
        return mContainer[a.x] + mContainer[a.y] + mContainer[a.z]
            > mContainer[b.x] + mContainer[b.y] + mContainer[b.z];
    }

    const Container& mContainer;
};
*/

// calculate the inclusive prefix sum, equivalent of std::partial_sum
template <typename T>
void prefixSum(const T* first, const T* last, T* dest)
{
	if(first == last)
		return;
	else
	{
		*(dest++) = *(first++);

		for(; first != last; ++first, ++dest)
			*dest = *(dest - 1) + *first;
	}
}
}

// CUDA version
void cloth::TripletScheduler::warp(uint32_t numParticles, uint32_t warpWidth)
{
	// PX_ASSERT(warpWidth == 32 || warpWidth == 16);

	if(mTriplets.empty())
		return;

	TripletIter tIt, tEnd = mTriplets.end();
	uint32_t tripletIndex;

	// count number of triplets per particle
	Vector<uint32_t>::Type adjacentCount(numParticles + 1, uint32_t(0));
	for(tIt = mTriplets.begin(); tIt != tEnd; ++tIt)
		for(int i = 0; i < 3; ++i)
			++adjacentCount[(*tIt)[i]];

	/* neither of those were really improving number of batches:
	// run simd version to pre-sort particles
	simd(numParticles, blockWidth); mSetSizes.resize(0);
	// sort according to triplet degree (estimated by sum of adjacentCount)
	std::sort(mTriplets.begin(), tEnd, GreaterSum(adjacentCount));
	*/

	uint32_t maxTripletCount = *maxElement(adjacentCount.begin(), adjacentCount.end());

	// compute in place prefix sum (inclusive)
	prefixSum(adjacentCount.begin(), adjacentCount.end(), adjacentCount.begin());

	// initialize adjacencies (for each particle, collect touching triplets)
	// also converts partial sum in adjacentCount from inclusive to exclusive
	Vector<uint32_t>::Type adjacencies(adjacentCount.back());
	for(tIt = mTriplets.begin(), tripletIndex = 0; tIt != tEnd; ++tIt, ++tripletIndex)
		for(int i = 0; i < 3; ++i)
			adjacencies[--adjacentCount[(*tIt)[i]]] = tripletIndex;

	uint32_t warpMask = warpWidth - 1;

	uint32_t numSets = maxTripletCount; // start with minimum number of sets
	Vector<TripletSet>::Type sets(numSets);
	Vector<uint32_t>::Type setIndices(mTriplets.size(), uint32_t(-1));
	mSetSizes.resize(numSets);

	// color triplets (assign to sets)
	Vector<uint32_t>::Type::ConstIterator aBegin = adjacencies.begin(), aIt, aEnd;
	for(tIt = mTriplets.begin(), tripletIndex = 0; tIt != tEnd; ++tIt, ++tripletIndex)
	{
		// mark sets of adjacent triplets
		for(int i = 0; i < 3; ++i)
		{
			uint32_t particleIndex = (*tIt)[i];
			aIt = aBegin + adjacentCount[particleIndex];
			aEnd = aBegin + adjacentCount[particleIndex + 1];
			for(uint32_t setIndex; aIt != aEnd; ++aIt)
				if(numSets > (setIndex = setIndices[*aIt]))
					sets[setIndex].mMark = tripletIndex;
		}

		// find valid set with smallest number of bank conflicts
		uint32_t bestIndex = numSets;
		uint32_t minReplays = 4;
		for(uint32_t setIndex = 0; setIndex < numSets && minReplays; ++setIndex)
		{
			const TripletSet& set = sets[setIndex];

			if(set.mMark == tripletIndex)
				continue; // triplet collision

			uint32_t numReplays = 0;
			for(uint32_t i = 0; i < 3; ++i)
				numReplays += set.mNumReplays[i] == set.mNumConflicts[i][warpMask & (*tIt)[i]];

			if(minReplays > numReplays)
			{
				minReplays = numReplays;
				bestIndex = setIndex;
			}
		}

		// add new set if none found
		if(bestIndex == numSets)
		{
			sets.pushBack(TripletSet());
			mSetSizes.pushBack(0);
			++numSets;
		}

		// increment bank conflicts or reset if warp filled
		TripletSet& set = sets[bestIndex];
		if(++mSetSizes[bestIndex] & warpMask)
			for(uint32_t i = 0; i < 3; ++i)
				set.mNumReplays[i] = PxMax(set.mNumReplays[i], ++set.mNumConflicts[i][warpMask & (*tIt)[i]]);
		else
			set = TripletSet();

		setIndices[tripletIndex] = bestIndex;
	}

	// reorder triplets
	Vector<uint32_t>::Type setOffsets(mSetSizes.size());
	prefixSum(mSetSizes.begin(), mSetSizes.end(), setOffsets.begin());

	Vector<Vec4u>::Type triplets(mTriplets.size());
	Vector<uint32_t>::Type::ConstIterator iIt = setIndices.begin();
	for(tIt = mTriplets.begin(), tripletIndex = 0; tIt != tEnd; ++tIt, ++iIt)
		triplets[--setOffsets[*iIt]] = *tIt;

	mTriplets.swap(triplets);
}
