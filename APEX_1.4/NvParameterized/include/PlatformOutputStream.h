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

#ifndef PLATFORM_OUTPUT_STREAM_H_
#define PLATFORM_OUTPUT_STREAM_H_

// WARNING: before doing any changes to this file
// check comments at the head of BinSerializer.cpp

#include "PlatformStream.h"
#include "ApbDefinitions.h"

#ifndef WITHOUT_APEX_SERIALIZATION

namespace NvParameterized
{

class PlatformOutputStream;

//Info about pointed data (and its subdata)
struct Reloc
{
	RelocType type;
	uint32_t ptrPos;
	PlatformOutputStream *ptrData;
	Traits *traits;

	Reloc(RelocType type_, uint32_t ptrPos_, const PlatformOutputStream &parent);

	Reloc(const Reloc &cinfo);

	~Reloc();
};

//Info about pointed string
struct StringReloc
{
	uint32_t ptrPos;
	const char *s;

	StringReloc(uint32_t ptrPos_, const char *s_): ptrPos(ptrPos_), s(s_) {}
};

//Info about reloc which was already merged
struct MergedReloc
{
	uint32_t ptrPos;
	uint32_t targetPos;
	RelocType type;
	bool isExtern;
};

//ABI-aware output stream
class PlatformOutputStream: public PlatformStream
{
	void operator = (const PlatformOutputStream &); //Don't

	friend struct Reloc;
public:
	PlatformOutputStream(const PlatformABI &targetParams, Traits *traits, Dictionary &dict_);

	//Array's copy constructor is broken so we implement it by hand
	PlatformOutputStream(const PlatformOutputStream &s);

#ifndef NDEBUG
	void dump() const;
#endif

	PX_INLINE StringBuf &getData() { return data; }

	uint32_t size() const { return data.size(); }

	//Update uint32_t at given position
	void storeU32At(uint32_t x, uint32_t i);

	uint32_t storeString(const char *s);

	//Serialize raw bytes
	uint32_t storeBytes(const char *s, uint32_t n);

	//Serialize value of primitive type with proper alignment
	//(this is overriden for bools below)
	template <typename T> PX_INLINE uint32_t storeSimple(T x);

	PX_INLINE uint32_t storeSimple(float* x, uint32_t size);

	//Serialize array of primitive type with proper alignment (slow path)
	template <typename T> PX_INLINE int32_t storeSimpleArraySlow(Handle &handle);

	//Serialize array of structs of primitive types with proper alignment
	uint32_t storeSimpleStructArray(Handle &handle);

	//Serialize array of primitive type with proper alignment
	template <typename T> PX_INLINE uint32_t storeSimpleArray(Handle &handle);

	//Serialize header of NvParameterized object (see wiki for details)
	uint32_t storeObjHeader(const NvParameterized::Interface &obj, bool isIncluded = true);

	//Serialize NvParameters-part of NvParameterized object
	uint32_t beginObject(const NvParameterized::Interface &obj, bool /*isRoot*/, const Definition *pd);

	//Insert tail padding bytes for NvParameterized object
	void closeObject() { closeStruct(); }

	//Align current offset according to supplied alignment and padding
	uint32_t beginStruct(uint32_t align_, uint32_t pad_);

	//Align current offset according to supplied alignment (padding = alignment)
	uint32_t beginStruct(uint32_t align_);

	//Align current offset according to supplied DataType
	uint32_t beginStruct(const Definition *pd);

	//Insert tail padding
	void closeStruct();

	//beginStruct for DummyStringStruct
	uint32_t beginString();

	//closeStruct for DummyStringStruct
	void closeString();

	//beginStruct for arrays
	uint32_t beginArray(const Definition *pd);

	//beginStruct for arrays
	uint32_t beginArray(uint32_t align_);

	//closeStruct for arrays
	void closeArray();

	void skipBytes(uint32_t nbytes);

	//Align offset to be n*border
	void align(uint32_t border);

	//Align offset to be n * alignment of T
	template <typename T> PX_INLINE void align();

	//Add data (including relocations) from another stream
	uint32_t merge(const PlatformOutputStream &mergee);

	//Merge pointed data (including strings) thus flattening the stream
	void flatten();

	//Create relocation table
	uint32_t writeRelocs();

	//Serialize dictionary data
	void mergeDict();

	uint32_t storeNullPtr();

	Reloc &storePtr(RelocType type, const Definition *pd);

	Reloc &storePtr(RelocType type, uint32_t align);

	void storeStringPtr(const char *s);

	PX_INLINE uint32_t alignment() const { return mTotalAlign; }

	PX_INLINE void setAlignment(uint32_t newAlign) { mTotalAlign = newAlign; }

private:

	//Byte stream
	StringBuf data;

	//Generic relocations
	physx::shdfnd::Array<Reloc, Traits::Allocator> mRelocs;

	//String relocations
	physx::shdfnd::Array<StringReloc, Traits::Allocator> mStrings;

	//Keep info about relocations after flattening
	physx::shdfnd::Array<MergedReloc, Traits::Allocator> mMerges;

	Dictionary &dict;

	uint32_t mTotalAlign;
};

#include "PlatformOutputStream.inl"

}

#endif
#endif
