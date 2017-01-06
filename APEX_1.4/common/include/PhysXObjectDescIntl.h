/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef PHYSX_OBJECT_DESC_INTL_H
#define PHYSX_OBJECT_DESC_INTL_H

#include "PhysXObjectDesc.h"

namespace nvidia
{
namespace apex
{

/**
 * Module/Asset interface to actor info structure.  This allows the asset to
 * set the various flags without knowing their implementation.
 */
class PhysXObjectDescIntl : public PhysXObjectDesc
{
public:
	void setIgnoreTransform(bool b)
	{
		if (b)
		{
			mFlags |= TRANSFORM;
		}
		else
		{
			mFlags &= ~(uint32_t)TRANSFORM;
		}
	};
	void setIgnoreRaycasts(bool b)
	{
		if (b)
		{
			mFlags |= RAYCASTS;
		}
		else
		{
			mFlags &= ~(uint32_t)RAYCASTS;
		}
	};
	void setIgnoreContacts(bool b)
	{
		if (b)
		{
			mFlags |= CONTACTS;
		}
		else
		{
			mFlags &= ~(uint32_t)CONTACTS;
		}
	};
	void setUserDefinedFlag(uint32_t index, bool b)
	{
		if (b)
		{
			mFlags |= (1 << index);
		}
		else
		{
			mFlags &= ~(1 << index);
		}
	}

	/**
	\brief Implementation of pure virtual functions in PhysXObjectDesc, used for external (read-only)
	access to the Actor list
	*/
	uint32_t				getApexActorCount() const
	{
		return mApexActors.size();
	}
	const Actor*	getApexActor(uint32_t i) const
	{
		return mApexActors[i];
	}


	void swap(PhysXObjectDescIntl& rhs)
	{
		mApexActors.swap(rhs.mApexActors);
		nvidia::swap(mPhysXObject, rhs.mPhysXObject);

		nvidia::swap(userData, rhs.userData);
		nvidia::swap(mFlags, rhs.mFlags);
	}

	/**
	\brief Array of pointers to APEX actors assiciated with this PhysX object

	Pointers may be NULL in cases where the APEX actor has been deleted
	but PhysX actor cleanup has been deferred
	*/
	physx::Array<const Actor*>	mApexActors;

	/**
	\brief the PhysX object which uses this descriptor
	*/
	const void* mPhysXObject;
protected:
	virtual ~PhysXObjectDescIntl(void) {}
};

}
} // end namespace nvidia::apex

#endif // PHYSX_OBJECT_DESC_INTL_H
