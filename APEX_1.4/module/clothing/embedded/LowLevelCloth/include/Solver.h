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

#include "Types.h"

namespace physx
{
	class PxBaseTask;
}

namespace nvidia
{
namespace cloth
{

class Cloth;

// called during inter-collision, user0 and user1 are the user data from each cloth
typedef bool (*InterCollisionFilter)(void* user0, void* user1);

/// base class for solvers
class Solver
{
  protected:
	Solver(const Solver&);
	Solver& operator=(const Solver&);

  protected:
	Solver()
	{
	}

  public:
	virtual ~Solver()
	{
	}

	/// add cloth object, returns true if successful
	virtual void addCloth(Cloth*) = 0;

	/// remove cloth object
	virtual void removeCloth(Cloth*) = 0;

	/// simulate one time step
	virtual PxBaseTask& simulate(float dt, PxBaseTask&) = 0;

	// inter-collision parameters
	virtual void setInterCollisionDistance(float distance) = 0;
	virtual float getInterCollisionDistance() const = 0;
	virtual void setInterCollisionStiffness(float stiffness) = 0;
	virtual float getInterCollisionStiffness() const = 0;
	virtual void setInterCollisionNbIterations(uint32_t nbIterations) = 0;
	virtual uint32_t getInterCollisionNbIterations() const = 0;
	virtual void setInterCollisionFilter(InterCollisionFilter filter) = 0;

//	virtual uint32_t getNumSharedPositions( const Cloth* ) const = 0;

	/// returns true if an unrecoverable error has occurred
	virtual bool hasError() const = 0;
};

} // namespace cloth
} // namespace nvidia
