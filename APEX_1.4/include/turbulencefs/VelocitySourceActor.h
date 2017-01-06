/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef VELOCITY_SOURCE_ACTOR_H
#define VELOCITY_SOURCE_ACTOR_H

#include "Apex.h"
#include "Shape.h"

namespace nvidia
{
namespace apex
{

class VelocitySourceAsset;


/**
 \brief Turbulence VelocitySource Actor class
 */
class VelocitySourceActor : public Actor
{
protected:
	virtual ~VelocitySourceActor() {}

public:
	 ///Returns the asset the instance has been created from.
	virtual VelocitySourceAsset*		getVelocitySourceAsset() const = 0;

	///enable/disable the velocity source
	virtual void 						setEnabled(bool enable) = 0;

	///Returns true if velocity source enabled
	virtual bool 						isEnabled() const = 0;

	///intersect the collision shape against a given AABB
	virtual bool						intersectAgainstAABB(PxBounds3) = 0;

	///Returns pointer to Shape of velocity source
	virtual  Shape* 					getShape() const = 0;

	///If it is a box, cast to box class, return NULL otherwise
	virtual BoxShape* 					getBoxShape() = 0;

	///If it is a sphere, cast to sphere class, return NULL otherwise
	virtual SphereShape* 				getSphereShape() = 0;

	///Return average value of velocity
	virtual float						getAverageVelocity() const = 0;

	///Return STD value of velocity
	virtual float						getStdVelocity() const = 0;

	///get the pose of a velocity source shape
	virtual PxMat44						getPose() const = 0;

	///Set average and STD values for velocity
	virtual void						setVelocity(float averageVelocity, float stdVelocity) = 0;

	///set the pose of a velocity source shape
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
