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

#include "PsString.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#if PX_WINDOWS_FAMILY
#pragma warning(push)
#pragma warning(disable : 4996) // unsafe string functions
#endif

#if PX_PS4 || PX_APPLE_FAMILY || (PX_LINUX && PX_X86 && PX_CLANG)
#pragma clang diagnostic push
// error : format string is not a string literal
#pragma clang diagnostic ignored "-Wformat-nonliteral"
#endif

namespace physx
{
namespace shdfnd
{
// cross-platform implementations

int32_t strcmp(const char* str1, const char* str2)
{
	return ::strcmp(str1, str2);
}

int32_t strncmp(const char* str1, const char* str2, size_t count)
{
	return ::strncmp(str1, str2, count);
}

int32_t snprintf(char* dst, size_t dstSize, const char* format, ...)
{
	va_list arg;
	va_start(arg, format);
	int32_t r = shdfnd::vsnprintf(dst, dstSize, format, arg);
	va_end(arg);
	return r;
}

int32_t sscanf(const char* buffer, const char* format, ...)
{
	va_list arg;
	va_start(arg, format);
#if (PX_VC < 12) && !PX_LINUX
	int32_t r = ::sscanf(buffer, format, arg);
#else
	int32_t r = ::vsscanf(buffer, format, arg);
#endif
	va_end(arg);

	return r;
}

size_t strlcpy(char* dst, size_t dstSize, const char* src)
{
	size_t i = 0;
	if(dst && dstSize)
	{
		for(; i + 1 < dstSize && src[i]; i++) // copy up to dstSize-1 bytes
			dst[i] = src[i];
		dst[i] = 0; // always null-terminate
	}

	while(src[i]) // read any remaining characters in the src string to get the length
		i++;

	return i;
}

size_t strlcat(char* dst, size_t dstSize, const char* src)
{
	size_t i = 0, s = 0;
	if(dst && dstSize)
	{
		s = strlen(dst);
		for(; i + s + 1 < dstSize && src[i]; i++) // copy until total is at most dstSize-1
			dst[i + s] = src[i];
		dst[i + s] = 0; // always null-terminate
	}

	while(src[i]) // read any remaining characters in the src string to get the length
		i++;

	return i + s;
}

void strlwr(char* str)
{
	for(; *str; str++)
		if(*str >= 'A' && *str <= 'Z')
			*str += 32;
}

void strupr(char* str)
{
	for(; *str; str++)
		if(*str >= 'a' && *str <= 'z')
			*str -= 32;
}

int32_t vsnprintf(char* dst, size_t dstSize, const char* src, va_list arg)
{

#if PX_VC // MSVC is not C99-compliant...
	int32_t result = dst ? ::vsnprintf(dst, dstSize, src, arg) : -1;
	if(dst && (result == int32_t(dstSize) || result < 0))
		dst[dstSize - 1] = 0; // string was truncated or there wasn't room for the NULL
	if(result < 0)
		result = _vscprintf(src, arg); // work out how long the answer would have been.
#else
	int32_t result = ::vsnprintf(dst, dstSize, src, arg);
#endif
	return result;
}

int32_t stricmp(const char* str, const char* str1)
{
#if PX_VC
	return (::_stricmp(str, str1));
#else
	return (::strcasecmp(str, str1));
#endif
}

int32_t strnicmp(const char* str, const char* str1, size_t n)
{
#if PX_VC
	return (::_strnicmp(str, str1, n));
#else
	return (::strncasecmp(str, str1, n));
#endif
}

void printFormatted(const char* format, ...)
{
	char buf[MAX_PRINTFORMATTED_LENGTH];

	va_list arg;
	va_start(arg, format);
	vsnprintf(buf, MAX_PRINTFORMATTED_LENGTH, format, arg);
	va_end(arg);

	printString(buf);
}
}
}

#if PX_PS4 || PX_APPLE_FAMILY || (PX_LINUX && PX_X86 && PX_CLANG)
#pragma clang diagnostic pop
#endif

#if PX_WINDOWS_FAMILY
#pragma warning(pop)
#endif
