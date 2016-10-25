/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */



#include "ModifierImpl.h"

#include "ApexSDKHelpers.h"
#include "ApexSharedUtils.h"
//#include "ApexSharedSerialization.h"
#include "InstancedObjectSimulationIntl.h"
#include "IofxActor.h"
#include "ParamArray.h"
#include "IofxAssetImpl.h"


namespace nvidia
{
namespace iofx
{

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
RotationModifierImpl::RotationModifierImpl(RotationModifierParams* params) :
	mParams(params),
	mRollType(ApexMeshParticleRollType::SPHERICAL),
	mRollAxis(0),
	mRollSign(0),
	mLastUpdateTime(0.0f)
{
	NvParameterized::Handle h(*mParams);
	mParams->getParameterHandle("rollType", h);
	setRollType((ApexMeshParticleRollType::Enum) h.parameterDefinition()->enumValIndex(mParams->rollType));
}

// ------------------------------------------------------------------------------------------------
void RotationModifierImpl::setRollType(ApexMeshParticleRollType::Enum rollType)
{
	PX_ASSERT(rollType < ApexMeshParticleRollType::COUNT);
	mRollType = rollType;

	NvParameterized::Handle h(*mParams);
	mParams->getParameterHandle("rollType", h);
	mParams->setParamEnum(h, h.parameterDefinition()->enumVal((int)rollType));

	switch (mRollType)
	{
	default:
		mRollSign = 1.0f;
		mRollAxis = -1;
		break;
	case ApexMeshParticleRollType::FLAT_X:
		mRollSign = 1.0f;
		mRollAxis = 0;
		break;
	case ApexMeshParticleRollType::FLAT_Y:
		mRollSign = 1.0f;
		mRollAxis = 1;
		break;
	case ApexMeshParticleRollType::FLAT_Z:
		mRollSign = 1.0f;
		mRollAxis = 2;
		break;
	case ApexMeshParticleRollType::LONG_X:
		mRollSign = -1.0f;
		mRollAxis = 0;
		break;
	case ApexMeshParticleRollType::LONG_Y:
		mRollSign = -1.0f;
		mRollAxis = 1;
		break;
	case ApexMeshParticleRollType::LONG_Z:
		mRollSign = -1.0f;
		mRollAxis = 2;
		break;
	}
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
SimpleScaleModifierImpl::SimpleScaleModifierImpl(SimpleScaleModifierParams* params) :
	mParams(params)
{ }


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
RandomScaleModifierImpl::RandomScaleModifierImpl(RandomScaleModifierParams* param) :
	mParams(param)
{ }


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
ScaleByMassModifierImpl::ScaleByMassModifierImpl(ScaleByMassModifierParams* params) :
	mParams(params)
{ }


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
ColorVsLifeModifierImpl::ColorVsLifeModifierImpl(ColorVsLifeModifierParams* params) :
	mParams(params)
{
	NvParameterized::Handle h(*mParams);
	mParams->getParameterHandle("colorChannel", h);

	mColorChannel = (ColorChannel)(h.parameterDefinition()->enumValIndex(mParams->colorChannel));

	//do this in the Curve constructor... (for inplace stuff)
	ParamArray<Vec2R> cp(mParams, "controlPoints", (ParamDynamicArrayStruct*)&mParams->controlPoints);
	for (uint32_t i = 0; i < cp.size(); i++)
	{
		mCurveFunction.addControlPoint(cp[i]);
	}
}

// ------------------------------------------------------------------------------------------------

void ColorVsLifeModifierImpl::setColorChannel(ColorChannel colorChannel)
{
	mColorChannel = colorChannel;

	NvParameterized::Handle h(*mParams);
	mParams->getParameterHandle("colorChannel", h);
	mParams->setParamEnum(h, h.parameterDefinition()->enumVal((int)colorChannel));
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
ColorVsDensityModifierImpl::ColorVsDensityModifierImpl(ColorVsDensityModifierParams* params) :
	mParams(params)
{
	NvParameterized::Handle h(*mParams);
	mParams->getParameterHandle("colorChannel", h);

	mColorChannel = (ColorChannel)(h.parameterDefinition()->enumValIndex(mParams->colorChannel));

	//do this in the Curve constructor... (for inplace stuff)
	ParamArray<Vec2R> cp(mParams, "controlPoints", (ParamDynamicArrayStruct*)&mParams->controlPoints);
	for (uint32_t i = 0; i < cp.size(); i++)
	{
		mCurveFunction.addControlPoint(cp[i]);
	}
}

// ------------------------------------------------------------------------------------------------

void ColorVsDensityModifierImpl::setColorChannel(ColorChannel colorChannel)
{
	mColorChannel = colorChannel;

	NvParameterized::Handle h(*mParams);
	mParams->getParameterHandle("colorChannel", h);
	mParams->setParamEnum(h, h.parameterDefinition()->enumVal((int)colorChannel));
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
ColorVsVelocityModifierImpl::ColorVsVelocityModifierImpl(ColorVsVelocityModifierParams* params) :
	mParams(params)
{
	NvParameterized::Handle h(*mParams);
	mParams->getParameterHandle("colorChannel", h);

	mColorChannel = (ColorChannel)(h.parameterDefinition()->enumValIndex(mParams->colorChannel));

	//do this in the Curve constructor... (for inplace stuff)
	ParamArray<Vec2R> cp(mParams, "controlPoints", (ParamDynamicArrayStruct*)&mParams->controlPoints);
	for (uint32_t i = 0; i < cp.size(); i++)
	{
		mCurveFunction.addControlPoint(cp[i]);
	}
}

// ------------------------------------------------------------------------------------------------

void ColorVsVelocityModifierImpl::setColorChannel(ColorChannel colorChannel)
{
	mColorChannel = colorChannel;

	NvParameterized::Handle h(*mParams);
	mParams->getParameterHandle("colorChannel", h);
	mParams->setParamEnum(h, h.parameterDefinition()->enumVal((int)colorChannel));
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
SubtextureVsLifeModifierImpl::SubtextureVsLifeModifierImpl(SubtextureVsLifeModifierParams* params) :
	mParams(params)
{
	//do this in the Curve constructor... (for inplace stuff)
	ParamArray<Vec2R> cp(mParams, "controlPoints", (ParamDynamicArrayStruct*)&mParams->controlPoints);
	for (uint32_t i = 0; i < cp.size(); i++)
	{
		mCurveFunction.addControlPoint(cp[i]);
	}
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
OrientAlongVelocityModifierImpl::OrientAlongVelocityModifierImpl(OrientAlongVelocityModifierParams* params) :
	mParams(params)
{ }

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
ScaleAlongVelocityModifierImpl::ScaleAlongVelocityModifierImpl(ScaleAlongVelocityModifierParams* params) :
	mParams(params)
{ }


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
RandomSubtextureModifierImpl::RandomSubtextureModifierImpl(RandomSubtextureModifierParams* params) :
	mParams(params)
{}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
RandomRotationModifierImpl::RandomRotationModifierImpl(RandomRotationModifierParams* params) :
	mParams(params)
{}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
ScaleVsLifeModifierImpl::ScaleVsLifeModifierImpl(ScaleVsLifeModifierParams* params) :
	mParams(params)
{
	NvParameterized::Handle h(*mParams);
	mParams->getParameterHandle("scaleAxis", h);

	mScaleAxis = (ScaleAxis)(h.parameterDefinition()->enumValIndex(mParams->scaleAxis));

	//do this in the Curve constructor... (for inplace stuff)
	ParamArray<Vec2R> cp(mParams, "controlPoints", (ParamDynamicArrayStruct*)&mParams->controlPoints);
	for (uint32_t i = 0; i < cp.size(); i++)
	{
		mCurveFunction.addControlPoint(cp[i]);
	}
}

void ScaleVsLifeModifierImpl::setScaleAxis(ScaleAxis a)
{
	mScaleAxis = a;

	NvParameterized::Handle h(*mParams);
	mParams->getParameterHandle("scaleAxis", h);
	mParams->setParamEnum(h, h.parameterDefinition()->enumVal((int)a));
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
ScaleVsDensityModifierImpl::ScaleVsDensityModifierImpl(ScaleVsDensityModifierParams* params) :
	mParams(params)
{
	NvParameterized::Handle h(*mParams);
	mParams->getParameterHandle("scaleAxis", h);

	mScaleAxis = (ScaleAxis)(h.parameterDefinition()->enumValIndex(mParams->scaleAxis));

	//do this in the Curve constructor... (for inplace stuff)
	ParamArray<Vec2R> cp(mParams, "controlPoints", (ParamDynamicArrayStruct*)&mParams->controlPoints);
	for (uint32_t i = 0; i < cp.size(); i++)
	{
		mCurveFunction.addControlPoint(cp[i]);
	}
}

void ScaleVsDensityModifierImpl::setScaleAxis(ScaleAxis a)
{
	mScaleAxis = a;

	NvParameterized::Handle h(*mParams);
	mParams->getParameterHandle("scaleAxis", h);
	mParams->setParamEnum(h, h.parameterDefinition()->enumVal((int)a));
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
ScaleVsCameraDistanceModifierImpl::ScaleVsCameraDistanceModifierImpl(ScaleVsCameraDistanceModifierParams* params) :
	mParams(params)
{
	NvParameterized::Handle h(*mParams);
	mParams->getParameterHandle("scaleAxis", h);

	mScaleAxis = (ScaleAxis)(h.parameterDefinition()->enumValIndex(mParams->scaleAxis));

	//do this in the Curve constructor... (for inplace stuff)
	ParamArray<Vec2R> cp(mParams, "controlPoints", (ParamDynamicArrayStruct*)&mParams->controlPoints);
	for (uint32_t i = 0; i < cp.size(); i++)
	{
		mCurveFunction.addControlPoint(cp[i]);
	}
}

void ScaleVsCameraDistanceModifierImpl::setScaleAxis(ScaleAxis a)
{
	mScaleAxis = a;

	NvParameterized::Handle h(*mParams);
	mParams->getParameterHandle("scaleAxis", h);
	mParams->setParamEnum(h, h.parameterDefinition()->enumVal((int)a));
}



ViewDirectionSortingModifierImpl::ViewDirectionSortingModifierImpl(ViewDirectionSortingModifierParams* params)
	: mParams(params)
{
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
RotationRateModifierImpl::RotationRateModifierImpl(RotationRateModifierParams* params) :
	mParams(params)
{
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
RotationRateVsLifeModifierImpl::RotationRateVsLifeModifierImpl(RotationRateVsLifeModifierParams* params) :
	mParams(params)
{
	//do this in the Curve constructor... (for inplace stuff)
	ParamArray<Vec2R> cp(mParams, "controlPoints", (ParamDynamicArrayStruct*)&mParams->controlPoints);
	for (uint32_t i = 0; i < cp.size(); i++)
	{
		mCurveFunction.addControlPoint(cp[i]);
	}
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
OrientScaleAlongScreenVelocityModifierImpl::OrientScaleAlongScreenVelocityModifierImpl(OrientScaleAlongScreenVelocityModifierParams* params) :
	mParams(params)
{
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
Modifier* CreateModifier(ModifierTypeEnum modifierType, NvParameterized::Interface* objParam, NvParameterized::Handle& h)
{
	PX_UNUSED(objParam);
	PX_ASSERT(objParam == h.getConstInterface());

#define _MODIFIER(Type) \
case ModifierType_##Type: \
	h.initParamRef(#Type "ModifierParams", true); \
	h.getParamRef(refParam); \
	return PX_NEW(Type##ModifierImpl)((Type##ModifierParams*)refParam); \
	 
	NvParameterized::Interface* refParam = 0;

	// TODO: This should go to an actual factory which can be used to extend modifiers.
	switch (modifierType)
	{
#include "ModifierList.h"

	default:
		PX_ALWAYS_ASSERT();
	}
	return 0;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------

const ModifierImpl* ModifierImpl::castFrom(const Modifier* modifier)
{
#define _MODIFIER(Type) \
case ModifierType_##Type: \
	return static_cast<const ModifierImpl*>( static_cast<const Type ## ModifierImpl *>(modifier) ); \
	 
	ModifierTypeEnum modifierType = modifier->getModifierType();
	switch (modifierType)
	{
#include "ModifierList.h"

	default:
		PX_ALWAYS_ASSERT();
		return 0;
	}
}

}
} // namespace nvidia
