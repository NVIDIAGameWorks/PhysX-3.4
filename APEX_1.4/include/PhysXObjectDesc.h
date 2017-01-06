/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef PHYS_XOBJECT_DESC_H
#define PHYS_XOBJECT_DESC_H

/*!
\file
\brief class PhysXObjectDesc
*/

#include "ApexUsingNamespace.h"

namespace nvidia
{
namespace apex
{

class Actor;

PX_PUSH_PACK_DEFAULT

/**
 * \brief PhysX object descriptor
 *
 * Class which describes how a PhysX object is being used by APEX.  Data access is
 * non virtual for performance reasons.
 */
class PhysXObjectDesc
{
protected:
	/**
	\brief Object interaction flags

	These flags determine how this PhysX object should interact with user callbacks.  For
	instance whether contact callbacks with the object should be ignored.
	*/
	uint32_t				mFlags;

	enum
	{
		TRANSFORM   = (1U << 31),	//!< If set, ignore this object's xform in active actor callbacks
		RAYCASTS    = (1U << 30), //!< If set, ignore this object in raycasts
		CONTACTS    = (1U << 29) //!< If set, ignore this object in contact callbacks
	};


public:
	/** \brief Returns the number of Actors associated with the PhysX object */
	virtual uint32_t				getApexActorCount() const = 0;
	/** \brief Returns the indexed Actor pointer */
	virtual const Actor*	getApexActor(uint32_t i) const = 0;

	/** \brief Returns whether this object's xform should be ignored */
	bool ignoreTransform() const
	{
		return (mFlags & (uint32_t)TRANSFORM) ? true : false;
	}
	/** \brief Returns whether this object should be ignored by raycasts */
	bool ignoreRaycasts() const
	{
		return (mFlags & (uint32_t)RAYCASTS) ? true : false;
	}
	/** \brief Returns whether this object should be ignored by contact report callbacks */
	bool ignoreContacts() const
	{
		return (mFlags & (uint32_t)CONTACTS) ? true : false;
	}
	/** \brief Returns a user defined status bit */
	bool getUserDefinedFlag(uint32_t index) const
	{
		return (mFlags & (uint32_t)(1 << index)) ? true : false;
	}

	/**
	\brief User data, for use by APEX

	For internal use by APEX.  Please do not modify this field.  You may use the PhysX object
	userData or Actor userData field.
	*/
	void* 				userData;
};

// To get owning Actor's authorable object type name:
//		getActor()->getOwner()->getAuthObjName();
// To get owning Actor's authorable object type ID:
//		getActor()->getOwner()->getAuthObjType();

PX_POP_PACK

}
} // end namespace nvidia::apex

#endif // PHYS_XOBJECT_DESC_H
