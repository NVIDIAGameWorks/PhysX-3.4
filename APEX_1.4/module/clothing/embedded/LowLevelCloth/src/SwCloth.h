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

#include "Cloth.h"
#include "Range.h"
#include "MovingAverage.h"
#include "PhaseConfig.h"
#include "IndexPair.h"
#include "Vec4T.h"
#include "Array.h"
#include "PxTransform.h"

namespace nvidia
{

namespace cloth
{

class SwFabric;
class SwFactory;
#if APEX_UE4
class SwCloth;
#endif

typedef AlignedVector<PxVec4, 16>::Type Vec4fAlignedVector;

struct SwConstraints
{
	void pop()
	{
		if(!mTarget.empty())
		{
			mStart.swap(mTarget);
			mTarget.resize(0);
		}
	}

	Vec4fAlignedVector mStart;
	Vec4fAlignedVector mTarget;
};

class SwCloth
{
	SwCloth& operator=(const SwCloth&); // not implemented
	struct SwContextLock
	{
		SwContextLock(const SwFactory&)
		{
		}
	};

  public:
	typedef SwFactory FactoryType;
	typedef SwFabric FabricType;
	typedef SwContextLock ContextLockType;

	typedef Vec4fAlignedVector& MappedVec4fVectorType;
	typedef Vector<IndexPair>::Type& MappedIndexVectorType;

	SwCloth(SwFactory&, SwFabric&, Range<const PxVec4>);
	SwCloth(SwFactory&, const SwCloth&);
	~SwCloth(); // not virtual on purpose

  public:
	bool isSleeping() const
	{
		return mSleepPassCounter >= mSleepAfterCount;
	}
	void wakeUp()
	{
		mSleepPassCounter = 0;
	}

	void notifyChanged()
	{
	}

	void setParticleBounds(const float*);

	Range<PxVec4> push(SwConstraints&);
	static void clear(SwConstraints&);

	static Range<const PxVec3> clampTriangleCount(Range<const PxVec3>, uint32_t);

  public:
	SwFactory& mFactory;
	SwFabric& mFabric;

	bool mClothCostDirty;

	// current and previous-iteration particle positions
	Vec4fAlignedVector mCurParticles;
	Vec4fAlignedVector mPrevParticles;

	PxVec3 mParticleBoundsCenter;
	PxVec3 mParticleBoundsHalfExtent;

	PxVec3 mGravity;
	PxVec3 mLogDamping;
	PxVec3 mLinearLogDrag;
	PxVec3 mAngularLogDrag;
	PxVec3 mLinearInertia;
	PxVec3 mAngularInertia;
	PxVec3 mCentrifugalInertia;
	float mSolverFrequency;
	float mStiffnessFrequency;

	PxTransform mTargetMotion;
	PxTransform mCurrentMotion;
	PxVec3 mLinearVelocity;
	PxVec3 mAngularVelocity;

	float mPrevIterDt;
	MovingAverage mIterDtAvg;

	Vector<PhaseConfig>::Type mPhaseConfigs; // transformed!

	// tether constraints stuff
	float mTetherConstraintLogStiffness;
	float mTetherConstraintScale;

	// motion constraints stuff
	SwConstraints mMotionConstraints;
	float mMotionConstraintScale;
	float mMotionConstraintBias;
	float mMotionConstraintLogStiffness;

	// separation constraints stuff
	SwConstraints mSeparationConstraints;

	// particle acceleration stuff
	Vec4fAlignedVector mParticleAccelerations;

	// collision stuff
	Vector<IndexPair>::Type mCapsuleIndices;
	Vec4fAlignedVector mStartCollisionSpheres;
	Vec4fAlignedVector mTargetCollisionSpheres;
	Vector<uint32_t>::Type mConvexMasks;
	Vec4fAlignedVector mStartCollisionPlanes;
	Vec4fAlignedVector mTargetCollisionPlanes;
	Vector<PxVec3>::Type mStartCollisionTriangles;
	Vector<PxVec3>::Type mTargetCollisionTriangles;
	bool mEnableContinuousCollision;
	float mCollisionMassScale;
	float mFriction;

	// virtual particles
	Vector<Vec4us>::Type mVirtualParticleIndices;
	Vec4fAlignedVector mVirtualParticleWeights;
	uint32_t mNumVirtualParticles;

	// self collision
	float mSelfCollisionDistance;
	float mSelfCollisionLogStiffness;

	Vector<uint32_t>::Type mSelfCollisionIndices;

	Vec4fAlignedVector mRestPositions;

	// sleeping
	uint32_t mSleepTestInterval; // how often to test for movement
	uint32_t mSleepAfterCount;   // number of tests to pass before sleep
	float mSleepThreshold;       // max movement delta to pass test
	uint32_t mSleepPassCounter;  // how many tests passed
	uint32_t mSleepTestCounter;  // how many iterations since tested

	// unused for CPU simulation
	bool mIsAllowedHalfPrecisionSolver;

#if APEX_UE4
	void* mSimulationTask;
	static void(*const sSimulationFunction)(void*, float);
#endif

	void* mUserData;

} PX_ALIGN_SUFFIX(16);

} // namespace cloth

// bounds = lower[3], upper[3]
inline void cloth::SwCloth::setParticleBounds(const float* bounds)
{
	for(uint32_t i = 0; i < 3; ++i)
	{
		mParticleBoundsCenter[i] = (bounds[3 + i] + bounds[i]) * 0.5f;
		mParticleBoundsHalfExtent[i] = (bounds[3 + i] - bounds[i]) * 0.5f;
	}
}
}
