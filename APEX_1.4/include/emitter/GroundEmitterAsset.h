/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef GROUND_EMITTER_ASSET_H
#define GROUND_EMITTER_ASSET_H

#include "Apex.h"

namespace nvidia
{
namespace apex
{


PX_PUSH_PACK_DEFAULT

#define GROUND_EMITTER_AUTHORING_TYPE_NAME "GroundEmitterAsset"

///Ground emitter asset. Used to create Ground emitter actors with specific properties.
class GroundEmitterAsset : public Asset
{
protected:
	PX_INLINE GroundEmitterAsset() {}
	virtual ~GroundEmitterAsset() {}

public:

	///Gets the velocity range.	The ground emitter actor will create objects with a random velocity within the velocity range.
	virtual const PxVec3 & 			getVelocityLow() const = 0;
	
	///Gets the velocity range.	The ground emitter actor will create objects with a random velocity within the velocity range.
	virtual const PxVec3 & 			getVelocityHigh() const = 0;
	
	///Gets the lifetime range. The ground emitter actor will create objects with a random lifetime (in seconds) within the lifetime range.
	virtual const float & 			getLifetimeLow() const = 0;
	
	///Gets the lifetime range. The ground emitter actor will create objects with a random lifetime (in seconds) within the lifetime range.
	virtual const float & 			getLifetimeHigh() const = 0;

	///Gets the radius.  The ground emitter actor will create objects within a circle of size 'radius'.
	virtual float                  	getRadius() const = 0;
	
	///Gets The maximum raycasts number per frame.
	virtual uint32_t				getMaxRaycastsPerFrame() const = 0;
	
	///Gets the height from which the ground emitter will cast rays at terrain/objects opposite of the 'upDirection'.
	virtual float					getRaycastHeight() const = 0;
	
	/**
	\brief Gets the height above the ground to emit particles.
	 If greater than 0, the ground emitter will refresh a disc above the player's position rather than
	 refreshing a circle around the player's position.
	*/
	virtual float					getSpawnHeight() const = 0;
	
	/// Gets collision groups name used to cast rays
	virtual const char* 			getRaycastCollisionGroupMaskName() const = 0;
};

///Ground emitter authoring class. Used to create Ground emitter assets.
class GroundEmitterAssetAuthoring : public AssetAuthoring
{
protected:
	virtual ~GroundEmitterAssetAuthoring() {}
};


PX_POP_PACK

}
} // end namespace nvidia

#endif // GROUND_EMITTER_ASSET_H
