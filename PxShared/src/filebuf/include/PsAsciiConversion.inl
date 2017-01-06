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

/*!
\file
\brief NvAsciiConversion namespace contains string/value helper functions
*/

#include <ctype.h>

PX_INLINE bool isWhiteSpace(char c)
{
	bool ret = false;
	if ( c == 32 || c == 9 || c == 13 || c == 10 || c == ',' ) ret = true;
	return ret;
}

PX_INLINE const char * skipNonWhiteSpace(const char *scan)
{
	while ( !isWhiteSpace(*scan) && *scan) scan++;
	if ( *scan == 0 ) scan = NULL;
	return scan;
}
PX_INLINE const char * skipWhiteSpace(const char *scan)
{
	while ( isWhiteSpace(*scan) && *scan ) scan++;
	if ( *scan == 0 ) scan = NULL;
	return scan;
}

static double strtod_fast(const char * pString)
{
    //---
    // Find the start of the string
	const char* pNumberStart = skipWhiteSpace(pString);

    //---
    // Find the end of the string
    const char* pNumberEnd = pNumberStart;

    // skip optional sign
    if( *pNumberEnd == '-' || *pNumberEnd == '+' )
        ++pNumberEnd;

    // skip optional digits
    while( isdigit(*pNumberEnd) )
        ++pNumberEnd;

    // skip optional decimal and digits
    if( *pNumberEnd == '.' )
    {
        ++pNumberEnd;

        while( isdigit(*pNumberEnd) )
            ++pNumberEnd;
    }

    // skip optional exponent
    if(    *pNumberEnd == 'd'
        || *pNumberEnd == 'D'
        || *pNumberEnd == 'e'
        || *pNumberEnd == 'E' )
    {
        ++pNumberEnd;

        if( *pNumberEnd == '-' || *pNumberEnd == '+' )
            ++pNumberEnd;

        while( isdigit(*pNumberEnd) )
            ++pNumberEnd;
    }

    //---
    // Process the string
	const uint32_t numberLen = (const uint32_t)(pNumberEnd-pNumberStart);
    char buffer[32];
    if( numberLen+1 < sizeof(buffer)/sizeof(buffer[0]) )
    {
        // copy into buffer and terminate with NUL before calling the
        // standard function
        memcpy( buffer, pNumberStart, numberLen*sizeof(buffer[0]) );
        buffer[numberLen] = '\0';
		const double result = strtod( buffer, NULL );

        return result;
    }
    else
    {
        // buffer was too small so just call the standard function on the
        // source input to get a proper result
        return strtod( pString, NULL );
    }
}

static float strtof_fast(const char* pString)
{
    return (float)strtod_fast(pString);
}


//////////////////////////
// str to value functions
//////////////////////////
PX_INLINE bool strToBool(const char *str, const char **endptr)
{
	bool ret = false;
	const char *begin = skipWhiteSpace(str);
	const char *end = skipNonWhiteSpace(begin);

	if( !end )
		end = begin + strlen(str);

	size_t len = (size_t)(end - begin);
	if ( physx::shdfnd::strnicmp(begin,"true", len) == 0 || physx::shdfnd::strnicmp(begin,"1", len) == 0 )
		ret = true;

	if( endptr )
		*endptr = skipNonWhiteSpace(begin);

	return ret;
}

PX_INLINE int8_t  strToI8(const char *str, const char **endptr)
{
	int8_t ret;
	const char *begin = skipWhiteSpace(str);
	const char *end = skipNonWhiteSpace(begin);

	if( !end )
		end = begin + strlen(str);

	if( strncmp(begin, "INT8_MIN", (size_t)(end-begin)) == 0)
		ret = INT8_MIN;
	else if( strncmp(begin, "INT8_MAX", (size_t)(end-begin)) == 0)
		ret = INT8_MAX;
	else if( strncmp(begin, "PX_MIN_I8", (size_t)(end-begin)) == 0)
		ret = INT8_MIN;
	else if( strncmp(begin, "PX_MAX_I8", (size_t)(end-begin)) == 0)
		ret = INT8_MAX;
	else
	 	ret = (int8_t)strtol(begin, 0, 0); //FIXME

	if( endptr )
		*endptr = skipNonWhiteSpace(begin);

	return ret;
}

PX_INLINE int16_t strToI16(const char *str, const char **endptr)
{
	int16_t ret;
	const char *begin = skipWhiteSpace(str);
	const char *end = skipNonWhiteSpace(begin);

	if( !end )
		end = begin + strlen(str);

	if( strncmp(begin, "INT16_MIN", (size_t)(end-begin)) == 0)
		ret = INT16_MIN;
	else if( strncmp(begin, "INT16_MAX", (size_t)(end-begin)) == 0)
		ret = INT16_MAX;
	else if( strncmp(begin, "PX_MIN_I16", (size_t)(end-begin)) == 0)
		ret = INT16_MIN;
	else if( strncmp(begin, "PX_MAX_I16", (size_t)(end-begin)) == 0)
		ret = INT16_MAX;
	else
	 	ret = (int16_t)strtol(begin, 0, 0); //FIXME

	if( endptr )
		*endptr = skipNonWhiteSpace(begin);

	return ret;
}

PX_INLINE int32_t strToI32(const char *str, const char **endptr)
{
	int32_t ret;
	const char *begin = skipWhiteSpace(str);
	const char *end = skipNonWhiteSpace(begin);

	if( !end )
		end = begin + strlen(str);

	if( strncmp(begin, "INT32_MIN", (size_t)(end-begin)) == 0)
		ret = INT32_MIN;
	else if( strncmp(begin, "INT32_MAX", (size_t)(end-begin)) == 0)
		ret = INT32_MAX;
	else if( strncmp(begin, "PX_MIN_I32", (size_t)(end-begin)) == 0)
		ret = INT32_MIN;
	else if( strncmp(begin, "PX_MAX_I32", (size_t)(end-begin)) == 0)
		ret = INT32_MAX;
	else
	 	ret = (int32_t)strtol(begin, 0, 0); //FIXME

	if( endptr )
		*endptr = skipNonWhiteSpace(begin);

	return ret;
}

PX_INLINE int64_t strToI64(const char *str, const char **endptr)
{
	int64_t ret;
	const char *begin = skipWhiteSpace(str);

	//FIXME
#ifdef _WIN32 //NV_WINDOWS, NV_XBOX
 	ret = (int64_t)_strtoi64(begin,0,10);
#else
	ret = (int64_t)strtoll(begin,0,10);
#endif

	if( endptr )
		*endptr = skipNonWhiteSpace(begin);

	return ret;
}

PX_INLINE uint8_t  strToU8(const char *str, const char **endptr)
{
	uint8_t ret;
	const char *begin = skipWhiteSpace(str);

	ret = (uint8_t)strtoul(begin, 0, 0);

	if( endptr )
		*endptr = skipNonWhiteSpace(begin);

	return ret;
}

PX_INLINE uint16_t strToU16(const char *str, const char **endptr)
{
	uint16_t ret;
	const char *end;
	const char *begin = skipWhiteSpace(str);

	end = skipNonWhiteSpace(begin);
	if( !end )
		end = begin + strlen(str);

	if( strncmp(begin, "UINT16_MAX", (size_t)(end-begin)) == 0)
		ret = UINT16_MAX;
	else if( strncmp(begin, "PX_MAX_U16", (size_t)(end-begin)) == 0)
		ret = UINT16_MAX;
	else
	 	ret = (uint16_t)strtoul(begin,0,0);

	if( endptr )
		*endptr = skipNonWhiteSpace(begin);

	return ret;
}

PX_INLINE uint32_t strToU32(const char *str, const char **endptr)
{
	uint32_t ret;
	const char *begin = skipWhiteSpace(str);
	const char *end = skipNonWhiteSpace(begin);

	if( !end )
		end = begin + strlen(str);

	if( strncmp(begin, "UINT32_MAX", (size_t)(end-begin)) == 0)
		ret = UINT32_MAX;
	else if( strncmp(begin, "PX_U32_MAX", (size_t)(end-begin)) == 0)
		ret = UINT32_MAX;
	else
	 	ret = (uint32_t)strtoul(begin,0,0);

	if( endptr )
		*endptr = skipNonWhiteSpace(begin);

	return ret;
}

PX_INLINE uint64_t strToU64(const char *str, const char **endptr)
{
	uint64_t ret;
	const char *begin;
	begin = skipWhiteSpace(str);

	//FIXME
#ifdef _WIN32 //NV_WINDOWS, NV_XBOX
 	ret = (uint64_t)_strtoui64(begin,0,10);
#else
	ret = (uint64_t)strtoull(begin,0,10);
#endif

	if( endptr )
		*endptr = skipNonWhiteSpace(begin);

	return ret;
}

#ifndef DEBUGGING_MISMATCHES
#define DEBUGGING_MISMATCHES 0
#endif

PX_INLINE float strToF32(const char *str, const char **endptr)
{
	float ret;
	const char *begin = skipWhiteSpace(str);
	const char *end = skipNonWhiteSpace(begin);

	if( !end )
		end = begin + strlen(str);

	const uint32_t len = (uint32_t)(end - begin);

	const char F32_MIN[] = "NV_MIN_F32";
	const char F32_MAX[] = "NV_MAX_F32";
	const char PX_F32_MIN[] = "PX_MIN_F32";
	const char PX_F32_MAX[] = "PX_MAX_F32";

	if( strncmp(begin, PX_F32_MIN, physx::PxMin(len, (uint32_t)(sizeof(PX_F32_MIN) - 1))) == 0)
		ret = -PX_MAX_F32;
	else if( strncmp(begin, PX_F32_MAX, physx::PxMin(len, (uint32_t)(sizeof(PX_F32_MAX) - 1))) == 0)
		ret = PX_MAX_F32;
	else if( strncmp(begin, F32_MIN, physx::PxMin(len, (uint32_t)(sizeof(F32_MIN) - 1))) == 0)
		ret = -PX_MAX_F32;
	else if( strncmp(begin, F32_MAX, physx::PxMin(len, (uint32_t)(sizeof(F32_MAX) - 1))) == 0)
		ret = PX_MAX_F32;
	else
	{
		ret = (float)strtof_fast(begin);
	}

#if DEBUGGING_MISMATCHES
	float testRet = (float)atof(begin);
	if( ret != testRet )
	{
		PX_ASSERT(0 && "Inaccurate float string");		
	}
#endif

	if( endptr )
		*endptr = skipNonWhiteSpace(begin);

	return ret;
}


PX_INLINE double strToF64(const char *str, const char **endptr)
{
	double ret;
	const char *begin = skipWhiteSpace(str);
	const char *end = skipNonWhiteSpace(begin);

	end = skipNonWhiteSpace(begin);

	if( !end )
		end = begin + strlen(str);

	const uint32_t len = (const uint32_t)(end - begin);

	const char F64_MIN[] = "PX_MIN_F364";
	const char F64_MAX[] = "PX_MAX_F64";
	const char PX_F64_MIN[] = "PX_MIN_F64";
	const char PX_F64_MAX[] = "PX_MAX_F64";

	if( strncmp(begin, F64_MIN, physx::PxMin(len, (uint32_t)(sizeof(F64_MIN) - 1))) == 0)
		ret = -PX_MAX_F64;
	else if( strncmp(begin, F64_MAX, physx::PxMin(len, (uint32_t)(sizeof(F64_MAX) - 1))) == 0)
		ret = PX_MAX_F64;
	else if( strncmp(begin, PX_F64_MIN, physx::PxMin(len, (uint32_t)(sizeof(PX_F64_MIN) - 1))) == 0)
		ret = -PX_MAX_F64;
	else if( strncmp(begin, PX_F64_MAX, physx::PxMin(len, (uint32_t)(sizeof(PX_F64_MAX) - 1))) == 0)
		ret = PX_MAX_F64;
	else
		ret = (double)strtod_fast(begin);

	if( endptr )
		*endptr = skipNonWhiteSpace(begin);

	return ret;
}

PX_INLINE void strToF32s(float *v,uint32_t count,const char *str, const char**endptr)
{
	const char *begin = skipWhiteSpace(str);

	if ( *begin == '(' ) begin++;
	for (uint32_t i=0; i<count && *begin; i++)
	{
		v[i] = (float)strToF32(begin, &begin);
	}

	if( endptr )
		*endptr = skipNonWhiteSpace(begin);
}


//////////////////////////
// value to str functions
//////////////////////////
PX_INLINE const char * valueToStr( bool val, char *buf, uint32_t n )
{
	physx::shdfnd::snprintf(buf, n,"%s",val ? "true" : "false");
	return buf;
}

PX_INLINE const char * valueToStr( int8_t val, char *buf, uint32_t n )
{
	if( val == INT8_MIN )
		physx::shdfnd::snprintf(buf, n,"%s","INT8_MIN" );
	else if( val == INT8_MAX )
		physx::shdfnd::snprintf(buf, n,"%s","INT8_MAX" );
	else
		physx::shdfnd::snprintf(buf, n, "%d", val);
	return buf;
}

PX_INLINE const char * valueToStr( int16_t val, char *buf, uint32_t n )
{
	if( val == INT16_MIN )
		physx::shdfnd::snprintf(buf, n,"%s","INT16_MIN" );
	else if( val == INT16_MAX )
		physx::shdfnd::snprintf(buf, n,"%s","INT16_MAX" );
	else
		physx::shdfnd::snprintf(buf, n,"%d",val );
	return buf;
}

PX_INLINE const char * valueToStr( int32_t val, char *buf, uint32_t n )
{
	if( val == INT32_MIN )
		physx::shdfnd::snprintf(buf, n,"%s","INT32_MIN" );
	else if( val == INT32_MAX )
		physx::shdfnd::snprintf(buf, n,"%s","INT32_MAX" );
	else
		physx::shdfnd::snprintf(buf, n,"%d",val );
	return buf;
}

PX_INLINE const char * valueToStr( int64_t val, char *buf, uint32_t n )
{
	physx::shdfnd::snprintf(buf, n,"%lld",val );
	return buf;
}

PX_INLINE const char * valueToStr( uint8_t val, char *buf, uint32_t n )
{
	physx::shdfnd::snprintf(buf, n, "%u", val);
	return buf;
}

PX_INLINE const char * valueToStr( uint16_t val, char *buf, uint32_t n )
{
	if( val == UINT16_MAX )
		physx::shdfnd::snprintf(buf, n,"%s","UINT16_MAX" );
	else
		physx::shdfnd::snprintf(buf, n,"%u",val );
	return buf;
}

PX_INLINE const char * valueToStr( uint32_t val, char *buf, uint32_t n )
{
	if( val == UINT32_MAX )
		physx::shdfnd::snprintf(buf, n,"%s","UINT32_MAX" );
	else
		physx::shdfnd::snprintf(buf, n,"%u",val );
	return buf;
}

PX_INLINE const char * valueToStr( uint64_t val, char *buf, uint32_t n )
{
	physx::shdfnd::snprintf(buf, n,"%llu",val );
	return buf;
}

PX_INLINE const char * valueToStr( float val, char *buf, uint32_t n )
{
	if( !physx::PxIsFinite(val) )
	{
		PX_ASSERT( 0 && "invalid floating point" );
		physx::shdfnd::snprintf(buf, n,"%s","0" );
	}
	else if( val == -PX_MAX_F32 )
		physx::shdfnd::snprintf(buf, n,"%s","PX_MIN_F32" );
	else if( val == PX_MAX_F32 )
		physx::shdfnd::snprintf(buf, n,"%s","PX_MAX_F32" );
    else if ( val == 1 )
    	physx::shdfnd::strlcpy(buf, n, "1");
    else if ( val == 0 )
    	physx::shdfnd::strlcpy(buf, n, "0");
    else if ( val == - 1 )
    	physx::shdfnd::strlcpy(buf, n, "-1");
    else
    {
		physx::shdfnd::snprintf(buf,n,"%.9g", (double)val ); // %g expects double
		const char *dot = strchr(buf,'.');
		const char *e = strchr(buf,'e');
		if ( dot && !e )
		{
			int32_t len = (int32_t)strlen(buf);
			char *foo = &buf[len-1];
			while ( *foo == '0' ) foo--;
			if ( *foo == '.' )
				*foo = 0;
			else
				foo[1] = 0;
		}
    }
	return buf;
}

PX_INLINE const char * valueToStr( double val, char *buf, uint32_t n )
{
	if( !physx::PxIsFinite(val) )
	{
		PX_ASSERT( 0 && "invalid floating point" );
		physx::shdfnd::snprintf(buf, n,"%s","0" );
	}
	else if( val == -PX_MAX_F64 )
		physx::shdfnd::snprintf(buf, n,"%s","PX_MIN_F64" );
	else if( val == PX_MAX_F64 )
		physx::shdfnd::snprintf(buf, n,"%s","PX_MAX_F64" );
    else if ( val == 1 )
		physx::shdfnd::strlcpy(buf, n, "1");
    else if ( val == 0 )
    	physx::shdfnd::strlcpy(buf, n, "0");
    else if ( val == - 1 )
    	physx::shdfnd::strlcpy(buf, n, "-1");
    else
    {
		physx::shdfnd::snprintf(buf,n,"%.18g", val );
		const char *dot = strchr(buf,'.');
		const char *e = strchr(buf,'e');
		if ( dot && !e )
		{
			int32_t len = (int32_t)strlen(buf);
			char *foo = &buf[len-1];
			while ( *foo == '0' ) foo--;
			if ( *foo == '.' )
				*foo = 0;
			else
				foo[1] = 0;
		}
    }
	return buf;
}
