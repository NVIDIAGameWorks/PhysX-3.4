/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef RESOURCE_CALLBACK_H
#define RESOURCE_CALLBACK_H

/*!
\file
\brief class ResourceCallback
*/

#include "ApexUsingNamespace.h"

namespace nvidia
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

/**
\brief User defined callback for resource management

The user may implement a subclass of this abstract class and provide an instance to the
ApexSDK descriptor.  These callbacks can only be triggered directly by ApexSDK API calls,
so they do not need to be re-entrant or thread safe.
*/
class ResourceCallback
{
public:
	virtual ~ResourceCallback() {}

	/**
	\brief Request a resource from the user

	Will be called by the ApexSDK if a named resource is required but has not yet been provided.
	The resource pointer is returned directly, ResourceProvider::setResource() should not be called.
	This function will be called at most once per named resource, unless an intermediate call to
	releaseResource() has been made.
	
	\note If this call results in the application calling ApexSDK::createAsset, the name given 
		  to the asset must match the input name parameter in this method.
	*/
	virtual void* requestResource(const char* nameSpace, const char* name) = 0;

	/**
	\brief Request the user to release a resource

	Will be called by the ApexSDK when all internal references to a named resource have been released.
	If this named resource is required again in the future, a new call to requestResource() will be made.
	*/
	virtual void  releaseResource(const char* nameSpace, const char* name, void* resource) = 0;
};

PX_POP_PACK

}
} // namespace nvidia::apex

#endif // RESOURCE_CALLBACK_H
