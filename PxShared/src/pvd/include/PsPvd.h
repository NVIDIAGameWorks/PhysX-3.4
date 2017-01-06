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

#ifndef PXPVDSDK_PSPVD_H
#define PXPVDSDK_PSPVD_H

/** \addtogroup pvd
@{
*/
#include "pvd/PxPvd.h"
#include "PsBroadcast.h"

#if !PX_DOXYGEN
namespace physx
{
#endif

class PxPvdTransport;

#if !PX_DOXYGEN
namespace pvdsdk
{
#endif

class PvdDataStream;
class PvdClient;
class PvdOMMetaDataProvider;

// PsPvd is used for advanced user, it support custom pvd client API
class PsPvd : public physx::PxPvd, public shdfnd::AllocationListener
{
  public:
	virtual void addClient(PvdClient* client) = 0;
	virtual void removeClient(PvdClient* client) = 0;
	
	virtual bool registerObject(const void* inItem) = 0;
	virtual bool unRegisterObject(const void* inItem) = 0;

	//AllocationListener
	void onAllocation(size_t size, const char* typeName, const char* filename, int line, void* allocatedMemory) = 0;
	void onDeallocation(void* addr) = 0;

	virtual PvdOMMetaDataProvider& getMetaDataProvider() = 0;
	
	virtual uint64_t getNextStreamId() = 0;
	// Call to flush events to PVD
	virtual void flush() = 0;

};

PX_PVDSDK_API void PX_CALL_CONV SetPvdAllocatorCallback(PxAllocatorCallback* inAllocatorCallback);

#if !PX_DOXYGEN
} // namespace pvdsdk
} // namespace physx
#endif

/** @} */
#endif // PXPVDSDK_PSPVD_H
