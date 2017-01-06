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

#if PX_PROFILE
#include "PxProfileEventSender.h"
#include "PxProfileZone.h"
#else
namespace physx
{
namespace profile
{
	class PxProfileZone;
}
}
#endif

namespace nvidia
{
namespace cloth
{

#if PX_PROFILE

struct ProfileZone
{
	ProfileZone(const char* name, profile::PxProfileZone* profiler)
	: mSender(profiler), mEventId(profiler ? profiler->getEventIdForName(name) : uint16_t(-1))
	{
		if(mSender)
			mSender->startEvent(mEventId, (uint64_t)intptr_t(this));
	}

	~ProfileZone()
	{
		if(mSender)
			mSender->stopEvent(mEventId, (uint64_t)intptr_t(this));
	}

	void setValue(int64_t value) const
	{
		if(mSender)
			mSender->eventValue(mEventId, (uint64_t)intptr_t(this), value);
	}

	profile::PxProfileEventSender* mSender;
	uint16_t mEventId;
};

#else // PX_PROFILE

struct ProfileZone
{
	ProfileZone(const char*, profile::PxProfileZone*)
	{
	}
	void setValue(int64_t) const
	{
	}
};

#endif // PX_PROFILE

class SwCloth;
struct SwClothData;
template <typename>
struct IterationState;
struct IndexPair;
struct SphereData;
struct ConeData;
struct TriangleData;

typedef StackAllocator<16> SwKernelAllocator;

/**
   Collision handler for SwSolver.
 */
template <typename Simd4f>
class SwCollision
{
	typedef typename Simd4fToSimd4i<Simd4f>::Type Simd4i;

  public:
	struct ShapeMask
	{
		Simd4i mCones;
		Simd4i mSpheres;

		ShapeMask& operator=(const ShapeMask&);
		ShapeMask& operator&=(const ShapeMask&);
	};

	struct CollisionData
	{
		CollisionData();
		SphereData* mSpheres;
		ConeData* mCones;
	};

	struct ImpulseAccumulator;

  public:
	SwCollision(SwClothData& clothData, SwKernelAllocator& alloc, profile::PxProfileZone* profiler);
	~SwCollision();

	void operator()(const IterationState<Simd4f>& state);

	static size_t estimateTemporaryMemory(const SwCloth& cloth);
	static size_t estimatePersistentMemory(const SwCloth& cloth);

  private:
	SwCollision& operator=(const SwCollision&); // not implemented
	void allocate(CollisionData&);
	void deallocate(const CollisionData&);

	void computeBounds();

	void buildSphereAcceleration(const SphereData*);
	void buildConeAcceleration();
	static void mergeAcceleration(uint32_t*);
	bool buildAcceleration();

	static ShapeMask getShapeMask(const Simd4f&, const Simd4i*, const Simd4i*);
	ShapeMask getShapeMask(const Simd4f*) const;
	ShapeMask getShapeMask(const Simd4f*, const Simd4f*) const;

	void collideSpheres(const Simd4i&, const Simd4f*, ImpulseAccumulator&) const;
	Simd4i collideCones(const Simd4f*, ImpulseAccumulator&) const;

	void collideSpheres(const Simd4i&, const Simd4f*, Simd4f*, ImpulseAccumulator&) const;
	Simd4i collideCones(const Simd4f*, Simd4f*, ImpulseAccumulator&) const;

	void collideParticles();
	void collideVirtualParticles();
	void collideContinuousParticles();

	void collideConvexes(const IterationState<Simd4f>&);
	void collideConvexes(const Simd4f*, Simd4f*, ImpulseAccumulator&);

	void collideTriangles(const IterationState<Simd4f>&);
	void collideTriangles(const TriangleData*, Simd4f*, ImpulseAccumulator&);

  public:
	// acceleration structure
	static const uint32_t sGridSize = 8;
	Simd4i mSphereGrid[6 * sGridSize / 4];
	Simd4i mConeGrid[6 * sGridSize / 4];
	Simd4f mGridScale, mGridBias;

	CollisionData mPrevData;
	CollisionData mCurData;

	SwClothData& mClothData;
	SwKernelAllocator& mAllocator;

	uint32_t mNumCollisions;

	profile::PxProfileZone* mProfiler;

	static const Simd4f sSkeletonWidth;
};
}
}
