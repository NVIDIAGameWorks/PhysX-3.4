/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef BASIC_IOS_ACTOR_H
#define BASIC_IOS_ACTOR_H

#include "Apex.h"

namespace nvidia
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

/**
\brief BasicIOS Actor. A simple actor that simulates a particle system.
 */
class BasicIosActor : public Actor
{
public:

	/// Get the particle radius
	virtual float						getParticleRadius() const = 0;
	
	/// Get the particle rest density
	virtual float						getRestDensity() const = 0;

	/// Get the current number of particles
	virtual uint32_t					getParticleCount() const = 0;

protected:

	virtual ~BasicIosActor()	{}
};

PX_POP_PACK

}
} // namespace nvidia

#endif // BASIC_IOS_ACTOR_H
