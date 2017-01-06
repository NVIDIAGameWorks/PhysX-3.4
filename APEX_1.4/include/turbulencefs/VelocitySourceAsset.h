/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef VELOCITY_SOURCE_ASSET_H
#define VELOCITY_SOURCE_ASSET_H

#include "Apex.h"
#include "CustomBufferIterator.h"

#define VELOCITY_SOURCE_AUTHORING_TYPE_NAME "VelocitySourceAsset"

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
class VelocitySourceAsset : public Asset
{
protected:
	virtual ~VelocitySourceAsset() {}
};


/**
 \brief Turbulence FieldSampler Asset Authoring class
 */
class VelocitySourceAssetAuthoring : public AssetAuthoring
{
protected:
	virtual ~VelocitySourceAssetAuthoring() {}
};


}
} // end namespace nvidia

#endif // VELOCITY_SOURCE_ASSET_H
