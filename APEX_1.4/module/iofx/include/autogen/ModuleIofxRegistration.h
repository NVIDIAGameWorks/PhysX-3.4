/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef MODULE_MODULEIOFXREGISTRATIONH_H
#define MODULE_MODULEIOFXREGISTRATIONH_H

#include "PsAllocator.h"
#include "NvRegistrationsForTraitsBase.h"
#include "nvparameterized/NvParameterizedTraits.h"
#include "PxAssert.h"
#include <stdint.h>

// INCLUDE GENERATED FACTORIES
#include "IofxDebugRenderParams.h"
#include "IofxAssetParameters.h"
#include "MeshIofxParameters.h"
#include "SpriteIofxParameters.h"
#include "RotationModifierParams.h"
#include "SimpleScaleModifierParams.h"
#include "ScaleByMassModifierParams.h"
#include "RandomScaleModifierParams.h"
#include "ColorVsLifeModifierParams.h"
#include "ColorVsLifeCompositeModifierParams.h"
#include "InitialColorModifierParams.h"
#include "ScaleVsLifeModifierParams.h"
#include "ScaleVsLife3DModifierParams.h"
#include "ScaleVsLife2DModifierParams.h"
#include "ScaleVsDensityModifierParams.h"
#include "ScaleVsDensity3DModifierParams.h"
#include "ScaleVsDensity2DModifierParams.h"
#include "ScaleVsCameraDistanceModifierParams.h"
#include "ScaleVsCameraDistance3DModifierParams.h"
#include "ScaleVsCameraDistance2DModifierParams.h"
#include "ColorVsDensityModifierParams.h"
#include "ColorVsDensityCompositeModifierParams.h"
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
#include "ColorVsVelocityCompositeModifierParams.h"
#include "ColorVsTemperatureModifierParams.h"
#include "ColorVsTemperatureCompositeModifierParams.h"
#include "ScaleVsTemperatureModifierParams.h"
#include "ScaleVsTemperature3DModifierParams.h"
#include "ScaleVsTemperature2DModifierParams.h"
#include "IofxModuleParameters.h"


// INCLUDE GENERATED CONVERSION


namespace nvidia {
namespace iofx {


class ModuleIofxRegistration : public NvParameterized::RegistrationsForTraitsBase
{
public:
	static void invokeRegistration(NvParameterized::Traits* parameterizedTraits)
	{
		if (parameterizedTraits)
		{
			ModuleIofxRegistration().registerAll(*parameterizedTraits);
		}
	}

	static void invokeUnregistration(NvParameterized::Traits* parameterizedTraits)
	{
		if (parameterizedTraits)
		{
			ModuleIofxRegistration().unregisterAll(*parameterizedTraits);
		}
	}

	void registerAvailableFactories(NvParameterized::Traits& parameterizedTraits)
	{
		::NvParameterized::Factory* factoriesToRegister[] = {
// REGISTER GENERATED FACTORIES
			new nvidia::iofx::IofxDebugRenderParamsFactory(),
			new nvidia::iofx::IofxAssetParametersFactory(),
			new nvidia::iofx::MeshIofxParametersFactory(),
			new nvidia::iofx::SpriteIofxParametersFactory(),
			new nvidia::iofx::RotationModifierParamsFactory(),
			new nvidia::iofx::SimpleScaleModifierParamsFactory(),
			new nvidia::iofx::ScaleByMassModifierParamsFactory(),
			new nvidia::iofx::RandomScaleModifierParamsFactory(),
			new nvidia::iofx::ColorVsLifeModifierParamsFactory(),
			new nvidia::iofx::ColorVsLifeCompositeModifierParamsFactory(),
			new nvidia::iofx::InitialColorModifierParamsFactory(),
			new nvidia::iofx::ScaleVsLifeModifierParamsFactory(),
			new nvidia::iofx::ScaleVsLife3DModifierParamsFactory(),
			new nvidia::iofx::ScaleVsLife2DModifierParamsFactory(),
			new nvidia::iofx::ScaleVsDensityModifierParamsFactory(),
			new nvidia::iofx::ScaleVsDensity3DModifierParamsFactory(),
			new nvidia::iofx::ScaleVsDensity2DModifierParamsFactory(),
			new nvidia::iofx::ScaleVsCameraDistanceModifierParamsFactory(),
			new nvidia::iofx::ScaleVsCameraDistance3DModifierParamsFactory(),
			new nvidia::iofx::ScaleVsCameraDistance2DModifierParamsFactory(),
			new nvidia::iofx::ColorVsDensityModifierParamsFactory(),
			new nvidia::iofx::ColorVsDensityCompositeModifierParamsFactory(),
			new nvidia::iofx::SubtextureVsLifeModifierParamsFactory(),
			new nvidia::iofx::OrientAlongVelocityModifierParamsFactory(),
			new nvidia::iofx::ScaleAlongVelocityModifierParamsFactory(),
			new nvidia::iofx::RandomSubtextureModifierParamsFactory(),
			new nvidia::iofx::RandomRotationModifierParamsFactory(),
			new nvidia::iofx::ViewDirectionSortingModifierParamsFactory(),
			new nvidia::iofx::RotationRateModifierParamsFactory(),
			new nvidia::iofx::RotationRateVsLifeModifierParamsFactory(),
			new nvidia::iofx::OrientScaleAlongScreenVelocityModifierParamsFactory(),
			new nvidia::iofx::ColorVsVelocityModifierParamsFactory(),
			new nvidia::iofx::ColorVsVelocityCompositeModifierParamsFactory(),
			new nvidia::iofx::ColorVsTemperatureModifierParamsFactory(),
			new nvidia::iofx::ColorVsTemperatureCompositeModifierParamsFactory(),
			new nvidia::iofx::ScaleVsTemperatureModifierParamsFactory(),
			new nvidia::iofx::ScaleVsTemperature3DModifierParamsFactory(),
			new nvidia::iofx::ScaleVsTemperature2DModifierParamsFactory(),
			new nvidia::iofx::IofxModuleParametersFactory(),

		};

		for (size_t i = 0; i < sizeof(factoriesToRegister)/sizeof(factoriesToRegister[0]); ++i)
		{
			parameterizedTraits.registerFactory(*factoriesToRegister[i]);
		}
	}

	virtual void registerAvailableConverters(NvParameterized::Traits& parameterizedTraits)
	{
// REGISTER GENERATED CONVERSION
PX_UNUSED(parameterizedTraits);

	}

	void unregisterAvailableFactories(NvParameterized::Traits& parameterizedTraits)
	{
		struct FactoryDesc
		{
			const char* name;
			uint32_t version;
		};

		::NvParameterized::Factory* factoriesToUnregister[] = {
// UNREGISTER GENERATED FACTORIES
			new nvidia::iofx::IofxDebugRenderParamsFactory(),
			new nvidia::iofx::IofxAssetParametersFactory(),
			new nvidia::iofx::MeshIofxParametersFactory(),
			new nvidia::iofx::SpriteIofxParametersFactory(),
			new nvidia::iofx::RotationModifierParamsFactory(),
			new nvidia::iofx::SimpleScaleModifierParamsFactory(),
			new nvidia::iofx::ScaleByMassModifierParamsFactory(),
			new nvidia::iofx::RandomScaleModifierParamsFactory(),
			new nvidia::iofx::ColorVsLifeModifierParamsFactory(),
			new nvidia::iofx::ColorVsLifeCompositeModifierParamsFactory(),
			new nvidia::iofx::InitialColorModifierParamsFactory(),
			new nvidia::iofx::ScaleVsLifeModifierParamsFactory(),
			new nvidia::iofx::ScaleVsLife3DModifierParamsFactory(),
			new nvidia::iofx::ScaleVsLife2DModifierParamsFactory(),
			new nvidia::iofx::ScaleVsDensityModifierParamsFactory(),
			new nvidia::iofx::ScaleVsDensity3DModifierParamsFactory(),
			new nvidia::iofx::ScaleVsDensity2DModifierParamsFactory(),
			new nvidia::iofx::ScaleVsCameraDistanceModifierParamsFactory(),
			new nvidia::iofx::ScaleVsCameraDistance3DModifierParamsFactory(),
			new nvidia::iofx::ScaleVsCameraDistance2DModifierParamsFactory(),
			new nvidia::iofx::ColorVsDensityModifierParamsFactory(),
			new nvidia::iofx::ColorVsDensityCompositeModifierParamsFactory(),
			new nvidia::iofx::SubtextureVsLifeModifierParamsFactory(),
			new nvidia::iofx::OrientAlongVelocityModifierParamsFactory(),
			new nvidia::iofx::ScaleAlongVelocityModifierParamsFactory(),
			new nvidia::iofx::RandomSubtextureModifierParamsFactory(),
			new nvidia::iofx::RandomRotationModifierParamsFactory(),
			new nvidia::iofx::ViewDirectionSortingModifierParamsFactory(),
			new nvidia::iofx::RotationRateModifierParamsFactory(),
			new nvidia::iofx::RotationRateVsLifeModifierParamsFactory(),
			new nvidia::iofx::OrientScaleAlongScreenVelocityModifierParamsFactory(),
			new nvidia::iofx::ColorVsVelocityModifierParamsFactory(),
			new nvidia::iofx::ColorVsVelocityCompositeModifierParamsFactory(),
			new nvidia::iofx::ColorVsTemperatureModifierParamsFactory(),
			new nvidia::iofx::ColorVsTemperatureCompositeModifierParamsFactory(),
			new nvidia::iofx::ScaleVsTemperatureModifierParamsFactory(),
			new nvidia::iofx::ScaleVsTemperature3DModifierParamsFactory(),
			new nvidia::iofx::ScaleVsTemperature2DModifierParamsFactory(),
			new nvidia::iofx::IofxModuleParametersFactory(),

		};

		for (size_t i = 0; i < sizeof(factoriesToUnregister)/sizeof(factoriesToUnregister[0]); ++i)
		{
			::NvParameterized::Factory* removedFactory = parameterizedTraits.removeFactory(factoriesToUnregister[i]->getClassName(), factoriesToUnregister[i]->getVersion());
			if (!removedFactory) 
			{
				PX_ASSERT_WITH_MESSAGE(0, "Factory can not be removed!");
			}
			else
			{
				removedFactory->freeParameterDefinitionTable(&parameterizedTraits);
				delete removedFactory;
				delete factoriesToUnregister[i];
			}
		}
	}

	virtual void unregisterAvailableConverters(NvParameterized::Traits& parameterizedTraits)
	{
// UNREGISTER GENERATED CONVERSION
PX_UNUSED(parameterizedTraits);

	}

};


}
} //nvidia::iofx

#endif
