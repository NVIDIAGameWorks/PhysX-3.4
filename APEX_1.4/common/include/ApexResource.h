/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef APEX_RESOURCE_H
#define APEX_RESOURCE_H

#include "ApexUsingNamespace.h"
#include "PsUserAllocated.h"

namespace physx
{
	namespace pvdsdk
	{
		class PvdDataStream;
	}
}
namespace nvidia
{
namespace apex
{

/**
 *  Class defines semi-public interface to ApexResource objects
 *	Resource - gets added to a list, will be deleted when the list is deleted
 */
class ApexResourceInterface
{
public:
	virtual void    release() = 0;
	virtual void	setListIndex(class ResourceList& list, uint32_t index) = 0;
	virtual uint32_t	getListIndex() const = 0;
	virtual void	initPvdInstances(pvdsdk::PvdDataStream& /*pvdStream*/) {};
};

/**
Class that implements resource ID and bank
*/
class ApexResource : public UserAllocated
{
public:
	ApexResource() : m_listIndex(0xFFFFFFFF), m_list(NULL) {}
	void removeSelf();
	virtual ~ApexResource();

	uint32_t			m_listIndex;
	class ResourceList*	m_list;
};


/**
Initialized Template class.
*/
template <class DescType>class InitTemplate
{
	//gotta make a derived class cause of protected ctor
public:
	InitTemplate() : isSet(false) {}

	bool isSet;
	DescType data;


	void set(const DescType* desc)
	{
		if (desc)
		{
			isSet = true;
			//memcpy(this,desc, sizeof(DescType));
			data = *desc;
		}
		else
		{
			isSet = false;
		}
	}


	bool get(DescType& dest) const
	{
		if (isSet)
		{
			//memcpy(&dest,this, sizeof(DescType));
			dest = data;
			return true;
		}
		else
		{
			return false;
		}

	}
};

} // namespace apex
} // namespace nvidia

#endif // APEX_RESOURCE_H
