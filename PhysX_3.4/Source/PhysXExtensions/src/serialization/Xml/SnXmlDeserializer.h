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
#ifndef PX_XML_DESERIALIZER_H
#define PX_XML_DESERIALIZER_H

#include "SnXmlVisitorReader.h"

namespace physx { namespace Sn {
	
	//Definitions needed internally in the Serializer headers.
	template<typename TTriIndexElem>
	struct Triangle
	{
		TTriIndexElem mIdx0;
		TTriIndexElem mIdx1;
		TTriIndexElem mIdx2;
		Triangle( TTriIndexElem inIdx0 = 0, TTriIndexElem inIdx1 = 0, TTriIndexElem inIdx2 = 0)
			: mIdx0( inIdx0 )
			, mIdx1( inIdx1 )
			, mIdx2( inIdx2 )
		{
		}
	};

	struct XmlMemoryAllocateMemoryPoolAllocator
	{
		XmlMemoryAllocator* mAllocator;
		XmlMemoryAllocateMemoryPoolAllocator( XmlMemoryAllocator* inAlloc ) : mAllocator( inAlloc ) {}

		PxU8* allocate( PxU32 inSize ) { return mAllocator->allocate( inSize ); }
		void deallocate( PxU8* inMem ) { mAllocator->deallocate( inMem ); }
	};

	inline bool isEmpty(const char *s)
	{
		while (*s != '\0')
		{
			if (!isspace(*s))
				return false;
			s++;
		}
		return true;
	}

	inline void strtoLong( Triangle<PxU32>& ioDatatype,const char*& ioData )
	{
		strto( ioDatatype.mIdx0, ioData );
		strto( ioDatatype.mIdx1, ioData );
		strto( ioDatatype.mIdx2, ioData );
	}

	inline void strtoLong( PxHeightFieldSample& ioDatatype,const char*& ioData )
	{
		PxU32 tempData;
		strto( tempData, ioData );
		if ( isBigEndian() )
		{
			PxU32& theItem(tempData);
			PxU32 theDest = 0;
			PxU8* theReadPtr( reinterpret_cast< PxU8* >( &theItem ) );
			PxU8* theWritePtr( reinterpret_cast< PxU8* >( &theDest ) );
			//A height field sample is a 16 bit number
			//followed by two bytes.

			//We write this out as a 32 bit integer, LE.
			//Thus, on a big endian, we need to move the bytes
			//around a bit.
			//LE - 1 2 3 4
			//BE - 4 3 2 1 - after convert from xml number
			//Correct BE - 2 1 3 4, just like LE but with the 16 number swapped
			theWritePtr[0] = theReadPtr[2];
			theWritePtr[1] = theReadPtr[3];
			theWritePtr[2] = theReadPtr[1];
			theWritePtr[3] = theReadPtr[0];
			theItem = theDest;
		}
		ioDatatype = *reinterpret_cast<PxHeightFieldSample*>( &tempData );
	}

	template<typename TDataType>
	inline void readStridedFlagsProperty( XmlReader& ioReader, const char* inPropName, TDataType*& outData, PxU32& outStride, PxU32& outCount, XmlMemoryAllocator& inAllocator,
		  const PxU32ToName* inConversions)
	{
		const char* theSrcData;
		outStride = sizeof( TDataType );
		outData = NULL;
		outCount = 0;
		if ( ioReader.read( inPropName, theSrcData ) )
		{
			XmlMemoryAllocateMemoryPoolAllocator tempAllocator( &inAllocator );
			MemoryBufferBase<XmlMemoryAllocateMemoryPoolAllocator> tempBuffer( &tempAllocator );

			if ( theSrcData )
			{
				static PxU32 theCount = 0;
				++theCount;
				char* theStartData = const_cast< char*>( copyStr( &tempAllocator, theSrcData ) );
				char* aData = strtok(theStartData, " \n");
				while( aData )
				{
					TDataType tempValue;
					stringToFlagsType( aData, inAllocator, tempValue, inConversions );						
					aData = strtok(NULL," \n");
					tempBuffer.write( &tempValue, sizeof(TDataType) );
				}
				outData = reinterpret_cast< TDataType* >( tempBuffer.mBuffer );
				outCount = tempBuffer.mWriteOffset / sizeof( TDataType );
				tempAllocator.deallocate( reinterpret_cast<PxU8*>(theStartData) );
			}
			tempBuffer.releaseBuffer();
		}
	}

	template<typename TDataType>
	inline void readStridedBufferProperty( XmlReader& ioReader, const char* inPropName, TDataType*& outData, PxU32& outStride, PxU32& outCount, XmlMemoryAllocator& inAllocator)
	{
		const char* theSrcData;
		outStride = sizeof( TDataType );
		outData = NULL;
		outCount = 0;
		if ( ioReader.read( inPropName, theSrcData ) )
		{
			XmlMemoryAllocateMemoryPoolAllocator tempAllocator( &inAllocator );
			MemoryBufferBase<XmlMemoryAllocateMemoryPoolAllocator> tempBuffer( &tempAllocator );

			if ( theSrcData )
			{
				static PxU32 theCount = 0;
				++theCount;
				char* theStartData = const_cast< char*>( copyStr( &tempAllocator, theSrcData ) );
				const char* theData = theStartData;
				while( !isEmpty(theData) )
				{
					//These buffers are whitespace delimited.
					TDataType theType;
					strtoLong( theType, theData );
					tempBuffer.write( &theType, sizeof(theType) );
				}
				outData = reinterpret_cast< TDataType* >( tempBuffer.mBuffer );
				outCount = tempBuffer.mWriteOffset / sizeof( TDataType );
				tempAllocator.deallocate( reinterpret_cast<PxU8*>(theStartData) );
			}
			tempBuffer.releaseBuffer();
		}
	}
	
	template<typename TDataType>
	inline void readStridedBufferProperty( XmlReader& ioReader, const char* inPropName, PxStridedData& ioData, PxU32& outCount, XmlMemoryAllocator& inAllocator)
	{
		TDataType* tempData = NULL;
		readStridedBufferProperty<TDataType>( ioReader, inPropName, tempData, ioData.stride, outCount, inAllocator ); 
		ioData.data = tempData;
	}
	
	template<typename TDataType>
	inline void readStridedBufferProperty( XmlReader& ioReader, const char* inPropName, PxTypedStridedData<TDataType>& ioData, PxU32& outCount, XmlMemoryAllocator& inAllocator)
	{
		TDataType* tempData = NULL;
		readStridedBufferProperty<TDataType>( ioReader, inPropName, tempData, ioData.stride, outCount, inAllocator );
		ioData.data = reinterpret_cast<PxMaterialTableIndex*>( tempData );
	}

	template<typename TDataType>
	inline void readStridedBufferProperty( XmlReader& ioReader, const char* inPropName, PxBoundedData& ioData, XmlMemoryAllocator& inAllocator)
	{
		return readStridedBufferProperty<TDataType>( ioReader, inPropName, ioData, ioData.count, inAllocator );
	}

} }

#endif
