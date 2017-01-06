/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef HEAT_SOURCE_ASSET_H
#define HEAT_SOURCE_ASSET_H

#include "Apex.h"
#include "CustomBufferIterator.h"

#define HEAT_SOURCE_AUTHORING_TYPE_NAME "HeatSourceAsset"

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
class HeatSourceAsset : public Asset
{
protected:
	virtual ~HeatSourceAsset() {}
};


/**
 \brief Turbulence FieldSampler Asset Authoring class
 */
class HeatSourceAssetAuthoring : public AssetAuthoring
{
protected:
	virtual ~HeatSourceAssetAuthoring() {}
};


}
} // end namespace nvidia

#endif // HEAT_SOURCE_ASSET_H
