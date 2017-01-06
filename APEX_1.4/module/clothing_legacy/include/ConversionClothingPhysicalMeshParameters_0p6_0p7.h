/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODULE_CONVERSIONCLOTHINGPHYSICALMESHPARAMETERS_0P6_0P7H_H
#define MODULE_CONVERSIONCLOTHINGPHYSICALMESHPARAMETERS_0P6_0P7H_H

#include "NvParamConversionTemplate.h"
#include "ClothingPhysicalMeshParameters_0p6.h"
#include "ClothingPhysicalMeshParameters_0p7.h"

namespace nvidia {
namespace apex {
namespace legacy {


typedef NvParameterized::ParamConversionTemplate<nvidia::parameterized::ClothingPhysicalMeshParameters_0p6, 
						nvidia::parameterized::ClothingPhysicalMeshParameters_0p7, 
						nvidia::parameterized::ClothingPhysicalMeshParameters_0p6::ClassVersion, 
						nvidia::parameterized::ClothingPhysicalMeshParameters_0p7::ClassVersion>
						ConversionClothingPhysicalMeshParameters_0p6_0p7Parent;

class ConversionClothingPhysicalMeshParameters_0p6_0p7: public ConversionClothingPhysicalMeshParameters_0p6_0p7Parent
{
public:
	static NvParameterized::Conversion* Create(NvParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionClothingPhysicalMeshParameters_0p6_0p7));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionClothingPhysicalMeshParameters_0p6_0p7)(t) : 0;
	}

protected:
	ConversionClothingPhysicalMeshParameters_0p6_0p7(NvParameterized::Traits* t) : ConversionClothingPhysicalMeshParameters_0p6_0p7Parent(t) {}

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
