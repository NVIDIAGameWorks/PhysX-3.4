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

#ifndef PXPVDSDK_PXPROFILEEVENTHANDLER_H
#define PXPVDSDK_PXPROFILEEVENTHANDLER_H

#include "PxProfileBase.h"
#include "PxProfileEventId.h"

namespace physx { namespace profile {

	/**
	\brief A client of the event system can expect to find these events in the event buffer.
	*/
	class PxProfileEventHandler
	{
	protected:
		virtual ~PxProfileEventHandler(){}
	public:
		/**
		\brief Event start - onStartEvent.
			
		\param[in] inId Profile event id.
		\param[in] threadId Thread id.
		\param[in] contextId Context id.
		\param[in] cpuId CPU id.
		\param[in] threadPriority Thread priority.
		\param[in] timestamp Timestamp in cycles.		
		 */
		virtual void onStartEvent( const PxProfileEventId& inId, uint32_t threadId, uint64_t contextId, uint8_t cpuId, uint8_t threadPriority, uint64_t timestamp ) = 0;

		/**
		\brief Event stop - onStopEvent.
			
		\param[in] inId Profile event id.
		\param[in] threadId Thread id.
		\param[in] contextId Context id.
		\param[in] cpuId CPU id.
		\param[in] threadPriority Thread priority.
		\param[in] timestamp Timestamp in cycles.		
		 */
		virtual void onStopEvent( const PxProfileEventId& inId, uint32_t threadId, uint64_t contextId, uint8_t cpuId, uint8_t threadPriority, uint64_t timestamp ) = 0;

		/**
		\brief Event value - onEventValue.
			
		\param[in] inId Profile event id.
		\param[in] threadId Thread id.
		\param[in] contextId Context id.
		\param[in] inValue Value.
		 */
		virtual void onEventValue( const PxProfileEventId& inId, uint32_t threadId, uint64_t contextId, int64_t inValue ) = 0;

		/**
		\brief Parse the flushed profile buffer which contains the profile events.
			
		\param[in] inBuffer The profile buffer with profile events.
		\param[in] inBufferSize Buffer size.
		\param[in] inHandler The profile event callback to receive the parsed events.
		\param[in] inSwapBytes Swap bytes possibility.		
		 */
		static void parseEventBuffer( const uint8_t* inBuffer, uint32_t inBufferSize, PxProfileEventHandler& inHandler, bool inSwapBytes );

		/**
		\brief Translates event duration in timestamp (cycles) into nanoseconds.
			
		\param[in] duration Timestamp duration of the event.

		\return event duration in nanoseconds. 
		 */
		static uint64_t durationToNanoseconds(uint64_t duration);
	};
} }

#endif // PXPVDSDK_PXPROFILEEVENTHANDLER_H
