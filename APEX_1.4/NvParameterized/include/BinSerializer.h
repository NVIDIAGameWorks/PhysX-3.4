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

#ifndef PX_BIN_SERIALIZER_H
#define PX_BIN_SERIALIZER_H

// APB serializer

// WARNING: before doing any changes to this file
// check comments at the head of BinSerializer.cpp

#include "nvparameterized/NvParameterized.h"
#include "nvparameterized/NvSerializer.h"

#include "AbstractSerializer.h"

#include "ApbDefinitions.h"

namespace NvParameterized
{

class DefinitionImpl;
class PlatformInputStream;
class PlatformOutputStream;
struct PlatformABI;

class BinSerializer : public AbstractSerializer
{
	// Methods for updating legacy formats
	Serializer::ErrorType updateInitial2AllCounted(BinaryHeader &hdr, char *start);
	Serializer::ErrorType updateAllCounted2WithAlignment(BinaryHeader &hdr, char *start);

	Serializer::ErrorType readMetadataInfo(const BinaryHeader &hdr, PlatformInputStream &s, DefinitionImpl *def);

	// Read array of arbitrary type (slow version)
	Serializer::ErrorType readArraySlow(Handle &handle, PlatformInputStream &s);

	// Read NvParameterized object data
	Serializer::ErrorType readObject(NvParameterized::Interface *&obj, PlatformInputStream &data);

	// Read binary data of NvParameterized object addressed by handle
	Serializer::ErrorType readBinaryData(Handle &handle, PlatformInputStream &data);

#ifndef WITHOUT_APEX_SERIALIZATION
	Serializer::ErrorType storeMetadataInfo(const Definition *def, PlatformOutputStream &s);

	// Store array of arbitrary type (slow version)
	Serializer::ErrorType storeArraySlow(Handle &handle, PlatformOutputStream &s);

	// Print binary data for part of NvParameterized object addressed by handle
	Serializer::ErrorType storeBinaryData(const NvParameterized::Interface &obj, Handle &handle, PlatformOutputStream &res, bool isRootObject = true);
#endif

	BinSerializer(BinSerializer &); // Don't
	void operator=(BinSerializer &); // Don't

	Serializer::ErrorType verifyFileHeader(
		const BinaryHeader &hdr,
		const BinaryHeaderExt *ext,
		uint32_t dataLen ) const;

	Serializer::ErrorType getPlatformInfo(
		BinaryHeader &hdr,
		BinaryHeaderExt *ext,
		PlatformABI &abi ) const;

	Serializer::ErrorType verifyObjectHeader(const ObjHeader &hdr, const Interface *obj, Traits *traits) const;

protected:

	Serializer::ErrorType internalDeserialize(physx::PxFileBuf &stream, Serializer::DeserializedData &res, bool &doesNeedUpdate);
	Serializer::ErrorType internalDeserializeInplace(void *mdata, uint32_t dataLen, Serializer::DeserializedData &res, bool &doesNeedUpdate);

#ifndef WITHOUT_APEX_SERIALIZATION
	Serializer::ErrorType internalSerialize(physx::PxFileBuf &stream,const NvParameterized::Interface **objs, uint32_t nobjs, bool doMetadata);
#endif

public:
	BinSerializer(Traits *traits): AbstractSerializer(traits) {}

	void release()
	{
		Traits *t = mTraits;
		this->~BinSerializer();
		serializerMemFree(this, t);
	}

	static const uint32_t Magic = APB_MAGIC;
	static const uint32_t Version = BinVersions::WithExtendedHeader;

	Serializer::ErrorType peekNumObjects(physx::PxFileBuf &stream, uint32_t &numObjects);

	Serializer::ErrorType peekNumObjectsInplace(const void *data, uint32_t dataLen, uint32_t &numObjects);

	Serializer::ErrorType peekClassNames(physx::PxFileBuf &stream, char **classNames, uint32_t &numClassNames);

	Serializer::ErrorType peekInplaceAlignment(physx::PxFileBuf& stream, uint32_t& align);

	Serializer::ErrorType deserializeMetadata(physx::PxFileBuf &stream, DeserializedMetadata &desData);
};

bool isBinaryFormat(physx::PxFileBuf &stream);

Serializer::ErrorType peekBinaryPlatform(physx::PxFileBuf &stream, SerializePlatform &platform);

} // namespace NvParameterized

#endif
