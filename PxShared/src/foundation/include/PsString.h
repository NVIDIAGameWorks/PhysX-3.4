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

#ifndef PSFOUNDATION_PSSTRING_H
#define PSFOUNDATION_PSSTRING_H

#include "foundation/PxPreprocessor.h"
#include "foundation/PxSimpleTypes.h"
#include <stdarg.h>

namespace physx
{
namespace shdfnd
{

// the following functions have C99 semantics. Note that C99 requires for snprintf and vsnprintf:
// * the resulting string is always NULL-terminated regardless of truncation.
// * in the case of truncation the return value is the number of characters that would have been created.

PX_FOUNDATION_API int32_t sscanf(const char* buffer, const char* format, ...);
PX_FOUNDATION_API int32_t strcmp(const char* str1, const char* str2);
PX_FOUNDATION_API int32_t strncmp(const char* str1, const char* str2, size_t count);
PX_FOUNDATION_API int32_t snprintf(char* dst, size_t dstSize, const char* format, ...);
PX_FOUNDATION_API int32_t vsnprintf(char* dst, size_t dstSize, const char* src, va_list arg);

// strlcat and strlcpy have BSD semantics:
// * dstSize is always the size of the destination buffer
// * the resulting string is always NULL-terminated regardless of truncation
// * in the case of truncation the return value is the length of the string that would have been created

PX_FOUNDATION_API size_t strlcat(char* dst, size_t dstSize, const char* src);
PX_FOUNDATION_API size_t strlcpy(char* dst, size_t dstSize, const char* src);

// case-insensitive string comparison
PX_FOUNDATION_API int32_t stricmp(const char* str1, const char* str2);
PX_FOUNDATION_API int32_t strnicmp(const char* str1, const char* str2, size_t count);

// in-place string case conversion
PX_FOUNDATION_API void strlwr(char* str);
PX_FOUNDATION_API void strupr(char* str);

/**
\brief The maximum supported formatted output string length
(number of characters after replacement).

@see printFormatted()
*/
static const size_t MAX_PRINTFORMATTED_LENGTH = 1024;

/**
\brief Prints the formatted data, trying to make sure it's visible to the app programmer

@see NS_MAX_PRINTFORMATTED_LENGTH
*/
PX_FOUNDATION_API void printFormatted(const char*, ...);

/**
\brief Prints the string literally (does not consume % specifier), trying to make sure it's visible to the app
programmer
*/
PX_FOUNDATION_API void printString(const char*);
}
}
#endif // #ifndef PSFOUNDATION_PSSTRING_H
