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

#ifndef PLATFORM_INPUT_STREAM_H_
#define PLATFORM_INPUT_STREAM_H_

// WARNING: before doing any changes to this file
// check comments at the head of BinSerializer.cpp

#include "PlatformStream.h"
#include "ApbDefinitions.h"

namespace NvParameterized
{

//ABI-aware input stream
class PlatformInputStream: public PlatformStream
{
	physx::PxFileBuf &mStream;
	physx::shdfnd::Array<uint32_t, Traits::Allocator> mPos;
	uint32_t mStartPos; //This is necessary to handle NvParameterized streams starting in the middle of other files

	PlatformInputStream(const PlatformInputStream &); //Don't
	void operator =(const PlatformInputStream &); //Don't

public:
	PlatformInputStream(physx::PxFileBuf &stream, const PlatformABI &targetParams, Traits *traits);

	Serializer::ErrorType skipBytes(uint32_t nbytes);

	Serializer::ErrorType pushPos(uint32_t newPos);

	void popPos();

	uint32_t getPos() const;

	//Read string stored at given offset
	//TODO: this could much faster if we loaded whole dictionary in the beginning
	Serializer::ErrorType readString(uint32_t off, const char *&s);

	//Deserialize header of NvParameterized object (see wiki for details)
	Serializer::ErrorType readObjHeader(ObjHeader &hdr);

	//Deserialize primitive type
	template<typename T> PX_INLINE Serializer::ErrorType read(T &x, bool doAlign = true);

	//Deserialize array of primitive type (slow path)
	template<typename T> PX_INLINE Serializer::ErrorType readSimpleArraySlow(Handle &handle);

	//Deserialize array of structs of primitive type
	Serializer::ErrorType readSimpleStructArray(Handle &handle);

	//Deserialize array of primitive type
	template<typename T> PX_INLINE Serializer::ErrorType readSimpleArray(Handle &handle);

	//Align current offset according to supplied alignment and padding
	void beginStruct(uint32_t align_, uint32_t pad_);

	//Align current offset according to supplied alignment (padding = alignment)
	void beginStruct(uint32_t align_);

	//Align current offset according to supplied DataType
	void beginStruct(const Definition *pd);

	//Insert tail padding
	void closeStruct();

	//beginStruct for DummyStringStruct
	void beginString();

	//closeStruct for DummyStringStruct
	void closeString();

	//beginStruct for arrays
	void beginArray(const Definition *pd);

	//closeStruct for arrays
	void closeArray();

	//Align offset to be n*border
	void align(uint32_t border);

	//Read value of pointer (offset from start of file)
	Serializer::ErrorType readPtr(uint32_t &val);
};

#include "PlatformInputStream.inl"

}

#endif
