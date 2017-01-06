/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef IOFX_ASSET_H
#define IOFX_ASSET_H

#include "Apex.h"
#include "ModifierDefs.h"

namespace nvidia
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

#define IOFX_AUTHORING_TYPE_NAME "IOFX"

class Modifier;
class ApexActor;

/**
 \brief IOFX Asset public interface. Used to define the way the visual parameters are created
		from physical parameters of a particle
*/
class IofxAsset : public Asset, public Context
{
public:
	///get the name of the material used for sprite-based particles visualization
	virtual const char*						getSpriteMaterialName() const = 0;

	///get the number of different mesh assets used for mesh-based particles visualization
	virtual uint32_t						getMeshAssetCount() const = 0;
	
	///get the name of one of the mesh assets used for mesh-based particles visualization
	/// \param index mesh asset internal index
	virtual const char*						getMeshAssetName(uint32_t index = 0) const = 0;
	
	///get the weight of one of the mesh assets used for mesh-based particles visualization. Can be any value; not normalized.
	/// \param index mesh asset internal index
	virtual uint32_t						getMeshAssetWeight(uint32_t index = 0) const = 0;

	///get the list of spawn modifiers
	virtual const Modifier*					getSpawnModifiers(uint32_t& outCount) const = 0;
	
	///get the list of continuous modifiers
	virtual const Modifier*					getContinuousModifiers(uint32_t& outCount) const = 0;

	///get the biggest possible scale given the current spawn- and continuous modifiers
	///note that some modifiers depend on velocity, so the scale can get arbitrarily large.
	/// \param maxVelocity this value defines what the highest expected velocity is to compute the upper bound
	virtual float							getScaleUpperBound(float maxVelocity) const = 0;

	///the IOFX asset needs to inform other actors when it is released
	/// \note only for internal use
	virtual void							addDependentActor(ApexActor* actor) = 0;
};

/**
 \brief IOFX Asset Authoring public interface.
 */
class IofxAssetAuthoring : public AssetAuthoring
{
};

PX_POP_PACK

}
} // namespace nvidia

#endif // IOFX_ASSET_H
