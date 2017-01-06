/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef APEX_DESC_H
#define APEX_DESC_H

/*!
\file
\brief class ApexDesc
*/

#include "ApexUsingNamespace.h"

namespace nvidia
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

/**
\brief Base class for all APEX Descriptor classes

A descriptor class of type NxXDesc is typically passed to a createX() function.  Descriptors have
several advantages over simply passing a number of explicit parameters to create():

- all parameters can have default values so the user only needs to know about the ones he needs to change
- new parameters can be added without changing the user code, along with defaults
- the user and the SDK can validate the parameter's correctness using isValid()
- if creation fails, the user can look at the code of isValid() to see what exactly is not being accepted by the SDK
- some object types can save out their state into descriptors again for serialization

Care should be taken that derived descriptor classes do not initialize their base class members multiple times,
once in the constructor, and once in setToDefault()!
*/
class ApexDesc
{
public:
	/**
	\brief for standard init of user data member
	*/
	void* userData;

	/**
	\brief constructor sets to default.
	*/
	PX_INLINE ApexDesc()
	{
		setToDefault();
	}
	/**
	\brief (re)sets the structure to the default.
	*/
	PX_INLINE void setToDefault()
	{
		userData = 0;
	}
	/**
	\brief Returns true if the descriptor is valid.
	\return true if the current settings are valid.
	*/
	PX_INLINE bool isValid() const
	{
		return true;
	}
};

PX_POP_PACK

}
} // end namespace nvidia::apex

#endif // APEX_DESC_H
