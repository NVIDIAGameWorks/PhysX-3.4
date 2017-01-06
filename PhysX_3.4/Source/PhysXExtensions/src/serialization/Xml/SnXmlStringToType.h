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
#ifndef PX_XML_STRINGTOTYPE_H
#define PX_XML_STRINGTOTYPE_H

#include <stdio.h>
#include <ctype.h>
#include "PsString.h"
#include "PxCoreUtilityTypes.h"
#include "PxFiltering.h"

//Remapping function name for gcc-based systems.
#ifndef _MSC_VER
#define _strtoui64 strtoull
#endif


namespace physx { namespace Sn {

	template<typename TDataType>
	struct StrToImpl
	{
		bool compile_error;
	};

	template<> struct StrToImpl<PxU64> { 
		//Id's (void ptrs) are written to file as unsigned
		//64 bit integers, so this method gets called more
		//often than one might think.
		PX_INLINE void strto( PxU64& ioDatatype,const char*& ioData )
		{
			ioDatatype = _strtoui64( ioData, const_cast<char **>(&ioData), 10 );
		}
	};

	PX_INLINE PxF32 strToFloat(const char *str,const char **nextScan)
	{
		PxF32 ret;
		while ( *str && isspace(static_cast<unsigned char>(*str))) str++; // skip leading whitespace
		char temp[256] = "";
		char *dest = temp;
		char *end = &temp[255];
		const char *begin = str;
		while ( *str && !isspace(static_cast<unsigned char>(*str)) && dest < end ) // copy the number up to the first whitespace or eos
		{
			*dest++ = *str++;
		}
		*dest = 0;
		ret = PxF32(strtod(temp,&end));
		if ( nextScan )
		{
			*nextScan = begin+(end-temp);
		}
		return ret;
	}
	

	template<> struct StrToImpl<PxU32> { 
	PX_INLINE void strto( PxU32& ioDatatype,const char*& ioData )
	{
		ioDatatype = static_cast<PxU32>( strtoul( ioData,const_cast<char **>(&ioData), 10 ) );
	}
	};

	template<> struct StrToImpl<PxI32> { 
	PX_INLINE void strto( PxI32& ioDatatype,const char*& ioData )
	{
		ioDatatype = static_cast<PxI32>( strtoul( ioData,const_cast<char **>(&ioData), 10 ) );
	}
	};


	template<> struct StrToImpl<PxU16> {
	PX_INLINE void strto( PxU16& ioDatatype,const char*& ioData )
	{
		ioDatatype = static_cast<PxU16>( strtoul( ioData,const_cast<char **>(&ioData), 10 ) );
	}
	};

	PX_INLINE void eatwhite(const char*& ioData )
	{
		if ( ioData )
		{
			while( isspace( static_cast<unsigned char>(*ioData) ) )
				++ioData;
		}
	}

	// copy the source data to the dest buffer until the first whitespace is encountered.
	// Do not overflow the buffer based on the bufferLen provided.
	// Advance the input 'ioData' pointer so that it sits just at the next whitespace
	PX_INLINE void nullTerminateWhite(const char*& ioData,char *buffer,PxU32 bufferLen)
	{
		if ( ioData )
		{
			char *eof = buffer+(bufferLen-1);
			char *dest = buffer;
			while( *ioData && !isspace(static_cast<unsigned char>(*ioData)) && dest < eof )
			{
				*dest++ = *ioData++;
			}
			*dest = 0;
		}
	}

	inline void nonNullTerminateWhite(const char*& ioData )
	{
		if ( ioData )
		{
			while( *ioData && !isspace( static_cast<unsigned char>(*ioData) ) )
				++ioData;
		}
	}
	
	template<> struct StrToImpl<PxF32> {
	inline void strto( PxF32& ioDatatype,const char*& ioData )
	{
		ioDatatype = strToFloat(ioData,&ioData); 
	}
	
	};

	
	template<> struct StrToImpl<void*> {
	inline void strto( void*& ioDatatype,const char*& ioData )
	{
		PxU64 theData;
		StrToImpl<PxU64>().strto( theData, ioData );
		ioDatatype = reinterpret_cast<void*>( static_cast<size_t>( theData ) );
	}
	};
	

	template<> struct StrToImpl<physx::PxVec3> {
	inline void strto( physx::PxVec3& ioDatatype,const char*& ioData )
	{
		StrToImpl<PxF32>().strto( ioDatatype[0], ioData );
		StrToImpl<PxF32>().strto( ioDatatype[1], ioData );
		StrToImpl<PxF32>().strto( ioDatatype[2], ioData );
	}
	};
	
	template<> struct StrToImpl<PxU8*> {
	inline void strto( PxU8*& /*ioDatatype*/,const char*& /*ioData*/)
	{
	}
	};

	template<> struct StrToImpl<bool> {
	inline void strto( bool& ioType,const char*& inValue )
	{
		ioType = physx::shdfnd::stricmp( inValue, "true" ) == 0 ? true : false;
	}
	};
	
	template<> struct StrToImpl<PxU8> {
	PX_INLINE void strto( PxU8& ioType,const char* & inValue)
	{
		ioType = static_cast<PxU8>( strtoul( inValue,const_cast<char **>(&inValue), 10 ) );
	}
	};

	template<> struct StrToImpl<PxFilterData> {
	PX_INLINE void strto( PxFilterData& ioType,const char*& inValue)
	{
		ioType.word0 = static_cast<PxU32>( strtoul( inValue,const_cast<char **>(&inValue), 10 ) );
		ioType.word1 = static_cast<PxU32>( strtoul( inValue,const_cast<char **>(&inValue), 10 ) );
		ioType.word2 = static_cast<PxU32>( strtoul( inValue,const_cast<char **>(&inValue), 10 ) );
		ioType.word3 = static_cast<PxU32>( strtoul( inValue, NULL, 10 ) );
	}
	};
	

	template<> struct StrToImpl<PxQuat> {
	PX_INLINE void strto( PxQuat& ioType,const char*& inValue )
	{
		ioType.x = static_cast<PxReal>( strToFloat( inValue, &inValue ) );
		ioType.y = static_cast<PxReal>( strToFloat( inValue, &inValue ) );
		ioType.z = static_cast<PxReal>( strToFloat( inValue, &inValue ) );
		ioType.w = static_cast<PxReal>( strToFloat( inValue, &inValue ) );
	}
	};
	
	template<> struct StrToImpl<PxTransform> {
	PX_INLINE void strto( PxTransform& ioType,const char*& inValue)
	{
		ioType.q.x = static_cast<PxReal>( strToFloat( inValue, &inValue ) );
		ioType.q.y = static_cast<PxReal>( strToFloat( inValue, &inValue ) );
		ioType.q.z = static_cast<PxReal>( strToFloat( inValue, &inValue ) );
		ioType.q.w = static_cast<PxReal>( strToFloat( inValue, &inValue ) );

		ioType.p[0] = static_cast<PxReal>( strToFloat( inValue, &inValue ) );
		ioType.p[1] = static_cast<PxReal>( strToFloat( inValue, &inValue ) );
		ioType.p[2] = static_cast<PxReal>( strToFloat( inValue, &inValue ) );
	}
	};

	template<> struct StrToImpl<PxBounds3> {
	PX_INLINE void strto( PxBounds3& ioType,const char*& inValue)
	{
		ioType.minimum[0] = static_cast<PxReal>( strToFloat( inValue, &inValue ) );
		ioType.minimum[1] = static_cast<PxReal>( strToFloat( inValue, &inValue ) );
		ioType.minimum[2] = static_cast<PxReal>( strToFloat( inValue, &inValue ) );

		ioType.maximum[0] = static_cast<PxReal>( strToFloat( inValue, &inValue ) );
		ioType.maximum[1] = static_cast<PxReal>( strToFloat( inValue, &inValue ) );
		ioType.maximum[2] = static_cast<PxReal>( strToFloat( inValue, &inValue ) );
	}
	};
	
	template<> struct StrToImpl<PxMetaDataPlane> {
	PX_INLINE void strto( PxMetaDataPlane& ioType,const char*& inValue)
	{
		ioType.normal.x = static_cast<PxReal>( strToFloat( inValue, &inValue ) );
		ioType.normal.y = static_cast<PxReal>( strToFloat( inValue, &inValue ) );
		ioType.normal.z = static_cast<PxReal>( strToFloat( inValue, &inValue ) );
		ioType.distance = static_cast<PxReal>( strToFloat( inValue, &inValue ) );		
	}
	};

	
	template<> struct StrToImpl<PxRigidDynamic*> { 
		PX_INLINE void strto( PxRigidDynamic*& /*ioDatatype*/,const char*& /*ioData*/)
		{
		}
	};

	template<typename TDataType>
	inline void strto( TDataType& ioType,const char*& ioData )
	{
		if ( ioData && *ioData ) StrToImpl<TDataType>().strto( ioType, ioData );
	}

	template<typename TDataType>
	inline void strtoLong( TDataType& ioType,const char*& ioData )
	{
		if ( ioData && *ioData ) StrToImpl<TDataType>().strto( ioType, ioData );
	}

	template<typename TDataType>
	inline void stringToType( const char* inValue, TDataType& ioType )
	{
		const char* theValue( inValue );
		return strto( ioType, theValue );
	}

} }

#endif
