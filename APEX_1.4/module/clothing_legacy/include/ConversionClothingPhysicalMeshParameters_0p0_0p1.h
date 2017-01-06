/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODULE_CONVERSIONCLOTHINGPHYSICALMESHPARAMETERS_0P0_0P1H_H
#define MODULE_CONVERSIONCLOTHINGPHYSICALMESHPARAMETERS_0P0_0P1H_H

#include "NvParamConversionTemplate.h"
#include "ClothingPhysicalMeshParameters_0p0.h"
#include "ClothingPhysicalMeshParameters_0p1.h"

namespace nvidia {
namespace apex {
namespace legacy {


typedef NvParameterized::ParamConversionTemplate<nvidia::parameterized::ClothingPhysicalMeshParameters_0p0, 
						nvidia::parameterized::ClothingPhysicalMeshParameters_0p1, 
						nvidia::parameterized::ClothingPhysicalMeshParameters_0p0::ClassVersion, 
						nvidia::parameterized::ClothingPhysicalMeshParameters_0p1::ClassVersion>
						ConversionClothingPhysicalMeshParameters_0p0_0p1Parent;

class ConversionClothingPhysicalMeshParameters_0p0_0p1: public ConversionClothingPhysicalMeshParameters_0p0_0p1Parent
{
public:
	static NvParameterized::Conversion* Create(NvParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionClothingPhysicalMeshParameters_0p0_0p1));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionClothingPhysicalMeshParameters_0p0_0p1)(t) : 0;
	}

protected:
	ConversionClothingPhysicalMeshParameters_0p0_0p1(NvParameterized::Traits* t) : ConversionClothingPhysicalMeshParameters_0p0_0p1Parent(t) {}

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

		for (int32_t i = 0; i < mNewData->submeshes.arraySizes[0]; i++)
		{
			mNewData->submeshes.buf[i].nxDeformableMesh = NULL;
		}

		return true;
	}
};


}
}
} //nvidia::apex::legacy

#endif
