/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef MODULE_MODULEIOFXLEGACYREGISTRATIONH_H
#define MODULE_MODULEIOFXLEGACYREGISTRATIONH_H

#include "PsAllocator.h"
#include "NvRegistrationsForTraitsBase.h"
#include "nvparameterized/NvParameterizedTraits.h"
#include "PxAssert.h"
#include <stdint.h>

// INCLUDE GENERATED FACTORIES
#include "IofxAssetParameters_0p0.h"
#include "IofxAssetParameters_0p1.h"
#include "MeshIofxParameters_0p0.h"
#include "MeshIofxParameters_0p1.h"
#include "MeshIofxParameters_0p2.h"
#include "MeshIofxParameters_0p3.h"
#include "OrientScaleAlongScreenVelocityModifierParams_0p0.h"
#include "RotationModifierParams_0p0.h"
#include "RotationModifierParams_0p1.h"
#include "SpriteIofxParameters_0p0.h"
#include "SpriteIofxParameters_0p1.h"
#include "SpriteIofxParameters_0p2.h"
#include "SpriteIofxParameters_0p3.h"
#include "IofxDebugRenderParams_0p0.h"
#include "IofxAssetParameters_0p2.h"
#include "MeshIofxParameters_0p4.h"
#include "SpriteIofxParameters_0p4.h"
#include "RotationModifierParams_0p2.h"
#include "SimpleScaleModifierParams_0p0.h"
#include "ScaleByMassModifierParams_0p0.h"
#include "RandomScaleModifierParams_0p0.h"
#include "ColorVsLifeModifierParams_0p0.h"
#include "ColorVsLifeCompositeModifierParams_0p0.h"
#include "InitialColorModifierParams_0p0.h"
#include "ScaleVsLifeModifierParams_0p0.h"
#include "ScaleVsLife3DModifierParams_0p0.h"
#include "ScaleVsLife2DModifierParams_0p0.h"
#include "ScaleVsDensityModifierParams_0p0.h"
#include "ScaleVsDensity3DModifierParams_0p0.h"
#include "ScaleVsDensity2DModifierParams_0p0.h"
#include "ScaleVsCameraDistanceModifierParams_0p0.h"
#include "ScaleVsCameraDistance3DModifierParams_0p0.h"
#include "ScaleVsCameraDistance2DModifierParams_0p0.h"
#include "ColorVsDensityModifierParams_0p0.h"
#include "ColorVsDensityCompositeModifierParams_0p0.h"
#include "SubtextureVsLifeModifierParams_0p0.h"
#include "OrientAlongVelocityModifierParams_0p0.h"
#include "ScaleAlongVelocityModifierParams_0p0.h"
#include "RandomSubtextureModifierParams_0p0.h"
#include "RandomRotationModifierParams_0p0.h"
#include "ViewDirectionSortingModifierParams_0p0.h"
#include "RotationRateModifierParams_0p0.h"
#include "RotationRateVsLifeModifierParams_0p0.h"
#include "OrientScaleAlongScreenVelocityModifierParams_0p1.h"
#include "ColorVsVelocityModifierParams_0p0.h"
#include "ColorVsVelocityCompositeModifierParams_0p0.h"
#include "ColorVsTemperatureModifierParams_0p0.h"
#include "ColorVsTemperatureCompositeModifierParams_0p0.h"
#include "ScaleVsTemperatureModifierParams_0p0.h"
#include "ScaleVsTemperature3DModifierParams_0p0.h"
#include "ScaleVsTemperature2DModifierParams_0p0.h"
#include "IofxModuleParameters_0p0.h"


// INCLUDE GENERATED CONVERSION
#include "ConversionIofxAssetParameters_0p0_0p1.h"
#include "ConversionIofxAssetParameters_0p1_0p2.h"
#include "ConversionMeshIofxParameters_0p0_0p1.h"
#include "ConversionMeshIofxParameters_0p1_0p2.h"
#include "ConversionMeshIofxParameters_0p2_0p3.h"
#include "ConversionMeshIofxParameters_0p3_0p4.h"
#include "ConversionOrientScaleAlongScreenVelocityModifierParams_0p0_0p1.h"
#include "ConversionRotationModifierParams_0p0_0p1.h"
#include "ConversionRotationModifierParams_0p1_0p2.h"
#include "ConversionSpriteIofxParameters_0p0_0p1.h"
#include "ConversionSpriteIofxParameters_0p1_0p2.h"
#include "ConversionSpriteIofxParameters_0p2_0p3.h"
#include "ConversionSpriteIofxParameters_0p3_0p4.h"


// global namespace

class ModuleIOFXLegacyRegistration : public NvParameterized::RegistrationsForTraitsBase
{
public:
	static void invokeRegistration(NvParameterized::Traits* parameterizedTraits)
	{
		if (parameterizedTraits)
		{
			ModuleIOFXLegacyRegistration().registerAll(*parameterizedTraits);
		}
	}

	static void invokeUnregistration(NvParameterized::Traits* parameterizedTraits)
	{
		if (parameterizedTraits)
		{
			ModuleIOFXLegacyRegistration().unregisterAll(*parameterizedTraits);
		}
	}

	void registerAvailableFactories(NvParameterized::Traits& parameterizedTraits)
	{
		::NvParameterized::Factory* factoriesToRegister[] = {
// REGISTER GENERATED FACTORIES
			new nvidia::parameterized::IofxAssetParameters_0p0Factory(),
			new nvidia::parameterized::IofxAssetParameters_0p1Factory(),
			new nvidia::parameterized::MeshIofxParameters_0p0Factory(),
			new nvidia::parameterized::MeshIofxParameters_0p1Factory(),
			new nvidia::parameterized::MeshIofxParameters_0p2Factory(),
			new nvidia::parameterized::MeshIofxParameters_0p3Factory(),
			new nvidia::parameterized::OrientScaleAlongScreenVelocityModifierParams_0p0Factory(),
			new nvidia::parameterized::RotationModifierParams_0p0Factory(),
			new nvidia::parameterized::RotationModifierParams_0p1Factory(),
			new nvidia::parameterized::SpriteIofxParameters_0p0Factory(),
			new nvidia::parameterized::SpriteIofxParameters_0p1Factory(),
			new nvidia::parameterized::SpriteIofxParameters_0p2Factory(),
			new nvidia::parameterized::SpriteIofxParameters_0p3Factory(),

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
			typedef nvidia::apex::legacy::ConversionIofxAssetParameters_0p0_0p1 ConverterToRegister;
			parameterizedTraits.registerConversion(ConverterToRegister::TOldClass::staticClassName(),
								ConverterToRegister::TOldClass::ClassVersion,
								ConverterToRegister::TNewClass::ClassVersion,
								*(ConverterToRegister::Create(&parameterizedTraits)));
			}
			{
			typedef nvidia::apex::legacy::ConversionIofxAssetParameters_0p1_0p2 ConverterToRegister;
			parameterizedTraits.registerConversion(ConverterToRegister::TOldClass::staticClassName(),
								ConverterToRegister::TOldClass::ClassVersion,
								ConverterToRegister::TNewClass::ClassVersion,
								*(ConverterToRegister::Create(&parameterizedTraits)));
			}
			{
			typedef nvidia::apex::legacy::ConversionMeshIofxParameters_0p0_0p1 ConverterToRegister;
			parameterizedTraits.registerConversion(ConverterToRegister::TOldClass::staticClassName(),
								ConverterToRegister::TOldClass::ClassVersion,
								ConverterToRegister::TNewClass::ClassVersion,
								*(ConverterToRegister::Create(&parameterizedTraits)));
			}
			{
			typedef nvidia::apex::legacy::ConversionMeshIofxParameters_0p1_0p2 ConverterToRegister;
			parameterizedTraits.registerConversion(ConverterToRegister::TOldClass::staticClassName(),
								ConverterToRegister::TOldClass::ClassVersion,
								ConverterToRegister::TNewClass::ClassVersion,
								*(ConverterToRegister::Create(&parameterizedTraits)));
			}
			{
			typedef nvidia::apex::legacy::ConversionMeshIofxParameters_0p2_0p3 ConverterToRegister;
			parameterizedTraits.registerConversion(ConverterToRegister::TOldClass::staticClassName(),
								ConverterToRegister::TOldClass::ClassVersion,
								ConverterToRegister::TNewClass::ClassVersion,
								*(ConverterToRegister::Create(&parameterizedTraits)));
			}
			{
			typedef nvidia::apex::legacy::ConversionMeshIofxParameters_0p3_0p4 ConverterToRegister;
			parameterizedTraits.registerConversion(ConverterToRegister::TOldClass::staticClassName(),
								ConverterToRegister::TOldClass::ClassVersion,
								ConverterToRegister::TNewClass::ClassVersion,
								*(ConverterToRegister::Create(&parameterizedTraits)));
			}
			{
			typedef nvidia::apex::legacy::ConversionOrientScaleAlongScreenVelocityModifierParams_0p0_0p1 ConverterToRegister;
			parameterizedTraits.registerConversion(ConverterToRegister::TOldClass::staticClassName(),
								ConverterToRegister::TOldClass::ClassVersion,
								ConverterToRegister::TNewClass::ClassVersion,
								*(ConverterToRegister::Create(&parameterizedTraits)));
			}
			{
			typedef nvidia::apex::legacy::ConversionRotationModifierParams_0p0_0p1 ConverterToRegister;
			parameterizedTraits.registerConversion(ConverterToRegister::TOldClass::staticClassName(),
								ConverterToRegister::TOldClass::ClassVersion,
								ConverterToRegister::TNewClass::ClassVersion,
								*(ConverterToRegister::Create(&parameterizedTraits)));
			}
			{
			typedef nvidia::apex::legacy::ConversionRotationModifierParams_0p1_0p2 ConverterToRegister;
			parameterizedTraits.registerConversion(ConverterToRegister::TOldClass::staticClassName(),
								ConverterToRegister::TOldClass::ClassVersion,
								ConverterToRegister::TNewClass::ClassVersion,
								*(ConverterToRegister::Create(&parameterizedTraits)));
			}
			{
			typedef nvidia::apex::legacy::ConversionSpriteIofxParameters_0p0_0p1 ConverterToRegister;
			parameterizedTraits.registerConversion(ConverterToRegister::TOldClass::staticClassName(),
								ConverterToRegister::TOldClass::ClassVersion,
								ConverterToRegister::TNewClass::ClassVersion,
								*(ConverterToRegister::Create(&parameterizedTraits)));
			}
			{
			typedef nvidia::apex::legacy::ConversionSpriteIofxParameters_0p1_0p2 ConverterToRegister;
			parameterizedTraits.registerConversion(ConverterToRegister::TOldClass::staticClassName(),
								ConverterToRegister::TOldClass::ClassVersion,
								ConverterToRegister::TNewClass::ClassVersion,
								*(ConverterToRegister::Create(&parameterizedTraits)));
			}
			{
			typedef nvidia::apex::legacy::ConversionSpriteIofxParameters_0p2_0p3 ConverterToRegister;
			parameterizedTraits.registerConversion(ConverterToRegister::TOldClass::staticClassName(),
								ConverterToRegister::TOldClass::ClassVersion,
								ConverterToRegister::TNewClass::ClassVersion,
								*(ConverterToRegister::Create(&parameterizedTraits)));
			}
			{
			typedef nvidia::apex::legacy::ConversionSpriteIofxParameters_0p3_0p4 ConverterToRegister;
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
			new nvidia::parameterized::IofxAssetParameters_0p0Factory(),
			new nvidia::parameterized::IofxAssetParameters_0p1Factory(),
			new nvidia::parameterized::MeshIofxParameters_0p0Factory(),
			new nvidia::parameterized::MeshIofxParameters_0p1Factory(),
			new nvidia::parameterized::MeshIofxParameters_0p2Factory(),
			new nvidia::parameterized::MeshIofxParameters_0p3Factory(),
			new nvidia::parameterized::OrientScaleAlongScreenVelocityModifierParams_0p0Factory(),
			new nvidia::parameterized::RotationModifierParams_0p0Factory(),
			new nvidia::parameterized::RotationModifierParams_0p1Factory(),
			new nvidia::parameterized::SpriteIofxParameters_0p0Factory(),
			new nvidia::parameterized::SpriteIofxParameters_0p1Factory(),
			new nvidia::parameterized::SpriteIofxParameters_0p2Factory(),
			new nvidia::parameterized::SpriteIofxParameters_0p3Factory(),

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
			typedef nvidia::apex::legacy::ConversionIofxAssetParameters_0p0_0p1 ConverterToUnregister;
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
			typedef nvidia::apex::legacy::ConversionIofxAssetParameters_0p1_0p2 ConverterToUnregister;
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
			typedef nvidia::apex::legacy::ConversionMeshIofxParameters_0p0_0p1 ConverterToUnregister;
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
			typedef nvidia::apex::legacy::ConversionMeshIofxParameters_0p1_0p2 ConverterToUnregister;
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
			typedef nvidia::apex::legacy::ConversionMeshIofxParameters_0p2_0p3 ConverterToUnregister;
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
			typedef nvidia::apex::legacy::ConversionMeshIofxParameters_0p3_0p4 ConverterToUnregister;
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
			typedef nvidia::apex::legacy::ConversionOrientScaleAlongScreenVelocityModifierParams_0p0_0p1 ConverterToUnregister;
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
			typedef nvidia::apex::legacy::ConversionRotationModifierParams_0p0_0p1 ConverterToUnregister;
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
			typedef nvidia::apex::legacy::ConversionRotationModifierParams_0p1_0p2 ConverterToUnregister;
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
			typedef nvidia::apex::legacy::ConversionSpriteIofxParameters_0p0_0p1 ConverterToUnregister;
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
			typedef nvidia::apex::legacy::ConversionSpriteIofxParameters_0p1_0p2 ConverterToUnregister;
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
			typedef nvidia::apex::legacy::ConversionSpriteIofxParameters_0p2_0p3 ConverterToUnregister;
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
			typedef nvidia::apex::legacy::ConversionSpriteIofxParameters_0p3_0p4 ConverterToUnregister;
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
