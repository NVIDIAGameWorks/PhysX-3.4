/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODULE_CONVERSIONCLOTHINGGRAPHICALLODPARAMETERS_0P1_0P2H_H
#define MODULE_CONVERSIONCLOTHINGGRAPHICALLODPARAMETERS_0P1_0P2H_H

#include "NvParamConversionTemplate.h"
#include "ClothingGraphicalLodParameters_0p1.h"
#include "ClothingGraphicalLodParameters_0p2.h"

namespace nvidia {
namespace apex {
namespace legacy {


typedef NvParameterized::ParamConversionTemplate<nvidia::parameterized::ClothingGraphicalLodParameters_0p1, 
						nvidia::parameterized::ClothingGraphicalLodParameters_0p2, 
						nvidia::parameterized::ClothingGraphicalLodParameters_0p1::ClassVersion, 
						nvidia::parameterized::ClothingGraphicalLodParameters_0p2::ClassVersion>
						ConversionClothingGraphicalLodParameters_0p1_0p2Parent;

class ConversionClothingGraphicalLodParameters_0p1_0p2: public ConversionClothingGraphicalLodParameters_0p1_0p2Parent
{
public:
	static NvParameterized::Conversion* Create(NvParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionClothingGraphicalLodParameters_0p1_0p2));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionClothingGraphicalLodParameters_0p1_0p2)(t) : 0;
	}

protected:
	ConversionClothingGraphicalLodParameters_0p1_0p2(NvParameterized::Traits* t) : ConversionClothingGraphicalLodParameters_0p1_0p2Parent(t) {}

	bool convert()
	{
		//TODO:
		//	Write custom conversion code here using mNewData and mLegacyData members.
		//
		//	Note that
		//		- mNewData was initialized with default values
		//		- same-named/same-typed members were copied from mLegacyData to mNewData
		//		- included references were moved to mNewData
		//			(and updated to preferred versions according to getPreferredVersions)
		//
		//	For more info see the versioning wiki.

		// PH: platform strings must be empty

		return true;
	}
};


}
}
} //nvidia::apex::legacy

#endif
