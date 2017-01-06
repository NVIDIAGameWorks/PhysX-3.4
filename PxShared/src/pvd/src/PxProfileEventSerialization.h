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


#ifndef PXPVDSDK_PXPROFILEEVENTSERIALIZATION_H
#define PXPVDSDK_PXPROFILEEVENTSERIALIZATION_H

#include "PxProfileBase.h"
#include "PxProfileDataParsing.h"
#include "PxProfileEvents.h"

namespace physx { namespace profile {

	/**
	 *	Array type must be a pxu8 container.  Templated so that this object can write
	 *	to different collections.
	 */
	
	template<typename TArrayType>
	struct EventSerializer
	{
		TArrayType* mArray;
		EventSerializer( TArrayType* inA ) : mArray( inA ) {}

		template<typename TDataType>
		uint32_t streamify( const char*, const TDataType& inType )
		{
			return mArray->write( inType );
		}

		uint32_t streamify( const char*, const char*& inType )
		{
			PX_ASSERT( inType != NULL );
			uint32_t len( static_cast<uint32_t>( strlen( inType ) ) );
			++len; //include the null terminator
			uint32_t writtenSize = 0;
			writtenSize = mArray->write(len);
			writtenSize += mArray->write(inType, len);
			return writtenSize;
		}
		
		uint32_t streamify( const char*, const uint8_t* inData, uint32_t len )
		{
			uint32_t writtenSize = mArray->write(len);
			if ( len )
				writtenSize += mArray->write(inData, len);
			return writtenSize;
		}

		uint32_t streamify( const char* nm, const uint64_t& inType, EventStreamCompressionFlags::Enum inFlags )
		{
			uint32_t writtenSize = 0;
			switch( inFlags )
			{
			case EventStreamCompressionFlags::U8:
					writtenSize = streamify(nm, static_cast<uint8_t>(inType));
					break;
			case EventStreamCompressionFlags::U16:
					writtenSize = streamify(nm, static_cast<uint16_t>(inType));
					break;
			case EventStreamCompressionFlags::U32:
					writtenSize = streamify(nm, static_cast<uint32_t>(inType));
					break;
			case EventStreamCompressionFlags::U64:
				writtenSize = streamify(nm, inType);
				break;
			}
			return writtenSize;
		}
		
		uint32_t streamify( const char* nm, const uint32_t& inType, EventStreamCompressionFlags::Enum inFlags )
		{
			uint32_t writtenSize = 0;
			switch( inFlags )
			{
			case EventStreamCompressionFlags::U8:
					writtenSize = streamify(nm, static_cast<uint8_t>(inType));
					break;
			case EventStreamCompressionFlags::U16:
					writtenSize = streamify(nm, static_cast<uint16_t>(inType));
					break;
			case EventStreamCompressionFlags::U32:
			case EventStreamCompressionFlags::U64:
				writtenSize = streamify(nm, inType);
				break;
			}
			return writtenSize;
		}
	};

	/**
	 *	The event deserializes takes a buffer implements the streamify functions
	 *	by setting the passed in data to the data in the buffer.
	 */	
	template<bool TSwapBytes>
	struct EventDeserializer
	{
		const uint8_t* mData;
		uint32_t		mLength;
		bool		mFail;

		EventDeserializer( const uint8_t* inData,  uint32_t inLength )
			: mData( inData )
			, mLength( inLength )
			, mFail( false )
		{
			if ( mData == NULL )
				mLength = 0;
		}

		bool val() { return TSwapBytes; }

		uint32_t streamify( const char* , uint8_t& inType )
		{
			uint8_t* theData = reinterpret_cast<uint8_t*>( &inType ); //type punned pointer...
			if ( mFail || sizeof( inType ) > mLength )
			{
				PX_ASSERT( false );
				mFail = true;
			}
			else
			{
				for( uint32_t idx = 0; idx < sizeof( uint8_t ); ++idx, ++mData, --mLength )
					theData[idx] = *mData;
			}
			return 0;
		}

		//default streamify reads things natively as bytes.
		template<typename TDataType>
		uint32_t streamify( const char* , TDataType& inType )
		{
			uint8_t* theData = reinterpret_cast<uint8_t*>( &inType ); //type punned pointer...
			if ( mFail || sizeof( inType ) > mLength )
			{
				PX_ASSERT( false );
				mFail = true;
			}
			else
			{
				for( uint32_t idx = 0; idx < sizeof( TDataType ); ++idx, ++mData, --mLength )
					theData[idx] = *mData;
				bool temp = val();
				if ( temp ) 
					BlockParseFunctions::swapBytes<sizeof(TDataType)>( theData );
			}
			return 0;
		}

		uint32_t streamify( const char*, const char*& inType )
		{
			uint32_t theLen;
			streamify( "", theLen );
			theLen = PxMin( theLen, mLength );
			inType = reinterpret_cast<const char*>( mData );
			mData += theLen;
			mLength -= theLen;
			return 0;
		}
		
		uint32_t streamify( const char*, const uint8_t*& inData, uint32_t& len )
		{
			uint32_t theLen;
			streamify( "", theLen );
			theLen = PxMin( theLen, mLength );
			len = theLen;
			inData = reinterpret_cast<const uint8_t*>( mData );
			mData += theLen;
			mLength -= theLen;
			return 0;
		}

		uint32_t streamify( const char* nm, uint64_t& inType, EventStreamCompressionFlags::Enum inFlags )
		{
			switch( inFlags )
			{
			case EventStreamCompressionFlags::U8:
				{
					uint8_t val=0;
					streamify( nm, val );
					inType = val;
				}
					break;
			case EventStreamCompressionFlags::U16:
				{
					uint16_t val;
					streamify( nm, val );
					inType = val;
				}
					break;
			case EventStreamCompressionFlags::U32:
				{
					uint32_t val;
					streamify( nm, val );
					inType = val;
				}
					break;
			case EventStreamCompressionFlags::U64:
				streamify( nm, inType );
				break;
			}
			return 0;
		}
		
		uint32_t streamify( const char* nm, uint32_t& inType, EventStreamCompressionFlags::Enum inFlags )
		{
			switch( inFlags )
			{
			case EventStreamCompressionFlags::U8:
				{
					uint8_t val=0;
					streamify( nm, val );
					inType = val;
				}
					break;
			case EventStreamCompressionFlags::U16:
				{
					uint16_t val=0;
					streamify( nm, val );
					inType = val;
				}
					break;
			case EventStreamCompressionFlags::U32:
			case EventStreamCompressionFlags::U64:
				streamify( nm, inType );
				break;
			}
			return 0;
		}
	};
}}
#endif // PXPVDSDK_PXPROFILEEVENTSERIALIZATION_H
