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

#ifndef PXPVDSDK_PXPROFILEEVENTBUFFERCLIENT_H
#define PXPVDSDK_PXPROFILEEVENTBUFFERCLIENT_H

#include "PxProfileBase.h"
#include "PxProfileEventNames.h"

namespace physx { namespace profile {
	
	/**
	\brief Client handles the data when an event buffer flushes.  This data
	can be parsed (PxProfileEventHandler.h) as a binary set of events.
	*/
	class PxProfileEventBufferClient
	{
	protected:
		virtual ~PxProfileEventBufferClient(){}
	public:
		/**
		\brief Callback when the event buffer is full. This data is serialized profile events
		and can be read back using: PxProfileEventHandler::parseEventBuffer.

		\param inData Provided buffer data.
		\param inLength Data length.

		@see PxProfileEventHandler::parseEventBuffer.
		 */
		virtual void handleBufferFlush( const uint8_t* inData, uint32_t inLength ) = 0;

		/**
		\brief Happens if something removes all the clients from the manager.
		*/
		virtual void handleClientRemoved() = 0; 
	};

	/**
	\brief Client handles new profile event add.
	*/
	class PxProfileZoneClient : public PxProfileEventBufferClient
	{
	protected:
		virtual ~PxProfileZoneClient(){}
	public:
		/**
		\brief Callback when new profile event is added.

		\param inName Added profile event name.
		*/
		virtual void handleEventAdded( const PxProfileEventName& inName ) = 0;
	};

} }


#endif // PXPVDSDK_PXPROFILEEVENTBUFFERCLIENT_H
