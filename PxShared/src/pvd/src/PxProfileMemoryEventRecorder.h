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


#ifndef PXPVDSDK_PXPROFILEMEMORYEVENTRECORDER_H
#define PXPVDSDK_PXPROFILEMEMORYEVENTRECORDER_H


#include "PxProfileBase.h"
#include "PxProfileAllocatorWrapper.h"
#include "PxProfileMemoryEvents.h"
#include "PxProfileMemoryEventTypes.h"

#include "PsHashMap.h"
#include "PsUserAllocated.h"
#include "PsBroadcast.h"
#include "PxProfileMemory.h"

namespace physx { namespace profile {

	//Remember outstanding events.
	//Remembers allocations, forwards them to a listener if one is attached
	//and will forward all outstanding allocations to a listener when one is
	//attached.
	struct MemoryEventRecorder : public shdfnd::AllocationListener
	{
		typedef PxProfileWrapperReflectionAllocator<uint8_t> TAllocatorType;
		typedef shdfnd::HashMap<uint64_t,FullAllocationEvent,shdfnd::Hash<uint64_t>,TAllocatorType> THashMapType;

		PxProfileAllocatorWrapper		mWrapper;
		THashMapType			mOutstandingAllocations;
		AllocationListener*		mListener;

		MemoryEventRecorder( PxAllocatorCallback* inFoundation )
			: mWrapper( inFoundation )
			, mOutstandingAllocations( TAllocatorType( mWrapper ) )
			, mListener( NULL )
		{
		}

		static uint64_t ToU64( void* inData ) { return PX_PROFILE_POINTER_TO_U64( inData ); }
		static void* ToVoidPtr( uint64_t inData ) { return reinterpret_cast<void*>(size_t(inData)); }
		virtual void onAllocation( size_t size, const char* typeName, const char* filename, int line, void* allocatedMemory )
		{
			onAllocation( size, typeName, filename, uint32_t(line), ToU64( allocatedMemory ) );
		}
		
		void onAllocation( size_t size, const char* typeName, const char* filename, uint32_t line, uint64_t allocatedMemory )
		{
			if ( allocatedMemory == 0 )
				return;
			FullAllocationEvent theEvent;
			theEvent.init( size, typeName, filename, line, allocatedMemory );
			mOutstandingAllocations.insert( allocatedMemory, theEvent );
			if ( mListener != NULL ) mListener->onAllocation( size, typeName, filename, int(line), ToVoidPtr(allocatedMemory) );
		}
		
		virtual void onDeallocation( void* allocatedMemory )
		{
			onDeallocation( ToU64( allocatedMemory ) );
		}

		void onDeallocation( uint64_t allocatedMemory )
		{
			if ( allocatedMemory == 0 )
				return;
			mOutstandingAllocations.erase( allocatedMemory );
			if ( mListener != NULL ) mListener->onDeallocation( ToVoidPtr( allocatedMemory ) );
		}

		void flushProfileEvents() {}

		void setListener( AllocationListener* inListener )
		{
			mListener = inListener;
			if ( mListener )
			{	
				for ( THashMapType::Iterator iter = mOutstandingAllocations.getIterator();
					!iter.done();
					++iter )
				{
					const FullAllocationEvent& evt( iter->second );
					mListener->onAllocation( evt.mSize, evt.mType, evt.mFile, int(evt.mLine), ToVoidPtr( evt.mAddress ) );
				}
			}
		}
	};

	class PxProfileMemoryEventRecorderImpl : public shdfnd::UserAllocated
											, public physx::profile::PxProfileMemoryEventRecorder
	{
		MemoryEventRecorder mRecorder;
	public:
		PxProfileMemoryEventRecorderImpl( PxAllocatorCallback* inFnd )
			: mRecorder( inFnd )
		{
		}

		virtual void onAllocation( size_t size, const char* typeName, const char* filename, int line, void* allocatedMemory )
		{
			mRecorder.onAllocation( size, typeName, filename, line, allocatedMemory );
		}

		virtual void onDeallocation( void* allocatedMemory )
		{
			mRecorder.onDeallocation( allocatedMemory );
		}
		
		virtual void setListener( AllocationListener* inListener )
		{
			mRecorder.setListener( inListener );
		}

		virtual void release()
		{
			PX_PROFILE_DELETE( mRecorder.mWrapper.getAllocator(), this );
		}
	};

}}
#endif // PXPVDSDK_PXPROFILEMEMORYEVENTRECORDER_H
