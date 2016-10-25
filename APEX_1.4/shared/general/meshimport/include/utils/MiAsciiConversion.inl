/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


/*!
\file
\brief MiAsciiConversion namespace contains string/value helper functions
*/

MI_INLINE bool isWhiteSpace(char c)
{
	bool ret = false;
	if ( c == 32 || c == 9 || c == 13 || c == 10 || c == ',' ) ret = true;
	return ret;
}

MI_INLINE const char * skipNonWhiteSpace(const char *scan)
{
	while ( !isWhiteSpace(*scan) && *scan) scan++;
	if ( *scan == 0 ) scan = NULL;
	return scan;
}
MI_INLINE const char * skipWhiteSpace(const char *scan)
{
	while ( isWhiteSpace(*scan) && *scan ) scan++;
	if ( *scan == 0 ) scan = NULL;
	return scan;
}

//////////////////////////
// str to value functions
//////////////////////////
MI_INLINE bool strToBool(const char *str, const char **endptr)
{
	bool ret = false;
	const char *begin = skipWhiteSpace(str);
	const char *end = skipNonWhiteSpace(begin);

	if( !end )
		end = begin + strlen(str);

	MiI32 len = (MiI32)(end - begin);
	if ( MESH_IMPORT_STRING::strnicmp(begin,"true", (size_t)len) == 0 || MESH_IMPORT_STRING::strnicmp(begin,"1", (size_t)len) == 0 )
		ret = true;	

	if( endptr )
		*endptr = skipNonWhiteSpace(begin);
	
	return ret;
}

MI_INLINE MiI8  strToI8(const char *str, const char **endptr)
{
	MiI8 ret;
	const char *begin = skipWhiteSpace(str);
	const char *end = skipNonWhiteSpace(begin);

	if( !end )
		end = begin + strlen(str);
	
	if( strncmp(begin, "PX_MIN_I8", size_t(end-begin)) == 0)
		ret = MI_MIN_I8;
	else if( strncmp(begin, "PX_MAX_I8", size_t(end-begin)) == 0)
		ret = MI_MAX_I8;
	else
	 	ret = (MiI8)strtol(begin, 0, 0); //FIXME

	if( endptr )
		*endptr = skipNonWhiteSpace(begin);

	return ret;
}

MI_INLINE MiI16 strToI16(const char *str, const char **endptr)
{
	MiI16 ret;
	const char *begin = skipWhiteSpace(str);
	const char *end = skipNonWhiteSpace(begin);

	if( !end )
		end = begin + strlen(str);
	
	if( strncmp(begin, "PX_MIN_I16", size_t(end-begin)) == 0)
		ret = MI_MIN_I16;
	else if( strncmp(begin, "PX_MAX_I16", size_t(end-begin)) == 0)
		ret = MI_MAX_I16;
	else
	 	ret = (MiI16)strtol(begin, 0, 0); //FIXME
	
	if( endptr )
		*endptr = skipNonWhiteSpace(begin);

	return ret;
}

MI_INLINE MiI32 strToI32(const char *str, const char **endptr)
{
	MiI32 ret;
	const char *begin = skipWhiteSpace(str);
	const char *end = skipNonWhiteSpace(begin);

	if( !end )
		end = begin + strlen(str);
	
	if( strncmp(begin, "PX_MIN_I32", size_t(end-begin)) == 0)
		ret = MI_MIN_I32;
	else if( strncmp(begin, "PX_MAX_I32", size_t(end-begin)) == 0)
		ret = MI_MAX_I32;
	else
	 	ret = (MiI32)strtol(begin, 0, 0); //FIXME

	if( endptr )
		*endptr = skipNonWhiteSpace(begin);

	return ret;
}

MI_INLINE MiI64 strToI64(const char *str, const char **endptr)
{
	MiI64 ret;
	const char *begin = skipWhiteSpace(str);
    
	//FIXME
#ifdef _WIN32 //MI_WINDOWS, MI_XBOX
 	ret = (MiI64)(MiU64)_strtoi64(begin,0,10);
#else
	ret = (MiI64)(MiU64)strtoll(begin,0,10);
#endif

	if( endptr )
		*endptr = skipNonWhiteSpace(begin);

	return ret;
}

MI_INLINE MiU8  strToU8(const char *str, const char **endptr)
{
	MiU8 ret;
	const char *begin = skipWhiteSpace(str);
	
	ret = (MiU8)strtoul(begin, 0, 0);

	if( endptr )
		*endptr = skipNonWhiteSpace(begin);

	return ret;
}

MI_INLINE MiU16 strToU16(const char *str, const char **endptr)
{
	MiU16 ret;
	const char *end;
	const char *begin = skipWhiteSpace(str);

	end = skipNonWhiteSpace(begin);
	if( !end )
		end = begin + strlen(str);
	
	if( strncmp(begin, "PX_MAX_U16", size_t(end-begin)) == 0)
		ret = MI_MAX_U16;
	else
	 	ret = (MiU16)strtoul(begin,0,0);

	if( endptr )
		*endptr = skipNonWhiteSpace(begin);

	return ret;
}

MI_INLINE MiU32 strToU32(const char *str, const char **endptr)
{
	MiU32 ret;
	const char *begin = skipWhiteSpace(str);
	const char *end = skipNonWhiteSpace(begin);

	if( !end )
		end = begin + strlen(str);
	
	if( strncmp(begin, "UINT32_MAX", size_t(end-begin)) == 0)
		ret = MI_MAX_U32;
	else
	 	ret = (MiU32)strtoul(begin,0,0);

	if( endptr )
		*endptr = skipNonWhiteSpace(begin);

	return ret;
}

MI_INLINE MiU64 strToU64(const char *str, const char **endptr)
{
	MiU64 ret;
	const char *begin;
	begin = skipWhiteSpace(str);

	//FIXME
#ifdef _WIN32 //MI_WINDOWS, MI_XBOX
 	ret = (MiU64)_strtoui64(begin,0,10);
#else
	ret = (MiU64)strtoull(begin,0,10);
#endif

	if( endptr )
		*endptr = skipNonWhiteSpace(begin);

	return ret;
}

MI_INLINE MiF32 strToF32(const char *str, const char **endptr)
{
	MiF32 ret;
	const char *begin = skipWhiteSpace(str);
	const char *end = skipNonWhiteSpace(begin);
	char tmpStr[MiF32StrLen];

	if( !end )
		end = begin + strlen(str);
	
	// str can be very very long, so we should copy the value to a tmp buffer
//	MESH_IMPORT_STRING::strncpy_s(tmpStr, MiF32StrLen, begin, size_t(end-begin));
	MESH_IMPORT_STRING::strlcpy(tmpStr, MiF32StrLen, begin);	// BRG1.4 - we can drop the last argument of the (former) strncpy_s, since we set a NUL terminator on the next line
	tmpStr[end-begin] = 0;

	if( strncmp(tmpStr, "PX_MIN_F32", size_t(end-begin)) == 0)
		ret = -MI_MAX_F32;
	else if( strncmp(tmpStr, "PX_MAX_F32", size_t(end-begin)) == 0)
		ret = MI_MAX_F32;
	else
		ret = (MiF32)atof(tmpStr);
	
#if DEBUGGING_MISMATCHES
	MiF32 testRet = (MiF32)atof(begin);
	if( ret != testRet )
	{
		MI_ASSERT(0 && "Inaccurate float string");
	}
#endif

	if( endptr )
		*endptr = skipNonWhiteSpace(begin);

	return ret;
}


MI_INLINE MiF64 strToF64(const char *str, const char **endptr)
{
	MiF64 ret;
	const char *begin = skipWhiteSpace(str);
	const char *end = skipNonWhiteSpace(begin);
	char tmpStr[MiF64StrLen];

	end = skipNonWhiteSpace(begin);

	if( !end )
		end = begin + strlen(str);
	
	// str can be very very long, so we should copy the value to a tmp buffer
//	MESH_IMPORT_STRING::strncpy_s(tmpStr, MiF64StrLen, begin, size_t(end-begin));
	MESH_IMPORT_STRING::strlcpy(tmpStr, MiF64StrLen, begin);	// BRG1.4 - we can drop the last argument of the (former) strncpy_s, since we set a NUL terminator on the next line
	tmpStr[end-begin] = 0;
	
	if( strncmp(tmpStr, "PX_MIN_F64", size_t(end-begin)) == 0)
		ret = -MI_MAX_F64;
	else if( strncmp(tmpStr, "PX_MAX_F64", size_t(end-begin)) == 0)
		ret = MI_MAX_F64;
	else 
		ret = (MiF64)atof(tmpStr);

	if( endptr )
		*endptr = skipNonWhiteSpace(begin);

	return ret;
}

MI_INLINE void strToF32s(MiF32 *v,MiU32 count,const char *str, const char**endptr)
{
	const char *begin = skipWhiteSpace(str);

	if ( *begin == '(' ) begin++;
	for (MiU32 i=0; i<count && *begin; i++)
	{
		v[i] = (MiF32)strToF32(begin, &begin);
	}

	if( endptr )
		*endptr = skipNonWhiteSpace(begin);
}


//////////////////////////
// value to str functions
//////////////////////////
MI_INLINE const char * valueToStr( bool val, char *buf, MiU32 n )
{
	MESH_IMPORT_STRING::snprintf(buf, n,"%s",val ? "true" : "false");
	return buf;
}

MI_INLINE const char * valueToStr( MiI8 val, char *buf, MiU32 n )
{
	if( val == MI_MIN_I8 )
		MESH_IMPORT_STRING::snprintf(buf, n,"%s","PX_MIN_I8" );
	else if( val == MI_MAX_I8 )
		MESH_IMPORT_STRING::snprintf(buf, n,"%s","PX_MAX_I8" );
	else
		MESH_IMPORT_STRING::snprintf(buf, n, "%d", val);
	return buf;
}

MI_INLINE const char * valueToStr( MiI16 val, char *buf, MiU32 n )
{
	if( val == MI_MIN_I16 )
		MESH_IMPORT_STRING::snprintf(buf, n,"%s","PX_MIN_I16" );
	else if( val == MI_MAX_I16 )
		MESH_IMPORT_STRING::snprintf(buf, n,"%s","PX_MAX_I16" );
	else
		MESH_IMPORT_STRING::snprintf(buf, n,"%d",val );
	return buf;
}

MI_INLINE const char * valueToStr( MiI32 val, char *buf, MiU32 n )
{
	if( val == MI_MIN_I32 )
		MESH_IMPORT_STRING::snprintf(buf, n,"%s","PX_MIN_I32" );
	else if( val == MI_MAX_I32 )
		MESH_IMPORT_STRING::snprintf(buf, n,"%s","PX_MAX_I32" );
	else
		MESH_IMPORT_STRING::snprintf(buf, n,"%d",val );
	return buf;
}

MI_INLINE const char * valueToStr( MiI64 val, char *buf, MiU32 n )
{
	MESH_IMPORT_STRING::snprintf(buf, n,"%lld",val );
	return buf;
}

MI_INLINE const char * valueToStr( MiU8 val, char *buf, MiU32 n )
{
	MESH_IMPORT_STRING::snprintf(buf, n, "%u", val);
	return buf;
}

MI_INLINE const char * valueToStr( MiU16 val, char *buf, MiU32 n )
{
	if( val == MI_MAX_U16 )
		MESH_IMPORT_STRING::snprintf(buf, n,"%s","PX_MAX_U16" );
	else
		MESH_IMPORT_STRING::snprintf(buf, n,"%u",val );
	return buf;
}

MI_INLINE const char * valueToStr( MiU32 val, char *buf, MiU32 n )
{
	if( val == MI_MAX_U32 )
		MESH_IMPORT_STRING::snprintf(buf, n,"%s","UINT32_MAX" );
	else
		MESH_IMPORT_STRING::snprintf(buf, n,"%u",val );
	return buf;
}

MI_INLINE const char * valueToStr( MiU64 val, char *buf, MiU32 n )
{
	MESH_IMPORT_STRING::snprintf(buf, n,"%llu",val );
	return buf;
}

MI_INLINE const char * valueToStr( MiF32 val, char *buf, MiU32 n )
{
	if( !MESH_IMPORT_INTRINSICS::isFinite(val) )
	{
		MI_ASSERT( 0 && "invalid floating point" );
		MESH_IMPORT_STRING::snprintf(buf, n,"%s","0" );
	}
	else if( val == -MI_MAX_F32 )
		MESH_IMPORT_STRING::snprintf(buf, n,"%s","PX_MIN_F32" );
	else if( val == MI_MAX_F32 )
		MESH_IMPORT_STRING::snprintf(buf, n,"%s","PX_MAX_F32" );
    else if ( val == 1 )
    	MESH_IMPORT_STRING::strlcpy(buf, n, "1");
    else if ( val == 0 )
    	MESH_IMPORT_STRING::strlcpy(buf, n, "0");
    else if ( val == - 1 )
    	MESH_IMPORT_STRING::strlcpy(buf, n, "-1");
    else
    {
		MESH_IMPORT_STRING::snprintf(buf,n,"%.9g", (double)val ); // %g expects double
		const char *dot = strchr(buf,'.');
		const char *e = strchr(buf,'e');
		if ( dot && !e )
		{
			MiI32 len = (MiI32)strlen(buf);
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

MI_INLINE const char * valueToStr( MiF64 val, char *buf, MiU32 n )
{
	if( !MESH_IMPORT_INTRINSICS::isFinite(val) )
	{
		MI_ASSERT( 0 && "invalid floating point" );
		MESH_IMPORT_STRING::snprintf(buf, n,"%s","0" );
	}
	else if( val == -MI_MAX_F64 )
		MESH_IMPORT_STRING::snprintf(buf, n,"%s","PX_MIN_F64" );
	else if( val == MI_MAX_F64 )
		MESH_IMPORT_STRING::snprintf(buf, n,"%s","PX_MAX_F64" );
    else if ( val == 1 )
		MESH_IMPORT_STRING::strlcpy(buf, n, "1");
    else if ( val == 0 )
    	MESH_IMPORT_STRING::strlcpy(buf, n, "0");
    else if ( val == - 1 )
    	MESH_IMPORT_STRING::strlcpy(buf, n, "-1");
    else
    {
		MESH_IMPORT_STRING::snprintf(buf,n,"%.18g", val );
		const char *dot = strchr(buf,'.');
		const char *e = strchr(buf,'e');
		if ( dot && !e )
		{
			MiI32 len = (MiI32)strlen(buf);
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
