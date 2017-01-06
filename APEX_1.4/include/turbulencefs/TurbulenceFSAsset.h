/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef TURBULENCE_FSASSET_H
#define TURBULENCE_FSASSET_H

#include "Apex.h"
#include "CustomBufferIterator.h"

#define TURBULENCE_FS_AUTHORING_TYPE_NAME "TurbulenceFSAsset"

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
class TurbulenceFSAsset : public Asset
{
protected:
	virtual ~TurbulenceFSAsset() {}

};

/**
 \brief Turbulence FieldSampler Asset Authoring class
 */
class TurbulenceFSAssetAuthoring : public AssetAuthoring
{
protected:
	virtual ~TurbulenceFSAssetAuthoring() {}

};

}
} // end namespace nvidia

#endif // TURBULENCE_FSASSET_H
