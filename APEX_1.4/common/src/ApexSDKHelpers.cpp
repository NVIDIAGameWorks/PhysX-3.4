/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "Apex.h"
#include "ApexSDKIntl.h"
#include "ApexResource.h"
#include "ApexSDKHelpers.h"
#include "AuthorableObjectIntl.h"

#include "ApexPvdClient.h"

#ifndef WITHOUT_PVD
#include "PxPvdDataStream.h"
#endif

namespace nvidia
{
using namespace physx::pvdsdk;

namespace apex
{

/*
	ResourceList functions
 */
ResourceList::~ResourceList()
{
	clear();
}

void ResourceList::clear()
{
	ScopedWriteLock scopedLock(mRWLock);

	uint32_t s = mArray.size();
	while (s--)
	{
		mArray.back()->release();
		if (mArray.size() != s)
		{
			PX_ASSERT(!"Error - resource did not remove itself from list upon release\n");
			if (mArray.size())
			{
				mArray.popBack();	// Force removal
			}
		}
	}
}

#ifndef WITHOUT_PVD
void ResourceList::setupForPvd(const void* owner, const char* listName, const char* entryName)
{
	mOwner = owner;
	mListName = listName;
	mEntryName = entryName;
}


void ResourceList::initPvdInstances(PvdDataStream& pvdStream)
{
	PX_UNUSED(pvdStream);
	ScopedWriteLock scopedLock(mRWLock);

	for (uint32_t i = 0; i < mArray.size(); ++i)
	{
		const void* entry = (const void*)mArray[i];
		pvdStream.createInstance(NamespacedName(APEX_PVD_NAMESPACE, mEntryName.c_str()), entry);
		pvdStream.pushBackObjectRef(mOwner, mListName.c_str(), entry);
		mArray[i]->initPvdInstances(pvdStream);
	}
}
#endif

void ResourceList::add(ApexResourceInterface& resource)
{
	ScopedWriteLock scopedLock(mRWLock);

	if (resource.getListIndex() != 0xFFFFFFFF)
	{
		PX_ASSERT(!"Error - attempting to add a resource to a list twice");
		return;
	}
	resource.setListIndex(*this, mArray.size());
	mArray.pushBack(&resource);

	// add to pvd
	if (mOwner != NULL)
	{
		ApexPvdClient* client = GetInternalApexSDK()->getApexPvdClient();
		if (client != NULL)
		{
			if (client->isConnected() && client->getPxPvd().getInstrumentationFlags() & PxPvdInstrumentationFlag::eDEBUG)
			{
				PvdDataStream* pvdStream = client->getDataStream();
				{
					if (pvdStream != NULL)
					{
						pvdStream->createInstance(NamespacedName(APEX_PVD_NAMESPACE, mEntryName.c_str()), &resource);
						pvdStream->pushBackObjectRef(mOwner, mListName.c_str(), &resource);
						resource.initPvdInstances(*pvdStream);
					}
				}
			}
		}
	}
}

void ResourceList::remove(uint32_t index)
{
	ScopedWriteLock scopedLock(mRWLock);

	PX_ASSERT(index < mArray.size());

	// remove from pvd
	if (mOwner != NULL)
	{
		ApexPvdClient* client = GetInternalApexSDK()->getApexPvdClient();
		if (client != NULL)
		{
			if (client->isConnected() && client->getPxPvd().getInstrumentationFlags() & PxPvdInstrumentationFlag::eDEBUG)
			{
				PvdDataStream* pvdStream = client->getDataStream();
				{
					if (pvdStream != NULL)
					{
						// would be nice to be able to call resource->destroyPvdInstances() here,
						// but the resource has already been destroyed, so it's too late here
						ApexResourceInterface* resource = mArray[index];
						pvdStream->removeObjectRef(mOwner, mListName.c_str(), resource);
						pvdStream->destroyInstance(resource);
					}
				}
			}
		}
	}

	mArray.replaceWithLast(index);
	if (index < mArray.size())
	{
		mArray[index]->setListIndex(*this, index);
	}
}

#if 0
// these are poison
void writeStreamHeader(PxFileBuf& stream, ApexSimpleString& streamName, uint32_t versionStamp)
{
	uint32_t streamStamp = GetStamp(streamName);

	stream.storeDword(versionStamp);
	stream.storeDword(streamStamp);
}

uint32_t readStreamHeader(const PxFileBuf& stream, ApexSimpleString& streamName)
{
	uint32_t version = stream.readDword();

	uint32_t streamStamp = stream.readDword();
	if (streamStamp != GetStamp(streamName))
	{
		APEX_INVALID_PARAMETER("Wrong input stream. The provided stream has to contain %s.", streamName.c_str());
		return (uint32_t) - 1;
	}

	return version;
}
#endif

/******************************************************************************
 * Helper function for loading assets
 *
 * This method's purpose is to generalize this process:
 *
 * 1. Get the module's namespace resource ID (this also checks that the module is loaded)
 * 2. If the asset has not been created yet, it will call createResource()
 * 3. It will call getResource() and return the result
 *
 * This allows both the asset's forceLoad method AND the actors init methods to get
 * an asset pointer.
 *
 * This also allows the forceLoad method to call this method repeatedly until getResource()
 * returns a valid pointer (for async loading).

 *****************************************************************************/
void* ApexAssetHelper::getAssetFromName(ApexSDKIntl*	sdk,
                                        const char*	authoringTypeName,
                                        const char*	assetName,
                                        ResID&		inOutResID,
                                        ResID		optionalPsID)
{
	/* Get the NRP */
	ResourceProviderIntl* nrp = sdk->getInternalResourceProvider();

	/* Get the asset namespace ID */
	ResID typePsID = INVALID_RESOURCE_ID;
	if (optionalPsID == INVALID_RESOURCE_ID)
	{
		AuthorableObjectIntl* ao = sdk->getAuthorableObject(authoringTypeName);
		if (ao)
		{
			typePsID = ao->getResID();
		}
		else
		{
			APEX_INTERNAL_ERROR("Unknown authorable type: %s, please load all required modules.", authoringTypeName);
			return NULL;
		}
	}
	else
	{
		typePsID = optionalPsID;
	}

	if (inOutResID == INVALID_RESOURCE_ID)
	{
		if (optionalPsID == sdk->getOpaqueMeshNameSpace())
		{
			AuthorableObjectIntl* ao = sdk->getAuthorableObject(RENDER_MESH_AUTHORING_TYPE_NAME);
			if (ao)
			{
				typePsID = ao->getResID();
			}

			ResID opaqueMesh = nrp->createResource(optionalPsID, assetName);
			inOutResID = nrp->createResource(typePsID, assetName);
			if (!nrp->checkResource(inOutResID))
			{
				UserOpaqueMesh* om = (UserOpaqueMesh*)nrp->getResource(opaqueMesh);
				Asset* asset = sdk->createAsset(assetName, om);
				nrp->setResource(authoringTypeName, assetName, asset, true, false);
			}
		}
		else
		{
			inOutResID = nrp->createResource(typePsID, assetName);
		}
	}

	// If resID is valid, get the resource
	if (inOutResID != INVALID_RESOURCE_ID)
	{
		return nrp->getResource(inOutResID);
	}
	else
	{
		APEX_DEBUG_INFO("ApexAssetHelper::getAssetFromName: Could not create resource ID asset: %s", assetName);
		return NULL;
	}

}



/* getAssetFromNameList
 *
 * This method's purpose is to generalize this process:
 *
 * 1. Find the asset name in this asset type's name list
 * 2. Call the SDK's helper method, getAssetFromName
 *
 * This allows both the asset's forceLoad method AND the actors init methods to get
 * an asset pointer.
 *
 * This also allows the forceLoad method to call this method repeatedly until getResource()
 * returns a valid pointer (for async loading).
 */
void* ApexAssetHelper::getAssetFromNameList(ApexSDKIntl*	sdk,
        const char* authoringTypeName,
        physx::Array<AssetNameIDMapping*>& nameIdList,
        const char* assetName,
        ResID assetPsId)
{
	// find the index of the asset name in our list of name->resID maps
	uint32_t assetIdx = 0;
	for (assetIdx = 0; assetIdx < nameIdList.size(); assetIdx++)
	{
		if (nameIdList[assetIdx]->assetName == assetName)
		{
			break;
		}
	}

	// This can't ever happen
	PX_ASSERT(assetIdx < nameIdList.size());

	if (assetIdx < nameIdList.size())
	{
		return ApexAssetHelper::getAssetFromName(sdk,
		        authoringTypeName,
		        assetName,
		        nameIdList[assetIdx]->resID,
		        assetPsId);
	}
	else
	{
		APEX_DEBUG_WARNING("Request for asset %s of type %s not registered in asset tracker's list", assetName, authoringTypeName);
		return NULL;
	}
}

}
} // end namespace nvidia::apex
