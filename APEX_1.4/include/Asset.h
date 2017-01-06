/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef ASSET_H
#define ASSET_H

/*!
\file
\brief class Asset
*/

#include "ApexInterface.h"
#include "ApexSDK.h"
#include "AssetPreviewScene.h"

namespace nvidia
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

/// Usual typedef
typedef const char* PlatformTag;

class Actor;
class AssetPreview;

/**
\brief Base class of all APEX assets
*/
class Asset : public ApexInterface
{
public:
	/**
	 * \brief Returns the name of this deserialized asset
	 */
	virtual const char* getName() const = 0;

	/**
	 * \brief Returns the ID of the asset's authorable object type.
	 *
	 * Every asset will correspond to an APEX authorable object type.
	 * The module must register those types with the SDK at startup.
	 */
	virtual AuthObjTypeID getObjTypeID() const = 0;

	/**
	 * \brief Returns the name of this asset's authorable object type
	 */
	virtual const char* getObjTypeName() const = 0;

	/**
	 * \brief Returns the number of assets force loaded by all of
	 * this asset's named asset references
	 */
	virtual uint32_t forceLoadAssets() = 0;

	/**
	 * \brief Returns the default actor descriptor NvParamaterized interface
	 * Memory ownership stays with this asset.  The user may modify the interface values, but they will not persist past another acll to 'getDefaultActorDesc'
	 * Typically used to create an actor after making small local editing changes.
	 */
	virtual ::NvParameterized::Interface* getDefaultActorDesc() = 0;

	/**
	 * \brief Returns the default AssetPreview descriptor NvParamaterized interface
	 */
	virtual ::NvParameterized::Interface* getDefaultAssetPreviewDesc() = 0;

	/**
	 * \brief Returns the asset's NvParamaterized interface
	 * This cannot be directly modified!  It is read only to the user.
	 */
	virtual const ::NvParameterized::Interface* getAssetNvParameterized() const = 0;

	/**
	 * \brief Creates an Actor representing the Asset in a Scene
	 */
	virtual Actor* createApexActor(const ::NvParameterized::Interface& actorParams, Scene& apexScene) = 0;

	/**
	 * \brief Creates an Asset Preview for the asset.
	 */
	virtual AssetPreview* createApexAssetPreview(const ::NvParameterized::Interface& params, AssetPreviewScene* previewScene) = 0;

	/**
	 * \brief Releases the ApexAsset but returns the NvParameterized::Interface and *ownership* to the caller.
	 */
	virtual NvParameterized::Interface* releaseAndReturnNvParameterizedInterface(void) = 0;

	/**
	 * \brief Returns true if the asset is in a state that is valid for creating an actor.
	 * \param actorParams parameters of actor to create
	 * \returns true if call to createApexActor will return true on inputs, false otherwise
	 */
	virtual bool isValidForActorCreation(const ::NvParameterized::Interface& actorParams, Scene& /*apexScene*/) const = 0;

	/**
	 * \brief Returns true if the parameterized object of the asset has been modified.
	 *
	 * This flag will be reset once the parameterized object has been serialized again.
	 */
	virtual bool isDirty() const = 0;

protected:
	virtual ~Asset() {}; // illegal, do not call
};

/**
\brief base class of all APEX asset authoring classes
*/
class AssetAuthoring : public ApexInterface
{
public:
	/**
	 * \brief Returns the name of asset author
	 */
	virtual const char* getName() const = 0;

	/**
	 * \brief Returns the name of this APEX authorable object type
	 */
	virtual const char* getObjTypeName() const = 0;

	/**
	 * \brief Prepares a fully authored Asset Authoring object for a specified platform
	 */
	virtual bool prepareForPlatform(nvidia::apex::PlatformTag) = 0;

	/**
	 * \brief Returns the asset's NvParameterized interface, may return NULL
	 */
	virtual ::NvParameterized::Interface* getNvParameterized() const = 0;

	/**
	 * \brief Releases the ApexAsset but returns the NvParameterized::Interface and *ownership* to the caller.
	 */
	virtual NvParameterized::Interface* releaseAndReturnNvParameterizedInterface(void) = 0;

	/**
	 * \brief Generates a string that is stored in the asset with all the relevant information about the build
	 *
	 * \param toolName			The name of the tool with proper casing, i.e. "Clothing Tool".
	 * \param toolVersion		The version of the tool as a string, can be NULL.
	 * \param toolChangelist	The CL# of the tool, will use internal tools directory CL if 0
	 */
	virtual void setToolString(const char* toolName, const char* toolVersion, uint32_t toolChangelist) = 0;
};

PX_POP_PACK

}
} // end namespace nvidia::apex

#endif // ASSET_H
