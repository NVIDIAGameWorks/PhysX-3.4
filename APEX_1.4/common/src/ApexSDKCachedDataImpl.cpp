/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "ApexSDKCachedDataImpl.h"
#include "nvparameterized/NvParameterized.h"

namespace nvidia
{
namespace apex
{

bool ApexSDKCachedDataImpl::registerModuleDataCache(ModuleCachedDataIntl* cache)
{
	if (cache == NULL)
	{
		return false;
	}

	for (uint32_t i = 0; i < mModuleCaches.size(); ++i)
	{
		if (cache == mModuleCaches[i])
		{
			return false;
		}
	}

	mModuleCaches.pushBack(cache);

	return true;
}

bool ApexSDKCachedDataImpl::unregisterModuleDataCache(ModuleCachedDataIntl* cache)
{
	if (cache == NULL)
	{
		return false;
	}

	for (uint32_t i = mModuleCaches.size(); i--;)
	{
		if (cache == mModuleCaches[i])
		{
			mModuleCaches.replaceWithLast(i);
			break;
		}
	}

	return false;
}

ApexSDKCachedDataImpl::ApexSDKCachedDataImpl()
{
}

ApexSDKCachedDataImpl::~ApexSDKCachedDataImpl()
{
}

ModuleCachedData* ApexSDKCachedDataImpl::getCacheForModule(AuthObjTypeID moduleID)
{
	for (uint32_t i = 0; i < mModuleCaches.size(); ++i)
	{
		if (moduleID == mModuleCaches[i]->getModuleID())
		{
			return mModuleCaches[i];
		}
	}

	return NULL;
}

PxFileBuf& ApexSDKCachedDataImpl::serialize(PxFileBuf& stream) const
{
	stream.storeDword((uint32_t)Version::Current);

	for (uint32_t i = 0; i < mModuleCaches.size(); ++i)
	{
		mModuleCaches[i]->serialize(stream);
	}

	return stream;
}

PxFileBuf& ApexSDKCachedDataImpl::deserialize(PxFileBuf& stream)
{
	clear(false); // false => don't delete cached data for referenced sets

	/*const uint32_t version =*/
	stream.readDword();	// Original version

	for (uint32_t i = 0; i < mModuleCaches.size(); ++i)
	{
		mModuleCaches[i]->deserialize(stream);
	}

	return stream;
}

void ApexSDKCachedDataImpl::clear(bool force)
{
	for (uint32_t i = mModuleCaches.size(); i--;)
	{
		mModuleCaches[i]->clear(force);
	}
}

}
} // end namespace nvidia::apex
