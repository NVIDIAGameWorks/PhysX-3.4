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

#include "Px.h"
#include "Types.h"

namespace nvidia
{
namespace cloth
{

class SwCloth;
class SwFabric;
struct PhaseConfig;
struct IndexPair;
struct SwTether;

// reference to cloth instance bulk data (POD)
struct SwClothData
{
	SwClothData(SwCloth&, const SwFabric&);
	void reconcile(SwCloth&) const;
	void verify() const;

	// particle data
	uint32_t mNumParticles;
	float* mCurParticles;
	float* mPrevParticles;

	float mCurBounds[6]; // lower[3], upper[3]
	float mPrevBounds[6];
	float mPadding; // write as simd

	// distance constraints
	const PhaseConfig* mConfigBegin;
	const PhaseConfig* mConfigEnd;

	const uint32_t* mPhases;
	uint32_t mNumPhases;

	const uint32_t* mSets;
	uint32_t mNumSets;

	const float* mRestvalues;
	uint32_t mNumRestvalues;

	const uint16_t* mIndices;
	uint32_t mNumIndices;

	const SwTether* mTethers;
	uint32_t mNumTethers;
	float mTetherConstraintStiffness;
	float mTetherConstraintScale;

	// motion constraint data
	const float* mStartMotionConstraints;
	const float* mTargetMotionConstraints;
	float mMotionConstraintStiffness;

	// separation constraint data
	const float* mStartSeparationConstraints;
	const float* mTargetSeparationConstraints;

	// particle acceleration data
	const float* mParticleAccelerations;

	// collision stuff
	const float* mStartCollisionSpheres;
	const float* mTargetCollisionSpheres;
	uint32_t mNumSpheres;

	const IndexPair* mCapsuleIndices;
	uint32_t mNumCapsules;

	const float* mStartCollisionPlanes;
	const float* mTargetCollisionPlanes;
	uint32_t mNumPlanes;

	const uint32_t* mConvexMasks;
	uint32_t mNumConvexes;

	const float* mStartCollisionTriangles;
	const float* mTargetCollisionTriangles;
	uint32_t mNumTriangles;

	const uint16_t* mVirtualParticlesBegin;
	const uint16_t* mVirtualParticlesEnd;

	const float* mVirtualParticleWeights;
	uint32_t mNumVirtualParticleWeights;

	bool mEnableContinuousCollision;
	float mFrictionScale;
	float mCollisionMassScale;

	float mSelfCollisionDistance;
	float mSelfCollisionStiffness;

	uint32_t mNumSelfCollisionIndices;
	const uint32_t* mSelfCollisionIndices;

	float* mRestPositions;

	// sleep data
	uint32_t mSleepPassCounter;
	uint32_t mSleepTestCounter;

} PX_ALIGN_SUFFIX(16);
}
}
