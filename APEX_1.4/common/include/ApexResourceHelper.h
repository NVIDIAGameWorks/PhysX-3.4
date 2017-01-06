/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __APEX_RESOURCE_HELPER_H__
#define __APEX_RESOURCE_HELPER_H__

#include "Apex.h"
#include "ResourceProviderIntl.h"

#if PX_PHYSICS_VERSION_MAJOR == 3
#include <PxFiltering.h>
#endif

namespace nvidia
{
namespace apex
{

class ApexResourceHelper
{
	ApexResourceHelper() {}
public:


#if PX_PHYSICS_VERSION_MAJOR == 3
	static PX_INLINE PxFilterData resolveCollisionGroup128(const char* collisionGroup128Name)
	{
		PxFilterData result; //default constructor sets all words to 0

		if (collisionGroup128Name)
		{
			/* create namespace for Collision Group (if it has not already been created) */
			ResourceProviderIntl* nrp = GetInternalApexSDK()->getInternalResourceProvider();
			ResID collisionGroup128NS = GetInternalApexSDK()->getCollisionGroup128NameSpace();
			ResID id = nrp->createResource(collisionGroup128NS, collisionGroup128Name);
			const uint32_t* resourcePtr = static_cast<const uint32_t*>(nrp->getResource(id));
			if (resourcePtr)
			{
				result.word0 = resourcePtr[0];
				result.word1 = resourcePtr[1];
				result.word2 = resourcePtr[2];
				result.word3 = resourcePtr[3];
			}
		}
		return result;
	}
#endif

	static PX_INLINE GroupsMask64 resolveCollisionGroup64(const char* collisionGroup64Name)
	{
		GroupsMask64 result(0, 0);

		if (collisionGroup64Name)
		{
			/* create namespace for Collision Group (if it has not already been created) */
			ResourceProviderIntl* nrp = GetInternalApexSDK()->getInternalResourceProvider();
			ResID collisionGroup64NS = GetInternalApexSDK()->getCollisionGroup64NameSpace();

			ResID id = nrp->createResource(collisionGroup64NS, collisionGroup64Name);
			const uint32_t* resourcePtr = static_cast<const uint32_t*>(nrp->getResource(id));
			if (resourcePtr)
			{
				result.bits0 = resourcePtr[0];
				result.bits1 = resourcePtr[1];
			}
		}
		return result;
	}

	static PX_INLINE uint32_t resolveCollisionGroupMask(const char* collisionGroupMaskName, uint32_t defGroupMask = 0xFFFFFFFFu)
	{
		uint32_t groupMask = defGroupMask;
		if (collisionGroupMaskName)
		{
			ResourceProviderIntl* nrp = GetInternalApexSDK()->getInternalResourceProvider();
			ResID collisionGroupMaskNS = GetInternalApexSDK()->getCollisionGroupMaskNameSpace();
			ResID id = nrp->createResource(collisionGroupMaskNS, collisionGroupMaskName);
			groupMask = (uint32_t)(size_t)(nrp->getResource(id));
		}
		return groupMask;
	}
};

}
} // end namespace nvidia::apex

#endif	// __APEX_RESOURCE_HELPER_H__
