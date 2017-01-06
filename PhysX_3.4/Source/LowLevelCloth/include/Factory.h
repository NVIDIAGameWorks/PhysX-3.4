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
#include "Range.h"

typedef struct CUstream_st* CUstream;

namespace physx
{

class PxTaskManager;

namespace profile
{
class PxProfileZone;
}

class PxCudaContextManager;
}

namespace physx
{

namespace cloth
{

class Fabric;
class Cloth;
class Solver;
class Character;

/// abstract factory to create context-specific simulation components
/// such as cloth, solver, collision, etc.
class Factory
{
  public:
	enum Platform
	{
		CPU,
		CUDA
	};

  protected:
	Factory(Platform platform) : mPlatform(platform)
	{
	}
	Factory(const Factory&);
	Factory& operator=(const Factory&);

  public:
	static Factory* createFactory(Platform, void* = 0);

	virtual ~Factory()
	{
	}

	Platform getPlatform() const
	{
		return mPlatform;
	}

	/**
	    Create fabric data used to setup cloth object.
	    @param numParticles number of particles, must be larger than any particle index
	    @param phases map from phase to set index
	    @param sets inclusive prefix sum of restvalue count per set
	    @param restvalues array of constraint rest values
	    @param indices array of particle index pair per constraint
	 */
	virtual Fabric* createFabric(uint32_t numParticles, Range<const uint32_t> phases, Range<const uint32_t> sets,
	                             Range<const float> restvalues, Range<const uint32_t> indices,
	                             Range<const uint32_t> anchors, Range<const float> tetherLengths,
	                             Range<const uint32_t> triangles) = 0;

	/**
	    Create cloth object.
	    @param particles initial particle positions.
	    @param fabric edge distance constraint structure
	 */
	virtual Cloth* createCloth(Range<const PxVec4> particles, Fabric& fabric) = 0;

	/**
	    Create cloth solver object.
	    @param taskMgr PxTaskManager used for simulation.
	 */
	virtual Solver* createSolver(PxTaskManager* taskMgr) = 0;

	/**
	    Create a copy of a cloth instance
	    @param cloth the instance to be cloned, need not match the factory type
	 */
	virtual Cloth* clone(const Cloth& cloth) = 0;

	/**
	    Extract original data from a fabric object
	    @param fabric to extract from, must match factory type
	    @param phases pre-allocated memory range to write phases
	    @param sets pre-allocated memory range to write sets
	    @param restvalues pre-allocated memory range to write restvalues
	    @param indices pre-allocated memory range to write indices
	 */
	virtual void extractFabricData(const Fabric& fabric, Range<uint32_t> phases, Range<uint32_t> sets,
	                               Range<float> restvalues, Range<uint32_t> indices, Range<uint32_t> anchors,
	                               Range<float> tetherLengths, Range<uint32_t> triangles) const = 0;

	/**
	    Extract current collision spheres and capsules from a cloth object
	    @param cloth the instance to extract from, must match factory type
	    @param spheres pre-allocated memory range to write spheres
	    @param capsules pre-allocated memory range to write capsules
	    @param planes pre-allocated memory range to write planes
	    @param convexes pre-allocated memory range to write convexes
	    @param triangles pre-allocated memory range to write triangles
	 */
	virtual void extractCollisionData(const Cloth& cloth, Range<PxVec4> spheres, Range<uint32_t> capsules,
	                                  Range<PxVec4> planes, Range<uint32_t> convexes, Range<PxVec3> triangles) const = 0;

	/**
	    Extract current motion constraints from a cloth object
	    @param cloth the instance to extract from, must match factory type
	    @param destConstraints pre-allocated memory range to write constraints
	 */
	virtual void extractMotionConstraints(const Cloth& cloth, Range<PxVec4> destConstraints) const = 0;

	/**
	    Extract current separation constraints from a cloth object
	    @param cloth the instance to extract from, must match factory type
	    @param destConstraints pre-allocated memory range to write constraints
	 */
	virtual void extractSeparationConstraints(const Cloth& cloth, Range<PxVec4> destConstraints) const = 0;

	/**
	    Extract current particle accelerations from a cloth object
	    @param cloth the instance to extract from, must match factory type
	    @param destAccelerations pre-allocated memory range to write accelerations
	 */
	virtual void extractParticleAccelerations(const Cloth& cloth, Range<PxVec4> destAccelerations) const = 0;

	/**
	    Extract virtual particles from a cloth object
	    @param cloth the instance to extract from, must match factory type
	    @param destIndices pre-allocated memory range to write indices
	    @param destWeights pre-allocated memory range to write weights
	 */
	virtual void extractVirtualParticles(const Cloth& cloth, Range<uint32_t[4]> destIndices,
	                                     Range<PxVec3> destWeights) const = 0;

	/**
	    Extract self collision indices from cloth object.
	    @param cloth the instance to extract from, must match factory type
	    @param destIndices pre-allocated memory range to write indices
	*/
	virtual void extractSelfCollisionIndices(const Cloth& cloth, Range<uint32_t> destIndices) const = 0;

	/**
	    Extract particle rest positions from cloth object.
	    @param cloth the instance to extract from, must match factory type
	    @param destRestPositions pre-allocated memory range to write rest positions
	*/
	virtual void extractRestPositions(const Cloth& cloth, Range<PxVec4> destRestPositions) const = 0;

  protected:
	const Platform mPlatform;
};

} // namespace cloth
} // namespace physx
