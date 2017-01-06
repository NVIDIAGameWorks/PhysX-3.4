/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODULE_CONVERSIONIMPACTOBJECTEVENT_0P0_0P1H_H
#define MODULE_CONVERSIONIMPACTOBJECTEVENT_0P0_0P1H_H

#include "NvParamConversionTemplate.h"
#include "ImpactObjectEvent_0p0.h"
#include "ImpactObjectEvent_0p1.h"

namespace nvidia {
namespace apex {
namespace legacy {


typedef NvParameterized::ParamConversionTemplate<nvidia::parameterized::ImpactObjectEvent_0p0, 
						nvidia::parameterized::ImpactObjectEvent_0p1, 
						nvidia::parameterized::ImpactObjectEvent_0p0::ClassVersion, 
						nvidia::parameterized::ImpactObjectEvent_0p1::ClassVersion>
						ConversionImpactObjectEvent_0p0_0p1Parent;

class ConversionImpactObjectEvent_0p0_0p1: public ConversionImpactObjectEvent_0p0_0p1Parent
{
public:
	static NvParameterized::Conversion* Create(NvParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionImpactObjectEvent_0p0_0p1));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionImpactObjectEvent_0p0_0p1)(t) : 0;
	}

protected:
	ConversionImpactObjectEvent_0p0_0p1(NvParameterized::Traits* t) : ConversionImpactObjectEvent_0p0_0p1Parent(t) {}

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
		mNewData->angleLow = mLegacyData->angleRange.min;
		mNewData->angleHigh = mLegacyData->angleRange.max;
		mNewData->lifeLow = mLegacyData->lifeRange.min;
		mNewData->lifeHigh = mLegacyData->lifeRange.max;
		mNewData->speedLow = mLegacyData->speedRange.min;
		mNewData->speedHigh = mLegacyData->speedRange.max;

		return true;
	}
};


}
}
} //nvidia::apex::legacy

#endif
