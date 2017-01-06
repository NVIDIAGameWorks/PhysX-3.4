/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __DESTRUCTIBLEACTORJOINT_PROXY_H__
#define __DESTRUCTIBLEACTORJOINT_PROXY_H__

#include "Apex.h"
#include "DestructibleActorJoint.h"
#include "DestructibleActorJointImpl.h"
#include "PsUserAllocated.h"
#include "ApexRWLockable.h"
#include "ReadCheck.h"
#include "WriteCheck.h"

namespace nvidia
{
namespace destructible
{

class DestructibleScene;	// Forward declaration

class DestructibleActorJointProxy : public DestructibleActorJoint, public ApexResourceInterface, public UserAllocated, public ApexRWLockable
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	DestructibleActorJointImpl impl;

#pragma warning(disable : 4355) // disable warning about this pointer in argument list
	DestructibleActorJointProxy(const DestructibleActorJointDesc& destructibleActorJointDesc, DestructibleScene& dscene, ResourceList& list) :
		impl(destructibleActorJointDesc, dscene)
	{
		list.add(*this);
	};

	~DestructibleActorJointProxy()
	{
	};

	// DestructibleActorJoint methods
	virtual PxJoint* joint()
	{
		WRITE_ZONE();
		return impl.getJoint();
	}

	virtual void release()
	{
		// impl.release();
		delete this;
	};

	// ApexResourceInterface methods
	virtual void	setListIndex(ResourceList& list, uint32_t index)
	{
		impl.m_listIndex = index;
		impl.m_list = &list;
	}

	virtual uint32_t	getListIndex() const
	{
		return impl.m_listIndex;
	}
};

}
} // end namespace nvidia

#endif // __DESTRUCTIBLEACTORJOINT_PROXY_H__
