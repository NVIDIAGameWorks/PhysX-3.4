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

#include "PsHash.h"
#include "PsHashMap.h"
#include "PsFoundation.h"
#include "CmIO.h"
#include "SnFile.h"
#include "PsString.h"
#include "extensions/PxSerialization.h"
#include "PxPhysics.h"
#include "PxPhysicsSerialization.h"
#include "SnSerializationContext.h"
#include "PxSerializer.h"
#include "serialization/SnSerializationRegistry.h"
#include "serialization/SnSerialUtils.h"
#include "CmCollection.h"
#include "SnConvX_Align.h"

using namespace physx;
using namespace Sn;

namespace
{
	PX_INLINE PxU8* alignPtr(PxU8* ptr, PxU32 alignment = PX_SERIAL_ALIGN)
	{
		if(!alignment)
			return ptr;

		const PxU32 padding = getPadding(size_t(ptr), alignment);
		PX_ASSERT(!getPadding(size_t(ptr + padding), alignment));
		return ptr + padding;
	}

	PX_FORCE_INLINE PxU32 read32(PxU8*& address)
	{
		const PxU32 value = *reinterpret_cast<PxU32*>(address);
		address += sizeof(PxU32);
		return value;
	}

	bool readHeader(PxU8*& address, PxU32& version)
	{
		const PxU32 header = read32(address);
		PX_UNUSED(header);
		
		version = read32(address);

		const PxU32 binaryVersion = read32(address);
		PX_UNUSED(binaryVersion);		
		const PxU32 buildNumber = read32(address);
		PX_UNUSED(buildNumber);
		const PxU32 platformTag = read32(address);
		PX_UNUSED(platformTag);
		const PxU32 markedPadding = read32(address);
		PX_UNUSED(markedPadding);

#if PX_CHECKED
		if (header != PX_MAKE_FOURCC('S','E','B','D'))
		{
			Ps::getFoundation().error(physx::PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, 
				"Buffer contains data with wrong header indicating invalid binary data.");
			return false;
		}

		if (!checkCompatibility(version, binaryVersion))
		{
			char buffer[512];
		    getCompatibilityVersionsStr(buffer, 512);

			Ps::getFoundation().error(physx::PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, 
				"Buffer contains data version (%x-%d) is incompatible with this PhysX sdk.\n These versions would be compatible: %s",
				version, binaryVersion, buffer);
			return false;
		}

		if (platformTag != getBinaryPlatformTag())
		{
			Ps::getFoundation().error(physx::PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, 
				"Buffer contains data with platform mismatch:\nExpected: %s \nActual: %s\n",
				getBinaryPlatformName(getBinaryPlatformTag()),
				getBinaryPlatformName(platformTag));
			return false;
		}
#endif
		return true;
	}

	bool checkImportReferences(const ImportReference* importReferences, PxU32 nbImportReferences, const Cm::Collection* externalRefs)
	{
		if (!externalRefs)
		{
			if (nbImportReferences > 0)
			{
				Ps::getFoundation().error(PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, "PxSerialization::createCollectionFromBinary: External references needed but no externalRefs collection specified.");
				return false;			
			}
		}
		else
		{
			for (PxU32 i=0; i<nbImportReferences;i++)
			{
				PxSerialObjectId id = importReferences[i].id;
				PxType type = importReferences[i].type;

				PxBase* referencedObject = externalRefs->find(id);
				if (!referencedObject)
				{
					Ps::getFoundation().error(PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, "PxSerialization::createCollectionFromBinary: External reference %" PX_PRIu64 " expected in externalRefs collection but not found.", id);
					return false;
				}
				if (referencedObject->getConcreteType() != type)
				{
					Ps::getFoundation().error(PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, "PxSerialization::createCollectionFromBinary: External reference %d type mismatch. Expected %d but found %d in externalRefs collection.", type, referencedObject->getConcreteType());
					return false;
				}
			}
		}
		return true;
	}
	
}

PxCollection* PxSerialization::createCollectionFromBinary(void* memBlock, PxSerializationRegistry& sr, const PxCollection* pxExternalRefs)
{
#if PX_CHECKED
	if(size_t(memBlock) & (PX_SERIAL_FILE_ALIGN-1))
	{
		Ps::getFoundation().error(PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, "Buffer must be 128-bytes aligned.");
		return NULL;
	}
#endif
	PxU8* address = reinterpret_cast<PxU8*>(memBlock);
	const Cm::Collection* externalRefs = static_cast<const Cm::Collection*>(pxExternalRefs);
			
	PxU32 version;
	if (!readHeader(address, version))
	{
		return NULL;
	}

	ManifestEntry* manifestTable;
	PxU32 nbObjectsInCollection;
	PxU32 objectDataEndOffset;

	// read number of objects in collection
	address = alignPtr(address);
	nbObjectsInCollection = read32(address);

	// read manifest (PxU32 offset, PxConcreteType type)
	{
		address = alignPtr(address);
		PxU32 nbManifestEntries = read32(address);
		PX_ASSERT(*reinterpret_cast<PxU32*>(address) == 0); //first offset is always 0
		manifestTable = (nbManifestEntries > 0) ? reinterpret_cast<ManifestEntry*>(address) : NULL;
		address += nbManifestEntries*sizeof(ManifestEntry);
		objectDataEndOffset = read32(address);
	}

	ImportReference* importReferences;
	PxU32 nbImportReferences;
	// read import references
	{
		address = alignPtr(address);
		nbImportReferences = read32(address);
		importReferences = (nbImportReferences > 0) ? reinterpret_cast<ImportReference*>(address) : NULL;
		address += nbImportReferences*sizeof(ImportReference);
	}

	if (!checkImportReferences(importReferences, nbImportReferences, externalRefs))
	{
		return NULL;
	}

	ExportReference* exportReferences;
	PxU32 nbExportReferences;
	// read export references
	{
		address = alignPtr(address);
		nbExportReferences = read32(address);
		exportReferences = (nbExportReferences > 0) ? reinterpret_cast<ExportReference*>(address) : NULL;
		address += nbExportReferences*sizeof(ExportReference);
	}

	// read internal references arrays
	PxU32 nbInternalPtrReferences = 0;
	PxU32 nbInternalIdxReferences = 0;
	InternalReferencePtr* internalPtrReferences = NULL;
	InternalReferenceIdx* internalIdxReferences = NULL;
	{
		address = alignPtr(address);

		nbInternalPtrReferences = read32(address);
		internalPtrReferences = (nbInternalPtrReferences > 0) ? reinterpret_cast<InternalReferencePtr*>(address) : NULL;
		address += nbInternalPtrReferences*sizeof(InternalReferencePtr);

		nbInternalIdxReferences = read32(address);
		internalIdxReferences = (nbInternalIdxReferences > 0) ? reinterpret_cast<InternalReferenceIdx*>(address) : NULL;
		address += nbInternalIdxReferences*sizeof(InternalReferenceIdx);
	}

	// create internal references map
	PxF32 loadFactor = 0.75f;
	PxF32 _loadFactor = 1.0f / loadFactor;
	PxU32 hashSize = PxU32((nbInternalPtrReferences + nbInternalIdxReferences + 1)*_loadFactor);
	InternalRefMap internalReferencesMap(hashSize, loadFactor);
	{
		//create hash (we should load the hashes directly from memory)
		for (PxU32 i=0;i<nbInternalPtrReferences;i++)
		{
			const InternalReferencePtr& ref = internalPtrReferences[i];
			internalReferencesMap.insertUnique( InternalRefKey(ref.reference, ref.kind), SerialObjectIndex(ref.objIndex));
		}
		for (PxU32 i=0;i<nbInternalIdxReferences;i++)
		{
			const InternalReferenceIdx& ref = internalIdxReferences[i];
			internalReferencesMap.insertUnique(InternalRefKey(ref.reference, ref.kind), SerialObjectIndex(ref.objIndex));
		}
	}

	SerializationRegistry& sn = static_cast<SerializationRegistry&>(sr);
	Cm::Collection* collection = static_cast<Cm::Collection*>(PxCreateCollection());
	PX_ASSERT(collection);
	collection->mObjects.reserve(PxU32(nbObjectsInCollection*_loadFactor) + 1);
	if(nbExportReferences > 0)
	    collection->mIds.reserve(PxU32(nbExportReferences*_loadFactor) + 1);

	PxU8* addressObjectData = alignPtr(address);
	PxU8* addressExtraData = alignPtr(addressObjectData + objectDataEndOffset);

	DeserializationContext context(manifestTable, importReferences, addressObjectData, internalReferencesMap, externalRefs, addressExtraData, version);
	
	// iterate over memory containing PxBase objects, create the instances, resolve the addresses, import the external data, add to collection.
	{
		PxU32 nbObjects = nbObjectsInCollection;

		while(nbObjects--)
		{
			address = alignPtr(address);
			context.alignExtraData();

			// read PxBase header with type and get corresponding serializer.
			PxBase* header = reinterpret_cast<PxBase*>(address);
			const PxType classType = header->getConcreteType();
			const PxSerializer* serializer = sn.getSerializer(classType);
			PX_ASSERT(serializer);

			PxBase* instance = serializer->createObject(address, context);
			if (!instance)
			{
				Ps::getFoundation().error(physx::PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, 
					"Cannot create class instance for concrete type %d.", classType);
				collection->release();
				return NULL;
			}

			collection->internalAdd(instance);
		}
	}

	PX_ASSERT(nbObjectsInCollection == collection->internalGetNbObjects());
	
	// update new collection with export references
	{
		PX_ASSERT(addressObjectData != NULL);
		for (PxU32 i=0;i<nbExportReferences;i++)
		{
			bool isExternal;
			PxU32 manifestIndex = exportReferences[i].objIndex.getIndex(isExternal);
			PX_ASSERT(!isExternal);
			PxBase* obj = reinterpret_cast<PxBase*>(addressObjectData + manifestTable[manifestIndex].offset);
			collection->mIds.insertUnique(exportReferences[i].id, obj);
			collection->mObjects[obj] = exportReferences[i].id;
		}
	}

	PxAddCollectionToPhysics(*collection);
	return collection;
}
