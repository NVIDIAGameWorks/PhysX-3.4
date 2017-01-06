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

#ifndef PXPVDSDK_PXPROFILEEVENTSENDER_H
#define PXPVDSDK_PXPROFILEEVENTSENDER_H

#include "PxProfileBase.h"

namespace physx { namespace profile {

	/**
	\brief Tagging interface to indicate an object that is capable of flushing a profile
	event stream at a certain point.
	 */
	class PxProfileEventFlusher
	{
	protected:
		virtual ~PxProfileEventFlusher(){}
	public:
		/**
		\brief Flush profile events. Sends the profile event buffer to hooked clients.
		*/
		virtual void flushProfileEvents() = 0;
	};

	/**
	\brief Sends the full events where the caller must provide the context and thread id.
	 */
	class PxProfileEventSender
	{
	protected:
		virtual ~PxProfileEventSender(){}
	public:
	
		/**
		\brief Use this as a thread id for events that start on one thread and end on another
		*/
		static const uint32_t CrossThreadId = 99999789;

		/**
		\brief Send a start profile event, optionally with a context. Events are sorted by thread
		and context in the client side.
		\param inId Profile event id.
		\param contextId Context id.
		*/
		virtual void startEvent( uint16_t inId, uint64_t contextId) = 0;
		/**
		\brief Send a stop profile event, optionally with a context. Events are sorted by thread
		and context in the client side.
		\param inId Profile event id.
		\param contextId Context id.
		*/
		virtual void stopEvent( uint16_t inId, uint64_t contextId) = 0;

		/**
		\brief Send a start profile event, optionally with a context. Events are sorted by thread
		and context in the client side.
		\param inId Profile event id.
		\param contextId Context id.
		\param threadId Thread id.
		*/
		virtual void startEvent( uint16_t inId, uint64_t contextId, uint32_t threadId) = 0;
		/**
		\brief Send a stop profile event, optionally with a context. Events are sorted by thread
		and context in the client side.
		\param inId Profile event id.
		\param contextId Context id.
		\param threadId Thread id.
		*/
		virtual void stopEvent( uint16_t inId, uint64_t contextId, uint32_t threadId ) = 0;

		virtual void atEvent(uint16_t inId, uint64_t contextId, uint32_t threadId, uint64_t start, uint64_t stop) = 0;

		/**
		\brief Set an specific events value. This is different than the profiling value
		for the event; it is a value recorded and kept around without a timestamp associated
		with it. This value is displayed when the event itself is processed.
		\param inId Profile event id.
		\param contextId Context id.
		\param inValue Value to set for the event.
		 */
		virtual void eventValue( uint16_t inId, uint64_t contextId, int64_t inValue ) = 0;
	};

	/**
	\brief Tagging interface to indicate an object that may or may not return
	an object capable of adding profile events to a buffer.
	*/
	class PxProfileEventSenderProvider
	{
	protected:
		virtual ~PxProfileEventSenderProvider(){}
	public:
		/**
		\brief This method can *always* return NULL.
		Thus need to always check that what you are getting is what
		you think it is.

		\return Perhaps a profile event sender.
		*/
		virtual PxProfileEventSender* getProfileEventSender() = 0;
	};
} }

#endif // PXPVDSDK_PXPROFILEEVENTSENDER_H
