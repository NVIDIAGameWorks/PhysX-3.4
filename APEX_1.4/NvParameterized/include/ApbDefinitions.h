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

#ifndef APB_DEFINITIONS_H_
#define APB_DEFINITIONS_H_

// This file contains definitions of various parts of APB file format

// WARNING: before doing any changes to this file
// check comments at the head of BinSerializer.cpp

#include "SerializerCommon.h"
#include "BinaryHelper.h"

namespace NvParameterized
{

#define APB_MAGIC 0x5A5B5C5D

namespace BinVersions
{
	static const uint32_t Initial = 0x00010000,
		AllRefsCounted = 0x00010001,
		WithAlignment = 0x00010002,
		WithExtendedHeader = 0x00010003;
}

// Type of relocation in binary file
enum RelocType
{
	//Raw bytes
	RELOC_ABS_RAW = 0,

	//NvParameterized (will be initialized on deserialization)
	RELOC_ABS_REF,

	RELOC_LAST
};

// Relocation record in binary file
struct BinaryReloc
{
	uint32_t type;
	uint32_t off;
};

// Format of data in binary file; only BINARY_TYPE_PLAIN is used for now
enum BinaryType
{
	BINARY_TYPE_PLAIN = 0,
	BINARY_TYPE_XML_GZ,
	BINARY_TYPE_LAST
};

// Some dummy version control systems insert '\r' before '\n'.
// This short string in header is used to catch this.
// We also use 0xff byte to guarantee that we have non-printable chars
// (s.t. VCS can detect that file is binary).
#define VCS_SAFETY_FLAGS "ab\n\xff"

// File header
#pragma pack(push,1) // For cross-platform compatibility!

// Convert to platform-independent format
static void CanonizeArrayOfU32s(char *data, uint32_t len)
{
	PX_ASSERT(len % 4U == 0);

	if( IsBigEndian() )
		return;

	for(uint32_t i = 0; i < len; i += 4U)
		SwapBytes(data + i, 4U, TYPE_U32);
}

// Main binary header
struct BinaryHeader
{
	uint32_t magic;
	uint32_t type;
	uint32_t version;
	int32_t numObjects;

	uint32_t fileLength;
	uint32_t dictOffset;
	uint32_t dataOffset;
	uint32_t relocOffset;

	uint32_t metadataOffset;
	uint32_t archType;
	uint32_t compilerType;
	uint32_t compilerVer;

	uint32_t osType;
	uint32_t osVer;
	uint32_t numMetadata;
	uint32_t alignment;

	static bool CheckAlignment()
	{
		bool isPushPackOk = 4 != offsetof(BinaryHeader, type);
		if( isPushPackOk )
		{
			DEBUG_ASSERT( 0 && "PX_PUSH_PACK failed!" );
			return false;
		}
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4127)
#endif
		if( sizeof(BinaryHeader) % 16 != 0 )
		{
			return false;
		}
#ifdef _MSC_VER
#pragma warning(pop)
#endif

		return true;
	}

	Serializer::ErrorType getPlatform(SerializePlatform &platform) const
	{
		if( archType >= SerializePlatform::ARCH_LAST
				|| compilerType >= SerializePlatform::COMP_LAST
				|| osType >= SerializePlatform::OS_LAST )
		{
			DEBUG_ALWAYS_ASSERT();
			return Serializer::ERROR_INVALID_PLATFORM;
		}
		
		platform = SerializePlatform(
			static_cast<SerializePlatform::ArchType>(archType),
			static_cast<SerializePlatform::CompilerType>(compilerType),
			compilerVer,
			static_cast<SerializePlatform::OsType>(osType),
			osVer
		);

		return Serializer::ERROR_NONE;
	}

	void canonize()
	{
		CanonizeArrayOfU32s((char*)this, sizeof(BinaryHeader));
	}

	void decanonize() { canonize(); }
};

// Extended header (only in new versions)
struct BinaryHeaderExt
{
	uint32_t vcsSafetyFlags;
	uint32_t res[3 + 8]; // Pad to multiple of 16 byte

	void canonize()
	{
		// vcsSafetyFlags should be stored as-is
	}

	void decanonize()
	{
		// vcsSafetyFlags should be stored as-is
	}
};

#pragma pack(pop)

// NvParameterized object header
struct ObjHeader
{
	uint32_t dataOffset;
	const char *className;
	const char *name;
	bool isIncluded;
	uint32_t version;
	uint32_t checksumSize;
	const uint32_t *checksum;
};

// Element of root references table
struct ObjectTableEntry
{
	Interface *obj;
	const char *className;
	const char *name;
	const char *filename;
};

} // namespace NvParameterized

#endif
