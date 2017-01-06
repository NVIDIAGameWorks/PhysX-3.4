/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODULE_CONVERSIONCLOTHINGGRAPHICALLODPARAMETERS_0P3_0P4H_H
#define MODULE_CONVERSIONCLOTHINGGRAPHICALLODPARAMETERS_0P3_0P4H_H

#include "NvParamConversionTemplate.h"
#include "ClothingGraphicalLodParameters_0p3.h"
#include "ClothingGraphicalLodParameters_0p4.h"

namespace nvidia {
namespace apex {
namespace legacy {


typedef NvParameterized::ParamConversionTemplate<nvidia::parameterized::ClothingGraphicalLodParameters_0p3, 
						nvidia::parameterized::ClothingGraphicalLodParameters_0p4, 
						nvidia::parameterized::ClothingGraphicalLodParameters_0p3::ClassVersion, 
						nvidia::parameterized::ClothingGraphicalLodParameters_0p4::ClassVersion>
						ConversionClothingGraphicalLodParameters_0p3_0p4Parent;

class ConversionClothingGraphicalLodParameters_0p3_0p4: public ConversionClothingGraphicalLodParameters_0p3_0p4Parent
{
public:
	static NvParameterized::Conversion* Create(NvParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionClothingGraphicalLodParameters_0p3_0p4));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionClothingGraphicalLodParameters_0p3_0p4)(t) : 0;
	}

protected:
	ConversionClothingGraphicalLodParameters_0p3_0p4(NvParameterized::Traits* t) : ConversionClothingGraphicalLodParameters_0p3_0p4Parent(t) {}

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

		float meshThickness = mLegacyData->skinClothMapThickness;
		NvParameterized::Handle skinClothMap(*mNewData, "skinClothMap");
		skinClothMap.resizeArray(mLegacyData->skinClothMapC.arraySizes[0]);
		for (int32_t i = 0; i < mLegacyData->skinClothMapC.arraySizes[0]; ++i)
		{
			const parameterized::ClothingGraphicalLodParameters_0p3NS::SkinClothMapC_Type& mapC = mLegacyData->skinClothMapC.buf[i];
			parameterized::ClothingGraphicalLodParameters_0p4NS::SkinClothMapD_Type& mapD = mNewData->skinClothMap.buf[i];

			mapD.vertexBary = mapC.vertexBary;
			mapD.vertexBary.z *= meshThickness;
			mapD.normalBary = mapC.normalBary;
			mapD.normalBary.z *= meshThickness;
			mapD.tangentBary = physx::PxVec3(PX_MAX_F32); // mark tangents as invalid
			mapD.vertexIndexPlusOffset = mapC.vertexIndexPlusOffset;
			//PX_ASSERT((uint32_t)i == mapC.vertexIndexPlusOffset);

			// Temporarily store the face index. The ClothingAsset update will look up the actual vertex indices.
			mapD.vertexIndex0 = mapC.faceIndex0;
		}

		return true;
	}
};


}
}
} //nvidia::apex::legacy

#endif
