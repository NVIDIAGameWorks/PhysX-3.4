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


#ifndef PXPVDSDK_PXPROFILEMEMORY_H
#define PXPVDSDK_PXPROFILEMEMORY_H

#include "PxProfileBase.h"
#include "PxProfileEventBufferClientManager.h"
#include "PxProfileEventSender.h"
#include "PsBroadcast.h"

namespace physx { namespace profile {

	/**
	\brief Record events so a late-connecting client knows about
	all outstanding allocations
	*/
	class PxProfileMemoryEventRecorder : public shdfnd::AllocationListener
	{
	protected:
		virtual ~PxProfileMemoryEventRecorder(){}
	public:
		/**
		\brief Set the allocation listener
		\param inListener Allocation listener.
		*/
		virtual void setListener(AllocationListener* inListener) = 0;
		/**
		\brief Release the instance.
		*/
		virtual void release() = 0;

		/**
		\brief Create the profile memory event recorder.
		\param inAllocator Allocation callback.
		*/
		static PxProfileMemoryEventRecorder& createRecorder(PxAllocatorCallback* inAllocator);
	};

	/**
	\brief Stores memory events into the memory buffer. 
	*/
	class PxProfileMemoryEventBuffer
		: public shdfnd::AllocationListener //add a new event to the buffer
		, public PxProfileEventBufferClientManager //add clients to handle the serialized memory events
		, public PxProfileEventFlusher //flush the buffer
	{
	protected:
		virtual ~PxProfileMemoryEventBuffer(){}
	public:

		/**
		\brief Release the instance.
		*/
		virtual void release() = 0;
		
		/**
		\brief Create a non-mutex-protected event buffer.		
		\param inAllocator Allocation callback.
		\param inBufferSize Internal buffer size.
		*/
		static PxProfileMemoryEventBuffer& createMemoryEventBuffer(PxAllocatorCallback& inAllocator, uint32_t inBufferSize = 0x1000);
	};



} } // namespace physx


#endif // PXPVDSDK_PXPROFILEMEMORY_H


