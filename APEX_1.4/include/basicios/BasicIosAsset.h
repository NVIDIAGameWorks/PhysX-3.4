/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef BASIC_IOS_ASSET_H
#define BASIC_IOS_ASSET_H

#include "Apex.h"
#include <limits.h>

namespace nvidia
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

#define BASIC_IOS_AUTHORING_TYPE_NAME "BasicIosAsset"

/**
 \brief APEX Particle System Asset
 */
class BasicIosAsset : public IosAsset
{
public:

	///Get the radius of a particle
	virtual float						getParticleRadius() const = 0;
	
	///Get the rest density of a particle
	virtual float						getRestDensity() const = 0;
	
	///Get the maximum number of particles that are allowed to be newly created on each frame
	virtual float						getMaxInjectedParticleCount() const	= 0;
	
	///Get the maximum number of particles that this IOS can simulate
	virtual uint32_t					getMaxParticleCount() const = 0;
	
	///Get the mass of a particle
	virtual float						getParticleMass() const = 0;

protected:
	virtual ~BasicIosAsset()	{}
};

/**
 \brief APEX Particle System Asset Authoring class
 */
class BasicIosAssetAuthoring : public AssetAuthoring
{
public:

	///Set the radius of a particle
	virtual void setParticleRadius(float) = 0;
	
	///Set the rest density of a particle
	virtual void setRestDensity(float) = 0;
	
	///Set the maximum number of particles that are allowed to be newly created on each frame
	virtual void setMaxInjectedParticleCount(float count) = 0;
	
	///Set the maximum number of particles that this IOS can simulate
	virtual void setMaxParticleCount(uint32_t count) = 0;
	
	///Set the mass of a particle
	virtual void setParticleMass(float) = 0;

	///Set the (NRP) name for the collision group.
	virtual void setCollisionGroupName(const char* collisionGroupName) = 0;
	
	///Set the (NRP) name for the collision group mask.
	virtual void setCollisionGroupMaskName(const char* collisionGroupMaskName) = 0;

protected:
	virtual ~BasicIosAssetAuthoring()	{}
};

PX_POP_PACK

}
} // namespace nvidia

#endif // BASIC_IOS_ASSET_H
