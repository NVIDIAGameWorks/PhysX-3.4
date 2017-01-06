/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef FORCE_FIELD_ACTOR_H
#define FORCE_FIELD_ACTOR_H

#include "Apex.h"


namespace nvidia
{
namespace apex
{


PX_PUSH_PACK_DEFAULT

class ForceFieldAsset;
class ForceFieldAssetParams;

/**
 \brief ForceField Actor
 */
class ForceFieldActor : public Actor
{
protected:
	virtual ~ForceFieldActor() {}

public:
	/**
	Return true if the force field actor is enabled.
	*/
	virtual bool					isEnable() = 0;

	/**
	Disable force field actor. Default status is enable. Can switch it freely.
	A disabled explosion actor still exists there, but has no effect to the scene.
	*/
	virtual bool					disable() = 0;

	/**
	Enable force field actor. Default status is enable. Can switch it freely.
	A disabled explosion actor still exists there, but has no effect to the scene.
	*/
	virtual bool					enable() = 0;

	/**
	Gets location and orientation of the force field.
	*/
	virtual PxMat44					getPose() const = 0;

	/**
	Sets location and orientation of the force field.
	*/
	virtual void					setPose(const PxMat44& pose) = 0;

	/**
	Gets the force field actor's scale.
	*/
	PX_DEPRECATED virtual float		getScale() const = 0;

	/**
	Sets the force field actor's scale. (0.0f, +inf)
	*/
	PX_DEPRECATED virtual void		setScale(float scale) = 0;

	/**
	Gets the force field actor's scale.
	*/
	PX_DEPRECATED virtual float		getCurrentScale() const = 0;

	/**
	Sets the force field actor's scale. (0.0f, +inf)
	*/
	PX_DEPRECATED virtual void		setCurrentScale(float scale) = 0;



	/**
	Retrieves the name string for the force field actor.
	*/
	virtual const char*				getName() const = 0;

	/**
	Set a name string for the force field actor that can be retrieved with getName().
	*/
	virtual void					setName(const char* name) = 0;

	/**
	Set strength for the force field actor.
	*/
	virtual void					setStrength(const float strength) = 0;

	/**
	Set lifetime for the force field actor.
	*/
	virtual void					setLifetime(const float lifetime) = 0;

	/**
	Set falloff type (linear, steep, scurve, custom, none) for the force field actor.
	Only works for radial force field types.
	*/
	PX_DEPRECATED virtual void		setFalloffType(const char* type) = 0;

	/**
	Set falloff multiplier for the force field actor.
	Only works for radial force field types.
	*/
	PX_DEPRECATED virtual void		setFalloffMultiplier(const float multiplier) = 0;

	/**
	Returns the asset the actor has been created from.
	*/
	virtual ForceFieldAsset* 	    getForceFieldAsset() const = 0;

};

PX_POP_PACK

} // namespace apex
} // namespace nvidia

#endif // FORCE_FIELD_ACTOR_H
