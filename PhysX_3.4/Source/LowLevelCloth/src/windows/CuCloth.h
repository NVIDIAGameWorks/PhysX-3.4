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

#pragma once

#include "foundation/PxTransform.h"
#include "foundation/PxVec4.h"
#include "Range.h"
#include "PhaseConfig.h"
#include "MovingAverage.h"
#include "IndexPair.h"
#include "BoundingBox.h"
#include "Vec4T.h"
#include "CuPhaseConfig.h"
#include "CuPinnedAllocator.h"
#include "CuContextLock.h"
#include "CuDeviceVector.h"

namespace physx
{
namespace cloth
{

class CuFabric;
class CuFactory;
struct CuClothData;

struct CuConstraints
{
	CuConstraints(physx::PxCudaContextManager* ctx)
	: mStart(ctx), mTarget(ctx), mHostCopy(CuHostAllocator(ctx, cudaHostAllocMapped))
	{
	}

	void pop()
	{
		if(!mTarget.empty())
		{
			mStart.swap(mTarget);
			mTarget.resize(0);
		}
	}

	CuDeviceVector<PxVec4> mStart;
	CuDeviceVector<PxVec4> mTarget;
	CuPinnedVector<PxVec4>::Type mHostCopy;
};

class CuCloth : protected CuContextLock
{
	CuCloth& operator=(const CuCloth&);

public:
	typedef CuFactory FactoryType;
	typedef CuFabric FabricType;
	typedef CuContextLock ContextLockType;

	typedef CuPinnedVector<PxVec4>::Type& MappedVec4fVectorType;
	typedef CuPinnedVector<IndexPair>::Type& MappedIndexVectorType;

	CuCloth(CuFactory&, CuFabric&, Range<const PxVec4>);
	CuCloth(CuFactory&, const CuCloth&);
	~CuCloth(); // not virtual on purpose

  public:
	bool isSleeping() const
	{
		return mSleepPassCounter >= mSleepAfterCount;
	}
	void wakeUp()
	{
		mSleepPassCounter = 0;
	}

	void notifyChanged();

	bool updateClothData(CuClothData&);   // expects acquired context
	uint32_t getSharedMemorySize() const; // without particle data

	// expects transformed configs, doesn't call notifyChanged()
	void setPhaseConfig(Range<const PhaseConfig>);

	Range<PxVec4> push(CuConstraints&);
	void clear(CuConstraints&);

	void syncDeviceParticles();
	void syncHostParticles();

	Range<const PxVec3> clampTriangleCount(Range<const PxVec3>, uint32_t);

  public:
	CuFactory& mFactory;
	CuFabric& mFabric;

	bool mClothDataDirty;

	// particle data
	uint32_t mNumParticles;
	CuDeviceVector<PxVec4> mParticles; // cur, prev
	CuPinnedVector<PxVec4>::Type mParticlesHostCopy;
	bool mDeviceParticlesDirty;
	bool mHostParticlesDirty;

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

	CuDeviceVector<CuPhaseConfig> mPhaseConfigs; // transformed!
	Vector<PhaseConfig>::Type mHostPhaseConfigs; // transformed!

	// tether constraints stuff
	float mTetherConstraintLogStiffness;
	float mTetherConstraintScale;

	// motion constraints stuff
	CuConstraints mMotionConstraints;
	float mMotionConstraintScale;
	float mMotionConstraintBias;
	float mMotionConstraintLogStiffness;

	// separation constraints stuff
	CuConstraints mSeparationConstraints;

	// particle acceleration stuff
	CuDeviceVector<PxVec4> mParticleAccelerations;
	CuPinnedVector<PxVec4>::Type mParticleAccelerationsHostCopy;

	// wind
	PxVec3 mWind;
	float mDragLogCoefficient;
	float mLiftLogCoefficient;

	// collision stuff
	CuPinnedVector<IndexPair>::Type mCapsuleIndices;
	CuPinnedVector<PxVec4>::Type mStartCollisionSpheres;
	CuPinnedVector<PxVec4>::Type mTargetCollisionSpheres;
	CuPinnedVector<uint32_t>::Type mConvexMasks;
	CuPinnedVector<PxVec4>::Type mStartCollisionPlanes;
	CuPinnedVector<PxVec4>::Type mTargetCollisionPlanes;
	CuPinnedVector<PxVec3>::Type mStartCollisionTriangles;
	CuPinnedVector<PxVec3>::Type mTargetCollisionTriangles;
	bool mEnableContinuousCollision;
	float mCollisionMassScale;
	float mFriction;

	// virtual particles
	CuDeviceVector<uint32_t> mVirtualParticleSetSizes;
	CuDeviceVector<Vec4us> mVirtualParticleIndices;
	CuDeviceVector<PxVec4> mVirtualParticleWeights;

	// self collision
	float mSelfCollisionDistance;
	float mSelfCollisionLogStiffness;

	CuDeviceVector<PxVec4> mRestPositions;
	CuDeviceVector<uint32_t> mSelfCollisionIndices;
	Vector<uint32_t>::Type mSelfCollisionIndicesHost;

	// 4 (position) + 2 (key) per particle + cellStart (8322)
	CuDeviceVector<float> mSelfCollisionData;

	// sleeping (see SwCloth for comments)
	uint32_t mSleepTestInterval;
	uint32_t mSleepAfterCount;
	float mSleepThreshold;
	uint32_t mSleepPassCounter;
	uint32_t mSleepTestCounter;

	uint32_t mSharedMemorySize;

	void* mUserData;
};
}
}
