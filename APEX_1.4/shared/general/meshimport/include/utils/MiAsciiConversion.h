/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef MI_ASCII_CONVERSION_H
#define MI_ASCII_CONVERSION_H

/*!
\file
\brief MiAsciiConversion namespace contains string/value helper functions
*/


#include "MiPlatformConfig.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <float.h>

namespace mimp
{

	namespace MiAsc
	{

const MiU32 MiF32StrLen = 24;
const MiU32 MiF64StrLen = 32;
const MiU32 IntStrLen = 32;

MI_INLINE bool isWhiteSpace(char c);
MI_INLINE const char * skipNonWhiteSpace(const char *scan);
MI_INLINE const char * skipWhiteSpace(const char *scan);

//////////////////////////
// str to value functions
//////////////////////////
MI_INLINE bool strToBool(const char *str, const char **endptr);
MI_INLINE MiI8  strToI8(const char *str, const char **endptr);
MI_INLINE MiI16 strToI16(const char *str, const char **endptr);
MI_INLINE MiI32 strToI32(const char *str, const char **endptr);
MI_INLINE MiI64 strToI64(const char *str, const char **endptr);
MI_INLINE MiU8  strToU8(const char *str, const char **endptr);
MI_INLINE MiU16 strToU16(const char *str, const char **endptr);
MI_INLINE MiU32 strToU32(const char *str, const char **endptr);
MI_INLINE MiU64 strToU64(const char *str, const char **endptr);
MI_INLINE MiF32 strToF32(const char *str, const char **endptr);
MI_INLINE MiF64 strToF64(const char *str, const char **endptr);
MI_INLINE void strToF32s(MiF32 *v,MiU32 count,const char *str, const char**endptr);


//////////////////////////
// value to str functions
//////////////////////////
MI_INLINE const char * valueToStr( bool val, char *buf, MiU32 n );
MI_INLINE const char * valueToStr( MiI8 val, char *buf, MiU32 n );
MI_INLINE const char * valueToStr( MiI16 val, char *buf, MiU32 n );
MI_INLINE const char * valueToStr( MiI32 val, char *buf, MiU32 n );
MI_INLINE const char * valueToStr( MiI64 val, char *buf, MiU32 n );
MI_INLINE const char * valueToStr( MiU8 val, char *buf, MiU32 n );
MI_INLINE const char * valueToStr( MiU16 val, char *buf, MiU32 n );
MI_INLINE const char * valueToStr( MiU32 val, char *buf, MiU32 n );
MI_INLINE const char * valueToStr( MiU64 val, char *buf, MiU32 n );
MI_INLINE const char * valueToStr( MiF32 val, char *buf, MiU32 n );
MI_INLINE const char * valueToStr( MiF64 val, char *buf, MiU32 n );

#include "MiAsciiConversion.inl"

	}; // end of MiAsc namespace

}; // end of namespace


#endif // MI_ASCII_CONVERSION_H
