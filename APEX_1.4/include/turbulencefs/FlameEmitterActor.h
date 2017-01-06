/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef FLAME_EMITTER_ACTOR_H
#define FLAME_EMITTER_ACTOR_H

#include "Apex.h"
#include "Shape.h"

namespace nvidia
{
namespace apex
{

class FlameEmitterAsset;


/**
 \brief Flame Emitter Actor class
 */
class FlameEmitterActor : public Actor
{
protected:
	virtual ~FlameEmitterActor() {}

public:
	///Returns the asset the instance has been created from.
	virtual FlameEmitterAsset*			getFlameEmitterAsset() const = 0;


	///enable/disable flame emitter
	virtual void 						setEnabled(bool enable) = 0;

	///Returns true if flame emitter enabled
	virtual bool 						isEnabled() const = 0;

	///intersect the collision shape against a given AABB
	virtual bool						intersectAgainstAABB(const PxBounds3&) const = 0;

	///get the global pose
	virtual PxMat44						getPose() const = 0;

	///set the global pose
	virtual void						setPose(PxMat44 pose) = 0;

	///Sets the uniform overall object scale
	virtual void						setCurrentScale(float scale) = 0;

	///Retrieves the uniform overall object scale
	virtual float						getCurrentScale(void) const = 0;

	virtual void						release() = 0;
};


}
} // end namespace nvidia

#endif 
