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
#include "foundation/PxProfiler.h"

#include "PxPvdImpl.h"
#include "PxPvdMemClient.h"
#include "PxProfileMemory.h"

namespace physx
{
namespace pvdsdk
{

PvdMemClient::PvdMemClient(PvdImpl& pvd)
: mSDKPvd(pvd)
, mPvdDataStream(NULL)
, mIsConnected(false)
, mMemEventBuffer(profile::PxProfileMemoryEventBuffer::createMemoryEventBuffer(*gPvdAllocatorCallback))
{
}

PvdMemClient::~PvdMemClient()
{
	mSDKPvd.removeClient(this);
	if(mMemEventBuffer.hasClients())
		mPvdDataStream->destroyInstance(&mMemEventBuffer);
	mMemEventBuffer.release();
}

PvdDataStream* PvdMemClient::getDataStream()
{
	return mPvdDataStream;
}

PvdUserRenderer* PvdMemClient::getUserRender()
{
	PX_ASSERT(0);
	return NULL;
}

void PvdMemClient::setObjectRegistrar(ObjectRegistrar*)
{
}

bool PvdMemClient::isConnected() const
{
	return mIsConnected;
}

void PvdMemClient::onPvdConnected()
{
	if(mIsConnected)
		return;
	mIsConnected = true;

	mPvdDataStream = PvdDataStream::create(&mSDKPvd);
	mPvdDataStream->createInstance(&mMemEventBuffer);
	mMemEventBuffer.addClient(*this);
}

void PvdMemClient::onPvdDisconnected()
{
	if(!mIsConnected)
		return;
	mIsConnected = false;

	flush();

	mMemEventBuffer.removeClient(*this);
	mPvdDataStream->release();
	mPvdDataStream = NULL;
}

void PvdMemClient::onAllocation(size_t inSize, const char* inType, const char* inFile, int inLine, void* inAddr)
{
	mMutex.lock();
	mMemEventBuffer.onAllocation(inSize, inType, inFile, inLine, inAddr);
	mMutex.unlock();
}

void PvdMemClient::onDeallocation(void* inAddr)
{
	mMutex.lock();
	mMemEventBuffer.onDeallocation(inAddr);
	mMutex.unlock();
}

void PvdMemClient::flush()
{
	mMutex.lock();
	mMemEventBuffer.flushProfileEvents();
	mMutex.unlock();
}

void PvdMemClient::handleBufferFlush(const uint8_t* inData, uint32_t inLength)
{
	if(mPvdDataStream)
	    mPvdDataStream->setPropertyValue(&mMemEventBuffer, "events", inData, inLength);
}

void PvdMemClient::handleClientRemoved()
{
}

} // pvd
} // physx
