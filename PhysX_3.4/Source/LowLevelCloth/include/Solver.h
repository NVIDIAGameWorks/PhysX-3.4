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

namespace physx
{

class PxBaseTask;

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
	virtual physx::PxBaseTask& simulate(float dt, physx::PxBaseTask&) = 0;

	// inter-collision parameters
	virtual void setInterCollisionDistance(float distance) = 0;
	virtual float getInterCollisionDistance() const = 0;
	virtual void setInterCollisionStiffness(float stiffness) = 0;
	virtual float getInterCollisionStiffness() const = 0;
	virtual void setInterCollisionNbIterations(uint32_t nbIterations) = 0;
	virtual uint32_t getInterCollisionNbIterations() const = 0;
	virtual void setInterCollisionFilter(InterCollisionFilter filter) = 0;

	/// returns true if an unrecoverable error has occurred
	virtual bool hasError() const = 0;
};

} // namespace cloth
} // namespace physx
