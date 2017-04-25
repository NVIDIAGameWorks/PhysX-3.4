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

#include "IterationState.h"
#include "SwCollision.h"
#include "SwSelfCollision.h"

namespace physx
{
namespace cloth
{

class SwCloth;
struct SwClothData;

template <typename Simd4f>
class SwSolverKernel
{
  public:
	SwSolverKernel(SwCloth const&, SwClothData&, SwKernelAllocator&, IterationStateFactory&);

	void operator()();

	// returns a conservative estimate of the
	// total memory requirements during a solve
	static size_t estimateTemporaryMemory(const SwCloth& c);

  private:
	void integrateParticles();
	void constrainTether();
	void solveFabric();
	void applyWind();
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

  private:
	SwSolverKernel<Simd4f>& operator=(const SwSolverKernel<Simd4f>&);
	template <typename AccelerationIterator>
	void integrateParticles(AccelerationIterator& accelIt, const Simd4f&);
};

#if PX_SUPPORT_EXTERN_TEMPLATE
//explicit template instantiation declaration
#if NV_SIMD_SIMD
extern template class SwSolverKernel<Simd4f>;
#endif
#if NV_SIMD_SCALAR
extern template class SwSolverKernel<Scalar4f>;
#endif
#endif

}
}
