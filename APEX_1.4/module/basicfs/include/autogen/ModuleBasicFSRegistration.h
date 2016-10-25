/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef MODULE_MODULEBASICFSREGISTRATIONH_H
#define MODULE_MODULEBASICFSREGISTRATIONH_H

#include "PsAllocator.h"
#include "NvRegistrationsForTraitsBase.h"
#include "nvparameterized/NvParameterizedTraits.h"
#include "PxAssert.h"
#include <stdint.h>

// INCLUDE GENERATED FACTORIES
#include "AttractorFSActorParams.h"
#include "AttractorFSAssetParams.h"
#include "AttractorFSPreviewParams.h"
#include "BasicFSDebugRenderParams.h"
#include "JetFSActorParams.h"
#include "JetFSAssetParams.h"
#include "JetFSPreviewParams.h"
#include "BasicFSModuleParameters.h"
#include "NoiseFSActorParams.h"
#include "NoiseFSAssetParams.h"
#include "NoiseFSPreviewParams.h"
#include "VortexFSActorParams.h"
#include "VortexFSAssetParams.h"
#include "VortexFSPreviewParams.h"
#include "WindFSActorParams.h"
#include "WindFSAssetParams.h"
#include "WindFSPreviewParams.h"


// INCLUDE GENERATED CONVERSION


namespace nvidia {
namespace basicfs {


class ModuleBasicFSRegistration : public NvParameterized::RegistrationsForTraitsBase
{
public:
	static void invokeRegistration(NvParameterized::Traits* parameterizedTraits)
	{
		if (parameterizedTraits)
		{
			ModuleBasicFSRegistration().registerAll(*parameterizedTraits);
		}
	}

	static void invokeUnregistration(NvParameterized::Traits* parameterizedTraits)
	{
		if (parameterizedTraits)
		{
			ModuleBasicFSRegistration().unregisterAll(*parameterizedTraits);
		}
	}

	void registerAvailableFactories(NvParameterized::Traits& parameterizedTraits)
	{
		::NvParameterized::Factory* factoriesToRegister[] = {
// REGISTER GENERATED FACTORIES
			new nvidia::basicfs::AttractorFSActorParamsFactory(),
			new nvidia::basicfs::AttractorFSAssetParamsFactory(),
			new nvidia::basicfs::AttractorFSPreviewParamsFactory(),
			new nvidia::basicfs::BasicFSDebugRenderParamsFactory(),
			new nvidia::basicfs::JetFSActorParamsFactory(),
			new nvidia::basicfs::JetFSAssetParamsFactory(),
			new nvidia::basicfs::JetFSPreviewParamsFactory(),
			new nvidia::basicfs::BasicFSModuleParametersFactory(),
			new nvidia::basicfs::NoiseFSActorParamsFactory(),
			new nvidia::basicfs::NoiseFSAssetParamsFactory(),
			new nvidia::basicfs::NoiseFSPreviewParamsFactory(),
			new nvidia::basicfs::VortexFSActorParamsFactory(),
			new nvidia::basicfs::VortexFSAssetParamsFactory(),
			new nvidia::basicfs::VortexFSPreviewParamsFactory(),
			new nvidia::basicfs::WindFSActorParamsFactory(),
			new nvidia::basicfs::WindFSAssetParamsFactory(),
			new nvidia::basicfs::WindFSPreviewParamsFactory(),

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
			new nvidia::basicfs::AttractorFSActorParamsFactory(),
			new nvidia::basicfs::AttractorFSAssetParamsFactory(),
			new nvidia::basicfs::AttractorFSPreviewParamsFactory(),
			new nvidia::basicfs::BasicFSDebugRenderParamsFactory(),
			new nvidia::basicfs::JetFSActorParamsFactory(),
			new nvidia::basicfs::JetFSAssetParamsFactory(),
			new nvidia::basicfs::JetFSPreviewParamsFactory(),
			new nvidia::basicfs::BasicFSModuleParametersFactory(),
			new nvidia::basicfs::NoiseFSActorParamsFactory(),
			new nvidia::basicfs::NoiseFSAssetParamsFactory(),
			new nvidia::basicfs::NoiseFSPreviewParamsFactory(),
			new nvidia::basicfs::VortexFSActorParamsFactory(),
			new nvidia::basicfs::VortexFSAssetParamsFactory(),
			new nvidia::basicfs::VortexFSPreviewParamsFactory(),
			new nvidia::basicfs::WindFSActorParamsFactory(),
			new nvidia::basicfs::WindFSAssetParamsFactory(),
			new nvidia::basicfs::WindFSPreviewParamsFactory(),

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
} //nvidia::basicfs

#endif
