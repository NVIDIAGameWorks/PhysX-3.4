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
#include "PxPvdProfileZoneClient.h"
#include "PxProfileZone.h"

namespace physx
{
namespace pvdsdk
{
struct ProfileZoneClient : public profile::PxProfileZoneClient, public shdfnd::UserAllocated
{
	profile::PxProfileZone& mZone;
	PvdDataStream& mStream;

	ProfileZoneClient(profile::PxProfileZone& zone, PvdDataStream& stream) : mZone(zone), mStream(stream)
	{
	}

	~ProfileZoneClient()
	{
		mZone.removeClient(*this);
	}

	virtual void createInstance()
	{
		mStream.addProfileZone(&mZone, mZone.getName());
		mStream.createInstance(&mZone);
		mZone.addClient(*this);
		profile::PxProfileNames names(mZone.getProfileNames());
		PVD_FOREACH(idx, names.eventCount)
		{
			handleEventAdded(names.events[idx]);
		}
	}

	virtual void handleEventAdded(const profile::PxProfileEventName& inName)
	{
		mStream.addProfileZoneEvent(&mZone, inName.name, inName.eventId.eventId, inName.eventId.compileTimeEnabled);
	}

	virtual void handleBufferFlush(const uint8_t* inData, uint32_t inLength)
	{
		mStream.setPropertyValue(&mZone, "events", inData, inLength);
	}

	virtual void handleClientRemoved()
	{
		mStream.destroyInstance(&mZone);
	}

  private:
	ProfileZoneClient& operator=(const ProfileZoneClient&);
};
}
}

using namespace physx;
using namespace pvdsdk;

PvdProfileZoneClient::PvdProfileZoneClient(PvdImpl& pvd) : mSDKPvd(pvd), mPvdDataStream(NULL), mIsConnected(false)
{
}

PvdProfileZoneClient::~PvdProfileZoneClient()
{
	mSDKPvd.removeClient(this);
	// all zones should removed
	PX_ASSERT(mProfileZoneClients.size() == 0);
}

PvdDataStream* PvdProfileZoneClient::getDataStream()
{
	return mPvdDataStream;
}

PvdUserRenderer* PvdProfileZoneClient::getUserRender()
{
	PX_ASSERT(0);
	return NULL;
}

void PvdProfileZoneClient::setObjectRegistrar(ObjectRegistrar*)
{
}

bool PvdProfileZoneClient::isConnected() const
{
	return mIsConnected;
}

void PvdProfileZoneClient::onPvdConnected()
{
	if(mIsConnected)
		return;
	mIsConnected = true;

	mPvdDataStream = PvdDataStream::create(&mSDKPvd);

}

void PvdProfileZoneClient::onPvdDisconnected()
{
	if(!mIsConnected)
		return;

	mIsConnected = false;
	flush();

	mPvdDataStream->release();
	mPvdDataStream = NULL;
}

void PvdProfileZoneClient::flush()
{
	PVD_FOREACH(idx, mProfileZoneClients.size())
	mProfileZoneClients[idx]->mZone.flushProfileEvents();
}

void PvdProfileZoneClient::onZoneAdded(profile::PxProfileZone& zone)
{
	PX_ASSERT(mIsConnected);
	ProfileZoneClient* client = PVD_NEW(ProfileZoneClient)(zone, *mPvdDataStream);
	mMutex.lock();
	client->createInstance();
	mProfileZoneClients.pushBack(client);
	mMutex.unlock();
}

void PvdProfileZoneClient::onZoneRemoved(profile::PxProfileZone& zone)
{
	for(uint32_t i = 0; i < mProfileZoneClients.size(); i++)
	{
		if(&zone == &mProfileZoneClients[i]->mZone)
		{
			mMutex.lock();
			ProfileZoneClient* client = mProfileZoneClients[i];
			mProfileZoneClients.replaceWithLast(i);
			PVD_DELETE(client);
			mMutex.unlock();
			return;
		}
	}
}
