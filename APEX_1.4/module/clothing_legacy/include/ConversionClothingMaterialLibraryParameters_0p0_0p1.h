/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODULE_CONVERSIONCLOTHINGMATERIALLIBRARYPARAMETERS_0P0_0P1H_H
#define MODULE_CONVERSIONCLOTHINGMATERIALLIBRARYPARAMETERS_0P0_0P1H_H

#include "NvParamConversionTemplate.h"
#include "ClothingMaterialLibraryParameters_0p0.h"
#include "ClothingMaterialLibraryParameters_0p1.h"

namespace nvidia {
namespace apex {
namespace legacy {


typedef NvParameterized::ParamConversionTemplate<nvidia::parameterized::ClothingMaterialLibraryParameters_0p0, 
						nvidia::parameterized::ClothingMaterialLibraryParameters_0p1, 
						nvidia::parameterized::ClothingMaterialLibraryParameters_0p0::ClassVersion, 
						nvidia::parameterized::ClothingMaterialLibraryParameters_0p1::ClassVersion>
						ConversionClothingMaterialLibraryParameters_0p0_0p1Parent;

class ConversionClothingMaterialLibraryParameters_0p0_0p1: public ConversionClothingMaterialLibraryParameters_0p0_0p1Parent
{
public:
	static NvParameterized::Conversion* Create(NvParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionClothingMaterialLibraryParameters_0p0_0p1));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionClothingMaterialLibraryParameters_0p0_0p1)(t) : 0;
	}

protected:
	ConversionClothingMaterialLibraryParameters_0p0_0p1(NvParameterized::Traits* t) : ConversionClothingMaterialLibraryParameters_0p0_0p1Parent(t) {}

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
		//		- mNewData was initialized with default values
		//		- same-named/same-typed members were copied from mLegacyData to mNewData
		//		- included references were moved to mNewData
		//			(and updated to preferred versions according to getPreferredVersions)
		//
		//	For more info see the versioning wiki.

		for (int32_t i = 0; i < mNewData->materials.arraySizes[0]; i++)
		{
			mNewData->materials.buf[i].compression.limit = 1.0f;
			mNewData->materials.buf[i].compression.stiffness = 0.5f;
		}

		return true;
	}
};


}
}
} //nvidia::apex::legacy

#endif
