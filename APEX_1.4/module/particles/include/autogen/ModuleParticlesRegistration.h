/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef MODULE_MODULEPARTICLESREGISTRATIONH_H
#define MODULE_MODULEPARTICLESREGISTRATIONH_H

#include "PsAllocator.h"
#include "NvRegistrationsForTraitsBase.h"
#include "nvparameterized/NvParameterizedTraits.h"
#include "PxAssert.h"
#include <stdint.h>

// INCLUDE GENERATED FACTORIES
#include "EffectPackageActorParams.h"
#include "EffectPackageAssetParams.h"
#include "HeatSourceEffect.h"
#include "EmitterEffect.h"
#include "ForceFieldEffect.h"
#include "TurbulenceFieldSamplerEffect.h"
#include "AttractorFieldSamplerEffect.h"
#include "JetFieldSamplerEffect.h"
#include "NoiseFieldSamplerEffect.h"
#include "VortexFieldSamplerEffect.h"
#include "SubstanceSourceEffect.h"
#include "WindFieldSamplerEffect.h"
#include "RigidBodyEffect.h"
#include "VelocitySourceEffect.h"
#include "FlameEmitterEffect.h"
#include "EffectPackageDatabaseParams.h"
#include "EffectPackageData.h"
#include "ParticlesDebugRenderParams.h"
#include "EffectPackageEmitterDatabaseParams.h"
#include "EmitterData.h"
#include "EffectPackageFieldSamplerDatabaseParams.h"
#include "AttractorFieldSamplerData.h"
#include "HeatSourceData.h"
#include "JetFieldSamplerData.h"
#include "TurbulenceFieldSamplerData.h"
#include "ForceFieldData.h"
#include "NoiseFieldSamplerData.h"
#include "VortexFieldSamplerData.h"
#include "SubstanceSourceData.h"
#include "WindFieldSamplerData.h"
#include "VelocitySourceData.h"
#include "FlameEmitterData.h"
#include "EffectPackageGraphicsMaterialsParams.h"
#include "GraphicsMaterialData.h"
#include "VolumeRenderMaterialData.h"
#include "EffectPackageIOFXDatabaseParams.h"
#include "GraphicsEffectData.h"
#include "EffectPackageIOSDatabaseParams.h"
#include "ParticleSimulationData.h"
#include "ParticlesModuleParameters.h"


// INCLUDE GENERATED CONVERSION


namespace nvidia {
namespace particles {


class ModuleParticlesRegistration : public NvParameterized::RegistrationsForTraitsBase
{
public:
	static void invokeRegistration(NvParameterized::Traits* parameterizedTraits)
	{
		if (parameterizedTraits)
		{
			ModuleParticlesRegistration().registerAll(*parameterizedTraits);
		}
	}

	static void invokeUnregistration(NvParameterized::Traits* parameterizedTraits)
	{
		if (parameterizedTraits)
		{
			ModuleParticlesRegistration().unregisterAll(*parameterizedTraits);
		}
	}

	void registerAvailableFactories(NvParameterized::Traits& parameterizedTraits)
	{
		::NvParameterized::Factory* factoriesToRegister[] = {
// REGISTER GENERATED FACTORIES
			new nvidia::particles::EffectPackageActorParamsFactory(),
			new nvidia::particles::EffectPackageAssetParamsFactory(),
			new nvidia::particles::HeatSourceEffectFactory(),
			new nvidia::particles::EmitterEffectFactory(),
			new nvidia::particles::ForceFieldEffectFactory(),
			new nvidia::particles::TurbulenceFieldSamplerEffectFactory(),
			new nvidia::particles::AttractorFieldSamplerEffectFactory(),
			new nvidia::particles::JetFieldSamplerEffectFactory(),
			new nvidia::particles::NoiseFieldSamplerEffectFactory(),
			new nvidia::particles::VortexFieldSamplerEffectFactory(),
			new nvidia::particles::SubstanceSourceEffectFactory(),
			new nvidia::particles::WindFieldSamplerEffectFactory(),
			new nvidia::particles::RigidBodyEffectFactory(),
			new nvidia::particles::VelocitySourceEffectFactory(),
			new nvidia::particles::FlameEmitterEffectFactory(),
			new nvidia::particles::EffectPackageDatabaseParamsFactory(),
			new nvidia::particles::EffectPackageDataFactory(),
			new nvidia::particles::ParticlesDebugRenderParamsFactory(),
			new nvidia::particles::EffectPackageEmitterDatabaseParamsFactory(),
			new nvidia::particles::EmitterDataFactory(),
			new nvidia::particles::EffectPackageFieldSamplerDatabaseParamsFactory(),
			new nvidia::particles::AttractorFieldSamplerDataFactory(),
			new nvidia::particles::HeatSourceDataFactory(),
			new nvidia::particles::JetFieldSamplerDataFactory(),
			new nvidia::particles::TurbulenceFieldSamplerDataFactory(),
			new nvidia::particles::ForceFieldDataFactory(),
			new nvidia::particles::NoiseFieldSamplerDataFactory(),
			new nvidia::particles::VortexFieldSamplerDataFactory(),
			new nvidia::particles::SubstanceSourceDataFactory(),
			new nvidia::particles::WindFieldSamplerDataFactory(),
			new nvidia::particles::VelocitySourceDataFactory(),
			new nvidia::particles::FlameEmitterDataFactory(),
			new nvidia::particles::EffectPackageGraphicsMaterialsParamsFactory(),
			new nvidia::particles::GraphicsMaterialDataFactory(),
			new nvidia::particles::VolumeRenderMaterialDataFactory(),
			new nvidia::particles::EffectPackageIOFXDatabaseParamsFactory(),
			new nvidia::particles::GraphicsEffectDataFactory(),
			new nvidia::particles::EffectPackageIOSDatabaseParamsFactory(),
			new nvidia::particles::ParticleSimulationDataFactory(),
			new nvidia::particles::ParticlesModuleParametersFactory(),

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
			new nvidia::particles::EffectPackageActorParamsFactory(),
			new nvidia::particles::EffectPackageAssetParamsFactory(),
			new nvidia::particles::HeatSourceEffectFactory(),
			new nvidia::particles::EmitterEffectFactory(),
			new nvidia::particles::ForceFieldEffectFactory(),
			new nvidia::particles::TurbulenceFieldSamplerEffectFactory(),
			new nvidia::particles::AttractorFieldSamplerEffectFactory(),
			new nvidia::particles::JetFieldSamplerEffectFactory(),
			new nvidia::particles::NoiseFieldSamplerEffectFactory(),
			new nvidia::particles::VortexFieldSamplerEffectFactory(),
			new nvidia::particles::SubstanceSourceEffectFactory(),
			new nvidia::particles::WindFieldSamplerEffectFactory(),
			new nvidia::particles::RigidBodyEffectFactory(),
			new nvidia::particles::VelocitySourceEffectFactory(),
			new nvidia::particles::FlameEmitterEffectFactory(),
			new nvidia::particles::EffectPackageDatabaseParamsFactory(),
			new nvidia::particles::EffectPackageDataFactory(),
			new nvidia::particles::ParticlesDebugRenderParamsFactory(),
			new nvidia::particles::EffectPackageEmitterDatabaseParamsFactory(),
			new nvidia::particles::EmitterDataFactory(),
			new nvidia::particles::EffectPackageFieldSamplerDatabaseParamsFactory(),
			new nvidia::particles::AttractorFieldSamplerDataFactory(),
			new nvidia::particles::HeatSourceDataFactory(),
			new nvidia::particles::JetFieldSamplerDataFactory(),
			new nvidia::particles::TurbulenceFieldSamplerDataFactory(),
			new nvidia::particles::ForceFieldDataFactory(),
			new nvidia::particles::NoiseFieldSamplerDataFactory(),
			new nvidia::particles::VortexFieldSamplerDataFactory(),
			new nvidia::particles::SubstanceSourceDataFactory(),
			new nvidia::particles::WindFieldSamplerDataFactory(),
			new nvidia::particles::VelocitySourceDataFactory(),
			new nvidia::particles::FlameEmitterDataFactory(),
			new nvidia::particles::EffectPackageGraphicsMaterialsParamsFactory(),
			new nvidia::particles::GraphicsMaterialDataFactory(),
			new nvidia::particles::VolumeRenderMaterialDataFactory(),
			new nvidia::particles::EffectPackageIOFXDatabaseParamsFactory(),
			new nvidia::particles::GraphicsEffectDataFactory(),
			new nvidia::particles::EffectPackageIOSDatabaseParamsFactory(),
			new nvidia::particles::ParticleSimulationDataFactory(),
			new nvidia::particles::ParticlesModuleParametersFactory(),

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
} //nvidia::particles

#endif
