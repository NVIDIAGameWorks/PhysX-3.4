/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODULE_CONVERSIONCLOTHINGMATERIALLIBRARYPARAMETERS_0P5_0P6H_H
#define MODULE_CONVERSIONCLOTHINGMATERIALLIBRARYPARAMETERS_0P5_0P6H_H

#include "NvParamConversionTemplate.h"
#include "ClothingMaterialLibraryParameters_0p5.h"
#include "ClothingMaterialLibraryParameters_0p6.h"

namespace nvidia {
namespace apex {
namespace legacy {


typedef NvParameterized::ParamConversionTemplate<nvidia::parameterized::ClothingMaterialLibraryParameters_0p5, 
						nvidia::parameterized::ClothingMaterialLibraryParameters_0p6, 
						nvidia::parameterized::ClothingMaterialLibraryParameters_0p5::ClassVersion, 
						nvidia::parameterized::ClothingMaterialLibraryParameters_0p6::ClassVersion>
						ConversionClothingMaterialLibraryParameters_0p5_0p6Parent;

class ConversionClothingMaterialLibraryParameters_0p5_0p6: public ConversionClothingMaterialLibraryParameters_0p5_0p6Parent
{
public:
	static NvParameterized::Conversion* Create(NvParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionClothingMaterialLibraryParameters_0p5_0p6));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionClothingMaterialLibraryParameters_0p5_0p6)(t) : 0;
	}

protected:
	ConversionClothingMaterialLibraryParameters_0p5_0p6(NvParameterized::Traits* t) : ConversionClothingMaterialLibraryParameters_0p5_0p6Parent(t) {}

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
		uint32_t numMaterials = (uint32_t)mNewData->materials.arraySizes[0];
		parameterized::ClothingMaterialLibraryParameters_0p5NS::ClothingMaterial_Type* oldMaterials = mLegacyData->materials.buf;
		parameterized::ClothingMaterialLibraryParameters_0p6NS::ClothingMaterial_Type* newMaterials = mNewData->materials.buf;
		PX_ASSERT((uint32_t)mLegacyData->materials.arraySizes[0] == numMaterials);
		for (uint32_t i = 0; i < numMaterials; ++i)
		{
			parameterized::ClothingMaterialLibraryParameters_0p5NS::ClothingMaterial_Type& oldMat = oldMaterials[i];
			parameterized::ClothingMaterialLibraryParameters_0p6NS::ClothingMaterial_Type& newMat = newMaterials[i];

			if (oldMat.solverIterations > 0)
			{
				newMat.solverFrequency = oldMat.solverIterations * 50.0f;
			}
			else
			{
				newMat.solverFrequency = 20.0f;
			}
		}

		return true;
	}
};


}
}
} //nvidia::apex::legacy

#endif
