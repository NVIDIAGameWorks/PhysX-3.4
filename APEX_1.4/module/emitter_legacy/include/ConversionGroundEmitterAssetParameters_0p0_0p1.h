/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODULE_CONVERSIONGROUNDEMITTERASSETPARAMETERS_0P0_0P1H_H
#define MODULE_CONVERSIONGROUNDEMITTERASSETPARAMETERS_0P0_0P1H_H

#include "NvParamConversionTemplate.h"
#include "GroundEmitterAssetParameters_0p0.h"
#include "GroundEmitterAssetParameters_0p1.h"

namespace nvidia {
namespace apex {
namespace legacy {


typedef NvParameterized::ParamConversionTemplate<nvidia::parameterized::GroundEmitterAssetParameters_0p0, 
						nvidia::parameterized::GroundEmitterAssetParameters_0p1, 
						nvidia::parameterized::GroundEmitterAssetParameters_0p0::ClassVersion, 
						nvidia::parameterized::GroundEmitterAssetParameters_0p1::ClassVersion>
						ConversionGroundEmitterAssetParameters_0p0_0p1Parent;

class ConversionGroundEmitterAssetParameters_0p0_0p1: public ConversionGroundEmitterAssetParameters_0p0_0p1Parent
{
public:
	static NvParameterized::Conversion* Create(NvParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionGroundEmitterAssetParameters_0p0_0p1));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionGroundEmitterAssetParameters_0p0_0p1)(t) : 0;
	}

protected:
	ConversionGroundEmitterAssetParameters_0p0_0p1(NvParameterized::Traits* t) : ConversionGroundEmitterAssetParameters_0p0_0p1Parent(t) {}

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
		mNewData->lifetimeLow = mLegacyData->lifetimeRange.min;
		mNewData->lifetimeHigh = mLegacyData->lifetimeRange.max;
		mNewData->velocityLow = mLegacyData->velocityRange.min;
		mNewData->velocityHigh = mLegacyData->velocityRange.max;		

		return true;
	}
};


}
}
} //nvidia::apex::legacy

#endif
