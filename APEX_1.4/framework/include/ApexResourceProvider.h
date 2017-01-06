/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __APEX_RESOURCE_PROVIDER_H__
#define __APEX_RESOURCE_PROVIDER_H__

#include "Apex.h"
#include "PsUserAllocated.h"
#include "PsHashMap.h"
#include "ResourceProviderIntl.h"
#include "ApexString.h"
#include "PsArray.h"

namespace nvidia
{
namespace apex
{

class ApexResourceProvider : public ResourceProviderIntl, public UserAllocated
{
public:
	/* == Public ResourceProvider interface == */
	virtual void 			registerCallback(ResourceCallback* impl);
	virtual void 			setResource(const char* nameSpace, const char* name, void* resource, bool incRefCount);
	virtual void   			setResourceU32(const char* nameSpace, const char* name, uint32_t id, bool incRefCount);
	virtual void* 			getResource(const char* nameSpace, const char* name);
	virtual uint32_t  		releaseAllResourcesInNamespace(const char* nameSpace);
	virtual uint32_t  		releaseResource(const char* nameSpace, const char* name);
	virtual bool    		findRefCount(const char* nameSpace, const char* name, uint32_t& refCount);
	virtual void* 			findResource(const char* nameSpace, const char* name);
	virtual uint32_t 		findResourceU32(const char* nameSpace, const char* name); // find an existing resource.
	virtual void** 			findAllResources(const char* nameSpace, uint32_t& count); // find all resources in this namespace
	virtual const char** 	findAllResourceNames(const char* nameSpace, uint32_t& count); // find all resources in this namespace
	virtual const char** 	findNameSpaces(uint32_t& count);
	virtual void		 	dumpResourceTable();

	/* == Internal ResourceProviderIntl interface == */
	void	setResource(const char* nameSpace, const char* name, void* resource, bool valueIsSet, bool incRefCount);
	ResID	createNameSpace(const char* nameSpace, bool releaseAtExit)
	{
		return createNameSpaceInternal(nameSpace, releaseAtExit);
	}
	ResID	createResource(ResID nameSpace, const char* name, bool refCount);
	bool	checkResource(ResID nameSpace, const char* name);
	bool	checkResource(ResID id);
	void    releaseResource(ResID id);
	void	generateUniqueName(ResID nameSpace, ApexSimpleString& name);
	void*   getResource(ResID id);
	const char* getResourceName(ResID id);
	const char* getResourceNameSpace(ResID id);
	bool	getResourceIDs(const char* nameSpace, ResID* outResIDs, uint32_t& outCount, uint32_t inCount);

	// [PVD INTEGRATION CODE] ===========================================
	PX_INLINE uint32_t	getResourceCount() const
	{
		return mResources.size();
	}
	// ==================================================================

	// the NRP can either be case sensitive or not, this method takes care of that option
	PX_INLINE bool isCaseSensitive()
	{
		return mCaseSensitive;
	}
	
	/**
	\brief Sets the resource provider's case sensitive mode.
	
	\note This must be done immediately after initialization so no names are hashed
		  using the wrong mode.
	*/
	void	setCaseSensitivity(bool caseSensitive)
	{
		mCaseSensitive = caseSensitive;
	}

	// uses the correct string matching function based on the case sensitivity mode
	bool	stringsMatch(const char* str0, const char* str1);

protected:
	ApexResourceProvider();
	virtual ~ApexResourceProvider();
	void	destroy();

private:
	ResID	createNameSpaceInternal(const char* &nameSpace, bool releaseAtExit);

	class NameSpace: public UserAllocated
	{
	public:
		NameSpace(ApexResourceProvider* arp, ResID nsid, bool releaseAtExit, const char* nameSpace);
		~NameSpace();

		ResID	getOrCreateID(const char* &name, const char* NSName);
		ResID	getID() const
		{
			return mId;
		}
		bool    releaseAtExit() const
		{
			return mReleaseAtExit;
		}
		const char* getNameSpace(void) const
		{
			return mNameSpace;
		};

	private:
		struct entryHeader
		{
			const char* nextEntry;
			ResID     id;
		};
		enum { HashSize = 1024 };
		bool					mReleaseAtExit;
		uint16_t				genHash(const char* name);
		const char*				hash[HashSize];
		ApexResourceProvider*	mArp;
		ResID					mId;
		char* 					mNameSpace;
	};

	// NOTE: someone thinks this struct needs to be padded to nearest 16 or 32 bytes
	// it's not padded at the moment, but watch out for this
	struct resource
	{
		void*		ptr;
		const char* name;
		const char* nameSpace;
		uint16_t	refCount;
		uint8_t		valueIsSet;
		uint8_t		usedGetResource;
	};

	typedef physx::HashMap<ResID, ResID> HashMapNSID;

	ResID		getNSID(const char* nsName);
	uint32_t	getNSIndex(ResID nameSpace);

	HashMapNSID	mNSID;

	NameSpace					mNSNames;
	physx::Array<resource>		mResources;
	physx::Array<NameSpace*>	mNameSpaces;
	ResourceCallback*   		mUserCallback;
	physx::Array< const char* > mCharResults;
	bool						mCaseSensitive;


	enum { UnknownValue = 0xFFFFFFFF };

	friend class ApexSDKImpl;
};

}
} // end namespace nvidia::apex

#endif // __APEX_RESOURCE_PROVIDER_H__
