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
// Copyright (c) 2008-2018 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.

#include "CuFabric.h"
#include "CuContextLock.h"
#include "CuFactory.h"

#if PX_VC
#pragma warning(disable : 4365) // 'action' : conversion from 'type_1' to 'type_2', signed/unsigned mismatch
#endif

using namespace physx;
using namespace shdfnd;

cloth::CuTether::CuTether(uint16_t anchor, uint16_t length) : mAnchor(anchor), mLength(length)
{
}

cloth::CuFabric::CuFabric(CuFactory& factory, uint32_t numParticles, Range<const uint32_t> phases,
                          Range<const uint32_t> sets, Range<const float> restvalues, Range<const uint32_t> indices,
                          Range<const uint32_t> anchors, Range<const float> tetherLengths,
                          Range<const uint32_t> triangles, uint32_t id)
: CuContextLock(factory)
, mFactory(factory)
, mNumParticles(numParticles)
, mPhases(mFactory.mContextManager, phases.begin(), phases.end())
, mSets(mFactory.mContextManager)
, mRestvalues(mFactory.mContextManager, restvalues.begin(), restvalues.end())
, mIndices(mFactory.mContextManager)
, mTethers(mFactory.mContextManager)
, mTriangles(mFactory.mContextManager)
, mId(id)
{
	// should no longer be prefixed with 0
	PX_ASSERT(sets.front() != 0);

	PX_ASSERT(sets.back() == restvalues.size());
	PX_ASSERT(restvalues.size() * 2 == indices.size());
	PX_ASSERT(mNumParticles > *maxElement(indices.begin(), indices.end()));

	// copy to device, add leading zero
	mSets.reserve(sets.size() + 1);
	mSets.push_back(0);
	mSets.push_back(sets.begin(), sets.end());

	// manually convert uint32_t indices to uint16_t in temp memory
	Vector<uint16_t>::Type hostIndices;
	hostIndices.resizeUninitialized(indices.size());
	Vector<uint16_t>::Type::Iterator dIt = hostIndices.begin();

	const uint32_t* it = indices.begin();
	const uint32_t* end = indices.end();
	for(; it != end; ++it, ++dIt)
		*dIt = uint16_t(*it);

	// copy to device vector in one go
	mIndices.assign(hostIndices.begin(), hostIndices.end());

	// gather data per phase
	mNumConstraintsInPhase.reserve(phases.size());
	CuDevicePointer<const float> devRestvalues = mRestvalues.begin();
	CuDevicePointer<const uint16_t> devIndices = mIndices.begin();
	for(const uint32_t* pIt = phases.begin(); pIt != phases.end(); ++pIt)
	{
		uint32_t setIndex = *pIt;
		uint32_t firstIndex = setIndex ? sets[setIndex - 1] : 0;
		uint32_t lastIndex = sets[setIndex];
		mNumConstraintsInPhase.pushBack(lastIndex - firstIndex);
		mRestvaluesInPhase.pushBack(devRestvalues + firstIndex);
		mIndicesInPhase.pushBack(devIndices + 2 * firstIndex);
	}

	// tethers
	PX_ASSERT(anchors.size() == tetherLengths.size());
	mTetherLengthScale =
	    tetherLengths.empty() ? 1.0f : *maxElement(tetherLengths.begin(), tetherLengths.end()) / USHRT_MAX;
	float inverseScale = 1 / (mTetherLengthScale + FLT_EPSILON);
	Vector<CuTether>::Type tethers;
	tethers.reserve(anchors.size());
	for(; !anchors.empty(); anchors.popFront(), tetherLengths.popFront())
	{
		tethers.pushBack(CuTether(uint16_t(anchors.front()), uint16_t(tetherLengths.front() * inverseScale + 0.5f)));
	}
	mTethers.assign(tethers.begin(), tethers.end());

	// triangles
	hostIndices.resizeUninitialized(triangles.size());
	dIt = hostIndices.begin();

	it = triangles.begin();
	end = triangles.end();
	for(; it != end; ++it, ++dIt)
		*dIt = uint16_t(*it);

	mTriangles.assign(hostIndices.begin(), hostIndices.end());

	CuContextLock::release();

	// add to factory
	mFactory.mFabrics.pushBack(this);
}

cloth::CuFabric::~CuFabric()
{
	CuContextLock::acquire();

	Vector<CuFabric*>::Type::Iterator fIt = mFactory.mFabrics.find(this);

	PX_ASSERT(fIt != mFactory.mFabrics.end());
	mFactory.mFabrics.replaceWithLast(fIt);
}

cloth::Factory& physx::cloth::CuFabric::getFactory() const
{
	return mFactory;
}

uint32_t cloth::CuFabric::getNumPhases() const
{
	return uint32_t(mPhases.size());
}

uint32_t cloth::CuFabric::getNumRestvalues() const
{
	return uint32_t(mRestvalues.size());
}

uint32_t cloth::CuFabric::getNumSets() const
{
	return uint32_t(mSets.size() - 1);
}

uint32_t cloth::CuFabric::getNumIndices() const
{
	return uint32_t(mIndices.size());
}

uint32_t cloth::CuFabric::getNumParticles() const
{
	return mNumParticles;
}

uint32_t physx::cloth::CuFabric::getNumTethers() const
{
	return uint32_t(mTethers.size());
}

uint32_t physx::cloth::CuFabric::getNumTriangles() const
{
	return uint32_t(mTriangles.size()) / 3;
}

void physx::cloth::CuFabric::scaleRestvalues(float scale)
{
	CuContextLock contextLock(mFactory);

	Vector<float>::Type restvalues(uint32_t(mRestvalues.size()));
	mFactory.copyToHost(mRestvalues.begin().get(), mRestvalues.end().get(), restvalues.begin());

	Vector<float>::Type::Iterator rIt, rEnd = restvalues.end();
	for(rIt = restvalues.begin(); rIt != rEnd; ++rIt)
		*rIt *= scale;

	mRestvalues = restvalues;
}

void physx::cloth::CuFabric::scaleTetherLengths(float scale)
{
	// cloth instances won't pick this up until CuClothData is dirty!
	mTetherLengthScale *= scale;
}
