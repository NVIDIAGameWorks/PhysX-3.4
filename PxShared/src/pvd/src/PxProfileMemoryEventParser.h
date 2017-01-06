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


#ifndef PXPVDSDK_PXPROFILEMEMORYEVENTPARSER_H
#define PXPVDSDK_PXPROFILEMEMORYEVENTPARSER_H

#include "PxProfileMemoryEvents.h"
#include "PxProfileAllocatorWrapper.h"
#include "PxProfileEventSerialization.h"

#include "PsHashMap.h"
#include "PsString.h"

namespace physx { namespace profile {

	template<bool TSwapBytes, typename TParserType, typename THandlerType> 
	bool parseEventData( TParserType& inParser, const uint8_t* inData, uint32_t inLength, THandlerType* inHandler );

	template<bool TSwapBytes>
	struct MemoryEventParser
	{
		typedef PxProfileWrapperReflectionAllocator<uint8_t> TAllocatorType;
		typedef shdfnd::HashMap<uint32_t, char*, shdfnd::Hash<uint32_t>, TAllocatorType > THdlToStringMap;
		typedef EventDeserializer<TSwapBytes>	TDeserializerType;
		
		PxProfileAllocatorWrapper	mWrapper;
		THdlToStringMap		mHdlToStringMap;
		TDeserializerType	mDeserializer;

		MemoryEventParser( PxAllocatorCallback& inAllocator )
			: mWrapper( inAllocator )
			, mHdlToStringMap( TAllocatorType( mWrapper ) )
			, mDeserializer ( 0, 0 )
		{
		}

		~MemoryEventParser()
		{
			for ( THdlToStringMap::Iterator iter( mHdlToStringMap.getIterator() ); iter.done() == false; ++iter )
				mWrapper.getAllocator().deallocate( reinterpret_cast<void*>(iter->second) );
		}

		template<typename TOperator>
		void parse(const StringTableEvent&, const MemoryEventHeader& inHeader, TOperator& inOperator)
		{
			StringTableEvent evt;
			evt.init();
			evt.streamify( mDeserializer, inHeader );
			uint32_t len = static_cast<uint32_t>( strlen( evt.mString ) );
			char* newStr = static_cast<char*>( mWrapper.getAllocator().allocate( len + 1, "const char*", __FILE__, __LINE__ ) );
			shdfnd::strlcpy( newStr, len+1, evt.mString );
			mHdlToStringMap[evt.mHandle] = newStr;
			inOperator( inHeader, evt );
		}

		const char* getString( uint32_t inHdl )
		{
			const THdlToStringMap::Entry* entry = mHdlToStringMap.find( inHdl );
			if ( entry ) return entry->second;
			return "";
		}

		//Slow reverse lookup used only for testing.
		uint32_t getHandle( const char* inStr )
		{
			for ( THdlToStringMap::Iterator iter = mHdlToStringMap.getIterator();
				!iter.done();
				++iter )
			{
				if ( safeStrEq( iter->second, inStr ) )
					return iter->first;
			}
			return 0;
		}

		template<typename TOperator>
		void parse(const AllocationEvent&, const MemoryEventHeader& inHeader, TOperator& inOperator)
		{
			AllocationEvent evt;
			evt.streamify( mDeserializer, inHeader );
			inOperator( inHeader, evt );
		}

		template<typename TOperator>
		void parse(const DeallocationEvent&, const MemoryEventHeader& inHeader, TOperator& inOperator)
		{
			DeallocationEvent evt;
			evt.streamify( mDeserializer, inHeader );
			inOperator( inHeader, evt );
		}

		template<typename TOperator>
		void parse(const FullAllocationEvent&, const MemoryEventHeader&, TOperator& )
		{
			PX_ASSERT( false ); //will never happen.
		}

		template<typename THandlerType>
		void parseEventData( const uint8_t* inData, uint32_t inLength, THandlerType* inOperator )
		{
			physx::profile::parseEventData<TSwapBytes>( *this, inData, inLength, inOperator );
		}
	};
	

	template<typename THandlerType, bool TSwapBytes>
	struct MemoryEventParseOperator
	{
		MemoryEventParser<TSwapBytes>* mParser;
		THandlerType* mOperator;
		MemoryEventHeader* mHeader;
		MemoryEventParseOperator( MemoryEventParser<TSwapBytes>* inParser, THandlerType* inOperator, MemoryEventHeader* inHeader )
			: mParser( inParser )
			, mOperator( inOperator )
			, mHeader( inHeader )
		{
		}

		bool wasSuccessful() { return mParser->mDeserializer.mFail == false; }

		bool parseHeader()
		{
			mHeader->streamify( mParser->mDeserializer );
			return wasSuccessful();
		}

		template<typename TDataType>
		bool operator()( const TDataType& inType )
		{
			mParser->parse( inType, *mHeader, *mOperator );
			return wasSuccessful();
		}
		
		bool operator()( uint8_t ) { PX_ASSERT( false ); return false;}
	};

	template<bool TSwapBytes, typename TParserType, typename THandlerType> 
	inline bool parseEventData( TParserType& inParser, const uint8_t* inData, uint32_t inLength, THandlerType* inHandler )
	{
		inParser.mDeserializer = EventDeserializer<TSwapBytes>( inData, inLength );
		MemoryEvent::EventData crapData;
		uint32_t eventCount = 0;
		MemoryEventHeader theHeader;
		MemoryEventParseOperator<THandlerType, TSwapBytes> theOp( &inParser, inHandler, &theHeader );
		while( inParser.mDeserializer.mLength && inParser.mDeserializer.mFail == false)
		{
			if ( theOp.parseHeader() )
			{
				if( visit<bool>( theHeader.getType(), crapData, theOp ) == false )
					inParser.mDeserializer.mFail = true;
			}
			++eventCount;
		}
		return inParser.mDeserializer.mFail == false;
	}
}}

#endif // PXPVDSDK_PXPROFILEMEMORYEVENTPARSER_H
