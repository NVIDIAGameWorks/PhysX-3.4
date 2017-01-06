/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef BASIC_FSASSET_H
#define BASIC_FSASSET_H

#include "Apex.h"

namespace nvidia
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

#define JET_FS_AUTHORING_TYPE_NAME	"JetFSAsset"
#define ATTRACTOR_FS_AUTHORING_TYPE_NAME	"AttractorFSAsset"
#define VORTEX_FS_AUTHORING_TYPE_NAME	"VortexFSAsset"
#define NOISE_FS_AUTHORING_TYPE_NAME "NoiseFSAsset"
#define WIND_FS_AUTHORING_TYPE_NAME	"WindFSAsset"

/**
 \brief BasicFS Asset class
 */
class BasicFSAsset : public Asset
{
protected:
	virtual ~BasicFSAsset() {}

public:
};

/**
 \brief BasicFS Asset Authoring class
 */
class BasicFSAssetAuthoring : public AssetAuthoring
{
protected:
	virtual ~BasicFSAssetAuthoring() {}

public:
};


PX_POP_PACK

}
} // end namespace nvidia::apex

#endif // BASIC_FSASSET_H
