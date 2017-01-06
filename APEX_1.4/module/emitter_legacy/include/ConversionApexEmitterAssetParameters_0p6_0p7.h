/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODULE_CONVERSIONAPEXEMITTERASSETPARAMETERS_0P6_0P7H_H
#define MODULE_CONVERSIONAPEXEMITTERASSETPARAMETERS_0P6_0P7H_H

#include "NvParamConversionTemplate.h"
#include "ApexEmitterAssetParameters_0p6.h"
#include "ApexEmitterAssetParameters_0p7.h"

namespace nvidia {
namespace apex {
namespace legacy {


typedef NvParameterized::ParamConversionTemplate<nvidia::parameterized::ApexEmitterAssetParameters_0p6, 
						nvidia::parameterized::ApexEmitterAssetParameters_0p7, 
						nvidia::parameterized::ApexEmitterAssetParameters_0p6::ClassVersion, 
						nvidia::parameterized::ApexEmitterAssetParameters_0p7::ClassVersion>
						ConversionApexEmitterAssetParameters_0p6_0p7Parent;

class ConversionApexEmitterAssetParameters_0p6_0p7: public ConversionApexEmitterAssetParameters_0p6_0p7Parent
{
public:
	static NvParameterized::Conversion* Create(NvParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionApexEmitterAssetParameters_0p6_0p7));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionApexEmitterAssetParameters_0p6_0p7)(t) : 0;
	}

protected:
	ConversionApexEmitterAssetParameters_0p6_0p7(NvParameterized::Traits* t) : ConversionApexEmitterAssetParameters_0p6_0p7Parent(t) {}

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
		mNewData->rate = mLegacyData->rateRange.max;
		mNewData->velocityLow = mLegacyData->velocityRange.min;
		mNewData->velocityHigh = mLegacyData->velocityRange.max;
		mNewData->lifetimeLow = mLegacyData->lifetimeRange.min;
		mNewData->lifetimeHigh = mLegacyData->lifetimeRange.max;

		return true;
	}
};


}
}
} //nvidia::apex::legacy

#endif
