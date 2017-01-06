// This code contains NVIDIA Confidential Information and is disclosed to you
// under a form of NVIDIA software license agreement provided separately to you.
//
// Notice
// NVIDIA Corporation and its licensors retain all intellectual property and
// proprietary rights in and to this software and related documentation and
// any modifications thereto. Any use, reproduction, disclosure, or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA Corporation is strictly prohibited.
//
// ALL NVIDIA DESIGN SPECIFICATIONS, CODE ARE PROVIDED "AS IS.". NVIDIA MAKES
// NO WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES OF NONINFRINGEMENT,
// MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Information and code furnished is believed to be accurate and reliable.
// However, NVIDIA Corporation assumes no responsibility for the consequences of use of such
// information or for any infringement of patents or other rights of third parties that may
// result from its use. No license is granted by implication or otherwise under any patent
// or patent rights of NVIDIA Corporation. Details are subject to change without notice.
// This code supersedes and replaces all information previously supplied.
// NVIDIA Corporation products are not authorized for use as critical
// components in life support devices or systems without express written approval of
// NVIDIA Corporation.
//
// Copyright (c) 2008-2017 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.

#ifndef PXPVDSDK_PXPVDIMPL_H
#define PXPVDSDK_PXPVDIMPL_H

#include "foundation/PxProfiler.h"

#include "PsAllocator.h"
#include "PsPvd.h"
#include "PsArray.h"
#include "PsMutex.h"
#include "PxPvdCommStreamTypes.h"
#include "PxPvdFoundation.h"
#include "PxPvdObjectModelMetaData.h"
#include "PxPvdObjectRegistrar.h"

namespace physx
{

namespace profile
{
	class PxProfileZoneManager;
}

namespace pvdsdk
{
class PvdMemClient;
class PvdProfileZoneClient;

struct MetaDataProvider : public PvdOMMetaDataProvider, public shdfnd::UserAllocated
{
    typedef shdfnd::Mutex::ScopedLock TScopedLockType;
    typedef shdfnd::HashMap<const void*, int32_t> TInstTypeMap;
	PvdObjectModelMetaData& mMetaData;
    shdfnd::Mutex mMutex;
	uint32_t mRefCount;
	TInstTypeMap mTypeMap;

	MetaDataProvider()
	: mMetaData(PvdObjectModelMetaData::create()), mRefCount(0), mTypeMap("MetaDataProvider::mTypeMap")
	{
		mMetaData.addRef();
	}
	virtual ~MetaDataProvider()
	{
		mMetaData.release();
	}

	virtual void addRef()
	{
		TScopedLockType locker(mMutex);
		++mRefCount;
	}
	virtual void release()
	{
		{
			TScopedLockType locker(mMutex);
			if(mRefCount)
				--mRefCount;
		}
		if(!mRefCount)
			PVD_DELETE(this);
	}
	virtual PvdObjectModelMetaData& lock()
	{
		mMutex.lock();
		return mMetaData;
	}
	virtual void unlock()
	{
		mMutex.unlock();
	}

	virtual bool createInstance(const NamespacedName& clsName, const void* instance)
	{
		TScopedLockType locker(mMutex);
		Option<ClassDescription> cls(mMetaData.findClass(clsName));
		if(cls.hasValue() == false)
			return false;
		int32_t instType = cls->mClassId;
		mTypeMap.insert(instance, instType);
		return true;
	}
	virtual bool isInstanceValid(const void* instance)
	{
		TScopedLockType locker(mMutex);
		ClassDescription classDesc;
		bool retval = mTypeMap.find(instance) != NULL;
#if PX_DEBUG
		if(retval)
			classDesc = mMetaData.getClass(mTypeMap.find(instance)->second);
#endif
		return retval;
	}
	virtual void destroyInstance(const void* instance)
	{
		{
			TScopedLockType locker(mMutex);
			mTypeMap.erase(instance);
		}
	}
	virtual int32_t getInstanceClassType(const void* instance)
	{
		TScopedLockType locker(mMutex);
		const TInstTypeMap::Entry* entry = mTypeMap.find(instance);
		if(entry)
			return entry->second;
		return -1;
	}

  private:
	MetaDataProvider& operator=(const MetaDataProvider&);
	MetaDataProvider(const MetaDataProvider&);
};

//////////////////////////////////////////////////////////////////////////
/*!
PvdImpl is the realization of PxPvd.
It implements the interface methods and provides richer functionality for advanced users or internal clients (such as
PhysX or APEX), including handler notification for clients.
*/
//////////////////////////////////////////////////////////////////////////
class PvdImpl : public PsPvd, public shdfnd::UserAllocated
{
	PX_NOCOPY(PvdImpl)

    typedef shdfnd::Mutex::ScopedLock TScopedLockType;
	typedef void (PvdImpl::*TAllocationHandler)(size_t size, const char* typeName, const char* filename, int line,
	                                            void* allocatedMemory);
	typedef void (PvdImpl::*TDeallocationHandler)(void* allocatedMemory);

  public:
	PvdImpl();
	virtual ~PvdImpl();
	void release();

	bool connect(PxPvdTransport& transport, PxPvdInstrumentationFlags flags);
	void disconnect();
	bool isConnected(bool useCachedStatus = true);
	void flush();

	PxPvdTransport* getTransport();
	PxPvdInstrumentationFlags getInstrumentationFlags();

	void addClient(PvdClient* client);
	void removeClient(PvdClient* client);

	PvdOMMetaDataProvider& getMetaDataProvider();

	bool registerObject(const void* inItem);
	bool unRegisterObject(const void* inItem);

	//AllocationListener
	void onAllocation(size_t size, const char* typeName, const char* filename, int line, void* allocatedMemory);
	void onDeallocation(void* addr);

	uint64_t getNextStreamId();

	static bool initialize();
	static PvdImpl* getInstance();

	// Profiling

	virtual void* zoneStart(const char* eventName, bool detached, uint64_t contextId);

	virtual void zoneEnd(void* profilerData, const char *eventName, bool detached, uint64_t contextId);

  private:
	void sendTransportInitialization();

	PxPvdTransport*						mPvdTransport;
	physx::shdfnd::Array<PvdClient*>	mPvdClients;

	MetaDataProvider*					mSharedMetaProvider; // shared between clients
	ObjectRegistrar						mObjectRegistrar;

	PvdMemClient*						mMemClient;

	PxPvdInstrumentationFlags			mFlags;
	bool								mIsConnected;
	bool								mIsNVTXSupportEnabled;
	uint32_t							mNVTXContext;
	uint64_t							mNextStreamId;
	physx::profile::PxProfileZoneManager*mProfileZoneManager;
	PvdProfileZoneClient*				mProfileClient;
	physx::profile::PxProfileZone*		mProfileZone;
	static PvdImpl*						sInstance;
	static uint32_t						sRefCount;
};

} // namespace pvdsdk
}

#endif // PXPVDSDK_PXPVDIMPL_H
