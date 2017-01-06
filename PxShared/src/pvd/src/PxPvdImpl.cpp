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

#include "pvd/PxPvdTransport.h"

#include "PxPvdImpl.h"
#include "PxPvdFoundation.h"
#include "PxPvdClient.h"
#include "PxPvdMemClient.h"
#include "PxPvdProfileZoneClient.h"
#include "PxPvdCommStreamTypes.h"
#include "PxProfileZoneManager.h"
#include "PxProfileZone.h"

#include "PsFoundation.h"

#if PX_NVTX
#include "nvToolsExt.h"
#endif

namespace
{
	const char* gSdkName = "PhysXSDK";
}

namespace physx
{
namespace pvdsdk
{

class CmEventNameProvider : public physx::profile::PxProfileNameProvider
{
public:
	physx::profile::PxProfileNames getProfileNames() const
	{
		physx::profile::PxProfileNames  ret;
		ret.eventCount = 0;
		return ret;
	}
};

CmEventNameProvider gProfileNameProvider;

void initializeModelTypes(PvdDataStream& stream)
{
	stream.createClass<profile::PxProfileZone>();
	stream.createProperty<profile::PxProfileZone, uint8_t>(
	    "events", PvdCommStreamEmbeddedTypes::getProfileEventStreamSemantic(), PropertyType::Array);

	stream.createClass<profile::PxProfileMemoryEventBuffer>();
	stream.createProperty<profile::PxProfileMemoryEventBuffer, uint8_t>(
	    "events", PvdCommStreamEmbeddedTypes::getMemoryEventStreamSemantic(), PropertyType::Array);

	stream.createClass<PvdUserRenderer>();
	stream.createProperty<PvdUserRenderer, uint8_t>(
	    "events", PvdCommStreamEmbeddedTypes::getRendererEventStreamSemantic(), PropertyType::Array);
}

PvdImpl* PvdImpl::sInstance = NULL;
uint32_t PvdImpl::sRefCount = 0;

PvdImpl::PvdImpl()
: mPvdTransport(NULL)
, mSharedMetaProvider(NULL)
, mMemClient(NULL)
, mIsConnected(false)
, mIsNVTXSupportEnabled(true)
, mNVTXContext(0)
, mNextStreamId(1)
, mProfileClient(NULL)
, mProfileZone(NULL)
{
	mProfileZoneManager = &physx::profile::PxProfileZoneManager::createProfileZoneManager(&physx::shdfnd::getAllocator());
	mProfileClient = PVD_NEW(PvdProfileZoneClient)(*this);
}

PvdImpl::~PvdImpl()
{
	if((mFlags & PxPvdInstrumentationFlag::ePROFILE) )
	{
		PxSetProfilerCallback(NULL);
	}

	disconnect();

	if ( mProfileZoneManager )
	{
		mProfileZoneManager->release();
		mProfileZoneManager = NULL;
	}

	PVD_DELETE(mProfileClient);
	mProfileClient = NULL;
}

bool PvdImpl::connect(PxPvdTransport& transport, PxPvdInstrumentationFlags flags)
{
	if(mIsConnected)
	{
		physx::shdfnd::getFoundation().error(PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, "PxPvd::connect - recall connect! Should call disconnect before re-connect.");
	    return false;
	}

	mFlags = flags;	
	mPvdTransport = &transport;

	mIsConnected = mPvdTransport->connect();

	if(mIsConnected)
	{
		mSharedMetaProvider = PVD_NEW(MetaDataProvider);
		sendTransportInitialization();

		PvdDataStream* stream = PvdDataStream::create(this);
		initializeModelTypes(*stream);
		stream->release();

		if(mFlags & PxPvdInstrumentationFlag::eMEMORY)
		{
			mMemClient = PVD_NEW(PvdMemClient)(*this);
			mPvdClients.pushBack(mMemClient);
		}

		if((mFlags & PxPvdInstrumentationFlag::ePROFILE) && mProfileZoneManager)
		{			
			mPvdClients.pushBack(mProfileClient);
			mProfileZone = &physx::profile::PxProfileZone::createProfileZone(&physx::shdfnd::getAllocator(),gSdkName,gProfileNameProvider.getProfileNames());
		}

		for(uint32_t i = 0; i < mPvdClients.size(); i++)
			mPvdClients[i]->onPvdConnected();

		if (mProfileZone)
		{
			mProfileZoneManager->addProfileZoneHandler(*mProfileClient);
			mProfileZoneManager->addProfileZone( *mProfileZone );
		}

		if ((mFlags & PxPvdInstrumentationFlag::ePROFILE))
		{
			PxSetProfilerCallback(this);
		}
	}
	return mIsConnected;
}

void PvdImpl::disconnect()
{
	if(mProfileZone)
	{
		mProfileZoneManager->removeProfileZoneHandler(*mProfileClient);		
		mProfileZoneManager->removeProfileZone( *mProfileZone );				
		mProfileZone->release();
		mProfileZone=NULL;	
		removeClient(mProfileClient);
	}

	if(mIsConnected)
	{
		for(uint32_t i = 0; i < mPvdClients.size(); i++)
			mPvdClients[i]->onPvdDisconnected();		

		if(mMemClient)
		{
			removeClient(mMemClient);
			PvdMemClient* tmp = mMemClient;  //avoid tracking deallocation itsself
			mMemClient = NULL;
			PVD_DELETE(tmp);	        
		}
		 
		mSharedMetaProvider->release();
		mPvdTransport->disconnect();
		mObjectRegistrar.clear();
		mIsConnected = false;
	}
}

void PvdImpl::flush()
{
	for(uint32_t i = 0; i < mPvdClients.size(); i++)
		mPvdClients[i]->flush();
	if ( mProfileZone )
	{
		mProfileZone->flushEventIdNameMap();
		mProfileZone->flushProfileEvents();
	}
}

bool PvdImpl::isConnected(bool useCachedStatus)
{
	if(mPvdTransport)
	    return useCachedStatus ? mIsConnected : mPvdTransport->isConnected();
	else
		return false;
}

PxPvdTransport* PvdImpl::getTransport()
{
	return mPvdTransport;
}

PxPvdInstrumentationFlags PvdImpl::getInstrumentationFlags()
{
	return mFlags;
}

void PvdImpl::sendTransportInitialization()
{
	StreamInitialization init;
	EventStreamifier<PxPvdTransport> stream(mPvdTransport->lock());
	init.serialize(stream);
	mPvdTransport->unlock();
}

void PvdImpl::addClient(PvdClient* client)
{
	PX_ASSERT(client);
	for(uint32_t i = 0; i < mPvdClients.size(); i++)
	{
		if(client == mPvdClients[i])
		    return;
	}
	mPvdClients.pushBack(client);
	if(mIsConnected)
	{
		client->onPvdConnected();
	}
}

void PvdImpl::removeClient(PvdClient* client)
{
	for(uint32_t i = 0; i < mPvdClients.size(); i++)
	{
		if(client == mPvdClients[i])
		{
			client->onPvdDisconnected();
			mPvdClients.remove(i);
		}
	}
}

void PvdImpl::onAllocation(size_t inSize, const char* inType, const char* inFile, int inLine, void* inAddr)
{
	if(mMemClient)
       mMemClient->onAllocation(inSize, inType, inFile, inLine, inAddr);
}

void PvdImpl::onDeallocation(void* inAddr)
{
	if(mMemClient)
       mMemClient->onDeallocation(inAddr);
}

PvdOMMetaDataProvider& PvdImpl::getMetaDataProvider()
{
	return *mSharedMetaProvider;
}

bool PvdImpl::registerObject(const void* inItem)
{
	return mObjectRegistrar.addItem(inItem);
}


bool PvdImpl::unRegisterObject(const void* inItem)
{
	return mObjectRegistrar.decItem(inItem);
}

uint64_t PvdImpl::getNextStreamId()
{
	uint64_t retval = ++mNextStreamId;
	return retval;
}

bool PvdImpl::initialize()
{
	if(0 == sRefCount)
	{
		sInstance = PVD_NEW(PvdImpl)();
	}
	++sRefCount;
	return !!sInstance;
}

void PvdImpl::release()
{
	if(sRefCount > 0)
	{
		if(--sRefCount)
			return;

		PVD_DELETE(sInstance);
		sInstance = NULL;
	}
}

PvdImpl* PvdImpl::getInstance()
{
	return sInstance;
}


/**************************************************************************************************************************
Instrumented profiling events
***************************************************************************************************************************/

static const uint32_t CrossThreadId = 99999789;

void* PvdImpl::zoneStart(const char* eventName, bool detached, uint64_t contextId)
{
	if(mProfileZone)
	{
		const uint16_t id = mProfileZone->getEventIdForName(eventName);
		if(detached)
			mProfileZone->startEvent(id, contextId, CrossThreadId);
		else
			mProfileZone->startEvent(id, contextId);
	}
#if PX_NVTX
	if(mIsNVTXSupportEnabled)
	{ 
		if(detached)
		{
			// TODO : Need to use the nvtxRangeStart API for cross thread events
			nvtxEventAttributes_t eventAttrib;
			memset(&eventAttrib, 0, sizeof(eventAttrib));
			eventAttrib.version = NVTX_VERSION;
			eventAttrib.size = NVTX_EVENT_ATTRIB_STRUCT_SIZE;
			eventAttrib.colorType = NVTX_COLOR_ARGB;
			eventAttrib.color = 0xFF00FF00;
			eventAttrib.messageType = NVTX_MESSAGE_TYPE_ASCII;
			eventAttrib.message.ascii = eventName;
			nvtxMarkEx(&eventAttrib);
		}
		else
		{
			nvtxRangePush(eventName);
		}
	}
#endif
	return NULL;
}

void PvdImpl::zoneEnd(void* /*profilerData*/, const char* eventName, bool detached, uint64_t contextId)
{
	if(mProfileZone)
	{
		const uint16_t id = mProfileZone->getEventIdForName(eventName);
		if(detached)
			mProfileZone->stopEvent(id, contextId, CrossThreadId);
		else
			mProfileZone->stopEvent(id, contextId);
	}
#if PX_NVTX
	if(mIsNVTXSupportEnabled)
	{
		if(detached)
		{
			nvtxEventAttributes_t eventAttrib;
			memset(&eventAttrib, 0, sizeof(eventAttrib));
			eventAttrib.version = NVTX_VERSION;
			eventAttrib.size = NVTX_EVENT_ATTRIB_STRUCT_SIZE;
			eventAttrib.colorType = NVTX_COLOR_ARGB;
			eventAttrib.color = 0xFFFF0000;
			eventAttrib.messageType = NVTX_MESSAGE_TYPE_ASCII;
			eventAttrib.message.ascii = eventName;
			nvtxMarkEx(&eventAttrib);
		}
		else
		{
			nvtxRangePop();
		}
	}
#endif
}
} // pvd

} // physx
