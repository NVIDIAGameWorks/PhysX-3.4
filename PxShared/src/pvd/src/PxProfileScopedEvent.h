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

#ifndef PXPVDSDK_PXPROFILESCOPEDEVENT_H
#define PXPVDSDK_PXPROFILESCOPEDEVENT_H

#include "PxProfileBase.h"
#include "PxProfileEventId.h"
#include "PxProfileCompileTimeEventFilter.h"

namespace physx { namespace profile {

#define TO_PXPVDSDK_PXPROFILEEVENTID( subsystem, eventId ) PxProfileEventId( SubsystemIds::subsystem, EventIds::subsystem##eventId );

	/**
	\brief Template version of startEvent, called directly on provided profile buffer.

	\param inBuffer Profile event buffer.
	\param inId Profile event id.
	\param inContext Profile event context.
	*/
	template<bool TEnabled, typename TBufferType>
	inline void startEvent( TBufferType* inBuffer, const PxProfileEventId& inId, uint64_t inContext )
	{
		if ( TEnabled && inBuffer ) inBuffer->startEvent( inId, inContext );
	}

	/**
	\brief Template version of stopEvent, called directly on provided profile buffer.

	\param inBuffer Profile event buffer.
	\param inId Profile event id.
	\param inContext Profile event context.
	*/
	template<bool TEnabled, typename TBufferType>
	inline void stopEvent( TBufferType* inBuffer, const PxProfileEventId& inId, uint64_t inContext )
	{
		if ( TEnabled && inBuffer ) inBuffer->stopEvent( inId, inContext );
	}
	
	/**
	\brief Template version of startEvent, called directly on provided profile buffer.

	\param inEnabled If profile event is enabled.
	\param inBuffer Profile event buffer.
	\param inId Profile event id.
	\param inContext Profile event context.
	*/
	template<typename TBufferType>
	inline void startEvent( bool inEnabled, TBufferType* inBuffer, const PxProfileEventId& inId, uint64_t inContext )
	{
		if ( inEnabled && inBuffer ) inBuffer->startEvent( inId, inContext );
	}

	/**
	\brief Template version of stopEvent, called directly on provided profile buffer.

	\param inEnabled If profile event is enabled.
	\param inBuffer Profile event buffer.
	\param inId Profile event id.
	\param inContext Profile event context.
	*/
	template<typename TBufferType>
	inline void stopEvent( bool inEnabled, TBufferType* inBuffer, const PxProfileEventId& inId, uint64_t inContext )
	{
		if ( inEnabled && inBuffer ) inBuffer->stopEvent( inId, inContext );
	}
	
	/**
	\brief Template version of eventValue, called directly on provided profile buffer.

	\param inEnabled If profile event is enabled.
	\param inBuffer Profile event buffer.
	\param inId Profile event id.
	\param inContext Profile event context.
	\param inValue Event value.
	*/
	template<typename TBufferType>
	inline void eventValue( bool inEnabled, TBufferType* inBuffer, const PxProfileEventId& inId, uint64_t inContext, int64_t inValue )
	{
		if ( inEnabled && inBuffer ) inBuffer->eventValue( inId, inContext, inValue );
	}

	template<bool TEnabled, typename TBufferType, uint16_t eventId>
	struct ScopedEventWithContext
	{
		uint64_t				mContext;
		TBufferType*		mBuffer;
		ScopedEventWithContext( TBufferType* inBuffer, uint64_t inContext)
			: mContext ( inContext )
			, mBuffer( inBuffer )
		{
			startEvent<true>( mBuffer, PxProfileEventId(eventId), mContext );
		}
		~ScopedEventWithContext()
		{
			stopEvent<true>( mBuffer, PxProfileEventId(eventId), mContext );
		}
	};

	template<typename TBufferType, uint16_t eventId>
	struct ScopedEventWithContext<false,TBufferType,eventId> { ScopedEventWithContext( TBufferType*, uint64_t) {} };

	template<typename TBufferType>
	struct DynamicallyEnabledScopedEvent
	{
		TBufferType*		mBuffer;
		PxProfileEventId	mId;
		uint64_t				mContext;
		DynamicallyEnabledScopedEvent( TBufferType* inBuffer, const PxProfileEventId& inId, uint64_t inContext)
			: mBuffer( inBuffer )
			, mId( inId )
			, mContext( inContext )
		{
			if(mBuffer)
				startEvent( mId.compileTimeEnabled, mBuffer, mId, mContext );
		}
		~DynamicallyEnabledScopedEvent()
		{
			if(mBuffer)
				stopEvent( mId.compileTimeEnabled, mBuffer, mId, mContext );
		}
	};
}}

#endif // PXPVDSDK_PXPROFILESCOPEDEVENT_H
