/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef SUBSTANCE_SOURCE_ACTOR_H
#define SUBSTANCE_SOURCE_ACTOR_H

#include "Apex.h"
#include "Shape.h"

namespace nvidia
{
namespace apex
{

class SubstanceSourceAsset;


/**
 \brief Turbulence ScalarSource Actor class
 */
class SubstanceSourceActor : public Actor
{
protected:
	virtual ~SubstanceSourceActor() {}

public:
	///Returns the asset the instance has been created from.
	virtual SubstanceSourceAsset*		getSubstanceSourceAsset() const = 0;

	/** 
	\brief enable whether or not to use density in the simulation (enabling density reduces performance).
	
	If you are enabling density then you also need to add substance sources (without substance sources you will see no effect of density on the simulation, except a drop in performance)
	*/
	virtual void 						setEnabled(bool enable) = 0;

	///Returns true if substance source enabled
	virtual bool 						isEnabled() const = 0;

	///intersect the collision shape against a given AABB
	virtual bool						intersectAgainstAABB(PxBounds3) = 0;

	///Returns pointer to Shape of substance source
	virtual  Shape* 					getShape() const = 0;

	///If it is a box, cast to box class, return NULL otherwise
	virtual BoxShape* 					getBoxShape() = 0;

	///If it is a sphere, cast to sphere class, return NULL otherwise
	virtual SphereShape* 				getSphereShape() = 0;

	///Return average value of density
	virtual float						getAverageDensity() const = 0;

	///Return STD value of density
	virtual float						getStdDensity() const = 0;

	///Set average and STD values for density
	virtual void						setDensity(float averageDensity, float stdDensity) = 0;

	///Sets the uniform overall object scale
	virtual void						setCurrentScale(float scale) = 0;

	///Retrieves the uniform overall object scale
	virtual float						getCurrentScale(void) const = 0;

	virtual void						release() = 0;
};


}
} // end namespace nvidia

#endif 
