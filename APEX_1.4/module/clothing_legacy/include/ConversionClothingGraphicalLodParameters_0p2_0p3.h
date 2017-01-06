/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODULE_CONVERSIONCLOTHINGGRAPHICALLODPARAMETERS_0P2_0P3H_H
#define MODULE_CONVERSIONCLOTHINGGRAPHICALLODPARAMETERS_0P2_0P3H_H

#include "NvParamConversionTemplate.h"
#include "ClothingGraphicalLodParameters_0p2.h"
#include "ClothingGraphicalLodParameters_0p3.h"

namespace nvidia {
namespace apex {
namespace legacy {


typedef NvParameterized::ParamConversionTemplate<nvidia::parameterized::ClothingGraphicalLodParameters_0p2, 
						nvidia::parameterized::ClothingGraphicalLodParameters_0p3, 
						nvidia::parameterized::ClothingGraphicalLodParameters_0p2::ClassVersion, 
						nvidia::parameterized::ClothingGraphicalLodParameters_0p3::ClassVersion>
						ConversionClothingGraphicalLodParameters_0p2_0p3Parent;

class ConversionClothingGraphicalLodParameters_0p2_0p3: public ConversionClothingGraphicalLodParameters_0p2_0p3Parent
{
public:
	static NvParameterized::Conversion* Create(NvParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionClothingGraphicalLodParameters_0p2_0p3));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionClothingGraphicalLodParameters_0p2_0p3)(t) : 0;
	}

protected:
	ConversionClothingGraphicalLodParameters_0p2_0p3(NvParameterized::Traits* t) : ConversionClothingGraphicalLodParameters_0p2_0p3Parent(t) {}

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

		mNewData->renderMeshAssetSorting = 0; // this will trigger a sort on asset load

		return true;
	}
};


}
}
} //nvidia::apex::legacy

#endif
