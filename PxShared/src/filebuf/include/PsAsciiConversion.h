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

#ifndef PSFILEBUFFER_PSASCIICONVERSION_H
#define PSFILEBUFFER_PSASCIICONVERSION_H

/*!
\file
\brief PxAsciiConversion namespace contains string/value helper functions
*/

#include "PxMath.h"
#include "PsString.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <float.h>

namespace physx
{
namespace general_string_parsing2
{
namespace PxAsc
{

const uint32_t PxF32StrLen = 24;
const uint32_t PxF64StrLen = 32;
const uint32_t IntStrLen = 32;

PX_INLINE bool isWhiteSpace(char c);
PX_INLINE const char * skipNonWhiteSpace(const char *scan);
PX_INLINE const char * skipWhiteSpace(const char *scan);

//////////////////////////
// str to value functions
//////////////////////////
PX_INLINE bool strToBool(const char *str, const char **endptr);
PX_INLINE int8_t  strToI8(const char *str, const char **endptr);
PX_INLINE int16_t strToI16(const char *str, const char **endptr);
PX_INLINE int32_t strToI32(const char *str, const char **endptr);
PX_INLINE int64_t strToI64(const char *str, const char **endptr);
PX_INLINE uint8_t  strToU8(const char *str, const char **endptr);
PX_INLINE uint16_t strToU16(const char *str, const char **endptr);
PX_INLINE uint32_t strToU32(const char *str, const char **endptr);
PX_INLINE uint64_t strToU64(const char *str, const char **endptr);
PX_INLINE float strToF32(const char *str, const char **endptr);
PX_INLINE double strToF64(const char *str, const char **endptr);
PX_INLINE void strToF32s(float *v,uint32_t count,const char *str, const char**endptr);


//////////////////////////
// value to str functions
//////////////////////////
PX_INLINE const char * valueToStr( bool val, char *buf, uint32_t n );
PX_INLINE const char * valueToStr( int8_t val, char *buf, uint32_t n );
PX_INLINE const char * valueToStr( int16_t val, char *buf, uint32_t n );
PX_INLINE const char * valueToStr( int32_t val, char *buf, uint32_t n );
PX_INLINE const char * valueToStr( int64_t val, char *buf, uint32_t n );
PX_INLINE const char * valueToStr( uint8_t val, char *buf, uint32_t n );
PX_INLINE const char * valueToStr( uint16_t val, char *buf, uint32_t n );
PX_INLINE const char * valueToStr( uint32_t val, char *buf, uint32_t n );
PX_INLINE const char * valueToStr( uint64_t val, char *buf, uint32_t n );
PX_INLINE const char * valueToStr( float val, char *buf, uint32_t n );
PX_INLINE const char * valueToStr( double val, char *buf, uint32_t n );

#include "PsAsciiConversion.inl"

} // end of namespace
} // end of namespace
using namespace general_string_parsing2;
} // end of namespace


#endif // PSFILEBUFFER_PSASCIICONVERSION_H
