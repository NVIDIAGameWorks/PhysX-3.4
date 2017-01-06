/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODULE_CONVERSIONCLOTHINGMATERIALLIBRARYPARAMETERS_0P7_0P8H_H
#define MODULE_CONVERSIONCLOTHINGMATERIALLIBRARYPARAMETERS_0P7_0P8H_H

#include "NvParamConversionTemplate.h"
#include "ClothingMaterialLibraryParameters_0p7.h"
#include "ClothingMaterialLibraryParameters_0p8.h"

namespace nvidia {
namespace apex {
namespace legacy {


typedef NvParameterized::ParamConversionTemplate<nvidia::parameterized::ClothingMaterialLibraryParameters_0p7, 
						nvidia::parameterized::ClothingMaterialLibraryParameters_0p8, 
						nvidia::parameterized::ClothingMaterialLibraryParameters_0p7::ClassVersion, 
						nvidia::parameterized::ClothingMaterialLibraryParameters_0p8::ClassVersion>
						ConversionClothingMaterialLibraryParameters_0p7_0p8Parent;

class ConversionClothingMaterialLibraryParameters_0p7_0p8: public ConversionClothingMaterialLibraryParameters_0p7_0p8Parent
{
public:
	static NvParameterized::Conversion* Create(NvParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionClothingMaterialLibraryParameters_0p7_0p8));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionClothingMaterialLibraryParameters_0p7_0p8)(t) : 0;
	}

protected:
	ConversionClothingMaterialLibraryParameters_0p7_0p8(NvParameterized::Traits* t) : ConversionClothingMaterialLibraryParameters_0p7_0p8Parent(t) {}

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

		for (int32_t i = 0; i < mNewData->materials.arraySizes[0]; i++)
		{
			mNewData->materials.buf[i].massScale = 0.0f;

			if (mNewData->materials.buf[i].solverFrequency < 20)
			{
				if (mNewData->materials.buf[i].solverIterations > 0)
				{
					mNewData->materials.buf[i].solverFrequency = mNewData->materials.buf[i].solverIterations * 50.0f;
				}
				else
				{
					mNewData->materials.buf[i].solverFrequency = 20;
				}
			}
		}

		return true;
	}
};


}
}
} //nvidia::apex::legacy

#endif
