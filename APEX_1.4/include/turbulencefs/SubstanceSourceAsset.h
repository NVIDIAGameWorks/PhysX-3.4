/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef SUBSTANCE_SOURCE_ASSET_H
#define SUBSTANCE_SOURCE_ASSET_H

#include "Apex.h"
#include "CustomBufferIterator.h"

#define SUBSTANCE_SOURCE_AUTHORING_TYPE_NAME "SubstanceSourceAsset"

namespace nvidia
{
namespace apex
{

class TurbulenceFSActorParams;
class TurbulenceFSActor;
class TurbulenceFSPreview;

/**
 \brief Turbulence FieldSampler Asset class
 */
class SubstanceSourceAsset : public Asset
{
protected:
	virtual ~SubstanceSourceAsset() {}
};


/**
 \brief Turbulence FieldSampler Asset Authoring class
 */
class SubstanceSourceAssetAuthoring : public AssetAuthoring
{
protected:
	virtual ~SubstanceSourceAssetAuthoring() {}
};


}
} // end namespace nvidia

#endif // SUBSTANCE_SOURCE_ASSET_H
