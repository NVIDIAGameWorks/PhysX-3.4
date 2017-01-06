/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef FLAME_EMITTER_ASSET_H
#define FLAME_EMITTER_ASSET_H

#include "Apex.h"
#include "CustomBufferIterator.h"

#define FLAME_EMITTER_AUTHORING_TYPE_NAME "FlameEmitterAsset"

namespace nvidia
{
namespace apex
{

/**
 \brief Flame Emitter Asset class
 */
class FlameEmitterAsset : public Asset
{
protected:
	virtual ~FlameEmitterAsset() {}
};


/**
 \brief Flame Emitter Asset Authoring class
 */
class FlameEmitterAssetAuthoring : public AssetAuthoring
{
protected:
	virtual ~FlameEmitterAssetAuthoring() {}
};


}
} // end namespace nvidia

#endif // FLAME_EMITTER_ASSET_H
