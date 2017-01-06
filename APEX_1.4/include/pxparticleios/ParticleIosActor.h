/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef PARTICLE_IOS_ACTOR_H
#define PARTICLE_IOS_ACTOR_H

#include "Apex.h"

#if PX_PHYSICS_VERSION_MAJOR == 3
#include "ScopedPhysXLock.h"
#endif

namespace nvidia
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

/**
\brief ParticleIOS Actor. A simple actor that simulates a particle system.
 */
class ParticleIosActor : public Actor
{
public:
	// This actor is not publically visible

protected:
	virtual ~ParticleIosActor()	{}
};

PX_POP_PACK

}
} // namespace nvidia

#endif // PARTICLE_IOS_ACTOR_H
