/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef MODULE_MODULEPARTICLESLEGACYREGISTRATIONH_H
#define MODULE_MODULEPARTICLESLEGACYREGISTRATIONH_H

#include "PsAllocator.h"
#include "NvRegistrationsForTraitsBase.h"
#include "nvparameterized/NvParameterizedTraits.h"
#include "PxAssert.h"
#include <stdint.h>

// INCLUDE GENERATED FACTORIES
#include "EffectPackageAssetParams_0p0.h"
#include "EffectPackageAssetParams_0p1.h"
#include "EffectPackageDatabaseParams_0p0.h"
#include "GraphicsMaterialData_0p0.h"
#include "GraphicsMaterialData_0p1.h"
#include "GraphicsMaterialData_0p2.h"
#include "GraphicsMaterialData_0p3.h"
#include "VolumeRenderMaterialData_0p0.h"
#include "EffectPackageActorParams_0p0.h"
#include "EffectPackageAssetParams_0p1.h"
#include "HeatSourceEffect_0p0.h"
#include "EmitterEffect_0p0.h"
#include "ForceFieldEffect_0p0.h"
#include "TurbulenceFieldSamplerEffect_0p0.h"
#include "AttractorFieldSamplerEffect_0p0.h"
#include "JetFieldSamplerEffect_0p0.h"
#include "NoiseFieldSamplerEffect_0p0.h"
#include "VortexFieldSamplerEffect_0p0.h"
#include "SubstanceSourceEffect_0p0.h"
#include "WindFieldSamplerEffect_0p0.h"
#include "RigidBodyEffect_0p0.h"
#include "VelocitySourceEffect_0p0.h"
#include "FlameEmitterEffect_0p0.h"
#include "EffectPackageDatabaseParams_0p1.h"
#include "EffectPackageData_0p0.h"
#include "ParticlesDebugRenderParams_0p0.h"
#include "EffectPackageEmitterDatabaseParams_0p0.h"
#include "EmitterData_0p0.h"
#include "EffectPackageFieldSamplerDatabaseParams_0p0.h"
#include "AttractorFieldSamplerData_0p0.h"
#include "HeatSourceData_0p0.h"
#include "JetFieldSamplerData_0p0.h"
#include "TurbulenceFieldSamplerData_0p0.h"
#include "ForceFieldData_0p0.h"
#include "NoiseFieldSamplerData_0p0.h"
#include "VortexFieldSamplerData_0p0.h"
#include "SubstanceSourceData_0p0.h"
#include "WindFieldSamplerData_0p0.h"
#include "VelocitySourceData_0p0.h"
#include "FlameEmitterData_0p0.h"
#include "EffectPackageGraphicsMaterialsParams_0p0.h"
#include "GraphicsMaterialData_0p4.h"
#include "VolumeRenderMaterialData_0p1.h"
#include "EffectPackageIOFXDatabaseParams_0p0.h"
#include "GraphicsEffectData_0p0.h"
#include "EffectPackageIOSDatabaseParams_0p0.h"
#include "ParticleSimulationData_0p0.h"
#include "ParticlesModuleParameters_0p0.h"


// INCLUDE GENERATED CONVERSION
#include "ConversionEffectPackageAssetParams_0p0_0p1.h"
#include "ConversionEffectPackageDatabaseParams_0p0_0p1.h"
#include "ConversionGraphicsMaterialData_0p0_0p1.h"
#include "ConversionGraphicsMaterialData_0p1_0p2.h"
#include "ConversionGraphicsMaterialData_0p2_0p3.h"
#include "ConversionGraphicsMaterialData_0p3_0p4.h"
#include "ConversionVolumeRenderMaterialData_0p0_0p1.h"


// global namespace

class ModuleParticlesLegacyRegistration : public NvParameterized::RegistrationsForTraitsBase
{
public:
	static void invokeRegistration(NvParameterized::Traits* parameterizedTraits)
	{
		if (parameterizedTraits)
		{
			ModuleParticlesLegacyRegistration().registerAll(*parameterizedTraits);
		}
	}

	static void invokeUnregistration(NvParameterized::Traits* parameterizedTraits)
	{
		if (parameterizedTraits)
		{
			ModuleParticlesLegacyRegistration().unregisterAll(*parameterizedTraits);
		}
	}

	void registerAvailableFactories(NvParameterized::Traits& parameterizedTraits)
	{
		::NvParameterized::Factory* factoriesToRegister[] = {
// REGISTER GENERATED FACTORIES
			new nvidia::parameterized::EffectPackageAssetParams_0p0Factory(),
			new nvidia::parameterized::EffectPackageDatabaseParams_0p0Factory(),
			new nvidia::parameterized::GraphicsMaterialData_0p0Factory(),
			new nvidia::parameterized::GraphicsMaterialData_0p1Factory(),
			new nvidia::parameterized::GraphicsMaterialData_0p2Factory(),
			new nvidia::parameterized::GraphicsMaterialData_0p3Factory(),
			new nvidia::parameterized::VolumeRenderMaterialData_0p0Factory(),

		};

		for (size_t i = 0; i < sizeof(factoriesToRegister)/sizeof(factoriesToRegister[0]); ++i)
		{
			parameterizedTraits.registerFactory(*factoriesToRegister[i]);
		}
	}

	virtual void registerAvailableConverters(NvParameterized::Traits& parameterizedTraits)
	{
// REGISTER GENERATED CONVERSION
			{
			typedef nvidia::apex::legacy::ConversionEffectPackageAssetParams_0p0_0p1 ConverterToRegister;
			parameterizedTraits.registerConversion(ConverterToRegister::TOldClass::staticClassName(),
								ConverterToRegister::TOldClass::ClassVersion,
								ConverterToRegister::TNewClass::ClassVersion,
								*(ConverterToRegister::Create(&parameterizedTraits)));
			}
			{
			typedef nvidia::apex::legacy::ConversionEffectPackageDatabaseParams_0p0_0p1 ConverterToRegister;
			parameterizedTraits.registerConversion(ConverterToRegister::TOldClass::staticClassName(),
								ConverterToRegister::TOldClass::ClassVersion,
								ConverterToRegister::TNewClass::ClassVersion,
								*(ConverterToRegister::Create(&parameterizedTraits)));
			}
			{
			typedef nvidia::apex::legacy::ConversionGraphicsMaterialData_0p0_0p1 ConverterToRegister;
			parameterizedTraits.registerConversion(ConverterToRegister::TOldClass::staticClassName(),
								ConverterToRegister::TOldClass::ClassVersion,
								ConverterToRegister::TNewClass::ClassVersion,
								*(ConverterToRegister::Create(&parameterizedTraits)));
			}
			{
			typedef nvidia::apex::legacy::ConversionGraphicsMaterialData_0p1_0p2 ConverterToRegister;
			parameterizedTraits.registerConversion(ConverterToRegister::TOldClass::staticClassName(),
								ConverterToRegister::TOldClass::ClassVersion,
								ConverterToRegister::TNewClass::ClassVersion,
								*(ConverterToRegister::Create(&parameterizedTraits)));
			}
			{
			typedef nvidia::apex::legacy::ConversionGraphicsMaterialData_0p2_0p3 ConverterToRegister;
			parameterizedTraits.registerConversion(ConverterToRegister::TOldClass::staticClassName(),
								ConverterToRegister::TOldClass::ClassVersion,
								ConverterToRegister::TNewClass::ClassVersion,
								*(ConverterToRegister::Create(&parameterizedTraits)));
			}
			{
			typedef nvidia::apex::legacy::ConversionGraphicsMaterialData_0p3_0p4 ConverterToRegister;
			parameterizedTraits.registerConversion(ConverterToRegister::TOldClass::staticClassName(),
								ConverterToRegister::TOldClass::ClassVersion,
								ConverterToRegister::TNewClass::ClassVersion,
								*(ConverterToRegister::Create(&parameterizedTraits)));
			}
			{
			typedef nvidia::apex::legacy::ConversionVolumeRenderMaterialData_0p0_0p1 ConverterToRegister;
			parameterizedTraits.registerConversion(ConverterToRegister::TOldClass::staticClassName(),
								ConverterToRegister::TOldClass::ClassVersion,
								ConverterToRegister::TNewClass::ClassVersion,
								*(ConverterToRegister::Create(&parameterizedTraits)));
			}

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
			new nvidia::parameterized::EffectPackageAssetParams_0p0Factory(),
			new nvidia::parameterized::EffectPackageDatabaseParams_0p0Factory(),
			new nvidia::parameterized::GraphicsMaterialData_0p0Factory(),
			new nvidia::parameterized::GraphicsMaterialData_0p1Factory(),
			new nvidia::parameterized::GraphicsMaterialData_0p2Factory(),
			new nvidia::parameterized::GraphicsMaterialData_0p3Factory(),
			new nvidia::parameterized::VolumeRenderMaterialData_0p0Factory(),

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
			{
			typedef nvidia::apex::legacy::ConversionEffectPackageAssetParams_0p0_0p1 ConverterToUnregister;
			::NvParameterized::Conversion* removedConv = parameterizedTraits.removeConversion(ConverterToUnregister::TOldClass::staticClassName(),
								ConverterToUnregister::TOldClass::ClassVersion,
								ConverterToUnregister::TNewClass::ClassVersion);
			if (removedConv) {
				removedConv->~Conversion(); parameterizedTraits.free(removedConv); // PLACEMENT DELETE 
			} else {
				// assert("Conversion was not found");
			}
			}
			{
			typedef nvidia::apex::legacy::ConversionEffectPackageDatabaseParams_0p0_0p1 ConverterToUnregister;
			::NvParameterized::Conversion* removedConv = parameterizedTraits.removeConversion(ConverterToUnregister::TOldClass::staticClassName(),
								ConverterToUnregister::TOldClass::ClassVersion,
								ConverterToUnregister::TNewClass::ClassVersion);
			if (removedConv) {
				removedConv->~Conversion(); parameterizedTraits.free(removedConv); // PLACEMENT DELETE 
			} else {
				// assert("Conversion was not found");
			}
			}
			{
			typedef nvidia::apex::legacy::ConversionGraphicsMaterialData_0p0_0p1 ConverterToUnregister;
			::NvParameterized::Conversion* removedConv = parameterizedTraits.removeConversion(ConverterToUnregister::TOldClass::staticClassName(),
								ConverterToUnregister::TOldClass::ClassVersion,
								ConverterToUnregister::TNewClass::ClassVersion);
			if (removedConv) {
				removedConv->~Conversion(); parameterizedTraits.free(removedConv); // PLACEMENT DELETE 
			} else {
				// assert("Conversion was not found");
			}
			}
			{
			typedef nvidia::apex::legacy::ConversionGraphicsMaterialData_0p1_0p2 ConverterToUnregister;
			::NvParameterized::Conversion* removedConv = parameterizedTraits.removeConversion(ConverterToUnregister::TOldClass::staticClassName(),
								ConverterToUnregister::TOldClass::ClassVersion,
								ConverterToUnregister::TNewClass::ClassVersion);
			if (removedConv) {
				removedConv->~Conversion(); parameterizedTraits.free(removedConv); // PLACEMENT DELETE 
			} else {
				// assert("Conversion was not found");
			}
			}
			{
			typedef nvidia::apex::legacy::ConversionGraphicsMaterialData_0p2_0p3 ConverterToUnregister;
			::NvParameterized::Conversion* removedConv = parameterizedTraits.removeConversion(ConverterToUnregister::TOldClass::staticClassName(),
								ConverterToUnregister::TOldClass::ClassVersion,
								ConverterToUnregister::TNewClass::ClassVersion);
			if (removedConv) {
				removedConv->~Conversion(); parameterizedTraits.free(removedConv); // PLACEMENT DELETE 
			} else {
				// assert("Conversion was not found");
			}
			}
			{
			typedef nvidia::apex::legacy::ConversionGraphicsMaterialData_0p3_0p4 ConverterToUnregister;
			::NvParameterized::Conversion* removedConv = parameterizedTraits.removeConversion(ConverterToUnregister::TOldClass::staticClassName(),
								ConverterToUnregister::TOldClass::ClassVersion,
								ConverterToUnregister::TNewClass::ClassVersion);
			if (removedConv) {
				removedConv->~Conversion(); parameterizedTraits.free(removedConv); // PLACEMENT DELETE 
			} else {
				// assert("Conversion was not found");
			}
			}
			{
			typedef nvidia::apex::legacy::ConversionVolumeRenderMaterialData_0p0_0p1 ConverterToUnregister;
			::NvParameterized::Conversion* removedConv = parameterizedTraits.removeConversion(ConverterToUnregister::TOldClass::staticClassName(),
								ConverterToUnregister::TOldClass::ClassVersion,
								ConverterToUnregister::TNewClass::ClassVersion);
			if (removedConv) {
				removedConv->~Conversion(); parameterizedTraits.free(removedConv); // PLACEMENT DELETE 
			} else {
				// assert("Conversion was not found");
			}
			}

	}

};

// global namespace

#endif
