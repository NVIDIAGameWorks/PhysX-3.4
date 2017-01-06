/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __APEX_SCENE_CACHED_DATA_H__

#define __APEX_SCENE_CACHED_DATA_H__

#include "ApexUsingNamespace.h"
#include "PxSimpleTypes.h"
#include "PxFileBuf.h"
#include "PsUserAllocated.h"
#include "PsArray.h"

#include "ApexSDKCachedData.h"

namespace nvidia
{
namespace apex
{

class ApexScene;
class ModuleSceneIntl;

/**
	Cached data is stored per-module.
*/
class ModuleCachedDataIntl : public ModuleCachedData
{
public:
	virtual AuthObjTypeID				getModuleID() const = 0;

	virtual NvParameterized::Interface*	getCachedDataForAssetAtScale(Asset& asset, const PxVec3& scale) = 0;
	virtual PxFileBuf&			serialize(PxFileBuf& stream) const = 0;
	virtual PxFileBuf&			deserialize(PxFileBuf& stream) = 0;
	virtual void						clear(bool force = true) = 0;	// If force == false, data in use by actors will not be deleted
};

//**************************************************************************************************************************
//**************************************************************************************************************************
//**** APEX SCENE CACHED DATA
//**************************************************************************************************************************
//**************************************************************************************************************************

class ApexSDKCachedDataImpl : public ApexSDKCachedData, public UserAllocated
{
public:
	bool							registerModuleDataCache(ModuleCachedDataIntl* cache);
	bool							unregisterModuleDataCache(ModuleCachedDataIntl* cache);

	// ApexSDKCachedData interface
	ApexSDKCachedDataImpl();
	virtual							~ApexSDKCachedDataImpl();

	virtual ModuleCachedData*	getCacheForModule(AuthObjTypeID moduleID);
	virtual PxFileBuf&		serialize(PxFileBuf& stream) const;
	virtual PxFileBuf&		deserialize(PxFileBuf& stream);
	virtual void					clear(bool force = true);

	struct Version
	{
		enum Enum
		{
			First = 0,

			Count,
			Current = Count - 1
		};
	};

	// Data
	physx::Array<ModuleCachedDataIntl*>	mModuleCaches;
};

}
} // end namespace nvidia::apex

#endif	// __APEX_SCENE_CACHED_DATA_H__
