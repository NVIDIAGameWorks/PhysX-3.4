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

#include "SwCloth.h"
#include "SwFabric.h"
#include "SwFactory.h"
#include "TripletScheduler.h"
#include "ClothBase.h"
#include "PsUtilities.h"

namespace physx
{
namespace cloth
{
PhaseConfig transform(const PhaseConfig&); // from PhaseConfig.cpp
}
}

using namespace physx;
using namespace shdfnd;

cloth::SwCloth::SwCloth(SwFactory& factory, SwFabric& fabric, Range<const PxVec4> particles)
: mFactory(factory), mFabric(fabric), mNumVirtualParticles(0), mUserData(0)
{
	PX_ASSERT(!particles.empty());

	initialize(*this, particles.begin(), particles.end());

#if PX_WINDOWS
	const uint32_t kSimdWidth = 8; // avx
#else
	const uint32_t kSimdWidth = 4; // sse
#endif

	mCurParticles.reserve(particles.size() + kSimdWidth - 1);
	mCurParticles.assign(reinterpret_cast<const PxVec4*>(particles.begin()),
	                     reinterpret_cast<const PxVec4*>(particles.end()));

	// 7 dummy particles used in SIMD solver
	mCurParticles.resize(particles.size() + kSimdWidth - 1, PxVec4(0.0f));
	mPrevParticles = mCurParticles;

	mCurParticles.resize(particles.size());
	mPrevParticles.resize(particles.size());

	mFabric.incRefCount();
}

namespace
{
// copy vector and make same capacity
void copyVector(cloth::Vec4fAlignedVector& dst, const cloth::Vec4fAlignedVector& src)
{
	dst.reserve(src.capacity());
	dst.assign(src.begin(), src.end());

	// ensure valid dummy data
	dst.resize(src.capacity(), PxVec4(0.0f));
	dst.resize(src.size());
}
}

// copy constructor, supports rebinding to a different factory
cloth::SwCloth::SwCloth(SwFactory& factory, const SwCloth& cloth)
: mFactory(factory)
, mFabric(cloth.mFabric)
, mPhaseConfigs(cloth.mPhaseConfigs)
, mCapsuleIndices(cloth.mCapsuleIndices)
, mStartCollisionSpheres(cloth.mStartCollisionSpheres)
, mTargetCollisionSpheres(cloth.mTargetCollisionSpheres)
, mStartCollisionPlanes(cloth.mStartCollisionPlanes)
, mTargetCollisionPlanes(cloth.mTargetCollisionPlanes)
, mStartCollisionTriangles(cloth.mStartCollisionTriangles)
, mTargetCollisionTriangles(cloth.mTargetCollisionTriangles)
, mVirtualParticleIndices(cloth.mVirtualParticleIndices)
, mVirtualParticleWeights(cloth.mVirtualParticleWeights)
, mNumVirtualParticles(cloth.mNumVirtualParticles)
, mSelfCollisionIndices(cloth.mSelfCollisionIndices)
, mRestPositions(cloth.mRestPositions)
{
	copy(*this, cloth);

	// carry over capacity (using as dummy particles)
	copyVector(mCurParticles, cloth.mCurParticles);
	copyVector(mPrevParticles, cloth.mPrevParticles);
	copyVector(mMotionConstraints.mStart, cloth.mMotionConstraints.mStart);
	copyVector(mMotionConstraints.mTarget, cloth.mMotionConstraints.mTarget);
	copyVector(mSeparationConstraints.mStart, cloth.mSeparationConstraints.mStart);
	copyVector(mSeparationConstraints.mTarget, cloth.mSeparationConstraints.mTarget);
	copyVector(mParticleAccelerations, cloth.mParticleAccelerations);

	mFabric.incRefCount();
}

cloth::SwCloth::~SwCloth()
{
	mFabric.decRefCount();
}

cloth::Range<PxVec4> cloth::SwCloth::push(SwConstraints& constraints)
{
	uint32_t n = mCurParticles.size();

	if(!constraints.mTarget.capacity())
		constraints.mTarget.resize((n + 3) & ~3, PxVec4(0.0f)); // reserve multiple of 4 for SIMD

	constraints.mTarget.resizeUninitialized(n);
	PxVec4* data = &constraints.mTarget.front();
	Range<PxVec4> result(data, data + constraints.mTarget.size());

	if(constraints.mStart.empty()) // initialize start first
		constraints.mStart.swap(constraints.mTarget);

	return result;
}

void cloth::SwCloth::clear(SwConstraints& constraints)
{
	Vec4fAlignedVector().swap(constraints.mStart);
	Vec4fAlignedVector().swap(constraints.mTarget);
}

cloth::Range<const PxVec3> cloth::SwCloth::clampTriangleCount(Range<const PxVec3> range, uint32_t)
{
	return range;
}

#include "ClothImpl.h"

namespace physx
{
namespace cloth
{

template <>
Cloth* ClothImpl<SwCloth>::clone(Factory& factory) const
{
	return factory.clone(*this);
}

template <>
uint32_t ClothImpl<SwCloth>::getNumParticles() const
{
	return mCloth.mCurParticles.size();
}

template <>
void ClothImpl<SwCloth>::lockParticles() const
{
}

template <>
void ClothImpl<SwCloth>::unlockParticles() const
{
}

template <>
MappedRange<PxVec4> ClothImpl<SwCloth>::getCurrentParticles()
{
	return getMappedParticles(&mCloth.mCurParticles.front());
}

template <>
MappedRange<const PxVec4> ClothImpl<SwCloth>::getCurrentParticles() const
{
	return getMappedParticles(&mCloth.mCurParticles.front());
}

template <>
MappedRange<PxVec4> ClothImpl<SwCloth>::getPreviousParticles()
{
	return getMappedParticles(&mCloth.mPrevParticles.front());
}

template <>
MappedRange<const PxVec4> ClothImpl<SwCloth>::getPreviousParticles() const
{
	return getMappedParticles(&mCloth.mPrevParticles.front());
}

template <>
GpuParticles ClothImpl<SwCloth>::getGpuParticles()
{
	GpuParticles result = { 0, 0, 0 };
	return result;
}

template <>
void ClothImpl<SwCloth>::setPhaseConfig(Range<const PhaseConfig> configs)
{
	mCloth.mPhaseConfigs.resize(0);

	// transform phase config to use in solver
	for(; !configs.empty(); configs.popFront())
		if(configs.front().mStiffness > 0.0f)
			mCloth.mPhaseConfigs.pushBack(transform(configs.front()));

	mCloth.wakeUp();
}

template <>
void ClothImpl<SwCloth>::setSelfCollisionIndices(Range<const uint32_t> indices)
{
	ContextLockType lock(mCloth.mFactory);
	mCloth.mSelfCollisionIndices.assign(indices.begin(), indices.end());
	mCloth.notifyChanged();
	mCloth.wakeUp();
}

template <>
uint32_t ClothImpl<SwCloth>::getNumVirtualParticles() const
{
	return uint32_t(mCloth.mNumVirtualParticles);
}

template <>
Range<PxVec4> ClothImpl<SwCloth>::getParticleAccelerations()
{
	if(mCloth.mParticleAccelerations.empty())
	{
		uint32_t n = mCloth.mCurParticles.size();
		mCloth.mParticleAccelerations.resize(n, PxVec4(0.0f));
	}

	mCloth.wakeUp();

	PxVec4* data = &mCloth.mParticleAccelerations.front();
	return Range<PxVec4>(data, data + mCloth.mParticleAccelerations.size());
}

template <>
void ClothImpl<SwCloth>::clearParticleAccelerations()
{
	Vec4fAlignedVector().swap(mCloth.mParticleAccelerations);
	mCloth.wakeUp();
}

template <>
void ClothImpl<SwCloth>::setVirtualParticles(Range<const uint32_t[4]> indices, Range<const PxVec3> weights)
{
	mCloth.mNumVirtualParticles = 0;

	// shuffle indices to form independent SIMD sets
	uint16_t numParticles = uint16_t(mCloth.mCurParticles.size());
	TripletScheduler scheduler(indices);
	scheduler.simd(numParticles, 4);

	// convert indices to byte offset
	Vec4us dummy(numParticles, uint16_t(numParticles + 1), uint16_t(numParticles + 2), 0);
	Vector<uint32_t>::Type::ConstIterator sIt = scheduler.mSetSizes.begin();
	Vector<uint32_t>::Type::ConstIterator sEnd = scheduler.mSetSizes.end();
	TripletScheduler::ConstTripletIter tIt = scheduler.mTriplets.begin(), tLast;
	mCloth.mVirtualParticleIndices.resize(0);
	mCloth.mVirtualParticleIndices.reserve(indices.size() + 3 * uint32_t(sEnd - sIt));
	for(; sIt != sEnd; ++sIt)
	{
		uint32_t setSize = *sIt;
		for(tLast = tIt + setSize; tIt != tLast; ++tIt, ++mCloth.mNumVirtualParticles)
			mCloth.mVirtualParticleIndices.pushBack(Vec4us(*tIt));
		mCloth.mVirtualParticleIndices.resize((mCloth.mVirtualParticleIndices.size() + 3) & ~3, dummy);
	}
	Vector<Vec4us>::Type(mCloth.mVirtualParticleIndices.begin(), mCloth.mVirtualParticleIndices.end())
	    .swap(mCloth.mVirtualParticleIndices);

	// precompute 1/dot(w,w)
	Vec4fAlignedVector().swap(mCloth.mVirtualParticleWeights);
	mCloth.mVirtualParticleWeights.reserve(weights.size());
	for(; !weights.empty(); weights.popFront())
	{
		PxVec3 w = reinterpret_cast<const PxVec3&>(weights.front());
		PxReal scale = 1 / w.magnitudeSquared();
		mCloth.mVirtualParticleWeights.pushBack(PxVec4(w.x, w.y, w.z, scale));
	}

	mCloth.notifyChanged();
}

//explicit template instantiation
template class ClothImpl<SwCloth>;

} // namespace cloth
} // namespace physx
