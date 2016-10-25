/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef MODULE_MODULEFORCEFIELDREGISTRATIONH_H
#define MODULE_MODULEFORCEFIELDREGISTRATIONH_H

#include "PsAllocator.h"
#include "NvRegistrationsForTraitsBase.h"
#include "nvparameterized/NvParameterizedTraits.h"
#include "PxAssert.h"
#include <stdint.h>

// INCLUDE GENERATED FACTORIES
#include "ForceFieldActorParams.h"
#include "ForceFieldAssetParams.h"
#include "ForceFieldAssetPreviewParams.h"
#include "ForceFieldDebugRenderParams.h"
#include "ForceFieldFalloffParams.h"
#include "ForceFieldNoiseParams.h"
#include "GenericForceFieldKernelParams.h"
#include "RadialForceFieldKernelParams.h"
#include "ForceFieldModuleParams.h"


// INCLUDE GENERATED CONVERSION


namespace nvidia {
namespace forcefield {


class ModuleForceFieldRegistration : public NvParameterized::RegistrationsForTraitsBase
{
public:
	static void invokeRegistration(NvParameterized::Traits* parameterizedTraits)
	{
		if (parameterizedTraits)
		{
			ModuleForceFieldRegistration().registerAll(*parameterizedTraits);
		}
	}

	static void invokeUnregistration(NvParameterized::Traits* parameterizedTraits)
	{
		if (parameterizedTraits)
		{
			ModuleForceFieldRegistration().unregisterAll(*parameterizedTraits);
		}
	}

	void registerAvailableFactories(NvParameterized::Traits& parameterizedTraits)
	{
		::NvParameterized::Factory* factoriesToRegister[] = {
// REGISTER GENERATED FACTORIES
			new nvidia::forcefield::ForceFieldActorParamsFactory(),
			new nvidia::forcefield::ForceFieldAssetParamsFactory(),
			new nvidia::forcefield::ForceFieldAssetPreviewParamsFactory(),
			new nvidia::forcefield::ForceFieldDebugRenderParamsFactory(),
			new nvidia::forcefield::ForceFieldFalloffParamsFactory(),
			new nvidia::forcefield::ForceFieldNoiseParamsFactory(),
			new nvidia::forcefield::GenericForceFieldKernelParamsFactory(),
			new nvidia::forcefield::RadialForceFieldKernelParamsFactory(),
			new nvidia::forcefield::ForceFieldModuleParamsFactory(),

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
			new nvidia::forcefield::ForceFieldActorParamsFactory(),
			new nvidia::forcefield::ForceFieldAssetParamsFactory(),
			new nvidia::forcefield::ForceFieldAssetPreviewParamsFactory(),
			new nvidia::forcefield::ForceFieldDebugRenderParamsFactory(),
			new nvidia::forcefield::ForceFieldFalloffParamsFactory(),
			new nvidia::forcefield::ForceFieldNoiseParamsFactory(),
			new nvidia::forcefield::GenericForceFieldKernelParamsFactory(),
			new nvidia::forcefield::RadialForceFieldKernelParamsFactory(),
			new nvidia::forcefield::ForceFieldModuleParamsFactory(),

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
} //nvidia::forcefield

#endif
