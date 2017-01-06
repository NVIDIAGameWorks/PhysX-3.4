/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODULE_EMITTER_H
#define MODULE_EMITTER_H

#include "Apex.h"

namespace nvidia
{
namespace apex
{


PX_PUSH_PACK_DEFAULT

class EmitterAsset;
class EmitterAssetAuthoring;

class GroundEmitterAsset;
class GroundEmitterAssetAuthoring;

class ImpactEmitterAsset;
class ImpactEmitterAssetAuthoring;


/**
\brief An APEX Module that provides generic Emitter classes
*/
class ModuleEmitter : public Module
{
protected:
	virtual ~ModuleEmitter() {}

public:

	/// get rate scale. Rate parameter in all emitters will be multiplied by rate scale.
	virtual float 	getRateScale() const = 0;
	
	/// get density scale. Density parameter in all emitters except ground emitters will be multiplied by density scale.
	virtual float 	getDensityScale() const = 0;
	
	/// get ground density scale. Density parameter in all ground emitters will be multiplied by ground density scale.
	virtual float 	getGroundDensityScale() const = 0;

	/// set rate scale. Rate parameter in all module emitters will be multiplied by rate scale.
	virtual void 	setRateScale(float rateScale) = 0;

	/// set density scale. Density parameter in all emitters except ground emitters will be multiplied by density scale.
	virtual void 	setDensityScale(float densityScale) = 0;

	/// set ground density scale. Density parameter in all ground emitters will be multiplied by ground density scale.
	virtual void 	setGroundDensityScale(float groundDensityScale) = 0;
};



PX_POP_PACK

}
} // end namespace nvidia

#endif // MODULE_EMITTER_H
