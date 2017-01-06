/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef ATTRACTOR_FSACTOR_H
#define ATTRACTOR_FSACTOR_H

#include "Apex.h"

namespace nvidia
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

class BasicFSAsset;


/**
 \brief AttractorFS Actor class
 */
class AttractorFSActor : public Actor, public Renderable
{
protected:
	virtual ~AttractorFSActor() {}

public:
	/**
	\brief Returns the asset the instance has been created from.
	*/
	virtual BasicFSAsset* 		getAttractorFSAsset() const = 0;

	/**
	\brief Gets the current position of the actor
	*/
	virtual PxVec3				getCurrentPosition() const = 0;
	
	/**
	\brief Sets the current position of the actor
	*/
	virtual void				setCurrentPosition(const PxVec3& pos) = 0;

	/**
	\brief Sets the attracting radius of the field
	*/
	virtual void				setFieldRadius(float) = 0;

	/**
	\brief Sets strength of the constant part of attracting field
	*/
	virtual void				setConstFieldStrength(float) = 0;

	/**
	\brief Sets strength coefficient for variable part of attracting field
	*/
	virtual void				setVariableFieldStrength(float) = 0;

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

#endif // ATTRACTOR_FSACTOR_H
