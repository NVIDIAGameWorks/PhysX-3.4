/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef MODULE_MODULETURBULENCEFSLEGACYREGISTRATIONH_H
#define MODULE_MODULETURBULENCEFSLEGACYREGISTRATIONH_H

#include "PsAllocator.h"
#include "NvRegistrationsForTraitsBase.h"
#include "nvparameterized/NvParameterizedTraits.h"
#include "PxAssert.h"
#include <stdint.h>

// INCLUDE GENERATED FACTORIES
#include "FlameEmitterActorParams_0p0.h"
#include "FlameEmitterPreviewParams_0p0.h"
#include "HeatSourceActorParams_0p0.h"
#include "HeatSourceAssetParams_0p0.h"
#include "HeatSourceAssetParams_0p1.h"
#include "HeatSourcePreviewParams_0p0.h"
#include "SubstanceSourceActorParams_0p0.h"
#include "SubstanceSourceAssetParams_0p0.h"
#include "TurbulenceFSActorParams_0p0.h"
#include "TurbulenceFSAssetParams_0p0.h"
#include "TurbulenceFSAssetParams_0p1.h"
#include "TurbulenceFSAssetParams_0p2.h"
#include "TurbulenceFSAssetParams_0p3.h"
#include "TurbulenceFSAssetParams_0p4.h"
#include "TurbulenceFSAssetParams_0p5.h"
#include "TurbulenceFSAssetParams_0p6.h"
#include "TurbulenceFSAssetParams_0p7.h"
#include "TurbulenceFSAssetParams_0p8.h"
#include "TurbulenceFSAssetParams_0p9.h"
#include "TurbulenceFSAssetParams_1p0.h"
#include "TurbulenceFSAssetParams_1p1.h"
#include "TurbulenceFSAssetParams_1p2.h"
#include "TurbulenceFSAssetParams_1p3.h"
#include "TurbulenceFSAssetParams_1p4.h"
#include "VelocitySourceActorParams_0p0.h"
#include "VelocitySourcePreviewParams_0p0.h"
#include "FlameEmitterActorParams_0p1.h"
#include "FlameEmitterAssetParams_0p0.h"
#include "FlameEmitterPreviewParams_0p1.h"
#include "HeatSourceActorParams_0p1.h"
#include "HeatSourceAssetParams_0p2.h"
#include "HeatSourceGeomBoxParams_0p0.h"
#include "HeatSourceGeomSphereParams_0p0.h"
#include "HeatSourcePreviewParams_0p1.h"
#include "TurbulenceFSModuleParameters_0p0.h"
#include "SubstanceSourceActorParams_0p1.h"
#include "SubstanceSourceAssetParams_0p1.h"
#include "TurbulenceFSActorParams_0p1.h"
#include "TurbulenceFSAssetParams_1p5.h"
#include "TurbulenceFSDebugRenderParams_0p0.h"
#include "TurbulenceFSPreviewParams_0p0.h"
#include "VelocitySourceActorParams_0p1.h"
#include "VelocitySourceAssetParams_0p0.h"
#include "VelocitySourcePreviewParams_0p1.h"


// INCLUDE GENERATED CONVERSION
#include "ConversionFlameEmitterActorParams_0p0_0p1.h"
#include "ConversionFlameEmitterPreviewParams_0p0_0p1.h"
#include "ConversionHeatSourceActorParams_0p0_0p1.h"
#include "ConversionHeatSourceAssetParams_0p0_0p1.h"
#include "ConversionHeatSourceAssetParams_0p1_0p2.h"
#include "ConversionHeatSourcePreviewParams_0p0_0p1.h"
#include "ConversionSubstanceSourceActorParams_0p0_0p1.h"
#include "ConversionSubstanceSourceAssetParams_0p0_0p1.h"
#include "ConversionTurbulenceFSActorParams_0p0_0p1.h"
#include "ConversionTurbulenceFSAssetParams_0p0_0p1.h"
#include "ConversionTurbulenceFSAssetParams_0p1_0p2.h"
#include "ConversionTurbulenceFSAssetParams_0p2_0p3.h"
#include "ConversionTurbulenceFSAssetParams_0p3_0p4.h"
#include "ConversionTurbulenceFSAssetParams_0p4_0p5.h"
#include "ConversionTurbulenceFSAssetParams_0p5_0p6.h"
#include "ConversionTurbulenceFSAssetParams_0p6_0p7.h"
#include "ConversionTurbulenceFSAssetParams_0p7_0p8.h"
#include "ConversionTurbulenceFSAssetParams_0p8_0p9.h"
#include "ConversionTurbulenceFSAssetParams_0p9_1p0.h"
#include "ConversionTurbulenceFSAssetParams_1p0_1p1.h"
#include "ConversionTurbulenceFSAssetParams_1p1_1p2.h"
#include "ConversionTurbulenceFSAssetParams_1p2_1p3.h"
#include "ConversionTurbulenceFSAssetParams_1p3_1p4.h"
#include "ConversionTurbulenceFSAssetParams_1p4_1p5.h"
#include "ConversionVelocitySourceActorParams_0p0_0p1.h"
#include "ConversionVelocitySourcePreviewParams_0p0_0p1.h"


// global namespace

class ModuleTurbulenceFSLegacyRegistration : public NvParameterized::RegistrationsForTraitsBase
{
public:
	static void invokeRegistration(NvParameterized::Traits* parameterizedTraits)
	{
		if (parameterizedTraits)
		{
			ModuleTurbulenceFSLegacyRegistration().registerAll(*parameterizedTraits);
		}
	}

	static void invokeUnregistration(NvParameterized::Traits* parameterizedTraits)
	{
		if (parameterizedTraits)
		{
			ModuleTurbulenceFSLegacyRegistration().unregisterAll(*parameterizedTraits);
		}
	}

	void registerAvailableFactories(NvParameterized::Traits& parameterizedTraits)
	{
		::NvParameterized::Factory* factoriesToRegister[] = {
// REGISTER GENERATED FACTORIES
			new nvidia::parameterized::FlameEmitterActorParams_0p0Factory(),
			new nvidia::parameterized::FlameEmitterPreviewParams_0p0Factory(),
			new nvidia::parameterized::HeatSourceActorParams_0p0Factory(),
			new nvidia::parameterized::HeatSourceAssetParams_0p0Factory(),
			new nvidia::parameterized::HeatSourceAssetParams_0p1Factory(),
			new nvidia::parameterized::HeatSourcePreviewParams_0p0Factory(),
			new nvidia::parameterized::SubstanceSourceActorParams_0p0Factory(),
			new nvidia::parameterized::SubstanceSourceAssetParams_0p0Factory(),
			new nvidia::parameterized::TurbulenceFSActorParams_0p0Factory(),
			new nvidia::parameterized::TurbulenceFSAssetParams_0p0Factory(),
			new nvidia::parameterized::TurbulenceFSAssetParams_0p1Factory(),
			new nvidia::parameterized::TurbulenceFSAssetParams_0p2Factory(),
			new nvidia::parameterized::TurbulenceFSAssetParams_0p3Factory(),
			new nvidia::parameterized::TurbulenceFSAssetParams_0p4Factory(),
			new nvidia::parameterized::TurbulenceFSAssetParams_0p5Factory(),
			new nvidia::parameterized::TurbulenceFSAssetParams_0p6Factory(),
			new nvidia::parameterized::TurbulenceFSAssetParams_0p7Factory(),
			new nvidia::parameterized::TurbulenceFSAssetParams_0p8Factory(),
			new nvidia::parameterized::TurbulenceFSAssetParams_0p9Factory(),
			new nvidia::parameterized::TurbulenceFSAssetParams_1p0Factory(),
			new nvidia::parameterized::TurbulenceFSAssetParams_1p1Factory(),
			new nvidia::parameterized::TurbulenceFSAssetParams_1p2Factory(),
			new nvidia::parameterized::TurbulenceFSAssetParams_1p3Factory(),
			new nvidia::parameterized::TurbulenceFSAssetParams_1p4Factory(),
			new nvidia::parameterized::VelocitySourceActorParams_0p0Factory(),
			new nvidia::parameterized::VelocitySourcePreviewParams_0p0Factory(),

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
			typedef nvidia::apex::legacy::ConversionFlameEmitterActorParams_0p0_0p1 ConverterToRegister;
			parameterizedTraits.registerConversion(ConverterToRegister::TOldClass::staticClassName(),
								ConverterToRegister::TOldClass::ClassVersion,
								ConverterToRegister::TNewClass::ClassVersion,
								*(ConverterToRegister::Create(&parameterizedTraits)));
			}
			{
			typedef nvidia::apex::legacy::ConversionFlameEmitterPreviewParams_0p0_0p1 ConverterToRegister;
			parameterizedTraits.registerConversion(ConverterToRegister::TOldClass::staticClassName(),
								ConverterToRegister::TOldClass::ClassVersion,
								ConverterToRegister::TNewClass::ClassVersion,
								*(ConverterToRegister::Create(&parameterizedTraits)));
			}
			{
			typedef nvidia::apex::legacy::ConversionHeatSourceActorParams_0p0_0p1 ConverterToRegister;
			parameterizedTraits.registerConversion(ConverterToRegister::TOldClass::staticClassName(),
								ConverterToRegister::TOldClass::ClassVersion,
								ConverterToRegister::TNewClass::ClassVersion,
								*(ConverterToRegister::Create(&parameterizedTraits)));
			}
			{
			typedef nvidia::apex::legacy::ConversionHeatSourceAssetParams_0p0_0p1 ConverterToRegister;
			parameterizedTraits.registerConversion(ConverterToRegister::TOldClass::staticClassName(),
								ConverterToRegister::TOldClass::ClassVersion,
								ConverterToRegister::TNewClass::ClassVersion,
								*(ConverterToRegister::Create(&parameterizedTraits)));
			}
			{
			typedef nvidia::apex::legacy::ConversionHeatSourceAssetParams_0p1_0p2 ConverterToRegister;
			parameterizedTraits.registerConversion(ConverterToRegister::TOldClass::staticClassName(),
								ConverterToRegister::TOldClass::ClassVersion,
								ConverterToRegister::TNewClass::ClassVersion,
								*(ConverterToRegister::Create(&parameterizedTraits)));
			}
			{
			typedef nvidia::apex::legacy::ConversionHeatSourcePreviewParams_0p0_0p1 ConverterToRegister;
			parameterizedTraits.registerConversion(ConverterToRegister::TOldClass::staticClassName(),
								ConverterToRegister::TOldClass::ClassVersion,
								ConverterToRegister::TNewClass::ClassVersion,
								*(ConverterToRegister::Create(&parameterizedTraits)));
			}
			{
			typedef nvidia::apex::legacy::ConversionSubstanceSourceActorParams_0p0_0p1 ConverterToRegister;
			parameterizedTraits.registerConversion(ConverterToRegister::TOldClass::staticClassName(),
								ConverterToRegister::TOldClass::ClassVersion,
								ConverterToRegister::TNewClass::ClassVersion,
								*(ConverterToRegister::Create(&parameterizedTraits)));
			}
			{
			typedef nvidia::apex::legacy::ConversionSubstanceSourceAssetParams_0p0_0p1 ConverterToRegister;
			parameterizedTraits.registerConversion(ConverterToRegister::TOldClass::staticClassName(),
								ConverterToRegister::TOldClass::ClassVersion,
								ConverterToRegister::TNewClass::ClassVersion,
								*(ConverterToRegister::Create(&parameterizedTraits)));
			}
			{
			typedef nvidia::apex::legacy::ConversionTurbulenceFSActorParams_0p0_0p1 ConverterToRegister;
			parameterizedTraits.registerConversion(ConverterToRegister::TOldClass::staticClassName(),
								ConverterToRegister::TOldClass::ClassVersion,
								ConverterToRegister::TNewClass::ClassVersion,
								*(ConverterToRegister::Create(&parameterizedTraits)));
			}
			{
			typedef nvidia::apex::legacy::ConversionTurbulenceFSAssetParams_0p0_0p1 ConverterToRegister;
			parameterizedTraits.registerConversion(ConverterToRegister::TOldClass::staticClassName(),
								ConverterToRegister::TOldClass::ClassVersion,
								ConverterToRegister::TNewClass::ClassVersion,
								*(ConverterToRegister::Create(&parameterizedTraits)));
			}
			{
			typedef nvidia::apex::legacy::ConversionTurbulenceFSAssetParams_0p1_0p2 ConverterToRegister;
			parameterizedTraits.registerConversion(ConverterToRegister::TOldClass::staticClassName(),
								ConverterToRegister::TOldClass::ClassVersion,
								ConverterToRegister::TNewClass::ClassVersion,
								*(ConverterToRegister::Create(&parameterizedTraits)));
			}
			{
			typedef nvidia::apex::legacy::ConversionTurbulenceFSAssetParams_0p2_0p3 ConverterToRegister;
			parameterizedTraits.registerConversion(ConverterToRegister::TOldClass::staticClassName(),
								ConverterToRegister::TOldClass::ClassVersion,
								ConverterToRegister::TNewClass::ClassVersion,
								*(ConverterToRegister::Create(&parameterizedTraits)));
			}
			{
			typedef nvidia::apex::legacy::ConversionTurbulenceFSAssetParams_0p3_0p4 ConverterToRegister;
			parameterizedTraits.registerConversion(ConverterToRegister::TOldClass::staticClassName(),
								ConverterToRegister::TOldClass::ClassVersion,
								ConverterToRegister::TNewClass::ClassVersion,
								*(ConverterToRegister::Create(&parameterizedTraits)));
			}
			{
			typedef nvidia::apex::legacy::ConversionTurbulenceFSAssetParams_0p4_0p5 ConverterToRegister;
			parameterizedTraits.registerConversion(ConverterToRegister::TOldClass::staticClassName(),
								ConverterToRegister::TOldClass::ClassVersion,
								ConverterToRegister::TNewClass::ClassVersion,
								*(ConverterToRegister::Create(&parameterizedTraits)));
			}
			{
			typedef nvidia::apex::legacy::ConversionTurbulenceFSAssetParams_0p5_0p6 ConverterToRegister;
			parameterizedTraits.registerConversion(ConverterToRegister::TOldClass::staticClassName(),
								ConverterToRegister::TOldClass::ClassVersion,
								ConverterToRegister::TNewClass::ClassVersion,
								*(ConverterToRegister::Create(&parameterizedTraits)));
			}
			{
			typedef nvidia::apex::legacy::ConversionTurbulenceFSAssetParams_0p6_0p7 ConverterToRegister;
			parameterizedTraits.registerConversion(ConverterToRegister::TOldClass::staticClassName(),
								ConverterToRegister::TOldClass::ClassVersion,
								ConverterToRegister::TNewClass::ClassVersion,
								*(ConverterToRegister::Create(&parameterizedTraits)));
			}
			{
			typedef nvidia::apex::legacy::ConversionTurbulenceFSAssetParams_0p7_0p8 ConverterToRegister;
			parameterizedTraits.registerConversion(ConverterToRegister::TOldClass::staticClassName(),
								ConverterToRegister::TOldClass::ClassVersion,
								ConverterToRegister::TNewClass::ClassVersion,
								*(ConverterToRegister::Create(&parameterizedTraits)));
			}
			{
			typedef nvidia::apex::legacy::ConversionTurbulenceFSAssetParams_0p8_0p9 ConverterToRegister;
			parameterizedTraits.registerConversion(ConverterToRegister::TOldClass::staticClassName(),
								ConverterToRegister::TOldClass::ClassVersion,
								ConverterToRegister::TNewClass::ClassVersion,
								*(ConverterToRegister::Create(&parameterizedTraits)));
			}
			{
			typedef nvidia::apex::legacy::ConversionTurbulenceFSAssetParams_0p9_1p0 ConverterToRegister;
			parameterizedTraits.registerConversion(ConverterToRegister::TOldClass::staticClassName(),
								ConverterToRegister::TOldClass::ClassVersion,
								ConverterToRegister::TNewClass::ClassVersion,
								*(ConverterToRegister::Create(&parameterizedTraits)));
			}
			{
			typedef nvidia::apex::legacy::ConversionTurbulenceFSAssetParams_1p0_1p1 ConverterToRegister;
			parameterizedTraits.registerConversion(ConverterToRegister::TOldClass::staticClassName(),
								ConverterToRegister::TOldClass::ClassVersion,
								ConverterToRegister::TNewClass::ClassVersion,
								*(ConverterToRegister::Create(&parameterizedTraits)));
			}
			{
			typedef nvidia::apex::legacy::ConversionTurbulenceFSAssetParams_1p1_1p2 ConverterToRegister;
			parameterizedTraits.registerConversion(ConverterToRegister::TOldClass::staticClassName(),
								ConverterToRegister::TOldClass::ClassVersion,
								ConverterToRegister::TNewClass::ClassVersion,
								*(ConverterToRegister::Create(&parameterizedTraits)));
			}
			{
			typedef nvidia::apex::legacy::ConversionTurbulenceFSAssetParams_1p2_1p3 ConverterToRegister;
			parameterizedTraits.registerConversion(ConverterToRegister::TOldClass::staticClassName(),
								ConverterToRegister::TOldClass::ClassVersion,
								ConverterToRegister::TNewClass::ClassVersion,
								*(ConverterToRegister::Create(&parameterizedTraits)));
			}
			{
			typedef nvidia::apex::legacy::ConversionTurbulenceFSAssetParams_1p3_1p4 ConverterToRegister;
			parameterizedTraits.registerConversion(ConverterToRegister::TOldClass::staticClassName(),
								ConverterToRegister::TOldClass::ClassVersion,
								ConverterToRegister::TNewClass::ClassVersion,
								*(ConverterToRegister::Create(&parameterizedTraits)));
			}
			{
			typedef nvidia::apex::legacy::ConversionTurbulenceFSAssetParams_1p4_1p5 ConverterToRegister;
			parameterizedTraits.registerConversion(ConverterToRegister::TOldClass::staticClassName(),
								ConverterToRegister::TOldClass::ClassVersion,
								ConverterToRegister::TNewClass::ClassVersion,
								*(ConverterToRegister::Create(&parameterizedTraits)));
			}
			{
			typedef nvidia::apex::legacy::ConversionVelocitySourceActorParams_0p0_0p1 ConverterToRegister;
			parameterizedTraits.registerConversion(ConverterToRegister::TOldClass::staticClassName(),
								ConverterToRegister::TOldClass::ClassVersion,
								ConverterToRegister::TNewClass::ClassVersion,
								*(ConverterToRegister::Create(&parameterizedTraits)));
			}
			{
			typedef nvidia::apex::legacy::ConversionVelocitySourcePreviewParams_0p0_0p1 ConverterToRegister;
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
			new nvidia::parameterized::FlameEmitterActorParams_0p0Factory(),
			new nvidia::parameterized::FlameEmitterPreviewParams_0p0Factory(),
			new nvidia::parameterized::HeatSourceActorParams_0p0Factory(),
			new nvidia::parameterized::HeatSourceAssetParams_0p0Factory(),
			new nvidia::parameterized::HeatSourceAssetParams_0p1Factory(),
			new nvidia::parameterized::HeatSourcePreviewParams_0p0Factory(),
			new nvidia::parameterized::SubstanceSourceActorParams_0p0Factory(),
			new nvidia::parameterized::SubstanceSourceAssetParams_0p0Factory(),
			new nvidia::parameterized::TurbulenceFSActorParams_0p0Factory(),
			new nvidia::parameterized::TurbulenceFSAssetParams_0p0Factory(),
			new nvidia::parameterized::TurbulenceFSAssetParams_0p1Factory(),
			new nvidia::parameterized::TurbulenceFSAssetParams_0p2Factory(),
			new nvidia::parameterized::TurbulenceFSAssetParams_0p3Factory(),
			new nvidia::parameterized::TurbulenceFSAssetParams_0p4Factory(),
			new nvidia::parameterized::TurbulenceFSAssetParams_0p5Factory(),
			new nvidia::parameterized::TurbulenceFSAssetParams_0p6Factory(),
			new nvidia::parameterized::TurbulenceFSAssetParams_0p7Factory(),
			new nvidia::parameterized::TurbulenceFSAssetParams_0p8Factory(),
			new nvidia::parameterized::TurbulenceFSAssetParams_0p9Factory(),
			new nvidia::parameterized::TurbulenceFSAssetParams_1p0Factory(),
			new nvidia::parameterized::TurbulenceFSAssetParams_1p1Factory(),
			new nvidia::parameterized::TurbulenceFSAssetParams_1p2Factory(),
			new nvidia::parameterized::TurbulenceFSAssetParams_1p3Factory(),
			new nvidia::parameterized::TurbulenceFSAssetParams_1p4Factory(),
			new nvidia::parameterized::VelocitySourceActorParams_0p0Factory(),
			new nvidia::parameterized::VelocitySourcePreviewParams_0p0Factory(),

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
			typedef nvidia::apex::legacy::ConversionFlameEmitterActorParams_0p0_0p1 ConverterToUnregister;
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
			typedef nvidia::apex::legacy::ConversionFlameEmitterPreviewParams_0p0_0p1 ConverterToUnregister;
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
			typedef nvidia::apex::legacy::ConversionHeatSourceActorParams_0p0_0p1 ConverterToUnregister;
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
			typedef nvidia::apex::legacy::ConversionHeatSourceAssetParams_0p0_0p1 ConverterToUnregister;
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
			typedef nvidia::apex::legacy::ConversionHeatSourceAssetParams_0p1_0p2 ConverterToUnregister;
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
			typedef nvidia::apex::legacy::ConversionHeatSourcePreviewParams_0p0_0p1 ConverterToUnregister;
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
			typedef nvidia::apex::legacy::ConversionSubstanceSourceActorParams_0p0_0p1 ConverterToUnregister;
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
			typedef nvidia::apex::legacy::ConversionSubstanceSourceAssetParams_0p0_0p1 ConverterToUnregister;
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
			typedef nvidia::apex::legacy::ConversionTurbulenceFSActorParams_0p0_0p1 ConverterToUnregister;
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
			typedef nvidia::apex::legacy::ConversionTurbulenceFSAssetParams_0p0_0p1 ConverterToUnregister;
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
			typedef nvidia::apex::legacy::ConversionTurbulenceFSAssetParams_0p1_0p2 ConverterToUnregister;
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
			typedef nvidia::apex::legacy::ConversionTurbulenceFSAssetParams_0p2_0p3 ConverterToUnregister;
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
			typedef nvidia::apex::legacy::ConversionTurbulenceFSAssetParams_0p3_0p4 ConverterToUnregister;
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
			typedef nvidia::apex::legacy::ConversionTurbulenceFSAssetParams_0p4_0p5 ConverterToUnregister;
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
			typedef nvidia::apex::legacy::ConversionTurbulenceFSAssetParams_0p5_0p6 ConverterToUnregister;
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
			typedef nvidia::apex::legacy::ConversionTurbulenceFSAssetParams_0p6_0p7 ConverterToUnregister;
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
			typedef nvidia::apex::legacy::ConversionTurbulenceFSAssetParams_0p7_0p8 ConverterToUnregister;
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
			typedef nvidia::apex::legacy::ConversionTurbulenceFSAssetParams_0p8_0p9 ConverterToUnregister;
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
			typedef nvidia::apex::legacy::ConversionTurbulenceFSAssetParams_0p9_1p0 ConverterToUnregister;
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
			typedef nvidia::apex::legacy::ConversionTurbulenceFSAssetParams_1p0_1p1 ConverterToUnregister;
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
			typedef nvidia::apex::legacy::ConversionTurbulenceFSAssetParams_1p1_1p2 ConverterToUnregister;
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
			typedef nvidia::apex::legacy::ConversionTurbulenceFSAssetParams_1p2_1p3 ConverterToUnregister;
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
			typedef nvidia::apex::legacy::ConversionTurbulenceFSAssetParams_1p3_1p4 ConverterToUnregister;
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
			typedef nvidia::apex::legacy::ConversionTurbulenceFSAssetParams_1p4_1p5 ConverterToUnregister;
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
			typedef nvidia::apex::legacy::ConversionVelocitySourceActorParams_0p0_0p1 ConverterToUnregister;
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
			typedef nvidia::apex::legacy::ConversionVelocitySourcePreviewParams_0p0_0p1 ConverterToUnregister;
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
