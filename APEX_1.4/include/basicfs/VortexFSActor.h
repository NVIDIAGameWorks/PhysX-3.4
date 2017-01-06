/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef VORTEX_FSACTOR_H
#define VORTEX_FSACTOR_H

#include "Apex.h"

namespace nvidia
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

class BasicFSAsset;


/**
 \brief VortexFS Actor class
 */
class VortexFSActor : public Actor, public Renderable
{
protected:
	virtual ~VortexFSActor() {}

public:

	/**
	\brief Returns the asset the instance has been created from.
	*/
	virtual BasicFSAsset* 		getVortexFSAsset() const = 0;

	/**
	\brief Gets the current pose of the actor
	*/
	virtual PxMat44				getCurrentPose() const = 0;
	
	/**
	\brief Sets the current pose of the actor
	*/
	virtual void				setCurrentPose(const PxTransform& pose) = 0;
	
	/**
	\brief Gets the current position of the actor
	*/
	virtual PxVec3				getCurrentPosition() const = 0;
	
	/**
	\brief Sets the current position of the actor
	*/
	virtual void				setCurrentPosition(const PxVec3& pos) = 0;

	/**
	\brief Sets the axis of the capsule in local coordinate system
	*/
	virtual void				setAxis(const PxVec3&) = 0;
	
	/**
	\brief Sets the height of the capsule
	*/
	virtual void				setHeight(float) = 0;
	
	/**
	\brief Sets the bottom radius of the capsule
	*/
	virtual void				setBottomRadius(float) = 0;
	
	/**
	\brief Sets the top radius of the capsule
	*/
	virtual void				setTopRadius(float) = 0;

	/**
	\brief Sets the bottom spherical force of the capsule
	*/
	virtual void				setBottomSphericalForce(bool) = 0;
	
	/**
	\brief Sets the top spherical force of the capsule
	*/
	virtual void				setTopSphericalForce(bool) = 0;

	/**
	\brief Sets strength of the rotational part of vortex field
	*/
	virtual void				setRotationalStrength(float) = 0;
	
	/**
	\brief Sets strength of the radial part of vortex field
	*/
	virtual void				setRadialStrength(float) = 0;
	
	/**
	\brief Sets strength of the lifting part of vortex field
	*/
	virtual void				setLiftStrength(float) = 0;

	/**
	\brief Enable/Disable the field simulation
	*/
	virtual void				setEnabled(bool isEnabled) = 0;
	

	/**
	\brief Sets the uniform overall object scale
	*/
	virtual void				setCurrentScale(float scale) = 0;

	/**
	\brief Retrieves the uniform overall object scale
	*/
	virtual float				getCurrentScale(void) const = 0;

};

PX_POP_PACK

}
} // end namespace nvidia::apex

#endif // VORTEX_FSACTOR_H
