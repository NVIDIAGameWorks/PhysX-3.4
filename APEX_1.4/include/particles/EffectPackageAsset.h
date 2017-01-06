/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef EFFECT_PACKAGE_ASSET_H
#define EFFECT_PACKAGE_ASSET_H

#include "Apex.h"

namespace nvidia
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

#define PARTICLES_EFFECT_PACKAGE_AUTHORING_TYPE_NAME "EffectPackageAsset"

/**
\brief Describes an EffectPackageAsset; a collection of particle related effects (emitters, field samplers, etc.)
*/
class EffectPackageAsset : public Asset
{
protected:

	virtual ~EffectPackageAsset() {}

public:
	/**
	\brief returns the duration of the effect; accounting for maximum duration of each sub-effect.  A duration of 'zero' means it is an infinite lifetime effect.
	*/
	virtual float getDuration() const = 0;

	/**
	\brief This method returns true if the authored asset has the 'hint' set to indicate the actors should be created with a unique render volume.
	*/
	virtual bool useUniqueRenderVolume() const = 0;


}; //

/**
 \brief Describes an EffectPackageAssetAuthoring class; not currently used.  The ParticleEffectTool is used to
		author EffectPackageAssets
 */
class EffectPackageAssetAuthoring : public AssetAuthoring
{
protected:
	virtual ~EffectPackageAssetAuthoring() {}
};

PX_POP_PACK

} // end of apex namespace
} // end of nvidia namespace

#endif
