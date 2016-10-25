/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef MODULE_MODULEBASICFSLEGACYREGISTRATIONH_H
#define MODULE_MODULEBASICFSLEGACYREGISTRATIONH_H

#include "PsAllocator.h"
#include "NvRegistrationsForTraitsBase.h"
#include "nvparameterized/NvParameterizedTraits.h"
#include "PxAssert.h"
#include <stdint.h>

// INCLUDE GENERATED FACTORIES
#include "AttractorFSActorParams_0p0.h"
#include "AttractorFSAssetParams_0p0.h"
#include "JetFSActorParams_0p0.h"
#include "JetFSAssetParams_0p0.h"
#include "NoiseFSActorParams_0p0.h"
#include "NoiseFSAssetParams_0p0.h"
#include "NoiseFSAssetParams_0p1.h"
#include "VortexFSActorParams_0p0.h"
#include "VortexFSAssetParams_0p0.h"
#include "VortexFSAssetParams_0p1.h"
#include "WindFSActorParams_0p0.h"
#include "WindFSAssetParams_0p0.h"
#include "AttractorFSActorParams_0p1.h"
#include "AttractorFSAssetParams_0p1.h"
#include "AttractorFSPreviewParams_0p0.h"
#include "BasicFSDebugRenderParams_0p0.h"
#include "JetFSActorParams_0p1.h"
#include "JetFSAssetParams_0p1.h"
#include "JetFSPreviewParams_0p0.h"
#include "BasicFSModuleParameters_0p0.h"
#include "NoiseFSActorParams_0p1.h"
#include "NoiseFSAssetParams_0p2.h"
#include "NoiseFSPreviewParams_0p0.h"
#include "VortexFSActorParams_0p1.h"
#include "VortexFSAssetParams_0p2.h"
#include "VortexFSPreviewParams_0p0.h"
#include "WindFSActorParams_0p1.h"
#include "WindFSAssetParams_0p1.h"
#include "WindFSPreviewParams_0p0.h"


// INCLUDE GENERATED CONVERSION
#include "ConversionAttractorFSActorParams_0p0_0p1.h"
#include "ConversionAttractorFSAssetParams_0p0_0p1.h"
#include "ConversionJetFSActorParams_0p0_0p1.h"
#include "ConversionJetFSAssetParams_0p0_0p1.h"
#include "ConversionNoiseFSActorParams_0p0_0p1.h"
#include "ConversionNoiseFSAssetParams_0p0_0p1.h"
#include "ConversionNoiseFSAssetParams_0p1_0p2.h"
#include "ConversionVortexFSActorParams_0p0_0p1.h"
#include "ConversionVortexFSAssetParams_0p0_0p1.h"
#include "ConversionVortexFSAssetParams_0p1_0p2.h"
#include "ConversionWindFSActorParams_0p0_0p1.h"
#include "ConversionWindFSAssetParams_0p0_0p1.h"


// global namespace

class ModuleBasicFSLegacyRegistration : public NvParameterized::RegistrationsForTraitsBase
{
public:
	static void invokeRegistration(NvParameterized::Traits* parameterizedTraits)
	{
		if (parameterizedTraits)
		{
			ModuleBasicFSLegacyRegistration().registerAll(*parameterizedTraits);
		}
	}

	static void invokeUnregistration(NvParameterized::Traits* parameterizedTraits)
	{
		if (parameterizedTraits)
		{
			ModuleBasicFSLegacyRegistration().unregisterAll(*parameterizedTraits);
		}
	}

	void registerAvailableFactories(NvParameterized::Traits& parameterizedTraits)
	{
		::NvParameterized::Factory* factoriesToRegister[] = {
// REGISTER GENERATED FACTORIES
			new nvidia::parameterized::AttractorFSActorParams_0p0Factory(),
			new nvidia::parameterized::AttractorFSAssetParams_0p0Factory(),
			new nvidia::parameterized::JetFSActorParams_0p0Factory(),
			new nvidia::parameterized::JetFSAssetParams_0p0Factory(),
			new nvidia::parameterized::NoiseFSActorParams_0p0Factory(),
			new nvidia::parameterized::NoiseFSAssetParams_0p0Factory(),
			new nvidia::parameterized::NoiseFSAssetParams_0p1Factory(),
			new nvidia::parameterized::VortexFSActorParams_0p0Factory(),
			new nvidia::parameterized::VortexFSAssetParams_0p0Factory(),
			new nvidia::parameterized::VortexFSAssetParams_0p1Factory(),
			new nvidia::parameterized::WindFSActorParams_0p0Factory(),
			new nvidia::parameterized::WindFSAssetParams_0p0Factory(),

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
			typedef nvidia::apex::legacy::ConversionAttractorFSActorParams_0p0_0p1 ConverterToRegister;
			parameterizedTraits.registerConversion(ConverterToRegister::TOldClass::staticClassName(),
								ConverterToRegister::TOldClass::ClassVersion,
								ConverterToRegister::TNewClass::ClassVersion,
								*(ConverterToRegister::Create(&parameterizedTraits)));
			}
			{
			typedef nvidia::apex::legacy::ConversionAttractorFSAssetParams_0p0_0p1 ConverterToRegister;
			parameterizedTraits.registerConversion(ConverterToRegister::TOldClass::staticClassName(),
								ConverterToRegister::TOldClass::ClassVersion,
								ConverterToRegister::TNewClass::ClassVersion,
								*(ConverterToRegister::Create(&parameterizedTraits)));
			}
			{
			typedef nvidia::apex::legacy::ConversionJetFSActorParams_0p0_0p1 ConverterToRegister;
			parameterizedTraits.registerConversion(ConverterToRegister::TOldClass::staticClassName(),
								ConverterToRegister::TOldClass::ClassVersion,
								ConverterToRegister::TNewClass::ClassVersion,
								*(ConverterToRegister::Create(&parameterizedTraits)));
			}
			{
			typedef nvidia::apex::legacy::ConversionJetFSAssetParams_0p0_0p1 ConverterToRegister;
			parameterizedTraits.registerConversion(ConverterToRegister::TOldClass::staticClassName(),
								ConverterToRegister::TOldClass::ClassVersion,
								ConverterToRegister::TNewClass::ClassVersion,
								*(ConverterToRegister::Create(&parameterizedTraits)));
			}
			{
			typedef nvidia::apex::legacy::ConversionNoiseFSActorParams_0p0_0p1 ConverterToRegister;
			parameterizedTraits.registerConversion(ConverterToRegister::TOldClass::staticClassName(),
								ConverterToRegister::TOldClass::ClassVersion,
								ConverterToRegister::TNewClass::ClassVersion,
								*(ConverterToRegister::Create(&parameterizedTraits)));
			}
			{
			typedef nvidia::apex::legacy::ConversionNoiseFSAssetParams_0p0_0p1 ConverterToRegister;
			parameterizedTraits.registerConversion(ConverterToRegister::TOldClass::staticClassName(),
								ConverterToRegister::TOldClass::ClassVersion,
								ConverterToRegister::TNewClass::ClassVersion,
								*(ConverterToRegister::Create(&parameterizedTraits)));
			}
			{
			typedef nvidia::apex::legacy::ConversionNoiseFSAssetParams_0p1_0p2 ConverterToRegister;
			parameterizedTraits.registerConversion(ConverterToRegister::TOldClass::staticClassName(),
								ConverterToRegister::TOldClass::ClassVersion,
								ConverterToRegister::TNewClass::ClassVersion,
								*(ConverterToRegister::Create(&parameterizedTraits)));
			}
			{
			typedef nvidia::apex::legacy::ConversionVortexFSActorParams_0p0_0p1 ConverterToRegister;
			parameterizedTraits.registerConversion(ConverterToRegister::TOldClass::staticClassName(),
								ConverterToRegister::TOldClass::ClassVersion,
								ConverterToRegister::TNewClass::ClassVersion,
								*(ConverterToRegister::Create(&parameterizedTraits)));
			}
			{
			typedef nvidia::apex::legacy::ConversionVortexFSAssetParams_0p0_0p1 ConverterToRegister;
			parameterizedTraits.registerConversion(ConverterToRegister::TOldClass::staticClassName(),
								ConverterToRegister::TOldClass::ClassVersion,
								ConverterToRegister::TNewClass::ClassVersion,
								*(ConverterToRegister::Create(&parameterizedTraits)));
			}
			{
			typedef nvidia::apex::legacy::ConversionVortexFSAssetParams_0p1_0p2 ConverterToRegister;
			parameterizedTraits.registerConversion(ConverterToRegister::TOldClass::staticClassName(),
								ConverterToRegister::TOldClass::ClassVersion,
								ConverterToRegister::TNewClass::ClassVersion,
								*(ConverterToRegister::Create(&parameterizedTraits)));
			}
			{
			typedef nvidia::apex::legacy::ConversionWindFSActorParams_0p0_0p1 ConverterToRegister;
			parameterizedTraits.registerConversion(ConverterToRegister::TOldClass::staticClassName(),
								ConverterToRegister::TOldClass::ClassVersion,
								ConverterToRegister::TNewClass::ClassVersion,
								*(ConverterToRegister::Create(&parameterizedTraits)));
			}
			{
			typedef nvidia::apex::legacy::ConversionWindFSAssetParams_0p0_0p1 ConverterToRegister;
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
			new nvidia::parameterized::AttractorFSActorParams_0p0Factory(),
			new nvidia::parameterized::AttractorFSAssetParams_0p0Factory(),
			new nvidia::parameterized::JetFSActorParams_0p0Factory(),
			new nvidia::parameterized::JetFSAssetParams_0p0Factory(),
			new nvidia::parameterized::NoiseFSActorParams_0p0Factory(),
			new nvidia::parameterized::NoiseFSAssetParams_0p0Factory(),
			new nvidia::parameterized::NoiseFSAssetParams_0p1Factory(),
			new nvidia::parameterized::VortexFSActorParams_0p0Factory(),
			new nvidia::parameterized::VortexFSAssetParams_0p0Factory(),
			new nvidia::parameterized::VortexFSAssetParams_0p1Factory(),
			new nvidia::parameterized::WindFSActorParams_0p0Factory(),
			new nvidia::parameterized::WindFSAssetParams_0p0Factory(),

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
			typedef nvidia::apex::legacy::ConversionAttractorFSActorParams_0p0_0p1 ConverterToUnregister;
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
			typedef nvidia::apex::legacy::ConversionAttractorFSAssetParams_0p0_0p1 ConverterToUnregister;
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
			typedef nvidia::apex::legacy::ConversionJetFSActorParams_0p0_0p1 ConverterToUnregister;
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
			typedef nvidia::apex::legacy::ConversionJetFSAssetParams_0p0_0p1 ConverterToUnregister;
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
			typedef nvidia::apex::legacy::ConversionNoiseFSActorParams_0p0_0p1 ConverterToUnregister;
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
			typedef nvidia::apex::legacy::ConversionNoiseFSAssetParams_0p0_0p1 ConverterToUnregister;
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
			typedef nvidia::apex::legacy::ConversionNoiseFSAssetParams_0p1_0p2 ConverterToUnregister;
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
			typedef nvidia::apex::legacy::ConversionVortexFSActorParams_0p0_0p1 ConverterToUnregister;
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
			typedef nvidia::apex::legacy::ConversionVortexFSAssetParams_0p0_0p1 ConverterToUnregister;
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
			typedef nvidia::apex::legacy::ConversionVortexFSAssetParams_0p1_0p2 ConverterToUnregister;
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
			typedef nvidia::apex::legacy::ConversionWindFSActorParams_0p0_0p1 ConverterToUnregister;
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
			typedef nvidia::apex::legacy::ConversionWindFSAssetParams_0p0_0p1 ConverterToUnregister;
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
