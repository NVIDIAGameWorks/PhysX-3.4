/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODULE_CONVERSIONCLOTHINGPHYSICALMESHPARAMETERS_0P2_0P3H_H
#define MODULE_CONVERSIONCLOTHINGPHYSICALMESHPARAMETERS_0P2_0P3H_H

#include "NvParamConversionTemplate.h"
#include "ClothingPhysicalMeshParameters_0p2.h"
#include "ClothingPhysicalMeshParameters_0p3.h"

namespace nvidia {
namespace apex {
namespace legacy {


typedef NvParameterized::ParamConversionTemplate<nvidia::parameterized::ClothingPhysicalMeshParameters_0p2, 
						nvidia::parameterized::ClothingPhysicalMeshParameters_0p3, 
						nvidia::parameterized::ClothingPhysicalMeshParameters_0p2::ClassVersion, 
						nvidia::parameterized::ClothingPhysicalMeshParameters_0p3::ClassVersion>
						ConversionClothingPhysicalMeshParameters_0p2_0p3Parent;

class ConversionClothingPhysicalMeshParameters_0p2_0p3: public ConversionClothingPhysicalMeshParameters_0p2_0p3Parent
{
public:
	static NvParameterized::Conversion* Create(NvParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionClothingPhysicalMeshParameters_0p2_0p3));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionClothingPhysicalMeshParameters_0p2_0p3)(t) : 0;
	}

protected:
	ConversionClothingPhysicalMeshParameters_0p2_0p3(NvParameterized::Traits* t) : ConversionClothingPhysicalMeshParameters_0p2_0p3Parent(t) {}

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

		return true;
	}
};


}
}
} //nvidia::apex::legacy

#endif
