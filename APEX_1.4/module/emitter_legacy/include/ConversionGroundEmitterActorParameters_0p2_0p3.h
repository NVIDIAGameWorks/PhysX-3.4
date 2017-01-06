/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODULE_CONVERSIONGROUNDEMITTERACTORPARAMETERS_0P2_0P3H_H
#define MODULE_CONVERSIONGROUNDEMITTERACTORPARAMETERS_0P2_0P3H_H

#include "NvParamConversionTemplate.h"
#include "GroundEmitterActorParameters_0p2.h"
#include "GroundEmitterActorParameters_0p3.h"

namespace nvidia {
namespace apex {
namespace legacy {


typedef NvParameterized::ParamConversionTemplate<nvidia::parameterized::GroundEmitterActorParameters_0p2, 
						nvidia::parameterized::GroundEmitterActorParameters_0p3, 
						nvidia::parameterized::GroundEmitterActorParameters_0p2::ClassVersion, 
						nvidia::parameterized::GroundEmitterActorParameters_0p3::ClassVersion>
						ConversionGroundEmitterActorParameters_0p2_0p3Parent;

class ConversionGroundEmitterActorParameters_0p2_0p3: public ConversionGroundEmitterActorParameters_0p2_0p3Parent
{
public:
	static NvParameterized::Conversion* Create(NvParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionGroundEmitterActorParameters_0p2_0p3));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionGroundEmitterActorParameters_0p2_0p3)(t) : 0;
	}

protected:
	ConversionGroundEmitterActorParameters_0p2_0p3(NvParameterized::Traits* t) : ConversionGroundEmitterActorParameters_0p2_0p3Parent(t) {}

	const NvParameterized::PrefVer* getPreferredVersions() const
	{
		static NvParameterized::PrefVer prefVers[] =
		{
			//TODO:
			//	Add your preferred versions for included references here.
			//	Entry format is
			//		{ (const char*)longName, (uint32_t)preferredVersion }

			{ 0, 0 } // Terminator (do not remove!)
		};

		return prefVers;
	}

	bool convert()
	{
		mNewData->density = mLegacyData->densityRange.max;

		return true;
	}
};


}
}
} //nvidia::apex::legacy

#endif
