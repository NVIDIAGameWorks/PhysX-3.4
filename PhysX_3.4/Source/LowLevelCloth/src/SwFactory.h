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

#include "Factory.h"
#include "Allocator.h"

namespace physx
{

namespace cloth
{

class SwFabric;
class SwCloth;
template <typename>
class ClothImpl;

class SwFactory : public UserAllocated, public Factory
{
  public:
	typedef SwFabric FabricType;
	typedef ClothImpl<SwCloth> ImplType;

	SwFactory();
	virtual ~SwFactory();

	virtual Fabric* createFabric(uint32_t numParticles, Range<const uint32_t> phases, Range<const uint32_t> sets,
	                             Range<const float> restvalues, Range<const uint32_t> indices,
	                             Range<const uint32_t> anchors, Range<const float> tetherLengths,
	                             Range<const uint32_t> triangles);

	virtual Cloth* createCloth(Range<const PxVec4> particles, Fabric& fabric);

	virtual Solver* createSolver(physx::PxTaskManager*);

	virtual Cloth* clone(const Cloth& cloth);

	virtual void extractFabricData(const Fabric& fabric, Range<uint32_t> phases, Range<uint32_t> sets,
	                               Range<float> restvalues, Range<uint32_t> indices, Range<uint32_t> anchors,
	                               Range<float> tetherLengths, Range<uint32_t> triangles) const;

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
	Vector<SwFabric*>::Type mFabrics;
};
}
}
