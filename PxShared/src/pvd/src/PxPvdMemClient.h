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

#ifndef PXPVDSDK_PXPVDMEMCLIENT_H
#define PXPVDSDK_PXPVDMEMCLIENT_H

#include "PxPvdClient.h"
#include "PsHashMap.h"
#include "PsMutex.h"
#include "PsBroadcast.h"
#include "PxProfileEventBufferClient.h"
#include "PxProfileMemory.h"

namespace physx
{
class PvdDataStream;

namespace pvdsdk
{
class PvdImpl;
class PvdMemClient : public PvdClient,                   
                     public profile::PxProfileEventBufferClient,
                     public shdfnd::UserAllocated
{
	PX_NOCOPY(PvdMemClient)
  public:
	PvdMemClient(PvdImpl& pvd);
	virtual ~PvdMemClient();

	bool isConnected() const;
	void onPvdConnected();
	void onPvdDisconnected();
	void flush();

	PvdDataStream* getDataStream();
	PvdUserRenderer* getUserRender();
	void setObjectRegistrar(ObjectRegistrar*);
	void sendMemEvents();

	// memory event
	void onAllocation(size_t size, const char* typeName, const char* filename, int line, void* allocatedMemory);
	void onDeallocation(void* addr);

  private:
	PvdImpl& mSDKPvd;
	PvdDataStream* mPvdDataStream;
	bool mIsConnected;

	// mem profile
	shdfnd::Mutex mMutex; // mem onallocation can called from different threads
	profile::PxProfileMemoryEventBuffer& mMemEventBuffer;
	void handleBufferFlush(const uint8_t* inData, uint32_t inLength);
	void handleClientRemoved();
};

} // namespace pvdsdk
} // namespace physx

#endif // PXPVDSDK_PXPVDMEMCLIENT_H
