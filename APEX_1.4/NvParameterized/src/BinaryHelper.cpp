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
// Copyright (c) 2008-2013 NVIDIA Corporation. All rights reserved.

// WARNING: before doing any changes to this file
// check comments at the head of BinSerializer.cpp

#include <ctype.h>
#include <stdio.h>

#include "BinaryHelper.h"

namespace NvParameterized
{

void dumpBytes(const char *data, uint32_t nbytes)
{
	printf("Total = %d bytes\n", nbytes);

	for(uint32_t i = 0; i < nbytes; i += 16)
	{
		printf("%08x: ", i);

		//Print bytes
		for(uint32_t j = i; j < i + 16; ++j)
		{
			if( nbytes < j )
			{
				//Pad with whites
				for(; j < i + 16; ++j)
					printf("   ");

				break;
			}

			unsigned char c = static_cast<unsigned char>(data[j]);
			printf("%02x ", c);
		}

		//Print chars
		for(uint32_t j = i; j < i + 16; ++j)
		{
			if( nbytes < j )
				break;

			unsigned char c = static_cast<unsigned char>(data[j]);
			printf("%c", isprint(c) ? c : '.');
		}

		printf("\n");
	}
}

void Dictionary::setOffset(const char *s, uint32_t off)
{
	for(uint32_t i = 0; i < entries.size(); ++i)
		if( 0 == strcmp(s, entries[i].s) )
		{
			entries[i].offset = off;
			return;
		}

	PX_ASSERT(0 && "String not found");
}

uint32_t Dictionary::getOffset(const char *s) const
{
	for(uint32_t i = 0; i < entries.size(); ++i)
		if( 0 == strcmp(s, entries[i].s) )
			return entries[i].offset;

	PX_ASSERT(0 && "String not found");
	return (uint32_t)-1;
}

void Dictionary::serialize(StringBuf &res) const
{
	res.append(Canonize(entries.size()));

	for(uint32_t i = 0; i < entries.size(); ++i)
	{
		const char *s = entries[i].s;
		res.appendBytes(s, 1 + (uint32_t)strlen(s));
	}
}

uint32_t Dictionary::put(const char *s)
{
	PX_ASSERT(s && "NULL in dictionary");

	for(uint32_t i = 0; i < entries.size(); ++i)
		if( 0 == strcmp(s, entries[i].s) )
			return i;

	Entry e = {s, 0};
	entries.pushBack(e);
	return entries.size() - 1;
}

}