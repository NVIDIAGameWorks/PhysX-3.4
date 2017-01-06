/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODULE_PARTICLE_IOS_H
#define MODULE_PARTICLE_IOS_H

#include "Apex.h"
#include <limits.h>

namespace nvidia
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

class ParticleIosAsset;
class ParticleIosAssetAuthoring;
class CudaTestManager;
class Scene;

/**
\brief ParticleIOS Module - Manages PhysX 3.0 PxParticleSystem and PxParticleFluid simulations
*/
class ModuleParticleIos : public Module
{
protected:
	virtual											~ModuleParticleIos() {}

public:
	/// Get ParticleIOS authoring type name
	virtual const char*								getParticleIosTypeName() = 0;
};


PX_POP_PACK

}
} // namespace nvidia

#endif // MODULE_PARTICLE_IOS_H
