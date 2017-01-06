/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef FORCE_FIELD_ASSET_H
#define FORCE_FIELD_ASSET_H

#include "Apex.h"
#include "ForceFieldPreview.h"

namespace nvidia
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

#define FORCEFIELD_AUTHORING_TYPE_NAME "ForceFieldAsset"

class ForceFieldActor;

/**
 \brief ForceField Asset
 */
class ForceFieldAsset : public Asset
{
protected:
	/**
	\brief force field asset default destructor.
	*/
	virtual ~ForceFieldAsset() {}

public:
	/**
	\brief returns the default scale of the asset.
	*/
	virtual float		getDefaultScale() const = 0;

	/**
	\brief release an actor created from this asset.
	*/
	virtual void		releaseForceFieldActor(ForceFieldActor&) = 0;
};

/**
 \brief ForceField Asset Authoring
 */
class ForceFieldAssetAuthoring : public AssetAuthoring
{
protected:
	/**
	\brief force field asset authoring default destructor.
	*/
	virtual ~ForceFieldAssetAuthoring() {}

public:
};


PX_POP_PACK

} // namespace apex
} // namespace nvidia

#endif // FORCE_FIELD_ASSET_H
