/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef RESOURCE_PROVIDER_H
#define RESOURCE_PROVIDER_H

/*!
\file
\brief class ResourceProvider
*/

#include "ApexUsingNamespace.h"

namespace nvidia
{
namespace apex
{

class ResourceCallback;

PX_PUSH_PACK_DEFAULT

/*!
\brief A user provided class for mapping names to pointers or integers

Named resource provider - a name-to-pointer utility.  User must provide the pointed-to data.
				        - also supports name-to-integer
*/

class ResourceProvider
{
public:
	/**
	\brief Register a callback

	Register a callback function for unresolved named resources.  This function will be called by APEX when
	an unresolved named resource is requested.  The function will be called at most once for each named
	resource.  The function must directly return the pointer to the resource or NULL.
	*/
	virtual void 		registerCallback(ResourceCallback* impl) = 0;

	/**
	\brief Provide the pointer for the specified named resource
	*/
	virtual void 		setResource(const char* nameSpace, const char* name, void* resource, bool incRefCount = false) = 0;


	/**
	\brief Provide the unsigned integer for the specified named resource
	*/
	virtual void 		setResourceU32(const char* nameSpace, const char* name, uint32_t id, bool incRefCount = false) = 0;

	/**
	\brief Retrieve the pointer to the specified named resource.

	The game application (level loader) may use this to find assets by
	name, as a convenience.  If the named resource has not yet been
	loaded it will trigger a call to ResourceCallback::requestResource(),
	assuming an ResourceCallback instance was registered with the APEX SDK.
	If the named resource has already been loaded, getResource will not
	increment the reference count.
	*/
	virtual void* 		getResource(const char* nameSpace, const char* name) = 0;

	/**
	\brief Releases all resources in this namespace.
	\return the total number of resources released.
	*/
	virtual uint32_t  	releaseAllResourcesInNamespace(const char* nameSpace) = 0;

	/**
	\brief Releases a single resource.
	\return the outstanding referernce count after the release is performed.
	*/
	virtual uint32_t  	releaseResource(const char* nameSpace, const char* name) = 0;

	/**
	\brief Reports if a current resource exists and, if so, the reference count.
	*/
	virtual bool    	findRefCount(const char* nameSpace, const char* name, uint32_t& refCount) = 0;

	/**
	\brief Locates an existing resource

	This function will *not* call back to the application if the resource does not exist.
	Only reports currently set resources.
	*/
	virtual void* 		findResource(const char* nameSpace, const char* name) = 0;

	/**
	\brief Locates an existing integer resource.
	*/
	virtual uint32_t 	findResourceU32(const char* nameSpace, const char* name) = 0;

	/**
	\brief Returns a list of all resources in a particular namespace.
	*/
	virtual void** 		findAllResources(const char* nameSpace, uint32_t& count) = 0;

	/**
	\brief Returns a list of the names of all resources within a particular namespace
	*/
	virtual const char** findAllResourceNames(const char* nameSpace, uint32_t& count) = 0;

	/**
	\brief Returns a list of all registered namespaces.
	*/
	virtual const char** findNameSpaces(uint32_t& count) = 0;

	/**
	\brief Write contents of resource table to error stream.
	*/
	virtual void dumpResourceTable() = 0;

	/**
	\brief Returns if the resource provider is operating in a case sensitive mode.

	\note By default the resource provider is NOT case sensitive
	
	\note It's not possible to change the case sensitivity of the NRP 
		  once the APEX SDK is created, so the the switch is available in the 
		  ApexSDKDesc::resourceProviderIsCaseSensitive member variable.
	*/
	virtual bool isCaseSensitive() = 0;
};

PX_POP_PACK

}
} // end namespace nvidia::apex

#endif // RESOURCE_PROVIDER_H
