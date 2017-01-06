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

#include "Types.h"
#ifndef __CUDACC__
#include "Simd.h"
#endif

namespace physx
{
namespace cloth
{

class CuCloth;
struct CuPhaseConfig;
template <typename>
struct IterationState;
struct IndexPair;
struct CuIterationData;
struct CuTether;

// reference to cloth instance bulk data (POD)
// should not need frequent updates (stored on device)
struct CuClothData
{
	CuClothData()
	{
	}
	CuClothData(CuCloth&);

	// particle data
	uint32_t mNumParticles;
	float* mParticles;
	float* mParticlesHostCopy;

	// fabric constraints
	uint32_t mNumPhases;
	const CuPhaseConfig* mPhaseConfigs;

	const CuTether* mTethers;
	uint32_t mNumTethers;
	float mTetherConstraintScale;

	const uint16_t* mTriangles;
	uint32_t mNumTriangles;

	// motion constraint data
	float mMotionConstraintScale;
	float mMotionConstraintBias;

	// collision data
	uint32_t mNumSpheres;  // don't change this order, it's
	uint32_t mNumCapsules; // needed by mergeAcceleration()
	const IndexPair* mCapsuleIndices;
	uint32_t mNumPlanes;
	uint32_t mNumConvexes;
	const uint32_t* mConvexMasks;
	uint32_t mNumCollisionTriangles;

	// virtual particle data
	const uint32_t* mVirtualParticleSetSizesBegin;
	const uint32_t* mVirtualParticleSetSizesEnd;
	const uint16_t* mVirtualParticleIndices;
	const float* mVirtualParticleWeights;

	bool mEnableContinuousCollision;
	float mCollisionMassScale;
	float mFrictionScale;

	float mSelfCollisionDistance;
	uint32_t mNumSelfCollisionIndices;
	const uint32_t* mSelfCollisionIndices;
	float* mSelfCollisionParticles;
	uint32_t* mSelfCollisionKeys;
	uint16_t* mSelfCollisionCellStart;

	// sleep data
	uint32_t mSleepTestInterval;
	uint32_t mSleepAfterCount;
	float mSleepThreshold;
};

// per-frame data (stored in pinned memory)
struct CuFrameData
{
	CuFrameData()
	{
	} // not initializing pointers to 0!

#ifndef __CUDACC__
	explicit CuFrameData(CuCloth&, uint32_t, const IterationState<Simd4f>&, const CuIterationData*);
#endif

	bool mDeviceParticlesDirty;

	// number of particle copies that fit in shared memory (0, 1, or 2)
	uint32_t mNumSharedPositions;

	// iteration data
	float mIterDt;
	uint32_t mNumIterations;
	const CuIterationData* mIterationData;

	float mTetherConstraintStiffness;

	// wind data
	float mDragCoefficient;
	float mLiftCoefficient;
	float mRotation[9];

	// motion constraint data
	const float* mStartMotionConstraints;
	float* mTargetMotionConstraints;
	const float* mHostMotionConstraints;
	float mMotionConstraintStiffness;

	// separation constraint data
	const float* mStartSeparationConstraints;
	float* mTargetSeparationConstraints;
	const float* mHostSeparationConstraints;

	// particle acceleration data
	float* mParticleAccelerations;
	const float* mHostParticleAccelerations;

	// rest positions
	const float* mRestPositions;

	// collision data
	const float* mStartCollisionSpheres;
	const float* mTargetCollisionSpheres;
	const float* mStartCollisionPlanes;
	const float* mTargetCollisionPlanes;
	const float* mStartCollisionTriangles;
	const float* mTargetCollisionTriangles;

	float mSelfCollisionStiffness;

	float mParticleBounds[6]; // maxX, -minX, maxY, ...

	uint32_t mSleepPassCounter;
	uint32_t mSleepTestCounter;

	float mStiffnessExponent;
};

// per-iteration data (stored in pinned memory)
struct CuIterationData
{
	CuIterationData()
	{
	} // not initializing!

#ifndef __CUDACC__
	explicit CuIterationData(const IterationState<Simd4f>&);
#endif

	float mIntegrationTrafo[24];
	float mWind[3];
	uint32_t mIsTurning;
};
}
}
