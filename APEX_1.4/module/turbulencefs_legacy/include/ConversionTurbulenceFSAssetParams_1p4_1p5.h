/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODULE_CONVERSIONTURBULENCEFSASSETPARAMS_1P4_1P5H_H
#define MODULE_CONVERSIONTURBULENCEFSASSETPARAMS_1P4_1P5H_H

#include "NvParamConversionTemplate.h"
#include "TurbulenceFSAssetParams_1p4.h"
#include "TurbulenceFSAssetParams_1p5.h"

namespace nvidia {
namespace apex {
namespace legacy {


typedef NvParameterized::ParamConversionTemplate<nvidia::parameterized::TurbulenceFSAssetParams_1p4, 
						nvidia::parameterized::TurbulenceFSAssetParams_1p5, 
						nvidia::parameterized::TurbulenceFSAssetParams_1p4::ClassVersion, 
						nvidia::parameterized::TurbulenceFSAssetParams_1p5::ClassVersion>
						ConversionTurbulenceFSAssetParams_1p4_1p5Parent;

class ConversionTurbulenceFSAssetParams_1p4_1p5: public ConversionTurbulenceFSAssetParams_1p4_1p5Parent
{
public:
	static NvParameterized::Conversion* Create(NvParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionTurbulenceFSAssetParams_1p4_1p5));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionTurbulenceFSAssetParams_1p4_1p5)(t) : 0;
	}

protected:
	ConversionTurbulenceFSAssetParams_1p4_1p5(NvParameterized::Traits* t) : ConversionTurbulenceFSAssetParams_1p4_1p5Parent(t) {}

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
		mNewData->gridXSize = mLegacyData->gridXRange.max;
		mNewData->gridYSize = mLegacyData->gridYRange.max;
		mNewData->gridZSize = mLegacyData->gridZRange.max;
		mNewData->updatesPerFrame = mLegacyData->updatesPerFrameRange.max;

		return true;
	}
};


}
}
} //nvidia::apex::legacy

#endif
