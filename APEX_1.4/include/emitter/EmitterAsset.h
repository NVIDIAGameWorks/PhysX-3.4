/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef EMITTER_ASSET_H
#define EMITTER_ASSET_H

#include "Apex.h"
#include "EmitterGeoms.h"

namespace nvidia
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

#define EMITTER_AUTHORING_TYPE_NAME "ApexEmitterAsset"

class EmitterActor;
class EmitterPreview;
class EmitterLodParamDesc;

///APEX Emitter asset. Emits particles within some shape.
class EmitterAsset : public Asset
{
protected:
	virtual ~EmitterAsset() {}

public:
	
	/// Returns the explicit geometry for the all actors based on this asset if the asset is explicit, NULL otherwise
	virtual EmitterGeomExplicit* 	isExplicitGeom() = 0;

	/// Returns the geometry used for the all actors based on this asset
	virtual const EmitterGeom*		getGeom() const = 0;

	/// Gets IOFX asset name that is used to visualize partices of this emitter
	virtual const char* 			getInstancedObjectEffectsAssetName(void) const = 0;
	
	/// Gets IOS asset name that is used to simulate partices of this emitter
	virtual const char* 			getInstancedObjectSimulatorAssetName(void) const = 0;
	
	/// Gets IOS asset class name that is used to simulate partices of this emitter
	virtual const char* 			getInstancedObjectSimulatorTypeName(void) const = 0;

	virtual const float & 			getDensity() const = 0; ///< Gets the range used to choose the density of particles	
	virtual const float & 			getRate() const = 0;	///< Gets the range used to choose the emission rate
	virtual const PxVec3 & 			getVelocityLow() const = 0; ///< Gets the range used to choose the velocity of particles
	virtual const PxVec3 & 			getVelocityHigh() const = 0; ///< Gets the range used to choose the velocity of particles
	virtual const float & 			getLifetimeLow() const = 0; ///< Gets the range used to choose the lifetime of particles
	virtual const float & 			getLifetimeHigh() const = 0; ///< Gets the range used to choose the lifetime of particles
	
	/// For an explicit emitter, Max Samples is ignored.  For shaped emitters, it is the maximum number of objects spawned in a step.
	virtual uint32_t                getMaxSamples() const = 0;

	/**
	\brief Gets the emitter duration in seconds
	\note If EmitterActor::startEmit() is called with persistent=true, then this duration is ignored.
	*/
	virtual float					getEmitDuration() const = 0;

	/// Gets LOD settings for this asset
	virtual const EmitterLodParamDesc& getLodParamDesc() const = 0;

};

///APEX Emitter Asset Authoring. Used to create APEX Emitter assets.
class EmitterAssetAuthoring : public AssetAuthoring
{
protected:
	virtual ~EmitterAssetAuthoring() {}
};


PX_POP_PACK

}
} // end namespace nvidia

#endif // EMITTER_ASSET_H
