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

#include "Factory.h"
#include "Allocator.h"

namespace physx
{
	class PxCudaContextManager;
}

namespace nvidia
{
namespace cloth
{

class CuFabric;
class CuCloth;
template <typename>
class ClothImpl;

class CuFactory : public UserAllocated, public Factory
{
  protected:
	CuFactory& operator=(const CuFactory&);

  public:
	typedef CuFabric FabricType;
	typedef ClothImpl<CuCloth> ImplType;

	CuFactory(PxCudaContextManager*);
	virtual ~CuFactory();

	virtual Fabric* createFabric(uint32_t numParticles, Range<const uint32_t> phases, Range<const uint32_t> sets,
	                             Range<const float> restvalues, Range<const uint32_t> indices,
	                             Range<const uint32_t> anchors, Range<const float> tetherLengths);

	virtual Cloth* createCloth(Range<const PxVec4> particles, Fabric& fabric);

	virtual Solver* createSolver(profile::PxProfileZone* profiler, PxTaskManager* taskMgr);

	virtual Cloth* clone(const Cloth& cloth);

	virtual void extractFabricData(const Fabric& fabric, Range<uint32_t> phases, Range<uint32_t> sets,
	                               Range<float> restvalues, Range<uint32_t> indices, Range<uint32_t> anchors,
	                               Range<float> tetherLengths) const;

	virtual void extractCollisionData(const Cloth& cloth, Range<PxVec4> spheres, Range<uint32_t> capsules,
	                                  Range<PxVec4> planes, Range<uint32_t> convexes, Range<PxVec3> triangles) const;

	virtual void extractMotionConstraints(const Cloth& cloth, Range<PxVec4> destConstraints) const;

	virtual void extractSeparationConstraints(const Cloth& cloth, Range<PxVec4> destConstraints) const;

	virtual void extractParticleAccelerations(const Cloth& cloth, Range<PxVec4> destAccelerations) const;

	virtual void extractVirtualParticles(const Cloth& cloth, Range<uint32_t[4]> destIndices,
	                                     Range<PxVec3> destWeights) const;

	virtual void extractSelfCollisionIndices(const Cloth& cloth, Range<uint32_t> destIndices) const;

	virtual void extractRestPositions(const Cloth& cloth, Range<PxVec4> destRestPositions) const;

  public:
	void copyToHost(const void* srcIt, const void* srcEnd, void* dstIt) const;

  public:
	Vector<CuFabric*>::Type mFabrics;

	PxCudaContextManager* mContextManager;

	uint32_t mNumThreadsPerBlock;

	const uint32_t mMaxThreadsPerBlock;
};
}
}
