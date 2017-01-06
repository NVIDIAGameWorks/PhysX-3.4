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

#include "IterationState.h"
#include "SwCollision.h"
#include "SwSelfCollision.h"

namespace nvidia
{
namespace cloth
{

class SwCloth;
struct SwClothData;

template <typename Simd4f>
class SwSolverKernel
{
  public:
	SwSolverKernel(SwCloth const&, SwClothData&, SwKernelAllocator&, IterationStateFactory&, nvidia::profile::PxProfileZone*);

	void operator()();

	// returns a conservative estimate of the
	// total memory requirements during a solve
	static size_t estimateTemporaryMemory(const SwCloth& c);

  private:
	void integrateParticles();
	void constrainTether();
	void solveFabric();
	void constrainMotion();
	void constrainSeparation();
	void collideParticles();
	void selfCollideParticles();
	void updateSleepState();

	void iterateCloth();
	void simulateCloth();

	SwCloth const& mCloth;
	SwClothData& mClothData;
	SwKernelAllocator& mAllocator;

	SwCollision<Simd4f> mCollision;
	SwSelfCollision<Simd4f> mSelfCollision;
	IterationState<Simd4f> mState;

	profile::PxProfileZone* mProfiler;

  private:
	SwSolverKernel<Simd4f>& operator=(const SwSolverKernel<Simd4f>&);
	template <typename AccelerationIterator>
	void integrateParticles(AccelerationIterator& accelIt, const Simd4f&);
};
}
}
