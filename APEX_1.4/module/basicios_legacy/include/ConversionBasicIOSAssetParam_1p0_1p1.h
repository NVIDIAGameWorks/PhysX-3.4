/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODULE_CONVERSIONBASICIOSASSETPARAM_1P0_1P1H_H
#define MODULE_CONVERSIONBASICIOSASSETPARAM_1P0_1P1H_H

#include "NvParamConversionTemplate.h"
#include "BasicIOSAssetParam_1p0.h"
#include "BasicIOSAssetParam_1p1.h"

namespace nvidia {
namespace apex {
namespace legacy {


typedef NvParameterized::ParamConversionTemplate<nvidia::parameterized::BasicIOSAssetParam_1p0, 
						nvidia::parameterized::BasicIOSAssetParam_1p1, 
						nvidia::parameterized::BasicIOSAssetParam_1p0::ClassVersion, 
						nvidia::parameterized::BasicIOSAssetParam_1p1::ClassVersion>
						ConversionBasicIOSAssetParam_1p0_1p1Parent;

class ConversionBasicIOSAssetParam_1p0_1p1: public ConversionBasicIOSAssetParam_1p0_1p1Parent
{
public:
	static NvParameterized::Conversion* Create(NvParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionBasicIOSAssetParam_1p0_1p1));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionBasicIOSAssetParam_1p0_1p1)(t) : 0;
	}

protected:
	ConversionBasicIOSAssetParam_1p0_1p1(NvParameterized::Traits* t) : ConversionBasicIOSAssetParam_1p0_1p1Parent(t) {}

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
		mNewData->GridDensity.Enabled = mLegacyData->GridDensityGrid.Enabled;
		// mNewData->GridDensity.Resolution = mLegacyData->GridDensityGrid.GridResolution;
		mNewData->GridDensity.GridSize = mLegacyData->GridDensityGrid.FrustumParams.GridSize;
		mNewData->GridDensity.MaxCellCount = mLegacyData->GridDensityGrid.FrustumParams.GridMaxCellCount;

		// enums are strings, better do it the safe way
		NvParameterized::Handle hEnumNew(*mNewData, "GridDensity.Resolution");
		NvParameterized::Handle hEnumOld(*mLegacyData, "GridDensityGrid.GridResolution");
		PX_ASSERT(hEnumNew.isValid());
		PX_ASSERT(hEnumOld.isValid());

		const NvParameterized::Definition* paramDefOld;
		paramDefOld = hEnumOld.parameterDefinition();
		int32_t index = paramDefOld->enumValIndex(mLegacyData->GridDensityGrid.GridResolution);

		const NvParameterized::Definition* paramDefNew;
		paramDefNew = hEnumNew.parameterDefinition();
		hEnumNew.setParamEnum(paramDefNew->enumVal(index));
		
		return true;
	}
};


}
}
} //nvidia::apex::legacy

#endif
