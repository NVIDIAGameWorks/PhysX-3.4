#ifndef MODULE_REGISTRATION_FOR_TRAITS_BASE_H
#define MODULE_REGISTRATION_FOR_TRAITS_BASE_H

#include "nvparameterized/NvParameterizedTraits.h"

namespace NvParameterized
{
	class RegistrationsForTraitsBase
	{
	public:
		virtual void registerAvailableFactories(NvParameterized::Traits& parameterizedTraits) = 0;
		virtual void registerAvailableConverters(NvParameterized::Traits& parameterizedTraits) = 0;
		virtual void registerAll(NvParameterized::Traits& parameterizedTraits)
		{
			registerAvailableFactories(parameterizedTraits);
			registerAvailableConverters(parameterizedTraits);
		}

		virtual void unregisterAvailableFactories(NvParameterized::Traits& parameterizedTraits) = 0;
		virtual void unregisterAvailableConverters(NvParameterized::Traits& parameterizedTraits) = 0;
		virtual void unregisterAll(NvParameterized::Traits& parameterizedTraits)
		{
			unregisterAvailableFactories(parameterizedTraits);
			unregisterAvailableConverters(parameterizedTraits);
		}
	};
}

#endif
