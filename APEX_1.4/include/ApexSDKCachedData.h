/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef APEX_SDKCACHED_DATA_H
#define APEX_SDKCACHED_DATA_H

/*!
\file
\brief classes ModuleCachedData, ApexSDKCachedData
*/

#include "ApexSDK.h"

namespace NvParameterized
{
class Interface;
};

namespace nvidia
{
namespace apex
{


/**
\brief Cached data is stored per-module.
*/
class ModuleCachedData
{
public:
	/**
	 * \brief Retreives the cached data for the asset, if it exists.
	 *
	 * Otherwise returns NULL.
	 */
	virtual ::NvParameterized::Interface*	getCachedDataForAssetAtScale(Asset& asset, const PxVec3& scale) = 0;

	/**	
	 *	\brief Serializes the cooked data for a single asset into a stream.
	 */
	virtual PxFileBuf& serializeSingleAsset(Asset& asset, PxFileBuf& stream) = 0;

	/**	
	 *	\brief Deserializes the cooked data for a single asset from a stream.
	 */
	virtual PxFileBuf& deserializeSingleAsset(Asset& asset, PxFileBuf& stream) = 0;
};

/**
\brief A method for storing actor data in a scene
*/
class ApexSDKCachedData
{
public:
	/**
	 * \brief Retreives the scene cached data for the actor, if it exists.
	 *
	 * Otherwise returns NULL.
	 */
	virtual ModuleCachedData*	getCacheForModule(AuthObjTypeID moduleID) = 0;

	/**
	 * \brief Save cache configuration to a stream
	 */
	virtual PxFileBuf&  		serialize(PxFileBuf&) const = 0;

	/**
	 * \brief Load cache configuration from a stream
	 */
	virtual PxFileBuf&  		deserialize(PxFileBuf&) = 0;

	/**
	 * \brief Clear data
	 */
	virtual void				clear(bool force = true) = 0;

protected:
	ApexSDKCachedData() {}
	virtual						~ApexSDKCachedData() {}
};

}
} // end namespace nvidia::apex


#endif // APEX_SDKCACHED_DATA_H
