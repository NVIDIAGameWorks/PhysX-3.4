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

#pragma once

#include "Types.h"

#include "StackAllocator.h"

#include "Simd4i.h"

#include "PxMat44.h"
#include "PxTransform.h"
#include "PxBounds3.h"

namespace physx
{
	namespace profile
	{
		class PxProfileZone;
	}
}

namespace nvidia
{
namespace cloth
{

class SwCloth;
struct SwClothData;

typedef StackAllocator<16> SwKernelAllocator;

typedef bool (*InterCollisionFilter)(void* cloth0, void* cloth1);

struct SwInterCollisionData
{
	SwInterCollisionData()
	{
	}
	SwInterCollisionData(PxVec4* particles, PxVec4* prevParticles, uint32_t numParticles, uint32_t* indices,
	                     const PxTransform& globalPose, const PxVec3& boundsCenter, const PxVec3& boundsHalfExtents,
	                     float impulseScale, void* userData)
	: mParticles(particles)
	, mPrevParticles(prevParticles)
	, mNumParticles(numParticles)
	, mIndices(indices)
	, mGlobalPose(globalPose)
	, mBoundsCenter(boundsCenter)
	, mBoundsHalfExtent(boundsHalfExtents)
	, mImpulseScale(impulseScale)
	, mUserData(userData)
	{
	}

	PxVec4* mParticles;
	PxVec4* mPrevParticles;
	uint32_t mNumParticles;
	uint32_t* mIndices;
	PxTransform mGlobalPose;
	PxVec3 mBoundsCenter;
	PxVec3 mBoundsHalfExtent;
	float mImpulseScale;
	void* mUserData;
};

template <typename Simd4f>
class SwInterCollision
{

  public:
	SwInterCollision(const SwInterCollisionData* cloths, uint32_t n, float colDist, float stiffness, uint32_t iterations,
	                 InterCollisionFilter filter, cloth::SwKernelAllocator& alloc, nvidia::profile::PxProfileZone* zone);

	~SwInterCollision();

	void operator()();

	static size_t estimateTemporaryMemory(SwInterCollisionData* cloths, uint32_t n);

  private:
	SwInterCollision& operator=(const SwInterCollision&); // not implemented

	static size_t getBufferSize(uint32_t);

	void collideParticles(const uint32_t* keys, uint32_t firstColumnSize, const uint32_t* sortedIndices,
	                      uint32_t numParticles, uint32_t collisionDistance);

	Simd4f& getParticle(uint32_t index);

	// better wrap these in a struct
	void collideParticle(uint32_t index);

	Simd4f mParticle;
	Simd4f mImpulse;

	Simd4f mCollisionDistance;
	Simd4f mCollisionSquareDistance;
	Simd4f mStiffness;

	uint16_t mClothIndex;
	uint32_t mClothMask;
	uint32_t mParticleIndex;

	uint32_t mNumIterations;

	const SwInterCollisionData* mInstances;
	uint32_t mNumInstances;

	uint16_t* mClothIndices;
	uint32_t* mParticleIndices;
	uint32_t mNumParticles;
	uint32_t* mOverlapMasks;

	uint32_t mTotalParticles;

	InterCollisionFilter mFilter;

	SwKernelAllocator& mAllocator;

	profile::PxProfileZone* mProfiler;

  public:
	mutable uint32_t mNumTests;
	mutable uint32_t mNumCollisions;
};

} // namespace cloth

} // namespace nvidia
