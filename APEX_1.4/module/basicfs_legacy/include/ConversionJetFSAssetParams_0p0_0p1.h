/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODULE_CONVERSIONJETFSASSETPARAMS_0P0_0P1H_H
#define MODULE_CONVERSIONJETFSASSETPARAMS_0P0_0P1H_H

#include "NvParamConversionTemplate.h"
#include "JetFSAssetParams_0p0.h"
#include "JetFSAssetParams_0p1.h"

namespace nvidia {
namespace apex {
namespace legacy {


typedef NvParameterized::ParamConversionTemplate<nvidia::parameterized::JetFSAssetParams_0p0, 
						nvidia::parameterized::JetFSAssetParams_0p1, 
						nvidia::parameterized::JetFSAssetParams_0p0::ClassVersion, 
						nvidia::parameterized::JetFSAssetParams_0p1::ClassVersion>
						ConversionJetFSAssetParams_0p0_0p1Parent;

class ConversionJetFSAssetParams_0p0_0p1: public ConversionJetFSAssetParams_0p0_0p1Parent
{
public:
	static NvParameterized::Conversion* Create(NvParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionJetFSAssetParams_0p0_0p1));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionJetFSAssetParams_0p0_0p1)(t) : 0;
	}

protected:
	ConversionJetFSAssetParams_0p0_0p1(NvParameterized::Traits* t) : ConversionJetFSAssetParams_0p0_0p1Parent(t) {}

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
	
	PX_INLINE void containMin(float& value, float min)
	{
		if (value < min) value = min;
	}

	PX_INLINE void containMinMax(float& value, float min, float max)
	{
		containMin(value, min);
		if (value > max) value = max;
	}

	bool convert()
	{
		//TODO:
		//	Write custom conversion code here using mNewData and mLegacyData members.
		//
		//	Note that
		//		- mNewData has already been initialized with default values
		//		- same-named/same-typed members have already been copied
		//			from mLegacyData to mNewData
		//		- included references were moved to mNewData
		//			(and updated to preferred versions according to getPreferredVersions)
		//
		//	For more info see the versioning wiki.

		// perhaps these values should be retrieved from the hints themselves (in case we decide to change these?)
		containMinMax(mNewData->boundaryFadePercentage, 0.0f, 1.0f);
		containMinMax(mNewData->fieldDirectionDeviationAngle, 0.0f, 180.0f);
		containMin(mNewData->fieldDirectionOscillationPeriod, 0.0f);
		containMin(mNewData->fieldStrength, 0.0f);
		containMinMax(mNewData->fieldStrengthDeviationPercentage, 0.0f, 1.0f);
		containMin(mNewData->fieldStrengthOscillationPeriod, 0.0f);

		return true;
	}
};


}
}
} //nvidia::apex::legacy

#endif
