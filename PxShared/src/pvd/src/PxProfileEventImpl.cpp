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

#include "foundation/PxErrorCallback.h"
#include "foundation/PxAllocatorCallback.h"

#include "PxProfileEvents.h"
#include "PxProfileEventSerialization.h"
#include "PxProfileEventBuffer.h"
#include "PxProfileZoneImpl.h"
#include "PxProfileZoneManagerImpl.h"
#include "PxProfileEventParser.h"
#include "PxProfileEventHandler.h"
#include "PxProfileScopedMutexLock.h"
#include "PxProfileEventFilter.h"
#include "PxProfileContextProvider.h"
#include "PxProfileEventMutex.h"
#include "PxProfileMemoryEventTypes.h"
#include "PxProfileMemoryEventRecorder.h"
#include "PxProfileMemoryEventBuffer.h"
#include "PxProfileMemoryEventParser.h"
#include "PxProfileContextProviderImpl.h"

#include "PsUserAllocated.h"
#include "PsTime.h"

#include <stdio.h>

namespace physx { namespace profile {


	uint64_t PxProfileEventHandler::durationToNanoseconds(uint64_t duration)
	{
		return shdfnd::Time::getBootCounterFrequency().toTensOfNanos(duration) * 10;
	}

	void PxProfileEventHandler::parseEventBuffer( const uint8_t* inBuffer, uint32_t inBufferSize, PxProfileEventHandler& inHandler, bool inSwapBytes )
	{
		if ( inSwapBytes == false )
			parseEventData<false>( inBuffer, inBufferSize, &inHandler );
		else
			parseEventData<true>( inBuffer, inBufferSize, &inHandler );
	}

	template<uint32_t TNumEvents>
	struct ProfileBulkEventHandlerBuffer
	{
		Event mEvents[TNumEvents];
		uint32_t mEventCount;
		PxProfileBulkEventHandler* mHandler;
		ProfileBulkEventHandlerBuffer( PxProfileBulkEventHandler* inHdl )
			: mEventCount( 0 )
			, mHandler( inHdl )
		{
		}
		void onEvent( const Event& inEvent )
		{
			mEvents[mEventCount] = inEvent;
			++mEventCount;
			if ( mEventCount == TNumEvents )
				flush();
		}
		void onEvent( const PxProfileEventId& inId, uint32_t threadId, uint64_t contextId, uint8_t cpuId, uint8_t threadPriority, uint64_t timestamp, EventTypes::Enum inType )
		{
			StartEvent theEvent;
			theEvent.init( threadId, contextId, cpuId, static_cast<uint8_t>( threadPriority ), timestamp );
			onEvent( Event( EventHeader( static_cast<uint8_t>( inType ), inId.eventId ), theEvent ) );
		}
		void onStartEvent( const PxProfileEventId& inId, uint32_t threadId, uint64_t contextId, uint8_t cpuId, uint8_t threadPriority, uint64_t timestamp )
		{
			onEvent( inId, threadId, contextId, cpuId, threadPriority, timestamp, EventTypes::StartEvent );
		}
		void onStopEvent( const PxProfileEventId& inId, uint32_t threadId, uint64_t contextId, uint8_t cpuId, uint8_t threadPriority, uint64_t timestamp )
		{
			onEvent( inId, threadId, contextId, cpuId, threadPriority, timestamp, EventTypes::StopEvent );
		}
		void onEventValue( const PxProfileEventId& inId, uint32_t threadId, uint64_t contextId, int64_t value )
		{
			EventValue theEvent;
			theEvent.init( value, contextId, threadId );
			onEvent( Event( inId.eventId, theEvent ) );
		}
		void flush()
		{
			if ( mEventCount )
				mHandler->handleEvents( mEvents, mEventCount );
			mEventCount = 0;
		}
	};


	void PxProfileBulkEventHandler::parseEventBuffer( const uint8_t* inBuffer, uint32_t inBufferSize, PxProfileBulkEventHandler& inHandler, bool inSwapBytes )
	{
		ProfileBulkEventHandlerBuffer<256> hdler( &inHandler );
		if ( inSwapBytes )
			parseEventData<true>( inBuffer, inBufferSize, &hdler );
		else
			parseEventData<false>( inBuffer, inBufferSize, &hdler );
		hdler.flush();
	}

	struct PxProfileNameProviderImpl
	{
		PxProfileNameProvider* mImpl;
		PxProfileNameProviderImpl( PxProfileNameProvider* inImpl )
			: mImpl( inImpl )
		{
		}
		PxProfileNames getProfileNames() const { return mImpl->getProfileNames(); }
	};

	
	struct PxProfileNameProviderForward
	{
		PxProfileNames mNames;
		PxProfileNameProviderForward( PxProfileNames inNames )
			: mNames( inNames )
		{
		}
		PxProfileNames getProfileNames() const { return mNames; }
	};

	
	PX_FOUNDATION_API PxProfileZone& PxProfileZone::createProfileZone( PxAllocatorCallback* inAllocator, const char* inSDKName, PxProfileNames inNames, uint32_t inEventBufferByteSize )
	{
		typedef ZoneImpl<PxProfileNameProviderForward> TSDKType;
		return *PX_PROFILE_NEW( inAllocator, TSDKType ) ( inAllocator, inSDKName, inEventBufferByteSize, PxProfileNameProviderForward( inNames ) );
	}
	
	PxProfileZoneManager& PxProfileZoneManager::createProfileZoneManager(PxAllocatorCallback* inAllocator )
	{
		return *PX_PROFILE_NEW( inAllocator, ZoneManagerImpl ) ( inAllocator );
	}

	PxProfileMemoryEventRecorder& PxProfileMemoryEventRecorder::createRecorder( PxAllocatorCallback* inAllocator )
	{
		return *PX_PROFILE_NEW( inAllocator, PxProfileMemoryEventRecorderImpl )( inAllocator );
	}
	
	PxProfileMemoryEventBuffer& PxProfileMemoryEventBuffer::createMemoryEventBuffer( PxAllocatorCallback& inAllocator, uint32_t inBufferSize )
	{
		return *PX_PROFILE_NEW( &inAllocator, PxProfileMemoryEventBufferImpl )( inAllocator, inBufferSize );
	}
	template<uint32_t TNumEvents>
	struct ProfileBulkMemoryEventHandlerBuffer
	{
		PxProfileBulkMemoryEvent mEvents[TNumEvents];
		uint32_t mEventCount;
		PxProfileBulkMemoryEventHandler* mHandler;
		ProfileBulkMemoryEventHandlerBuffer( PxProfileBulkMemoryEventHandler* inHdl )
			: mEventCount( 0 )
			, mHandler( inHdl )
		{
		}
		void onEvent( const PxProfileBulkMemoryEvent& evt )
		{
			mEvents[mEventCount] = evt;
			++mEventCount;
			if ( mEventCount == TNumEvents )
				flush();
		}

		template<typename TDataType>
		void operator()( const MemoryEventHeader&, const TDataType& ) {}

		void operator()( const MemoryEventHeader&, const AllocationEvent& evt )
		{
			onEvent( PxProfileBulkMemoryEvent( evt.mSize, evt.mType, evt.mFile, evt.mLine, evt.mAddress ) );
		}

		void operator()( const MemoryEventHeader&, const DeallocationEvent& evt )
		{
			onEvent( PxProfileBulkMemoryEvent( evt.mAddress ) );
		}

		void flush()
		{
			if ( mEventCount )
				mHandler->handleEvents( mEvents, mEventCount );
			mEventCount = 0;
		}
	};

	void PxProfileBulkMemoryEventHandler::parseEventBuffer( const uint8_t* inBuffer, uint32_t inBufferSize, PxProfileBulkMemoryEventHandler& inHandler, bool inSwapBytes, PxAllocatorCallback* inAlloc )
	{
		PX_ASSERT(inAlloc);

		ProfileBulkMemoryEventHandlerBuffer<0x1000>* theBuffer = PX_PROFILE_NEW(inAlloc, ProfileBulkMemoryEventHandlerBuffer<0x1000>)(&inHandler);

		if ( inSwapBytes )
		{			
			MemoryEventParser<true> theParser( *inAlloc );
			theParser.parseEventData( inBuffer, inBufferSize, theBuffer );
		}
		else
		{
			MemoryEventParser<false> theParser( *inAlloc );
			theParser.parseEventData( inBuffer, inBufferSize, theBuffer );
		}
		theBuffer->flush();

		PX_PROFILE_DELETE(*inAlloc, theBuffer);
	}

} }

