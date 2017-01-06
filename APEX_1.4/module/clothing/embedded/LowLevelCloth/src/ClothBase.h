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

#include "PsMathUtils.h"

namespace nvidia
{
namespace cloth
{

/* helper functions shared between SwCloth and CuCloth */

template <typename Cloth>
void initialize(Cloth& cloth, const PxVec4* pIt, const PxVec4* pEnd)
{
	// initialize particles bounding box
	PxVec4 lower(FLT_MAX), upper = -lower;
	for(; pIt != pEnd; ++pIt)
	{
		lower = lower.minimum(*pIt);
		upper = upper.maximum(*pIt);
	}
	PxVec4 center = (upper + lower) * 0.5f;
	PxVec4 extent = (upper - lower) * 0.5f;
	cloth.mParticleBoundsCenter = reinterpret_cast<const PxVec3&>(center);
	cloth.mParticleBoundsHalfExtent = reinterpret_cast<const PxVec3&>(extent);

	cloth.mGravity = PxVec3(0.0f);
	cloth.mLogDamping = PxVec3(0.0f);
	cloth.mLinearLogDrag = PxVec3(0.0f);
	cloth.mAngularLogDrag = PxVec3(0.0f);
	cloth.mLinearInertia = PxVec3(1.0f);
	cloth.mAngularInertia = PxVec3(1.0f);
	cloth.mCentrifugalInertia = PxVec3(1.0f);
	cloth.mSolverFrequency = 60.0f;
	cloth.mStiffnessFrequency = 10.0f;
	cloth.mTargetMotion = PxTransform(PxIdentity);
	cloth.mCurrentMotion = PxTransform(PxIdentity);
	cloth.mLinearVelocity = PxVec3(0.0f);
	cloth.mAngularVelocity = PxVec3(0.0f);
	cloth.mPrevIterDt = 0.0f;
	cloth.mIterDtAvg = MovingAverage(30);
	cloth.mTetherConstraintLogStiffness = float(-FLT_MAX_EXP);
	cloth.mTetherConstraintScale = 1.0f;
	cloth.mMotionConstraintScale = 1.0f;
	cloth.mMotionConstraintBias = 0.0f;
	cloth.mMotionConstraintLogStiffness = float(-FLT_MAX_EXP);
	cloth.mEnableContinuousCollision = false;
	cloth.mCollisionMassScale = 0.0f;
	cloth.mFriction = 0.0f;
	cloth.mSelfCollisionDistance = 0.0f;
	cloth.mSelfCollisionLogStiffness = float(-FLT_MAX_EXP);
	cloth.mSleepTestInterval = uint32_t(-1);
	cloth.mSleepAfterCount = uint32_t(-1);
	cloth.mSleepThreshold = 0.0f;
	cloth.mSleepPassCounter = 0;
	cloth.mSleepTestCounter = 0;
}

template <typename DstCloth, typename SrcCloth>
void copy(DstCloth& dstCloth, const SrcCloth& srcCloth)
{
	dstCloth.mParticleBoundsCenter = srcCloth.mParticleBoundsCenter;
	dstCloth.mParticleBoundsHalfExtent = srcCloth.mParticleBoundsHalfExtent;
	dstCloth.mGravity = srcCloth.mGravity;
	dstCloth.mLogDamping = srcCloth.mLogDamping;
	dstCloth.mLinearLogDrag = srcCloth.mLinearLogDrag;
	dstCloth.mAngularLogDrag = srcCloth.mAngularLogDrag;
	dstCloth.mLinearInertia = srcCloth.mLinearInertia;
	dstCloth.mAngularInertia = srcCloth.mAngularInertia;
	dstCloth.mCentrifugalInertia = srcCloth.mCentrifugalInertia;
	dstCloth.mSolverFrequency = srcCloth.mSolverFrequency;
	dstCloth.mStiffnessFrequency = srcCloth.mStiffnessFrequency;
	dstCloth.mTargetMotion = srcCloth.mTargetMotion;
	dstCloth.mCurrentMotion = srcCloth.mCurrentMotion;
	dstCloth.mLinearVelocity = srcCloth.mLinearVelocity;
	dstCloth.mAngularVelocity = srcCloth.mAngularVelocity;
	dstCloth.mPrevIterDt = srcCloth.mPrevIterDt;
	dstCloth.mIterDtAvg = srcCloth.mIterDtAvg;
	dstCloth.mTetherConstraintLogStiffness = srcCloth.mTetherConstraintLogStiffness;
	dstCloth.mTetherConstraintScale = srcCloth.mTetherConstraintScale;
	dstCloth.mMotionConstraintScale = srcCloth.mMotionConstraintScale;
	dstCloth.mMotionConstraintBias = srcCloth.mMotionConstraintBias;
	dstCloth.mMotionConstraintLogStiffness = srcCloth.mMotionConstraintLogStiffness;
	dstCloth.mEnableContinuousCollision = srcCloth.mEnableContinuousCollision;
	dstCloth.mCollisionMassScale = srcCloth.mCollisionMassScale;
	dstCloth.mFriction = srcCloth.mFriction;
	dstCloth.mSelfCollisionDistance = srcCloth.mSelfCollisionDistance;
	dstCloth.mSelfCollisionLogStiffness = srcCloth.mSelfCollisionLogStiffness;
	dstCloth.mSleepTestInterval = srcCloth.mSleepTestInterval;
	dstCloth.mSleepAfterCount = srcCloth.mSleepAfterCount;
	dstCloth.mSleepThreshold = srcCloth.mSleepThreshold;
	dstCloth.mSleepPassCounter = srcCloth.mSleepPassCounter;
	dstCloth.mSleepTestCounter = srcCloth.mSleepTestCounter;
	dstCloth.mIsAllowedHalfPrecisionSolver = srcCloth.mIsAllowedHalfPrecisionSolver;
	dstCloth.mUserData = srcCloth.mUserData;
}

} // namespace cloth
} // namespace nvidia
