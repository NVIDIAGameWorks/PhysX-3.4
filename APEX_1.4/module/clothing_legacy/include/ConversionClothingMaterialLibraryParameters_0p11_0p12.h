/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODULE_CONVERSIONCLOTHINGMATERIALLIBRARYPARAMETERS_0P11_0P12H_H
#define MODULE_CONVERSIONCLOTHINGMATERIALLIBRARYPARAMETERS_0P11_0P12H_H

#include "NvParamConversionTemplate.h"
#include "ClothingMaterialLibraryParameters_0p11.h"
#include "ClothingMaterialLibraryParameters_0p12.h"

namespace nvidia {
namespace apex {
namespace legacy {


typedef NvParameterized::ParamConversionTemplate<nvidia::parameterized::ClothingMaterialLibraryParameters_0p11, 
						nvidia::parameterized::ClothingMaterialLibraryParameters_0p12, 
						nvidia::parameterized::ClothingMaterialLibraryParameters_0p11::ClassVersion, 
						nvidia::parameterized::ClothingMaterialLibraryParameters_0p12::ClassVersion>
						ConversionClothingMaterialLibraryParameters_0p11_0p12Parent;

class ConversionClothingMaterialLibraryParameters_0p11_0p12: public ConversionClothingMaterialLibraryParameters_0p11_0p12Parent
{
public:
	static NvParameterized::Conversion* Create(NvParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionClothingMaterialLibraryParameters_0p11_0p12));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionClothingMaterialLibraryParameters_0p11_0p12)(t) : 0;
	}

protected:
	ConversionClothingMaterialLibraryParameters_0p11_0p12(NvParameterized::Traits* t) : ConversionClothingMaterialLibraryParameters_0p11_0p12Parent(t) {}

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

	void updateStiffnessScaling(parameterized::ClothingMaterialLibraryParameters_0p12NS::StiffnessScaling_Type& newScaling, 
								const parameterized::ClothingMaterialLibraryParameters_0p11NS::StiffnessScaling_Type& oldScaling)
	{
		if (oldScaling.range < 1.0f)
		{
			newScaling.compressionRange = oldScaling.range;
			newScaling.stretchRange = 1.0f;
		}
		else
		{
			newScaling.compressionRange = 1.0f;
			newScaling.stretchRange = oldScaling.range;
		}
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

		if (mNewData->materials.arraySizes[0] != mLegacyData->materials.arraySizes[0])
			return false;

		for (int32_t i = 0; i < mNewData->materials.arraySizes[0]; ++i)
		{
			updateStiffnessScaling(mNewData->materials.buf[i].verticalStiffnessScaling, mLegacyData->materials.buf[i].verticalStiffnessScaling);
			updateStiffnessScaling(mNewData->materials.buf[i].horizontalStiffnessScaling, mLegacyData->materials.buf[i].horizontalStiffnessScaling);
			updateStiffnessScaling(mNewData->materials.buf[i].bendingStiffnessScaling, mLegacyData->materials.buf[i].bendingStiffnessScaling);
			updateStiffnessScaling(mNewData->materials.buf[i].shearingStiffnessScaling, mLegacyData->materials.buf[i].shearingStiffnessScaling);
		}

		return true;
	}
};


}
}
} //nvidia::apex::legacy

#endif
