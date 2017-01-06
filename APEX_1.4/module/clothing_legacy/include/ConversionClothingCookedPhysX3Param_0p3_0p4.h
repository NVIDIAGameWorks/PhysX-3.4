/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODULE_CONVERSIONCLOTHINGCOOKEDPHYSX3PARAM_0P3_0P4H_H
#define MODULE_CONVERSIONCLOTHINGCOOKEDPHYSX3PARAM_0P3_0P4H_H

#include "NvParamConversionTemplate.h"
#include "ClothingCookedPhysX3Param_0p3.h"
#include "ClothingCookedPhysX3Param_0p4.h"

namespace nvidia {
namespace apex {
namespace legacy {


typedef NvParameterized::ParamConversionTemplate<nvidia::parameterized::ClothingCookedPhysX3Param_0p3, 
						nvidia::parameterized::ClothingCookedPhysX3Param_0p4, 
						nvidia::parameterized::ClothingCookedPhysX3Param_0p3::ClassVersion, 
						nvidia::parameterized::ClothingCookedPhysX3Param_0p4::ClassVersion>
						ConversionClothingCookedPhysX3Param_0p3_0p4Parent;

class ConversionClothingCookedPhysX3Param_0p3_0p4: public ConversionClothingCookedPhysX3Param_0p3_0p4Parent
{
public:
	static NvParameterized::Conversion* Create(NvParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionClothingCookedPhysX3Param_0p3_0p4));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionClothingCookedPhysX3Param_0p3_0p4)(t) : 0;
	}

protected:
	ConversionClothingCookedPhysX3Param_0p3_0p4(NvParameterized::Traits* t) : ConversionClothingCookedPhysX3Param_0p3_0p4Parent(t) {}

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

		return true;
	}
};


}
}
} //nvidia::apex::legacy

#endif
