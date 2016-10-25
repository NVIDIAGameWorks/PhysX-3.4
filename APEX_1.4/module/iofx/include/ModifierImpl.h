/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __MODIFIER_IMPL_H__
#define __MODIFIER_IMPL_H__

#include "Modifier.h"
#include "CurveImpl.h"
#include "PsUserAllocated.h"

#include "RotationModifierParams.h"
#include "SimpleScaleModifierParams.h"
#include "RandomScaleModifierParams.h"
#include "ScaleByMassModifierParams.h"
#include "ColorVsLifeModifierParams.h"
#include "ScaleVsLifeModifierParams.h"
#include "ScaleVsDensityModifierParams.h"
#include "ScaleVsCameraDistanceModifierParams.h"
#include "ColorVsDensityModifierParams.h"
#include "SubtextureVsLifeModifierParams.h"
#include "OrientAlongVelocityModifierParams.h"
#include "ScaleAlongVelocityModifierParams.h"
#include "RandomSubtextureModifierParams.h"
#include "RandomRotationModifierParams.h"
#include "ViewDirectionSortingModifierParams.h"
#include "RotationRateModifierParams.h"
#include "RotationRateVsLifeModifierParams.h"
#include "OrientScaleAlongScreenVelocityModifierParams.h"
#include "ColorVsVelocityModifierParams.h"

#include "ParamArray.h"

#include "PsArray.h"

namespace nvidia
{
namespace apex
{
class InplaceStorage;
class InplaceHandleBase;
struct RandState;
}
namespace iofx
{

/**
	Directions for adding a new APEX modifier type (an official modifier, not a user modifier).

	1) In Modifier.h, add a value to the end of ModifierTypeEnum, matching the naming convention of others.
	2) In Modifier.h, add a new subclass of ModifierT, specialized on your type.
		- e.g. class MyNewModifier : public ModifierT<NxMyNewModifier> { };
	3) MyNewModifier should include pure virtual getters (by value), and setters. Do not promise to return
	   by const-reference.
    4) In ModifierImpl.h (this file), add a subcless of your public class and ApexAllocateable
		- e.g. class MyNewModifier : public MyNewModifier, public UserAllocated { };
	5) The ModifierImpl.h class should provide concrete implementations of all of the functions.
		- Feel free to do the getters and setters inline in ModifierImpl.h
	6) In ModifierImpl.cpp, provide the implementations for the serailization functions and 'updateParticles.'
*/

// Ignore this warning temporarily. It's reset to default level at the bottom of the file.
// We'll fix this soon, just not "right this second"
#pragma warning( disable: 4100 )

class IofxModifierHelper
{
public:

	// assumes the mParams class contains a "controlPoints" member
	static void setCurve(const nvidia::apex::Curve* f,
	                       NvParameterized::Interface* mParams,
	                       ParamDynamicArrayStruct* controlPoints)
	{
		uint32_t cpSize;
		f->getControlPoints(cpSize);

		NvParameterized::Handle h(*mParams);
		mParams->getParameterHandle("controlPoints", h);
		h.resizeArray((int32_t)cpSize);

		ParamArray<ScaleVsCameraDistanceModifierParamsNS::vec2_Type>
		cpArray(mParams, "controlPoints", controlPoints);

		for (uint32_t i = 0; i < cpSize; i++)
		{
			cpArray[i].x = f->getControlPoints(cpSize)[i].x;
			cpArray[i].y = f->getControlPoints(cpSize)[i].y;
		}
	}
};


class ModifierParamsMapperCPU
{
public:
	virtual void beginParams(void* params, size_t size, size_t align, uint32_t randomCount) = 0;
	virtual void endParams() = 0;

	virtual void mapValue(size_t offset, int32_t value) = 0;
	void mapValue(size_t offset, uint32_t value)
	{
		mapValue(offset, static_cast<int32_t>(value));
	}

	virtual void mapValue(size_t offset, float value) = 0;
	void mapValue(size_t offset, const PxVec3& value)
	{
		mapValue(offset + offsetof(PxVec3, x), value.x);
		mapValue(offset + offsetof(PxVec3, y), value.y);
		mapValue(offset + offsetof(PxVec3, z), value.z);
	}

	virtual void mapCurve(size_t offset, const nvidia::apex::Curve* curve) = 0;
};

#if APEX_CUDA_SUPPORT

class ModifierParamsMapperGPU
{
public:
	virtual InplaceStorage& getStorage() = 0;

	virtual void onParams(InplaceHandleBase handle, uint32_t randomCount) = 0;
};
#endif

struct MeshInput;
struct MeshPublicState;
struct MeshPrivateState;
struct SpriteInput;
struct SpritePublicState;
struct SpritePrivateState;
struct ModifierCommonParams;

class ModifierImpl
{
public:
	static const ModifierImpl* castFrom(const Modifier*);

#if APEX_CUDA_SUPPORT
	virtual void mapParamsGPU(ModifierParamsMapperGPU& mapper) const
	{
		PX_ASSERT(!"unimpl");
	}
#endif
	virtual void mapParamsCPU(ModifierParamsMapperCPU& /*mapper*/) const
	{
		PX_ASSERT(!"unimpl");
	}


	typedef void (*updateSpriteFunc)(const void* params, const SpriteInput& input, SpritePublicState& pubState, SpritePrivateState& privState, const ModifierCommonParams& common, RandState& randState);
	typedef void (*updateMeshFunc)(const void* params, const MeshInput& input, MeshPublicState& pubState, MeshPrivateState& privState, const ModifierCommonParams& common, RandState& randState);

	virtual updateSpriteFunc getUpdateSpriteFunc(ModifierStage /*stage*/) const
	{
		PX_ASSERT(!"unimpl");
		return 0;
	}
	virtual updateMeshFunc getUpdateMeshFunc(ModifierStage /*stage*/) const
	{
		PX_ASSERT(!"unimpl");
		return 0;
	}
};

// ------------------------------------------------------------------------------------------------
class RotationModifierImpl : public RotationModifier, public ModifierImpl, public UserAllocated
{
public:
	RotationModifierImpl(RotationModifierParams* params);

	virtual ApexMeshParticleRollType::Enum getRollType() const
	{
		return mRollType;
	}
	virtual void setRollType(ApexMeshParticleRollType::Enum rollType);
	virtual float getMaxSettleRate() const
	{
		return mParams->maxSettleRatePerSec;
	}
	virtual void setMaxSettleRate(float settleRate)
	{
		mParams->maxSettleRatePerSec = settleRate;
	}
	virtual float getMaxRotationRate() const
	{
		return mParams->maxRotationRatePerSec;
	}
	virtual void setMaxRotationRate(float rotationRate)
	{
		mParams->maxRotationRatePerSec = rotationRate;
	}

#if APEX_CUDA_SUPPORT
	virtual void mapParamsGPU(ModifierParamsMapperGPU& mapper) const;
#endif
	virtual void mapParamsCPU(ModifierParamsMapperCPU& mapper) const;

	template <class Mapper, typename Params>
	void mapParams(Mapper& mapper, Params* params) const
	{
		mapper.beginParams(params, sizeof(Params), __alignof(Params), Params::RANDOM_COUNT);

		mapper.mapValue(offsetof(Params, rollType), uint32_t(mRollType));
		mapper.mapValue(offsetof(Params, rollAxis), mRollAxis);
		mapper.mapValue(offsetof(Params, rollSign), mRollSign);

		mapper.mapValue(offsetof(Params, maxSettleRatePerSec), mParams->maxSettleRatePerSec);
		mapper.mapValue(offsetof(Params, maxRotationRatePerSec), mParams->maxRotationRatePerSec);

		mapper.mapValue(offsetof(Params, inAirRotationMultiplier), mParams->inAirRotationMultiplier);
		mapper.mapValue(offsetof(Params, collisionRotationMultiplier), mParams->collisionRotationMultiplier);

		mapper.mapValue(offsetof(Params, includeVerticalDirection), uint32_t(mParams->includeVerticalDirection));

		mapper.endParams();
	}

	virtual updateMeshFunc getUpdateMeshFunc(ModifierStage stage) const;

private:
	RotationModifierParams* mParams;
	ApexMeshParticleRollType::Enum mRollType;
	int32_t mRollAxis;
	float mRollSign;
	float mLastUpdateTime;
};

// ------------------------------------------------------------------------------------------------
class SimpleScaleModifierImpl : public SimpleScaleModifier, public ModifierImpl, public UserAllocated
{
public:
	SimpleScaleModifierImpl(SimpleScaleModifierParams* params);

	virtual uint32_t getModifierSpriteSemantics()
	{
		return (uint32_t)(1 << IofxRenderSemantic::SCALE);
	}

	virtual PxVec3 getScaleFactor() const
	{
		return mParams->scaleFactor;
	}
	virtual void setScaleFactor(const PxVec3& s)
	{
		mParams->scaleFactor = s;
	}

#if APEX_CUDA_SUPPORT
	virtual void mapParamsGPU(ModifierParamsMapperGPU& mapper) const;
#endif
	virtual void mapParamsCPU(ModifierParamsMapperCPU& mapper) const;

	template <class Mapper, typename Params>
	void mapParams(Mapper& mapper, Params* params) const
	{
		mapper.beginParams(params, sizeof(Params), __alignof(Params), Params::RANDOM_COUNT);

		mapper.mapValue(offsetof(Params, scaleFactor), mParams->scaleFactor);

		mapper.endParams();
	}

	virtual updateSpriteFunc getUpdateSpriteFunc(ModifierStage stage) const;
	virtual updateMeshFunc getUpdateMeshFunc(ModifierStage stage) const;

private:
	SimpleScaleModifierParams* mParams;

};

// ------------------------------------------------------------------------------------------------
class ScaleByMassModifierImpl : public ScaleByMassModifier, public ModifierImpl, public UserAllocated
{
public:
	ScaleByMassModifierImpl(ScaleByMassModifierParams* params);

	virtual uint32_t getModifierSpriteSemantics()
	{
		return (uint32_t)(1 << IofxRenderSemantic::SCALE);
	}

#if APEX_CUDA_SUPPORT
	virtual void mapParamsGPU(ModifierParamsMapperGPU& mapper) const;
#endif
	virtual void mapParamsCPU(ModifierParamsMapperCPU& mapper) const;

	template <class Mapper, typename Params>
	void mapParams(Mapper& mapper, Params* params) const
	{
		mapper.beginParams(params, sizeof(Params), __alignof(Params), Params::RANDOM_COUNT);

		mapper.endParams();
	}

	virtual updateSpriteFunc getUpdateSpriteFunc(ModifierStage stage) const;
	virtual updateMeshFunc getUpdateMeshFunc(ModifierStage stage) const;

private:
	ScaleByMassModifierParams* mParams;

};

// ------------------------------------------------------------------------------------------------
class RandomScaleModifierImpl : public RandomScaleModifier, public ModifierImpl, public UserAllocated
{
public:
	RandomScaleModifierImpl(RandomScaleModifierParams* params);

	virtual uint32_t getModifierSpriteSemantics()
	{
		return (uint32_t)(1 << IofxRenderSemantic::SCALE);
	}

	virtual Range<float> getScaleFactor() const
	{
		Range<float> s;
		s.minimum = mParams->minScaleFactor;
		s.maximum = mParams->maxScaleFactor;
		return s;
	}

	virtual void setScaleFactor(const Range<float>& s)
	{
		mParams->minScaleFactor = s.minimum;
		mParams->maxScaleFactor = s.maximum;
	}

#if APEX_CUDA_SUPPORT
	virtual void mapParamsGPU(ModifierParamsMapperGPU& mapper) const;
#endif
	virtual void mapParamsCPU(ModifierParamsMapperCPU& mapper) const;

	template <class Mapper, typename Params>
	void mapParams(Mapper& mapper, Params* params) const
	{
		mapper.beginParams(params, sizeof(Params), __alignof(Params), Params::RANDOM_COUNT);

		mapper.mapValue(offsetof(Params, scaleFactorMin), mParams->minScaleFactor);
		mapper.mapValue(offsetof(Params, scaleFactorMax), mParams->maxScaleFactor);

		mapper.endParams();
	}

	virtual updateSpriteFunc getUpdateSpriteFunc(ModifierStage stage) const;
	virtual updateMeshFunc getUpdateMeshFunc(ModifierStage stage) const;

private:
	RandomScaleModifierParams* mParams;
};

// ------------------------------------------------------------------------------------------------
class ColorVsLifeModifierImpl : public ColorVsLifeModifier, public ModifierImpl, public UserAllocated
{
public:
	ColorVsLifeModifierImpl(ColorVsLifeModifierParams* params);

	// Methods from Modifier
	virtual uint32_t getModifierSpriteSemantics()
	{
		return (uint32_t)(1 << IofxRenderSemantic::COLOR);
	}
	virtual uint32_t getModifierMeshSemantics()
	{
		return (uint32_t)(1 << IofxRenderSemantic::COLOR);
	}

	// Access to expected data members
	virtual ColorChannel getColorChannel() const
	{
		return mColorChannel;
	}
	virtual void setColorChannel(ColorChannel colorChannel);

	virtual const nvidia::apex::Curve* getFunction() const
	{
		return static_cast<const nvidia::apex::Curve*>(&mCurveFunction);
	}
	virtual void setFunction(const nvidia::apex::Curve* f)
	{
		const CurveImpl* curve = static_cast<const CurveImpl*>(f);
		mCurveFunction = *curve;

		IofxModifierHelper::setCurve(f,
		                               mParams,
		                               (ParamDynamicArrayStruct*)&mParams->controlPoints);
	}

#if APEX_CUDA_SUPPORT
	virtual void mapParamsGPU(ModifierParamsMapperGPU& mapper) const;
#endif
	virtual void mapParamsCPU(ModifierParamsMapperCPU& mapper) const;

	template <class Mapper, typename Params>
	void mapParams(Mapper& mapper, Params* params) const
	{
		mapper.beginParams(params, sizeof(Params), __alignof(Params), Params::RANDOM_COUNT);

		mapper.mapValue(offsetof(Params, channel), uint32_t(mColorChannel));
		mapper.mapCurve(offsetof(Params, curve), static_cast<const CurveImpl*>(&mCurveFunction));

		mapper.endParams();
	}

	virtual updateSpriteFunc getUpdateSpriteFunc(ModifierStage stage) const;
	virtual updateMeshFunc getUpdateMeshFunc(ModifierStage stage) const;

private:
	ColorVsLifeModifierParams* mParams;
	ColorChannel mColorChannel;
	CurveImpl mCurveFunction;
};

// ------------------------------------------------------------------------------------------------
class ColorVsDensityModifierImpl : public ColorVsDensityModifier, public ModifierImpl, public UserAllocated
{
public:
	ColorVsDensityModifierImpl(ColorVsDensityModifierParams* params);

	// Methods from Modifier
	virtual uint32_t getModifierSpriteSemantics()
	{
		return (uint32_t)(1 << IofxRenderSemantic::COLOR);
	}
	virtual uint32_t getModifierMeshSemantics()
	{
		return (uint32_t)(1 << IofxRenderSemantic::COLOR);
	}

	// Access to expected data members
	virtual ColorChannel getColorChannel() const
	{
		return mColorChannel;
	}
	virtual void setColorChannel(ColorChannel colorChannel);

	virtual const nvidia::apex::Curve* getFunction() const
	{
		return static_cast<const nvidia::apex::Curve*>(&mCurveFunction);
	}
	virtual void setFunction(const nvidia::apex::Curve* f)
	{
		const CurveImpl* curve = static_cast<const CurveImpl*>(f);
		mCurveFunction = *curve;

		IofxModifierHelper::setCurve(f,
		                               mParams,
		                               (ParamDynamicArrayStruct*)&mParams->controlPoints);
	}

#if APEX_CUDA_SUPPORT
	virtual void mapParamsGPU(ModifierParamsMapperGPU& mapper) const;
#endif
	virtual void mapParamsCPU(ModifierParamsMapperCPU& mapper) const;

	template <class Mapper, typename Params>
	void mapParams(Mapper& mapper, Params* params) const
	{
		mapper.beginParams(params, sizeof(Params), __alignof(Params), Params::RANDOM_COUNT);

		mapper.mapValue(offsetof(Params, channel), uint32_t(mColorChannel));
		mapper.mapCurve(offsetof(Params, curve), static_cast<const CurveImpl*>(&mCurveFunction));

		mapper.endParams();
	}

	virtual updateSpriteFunc getUpdateSpriteFunc(ModifierStage stage) const;
	virtual updateMeshFunc getUpdateMeshFunc(ModifierStage stage) const;

private:
	ColorVsDensityModifierParams* mParams;
	ColorChannel mColorChannel;
	CurveImpl mCurveFunction;
};

// ------------------------------------------------------------------------------------------------
class ColorVsVelocityModifierImpl : public ColorVsVelocityModifier, public ModifierImpl, public UserAllocated
{
public:
	ColorVsVelocityModifierImpl(ColorVsVelocityModifierParams* params);

	// Methods from Modifier
	virtual uint32_t getModifierSpriteSemantics()
	{
		return (uint32_t)(1 << IofxRenderSemantic::COLOR);
	}
	virtual uint32_t getModifierMeshSemantics()
	{
		return (uint32_t)(1 << IofxRenderSemantic::COLOR);
	}

	// Access to expected data members
	virtual ColorChannel getColorChannel() const
	{
		return mColorChannel;
	}
	virtual void setColorChannel(ColorChannel colorChannel);

	virtual const nvidia::apex::Curve* getFunction() const
	{
		return static_cast<const nvidia::apex::Curve*>(&mCurveFunction);
	}
	virtual void setFunction(const nvidia::apex::Curve* f)
	{
		const CurveImpl* curve = static_cast<const CurveImpl*>(f);
		mCurveFunction = *curve;

		IofxModifierHelper::setCurve(f,
		                               mParams,
		                               (ParamDynamicArrayStruct*)&mParams->controlPoints);
	}

	virtual float getVelocity0() const
	{
		return mParams->velocity0;
	}
	virtual void setVelocity0(float value)
	{
		mParams->velocity0 = value;
	}

	virtual float getVelocity1() const
	{
		return mParams->velocity1;
	}
	virtual void setVelocity1(float value)
	{
		mParams->velocity1 = value;
	}

#if APEX_CUDA_SUPPORT
	virtual void mapParamsGPU(ModifierParamsMapperGPU& mapper) const;
#endif
	virtual void mapParamsCPU(ModifierParamsMapperCPU& mapper) const;

	template <class Mapper, typename Params>
	void mapParams(Mapper& mapper, Params* params) const
	{
		mapper.beginParams(params, sizeof(Params), __alignof(Params), Params::RANDOM_COUNT);

		mapper.mapValue(offsetof(Params, velocity0), mParams->velocity0);
		mapper.mapValue(offsetof(Params, velocity1), mParams->velocity1);
		mapper.mapValue(offsetof(Params, channel), uint32_t(mColorChannel));
		mapper.mapCurve(offsetof(Params, curve), static_cast<const CurveImpl*>(&mCurveFunction));

		mapper.endParams();
	}

	virtual updateSpriteFunc getUpdateSpriteFunc(ModifierStage stage) const;
	virtual updateMeshFunc getUpdateMeshFunc(ModifierStage stage) const;

private:
	ColorVsVelocityModifierParams* mParams;
	ColorChannel mColorChannel;
	CurveImpl mCurveFunction;
};

// ------------------------------------------------------------------------------------------------
class SubtextureVsLifeModifierImpl : public SubtextureVsLifeModifier, public ModifierImpl, public UserAllocated
{
public:
	SubtextureVsLifeModifierImpl(SubtextureVsLifeModifierParams* params);
	// Methods from Modifier
	virtual uint32_t getModifierSpriteSemantics()
	{
		return (uint32_t)(1 << IofxRenderSemantic::SUBTEXTURE);
	}

	// Access to expected data members
	virtual const nvidia::apex::Curve* getFunction() const
	{
		return static_cast<const nvidia::apex::Curve*>(&mCurveFunction);
	}
	virtual void setFunction(const nvidia::apex::Curve* f)
	{
		const CurveImpl* curve = static_cast<const CurveImpl*>(f);
		mCurveFunction = *curve;

		IofxModifierHelper::setCurve(f,
		                               mParams,
		                               (ParamDynamicArrayStruct*)&mParams->controlPoints);
	}

#if APEX_CUDA_SUPPORT
	virtual void mapParamsGPU(ModifierParamsMapperGPU& mapper) const;
#endif
	virtual void mapParamsCPU(ModifierParamsMapperCPU& mapper) const;

	template <class Mapper, typename Params>
	void mapParams(Mapper& mapper, Params* params) const
	{
		mapper.beginParams(params, sizeof(Params), __alignof(Params), Params::RANDOM_COUNT);

		mapper.mapCurve(offsetof(Params, curve), static_cast<const CurveImpl*>(&mCurveFunction));

		mapper.endParams();
	}

	virtual updateSpriteFunc getUpdateSpriteFunc(ModifierStage stage) const;

private:
	SubtextureVsLifeModifierParams* mParams;
	CurveImpl mCurveFunction;
};

// ------------------------------------------------------------------------------------------------
class OrientAlongVelocityModifierImpl : public OrientAlongVelocityModifier, public ModifierImpl, public UserAllocated
{
public:
	OrientAlongVelocityModifierImpl(OrientAlongVelocityModifierParams* params);

	// Methods from Modifier

	// Access to expected data members
	virtual PxVec3 getModelForward() const
	{
		return mParams->modelForward;
	}
	virtual void setModelForward(const PxVec3& s)
	{
		mParams->modelForward = s;
	}

#if APEX_CUDA_SUPPORT
	virtual void mapParamsGPU(ModifierParamsMapperGPU& mapper) const;
#endif
	virtual void mapParamsCPU(ModifierParamsMapperCPU& mapper) const;

	template <class Mapper, typename Params>
	void mapParams(Mapper& mapper, Params* params) const
	{
		mapper.beginParams(params, sizeof(Params), __alignof(Params), Params::RANDOM_COUNT);

		mapper.mapValue(offsetof(Params, modelForward), mParams->modelForward);

		mapper.endParams();
	}

	virtual updateMeshFunc getUpdateMeshFunc(ModifierStage stage) const;

private:
	OrientAlongVelocityModifierParams* mParams;
};

// ------------------------------------------------------------------------------------------------
class ScaleAlongVelocityModifierImpl : public ScaleAlongVelocityModifier, public ModifierImpl, public UserAllocated
{
public:
	ScaleAlongVelocityModifierImpl(ScaleAlongVelocityModifierParams* params);

	// Methods from Modifier

	// Access to expected data members
	virtual float getScaleFactor() const
	{
		return mParams->scaleFactor;
	}
	virtual void setScaleFactor(const float& s)
	{
		mParams->scaleFactor = s;
	}

#if APEX_CUDA_SUPPORT
	virtual void mapParamsGPU(ModifierParamsMapperGPU& mapper) const;
#endif
	virtual void mapParamsCPU(ModifierParamsMapperCPU& mapper) const;

	template <class Mapper, typename Params>
	void mapParams(Mapper& mapper, Params* params) const
	{
		mapper.beginParams(params, sizeof(Params), __alignof(Params), Params::RANDOM_COUNT);

		mapper.mapValue(offsetof(Params, scaleFactor), mParams->scaleFactor);

		mapper.endParams();
	}

	virtual updateMeshFunc getUpdateMeshFunc(ModifierStage stage) const;

private:
	ScaleAlongVelocityModifierParams* mParams;
};

// ------------------------------------------------------------------------------------------------
class RandomSubtextureModifierImpl : public RandomSubtextureModifier, public ModifierImpl, public UserAllocated
{
public:
	RandomSubtextureModifierImpl(RandomSubtextureModifierParams* params);

	// Methods from Modifier
	virtual uint32_t getModifierSpriteSemantics()
	{
		return (uint32_t)(1 << IofxRenderSemantic::SUBTEXTURE);
	}

	// Access to expected data members
	virtual Range<float> getSubtextureRange() const
	{
		Range<float> s;
		s.minimum = mParams->minSubtexture;
		s.maximum = mParams->maxSubtexture;
		return s;
	}

	virtual void setSubtextureRange(const Range<float>& s)
	{
		mParams->minSubtexture = s.minimum;
		mParams->maxSubtexture = s.maximum;
	}

#if APEX_CUDA_SUPPORT
	virtual void mapParamsGPU(ModifierParamsMapperGPU& mapper) const;
#endif
	virtual void mapParamsCPU(ModifierParamsMapperCPU& mapper) const;

	template <class Mapper, typename Params>
	void mapParams(Mapper& mapper, Params* params) const
	{
		mapper.beginParams(params, sizeof(Params), __alignof(Params), Params::RANDOM_COUNT);

		mapper.mapValue(offsetof(Params, subtextureRangeMin), mParams->minSubtexture);
		mapper.mapValue(offsetof(Params, subtextureRangeMax), mParams->maxSubtexture);

		mapper.endParams();
	}

	virtual updateSpriteFunc getUpdateSpriteFunc(ModifierStage stage) const;

private:
	RandomSubtextureModifierParams* mParams;
};

// ------------------------------------------------------------------------------------------------
class RandomRotationModifierImpl : public RandomRotationModifier, public ModifierImpl, public UserAllocated
{
public:
	RandomRotationModifierImpl(RandomRotationModifierParams* params);

	// Methods from Modifier
	virtual uint32_t getModifierSpriteSemantics()
	{
		return (uint32_t)(1 << IofxRenderSemantic::ORIENTATION);
	}

	// Access to expected data members
	virtual Range<float> getRotationRange() const
	{
		Range<float> s;
		s.minimum = mParams->minRotation;
		s.maximum = mParams->maxRotation;
		return s;
	}

	virtual void setRotationRange(const Range<float>& s)
	{
		mParams->minRotation = s.minimum;
		mParams->maxRotation = s.maximum;
	}

#if APEX_CUDA_SUPPORT
	virtual void mapParamsGPU(ModifierParamsMapperGPU& mapper) const;
#endif
	virtual void mapParamsCPU(ModifierParamsMapperCPU& mapper) const;

	template <class Mapper, typename Params>
	void mapParams(Mapper& mapper, Params* params) const
	{
		mapper.beginParams(params, sizeof(Params), __alignof(Params), Params::RANDOM_COUNT);

		mapper.mapValue(offsetof(Params, rotationRangeMin), mParams->minRotation);
		mapper.mapValue(offsetof(Params, rotationRangeMax), mParams->maxRotation);

		mapper.endParams();
	}

	virtual updateSpriteFunc getUpdateSpriteFunc(ModifierStage stage) const;

private:
	RandomRotationModifierParams* mParams;
};

// ------------------------------------------------------------------------------------------------
class ScaleVsLifeModifierImpl : public ScaleVsLifeModifier, public ModifierImpl, public UserAllocated
{
public:
	ScaleVsLifeModifierImpl(ScaleVsLifeModifierParams* params);

	// Methods from Modifier
	virtual uint32_t getModifierSpriteSemantics()
	{
		return (uint32_t)(1 << IofxRenderSemantic::SCALE);
	}

	// Access to expected data members
	virtual ScaleAxis getScaleAxis() const
	{
		return mScaleAxis;
	}
	virtual void setScaleAxis(ScaleAxis a);
	virtual const nvidia::apex::Curve* getFunction() const
	{
		return static_cast<const nvidia::apex::Curve*>(&mCurveFunction);
	}
	virtual void setFunction(const nvidia::apex::Curve* f)
	{
		const CurveImpl* curve = static_cast<const CurveImpl*>(f);
		mCurveFunction = *curve;

		IofxModifierHelper::setCurve(f,
		                               mParams,
		                               (ParamDynamicArrayStruct*)&mParams->controlPoints);
	}

#if APEX_CUDA_SUPPORT
	virtual void mapParamsGPU(ModifierParamsMapperGPU& mapper) const;
#endif
	virtual void mapParamsCPU(ModifierParamsMapperCPU& mapper) const;

	template <class Mapper, typename Params>
	void mapParams(Mapper& mapper, Params* params) const
	{
		mapper.beginParams(params, sizeof(Params), __alignof(Params), Params::RANDOM_COUNT);

		mapper.mapValue(offsetof(Params, axis), uint32_t(mScaleAxis));
		mapper.mapCurve(offsetof(Params, curve), static_cast<const CurveImpl*>(&mCurveFunction));

		mapper.endParams();
	}

	virtual updateSpriteFunc getUpdateSpriteFunc(ModifierStage stage) const;
	virtual updateMeshFunc getUpdateMeshFunc(ModifierStage stage) const;

private:
	ScaleVsLifeModifierParams* mParams;
	ScaleAxis mScaleAxis;
	CurveImpl mCurveFunction;
};

// ------------------------------------------------------------------------------------------------
class ScaleVsDensityModifierImpl : public ScaleVsDensityModifier, public ModifierImpl, public UserAllocated
{
public:
	ScaleVsDensityModifierImpl(ScaleVsDensityModifierParams* params);

	// Methods from Modifier
	virtual uint32_t getModifierSpriteSemantics()
	{
		return (uint32_t)(1 << IofxRenderSemantic::SCALE);
	}

	// Access to expected data members
	virtual ScaleAxis getScaleAxis() const
	{
		return mScaleAxis;
	}
	virtual void setScaleAxis(ScaleAxis a);
	virtual const nvidia::apex::Curve* getFunction() const
	{
		return static_cast<const nvidia::apex::Curve*>(&mCurveFunction);
	}
	virtual void setFunction(const nvidia::apex::Curve* f)
	{
		const CurveImpl* curve = static_cast<const CurveImpl*>(f);
		mCurveFunction = *curve;

		IofxModifierHelper::setCurve(f,
		                               mParams,
		                               (ParamDynamicArrayStruct*)&mParams->controlPoints);
	}

#if APEX_CUDA_SUPPORT
	virtual void mapParamsGPU(ModifierParamsMapperGPU& mapper) const;
#endif
	virtual void mapParamsCPU(ModifierParamsMapperCPU& mapper) const;

	template <class Mapper, typename Params>
	void mapParams(Mapper& mapper, Params* params) const
	{
		mapper.beginParams(params, sizeof(Params), __alignof(Params), Params::RANDOM_COUNT);

		mapper.mapValue(offsetof(Params, axis), uint32_t(mScaleAxis));
		mapper.mapCurve(offsetof(Params, curve), static_cast<const CurveImpl*>(&mCurveFunction));

		mapper.endParams();
	}

	virtual updateSpriteFunc getUpdateSpriteFunc(ModifierStage stage) const;
	virtual updateMeshFunc getUpdateMeshFunc(ModifierStage stage) const;

private:
	ScaleVsDensityModifierParams* mParams;
	ScaleAxis mScaleAxis;
	CurveImpl mCurveFunction;
};

// ------------------------------------------------------------------------------------------------
class ScaleVsCameraDistanceModifierImpl : public ScaleVsCameraDistanceModifier, public ModifierImpl, public UserAllocated
{
public:
	ScaleVsCameraDistanceModifierImpl(ScaleVsCameraDistanceModifierParams* params);

	// Methods from Modifier

	// Access to expected data members
	virtual ScaleAxis getScaleAxis() const
	{
		return mScaleAxis;
	}
	virtual void setScaleAxis(ScaleAxis a);
	virtual const nvidia::apex::Curve* getFunction() const
	{
		return static_cast<const nvidia::apex::Curve*>(&mCurveFunction);
	}
	virtual void setFunction(const nvidia::apex::Curve* f)
	{
		const CurveImpl* curve = static_cast<const CurveImpl*>(f);
		mCurveFunction = *curve;

		IofxModifierHelper::setCurve(f,
		                               mParams,
		                               (ParamDynamicArrayStruct*)&mParams->controlPoints);
	}

#if APEX_CUDA_SUPPORT
	virtual void mapParamsGPU(ModifierParamsMapperGPU& mapper) const;
#endif
	virtual void mapParamsCPU(ModifierParamsMapperCPU& mapper) const;

	template <class Mapper, typename Params>
	void mapParams(Mapper& mapper, Params* params) const
	{
		mapper.beginParams(params, sizeof(Params), __alignof(Params), Params::RANDOM_COUNT);

		mapper.mapValue(offsetof(Params, axis), uint32_t(mScaleAxis));
		mapper.mapCurve(offsetof(Params, curve), static_cast<const CurveImpl*>(&mCurveFunction));

		mapper.endParams();
	}

	virtual updateSpriteFunc getUpdateSpriteFunc(ModifierStage stage) const;
	virtual updateMeshFunc getUpdateMeshFunc(ModifierStage stage) const;

private:
	ScaleVsCameraDistanceModifierParams* mParams;
	ScaleAxis mScaleAxis;
	CurveImpl mCurveFunction;
};

/**
	NvParameterized::Factory for modifiers. TODO: This should be a class that you instantiate, which you can then register objects with,
	and that you then pass to the Asset class for deserialization.
*/
Modifier* CreateModifier(ModifierTypeEnum modifierType, NvParameterized::Interface* objParam, NvParameterized::Handle& h);

// ------------------------------------------------------------------------------------------------
class ViewDirectionSortingModifierImpl : public ViewDirectionSortingModifier, public ModifierImpl, public UserAllocated
{
public:
	ViewDirectionSortingModifierImpl(ViewDirectionSortingModifierParams* params);

	// Methods from Modifier
	virtual uint32_t getModifierSpriteSemantics()
	{
		return (uint32_t)0;
	}

	// Access to expected data members

#if APEX_CUDA_SUPPORT
	virtual void mapParamsGPU(ModifierParamsMapperGPU& mapper) const;
#endif
	virtual void mapParamsCPU(ModifierParamsMapperCPU& mapper) const;

	template <class Mapper, typename Params>
	void mapParams(Mapper& mapper, Params* params) const
	{
		mapper.beginParams(params, sizeof(Params), __alignof(Params), Params::RANDOM_COUNT);

		mapper.endParams();
	}

	virtual updateSpriteFunc getUpdateSpriteFunc(ModifierStage stage) const;

private:
	ViewDirectionSortingModifierParams* mParams;
};

// ------------------------------------------------------------------------------------------------
class RotationRateModifierImpl : public RotationRateModifier, public ModifierImpl, public UserAllocated
{
public:
	RotationRateModifierImpl(RotationRateModifierParams* params);
	// Methods from Modifier
	virtual uint32_t getModifierSpriteSemantics()
	{
		return (uint32_t)(1 << IofxRenderSemantic::ORIENTATION);
	}

	// Access to expected data members
	virtual float getRotationRate() const
	{
		return mParams->rotationRate;
	}
	virtual void setRotationRate(const float& r)
	{
		mParams->rotationRate = r;
	}

#if APEX_CUDA_SUPPORT
	virtual void mapParamsGPU(ModifierParamsMapperGPU& mapper) const;
#endif
	virtual void mapParamsCPU(ModifierParamsMapperCPU& mapper) const;

	template <class Mapper, typename Params>
	void mapParams(Mapper& mapper, Params* params) const
	{
		mapper.beginParams(params, sizeof(Params), __alignof(Params), Params::RANDOM_COUNT);

		mapper.mapValue(offsetof(Params, rotationRate), mParams->rotationRate);

		mapper.endParams();
	}

	virtual updateSpriteFunc getUpdateSpriteFunc(ModifierStage stage) const;

private:
	RotationRateModifierParams* mParams;
};

// ------------------------------------------------------------------------------------------------
class RotationRateVsLifeModifierImpl : public RotationRateVsLifeModifier, public ModifierImpl, public UserAllocated
{
public:
	RotationRateVsLifeModifierImpl(RotationRateVsLifeModifierParams* params);
	// Methods from Modifier
	virtual uint32_t getModifierSpriteSemantics()
	{
		return (uint32_t)(1 << IofxRenderSemantic::ORIENTATION);
	}

	// Access to expected data members
	virtual const nvidia::apex::Curve* getFunction() const
	{
		return static_cast<const nvidia::apex::Curve*>(&mCurveFunction);
	}
	virtual void setFunction(const nvidia::apex::Curve* f)
	{
		const CurveImpl* curve = static_cast<const CurveImpl*>(f);
		mCurveFunction = *curve;

		IofxModifierHelper::setCurve(f,
		                               mParams,
		                               (ParamDynamicArrayStruct*)&mParams->controlPoints);
	}

#if APEX_CUDA_SUPPORT
	virtual void mapParamsGPU(ModifierParamsMapperGPU& mapper) const;
#endif
	virtual void mapParamsCPU(ModifierParamsMapperCPU& mapper) const;

	template <class Mapper, typename Params>
	void mapParams(Mapper& mapper, Params* params) const
	{
		mapper.beginParams(params, sizeof(Params), __alignof(Params), Params::RANDOM_COUNT);

		mapper.mapCurve(offsetof(Params, curve), static_cast<const CurveImpl*>(&mCurveFunction));

		mapper.endParams();
	}

	virtual updateSpriteFunc getUpdateSpriteFunc(ModifierStage stage) const;

private:
	RotationRateVsLifeModifierParams* mParams;
	CurveImpl mCurveFunction;
};

// ------------------------------------------------------------------------------------------------
class OrientScaleAlongScreenVelocityModifierImpl : public OrientScaleAlongScreenVelocityModifier, public ModifierImpl, public UserAllocated
{
public:
	OrientScaleAlongScreenVelocityModifierImpl(OrientScaleAlongScreenVelocityModifierParams* params);
	// Methods from Modifier
	virtual uint32_t getModifierSpriteSemantics()
	{
		return (uint32_t)((1 << IofxRenderSemantic::ORIENTATION) | (1 << IofxRenderSemantic::SCALE));
	}

	// Access to expected data members
	virtual float getScalePerVelocity() const
	{
		return mParams->scalePerVelocity;
	}
	virtual void setScalePerVelocity(const float& s)
	{
		mParams->scalePerVelocity = s;
	}

	virtual float getScaleChangeLimit() const
	{
		return mParams->scaleChangeLimit;
	}
	virtual void setScaleChangeLimit(const float& s)
	{
		mParams->scaleChangeLimit = s;
	}

	virtual float getScaleChangeDelay() const
	{
		return mParams->scaleChangeDelay;
	}
	virtual void setScaleChangeDelay(const float& s)
	{
		mParams->scaleChangeDelay = s;
	}

#if APEX_CUDA_SUPPORT
	virtual void mapParamsGPU(ModifierParamsMapperGPU& mapper) const;
#endif
	virtual void mapParamsCPU(ModifierParamsMapperCPU& mapper) const;

	template <class Mapper, typename Params>
	void mapParams(Mapper& mapper, Params* params) const
	{
		mapper.beginParams(params, sizeof(Params), __alignof(Params), Params::RANDOM_COUNT);

		mapper.mapValue(offsetof(Params, scalePerVelocity), mParams->scalePerVelocity);
		mapper.mapValue(offsetof(Params, scaleChangeLimit), mParams->scaleChangeLimit);
		mapper.mapValue(offsetof(Params, scaleChangeDelay), mParams->scaleChangeDelay);

		mapper.endParams();
	}

	virtual updateSpriteFunc getUpdateSpriteFunc(ModifierStage stage) const;

private:
	OrientScaleAlongScreenVelocityModifierParams* mParams;
};

#pragma warning( default: 4100 )

}
} // namespace nvidia

#endif /* __MODIFIER_IMPL_H__ */
