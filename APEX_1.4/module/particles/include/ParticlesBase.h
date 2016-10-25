/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef PARTICLES_BASE_H

#define PARTICLES_BASE_H

#include "ApexActor.h"

namespace nvidia
{
namespace particles
{

class ParticlesBase : public ApexActor
{
public:
	enum ParticlesType
	{
		DST_EFFECT_PACKAGE_ACTOR,
		DST_LAST
	};

	virtual ParticlesType getParticlesType() const = 0;

};

}
}

#endif
