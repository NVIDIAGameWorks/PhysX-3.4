/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODULE_CONVERSIONCLOTHINGPHYSICALMESHPARAMETERS_0P9_0P10H_H
#define MODULE_CONVERSIONCLOTHINGPHYSICALMESHPARAMETERS_0P9_0P10H_H

#include "NvParamConversionTemplate.h"
#include "ClothingPhysicalMeshParameters_0p9.h"
#include "ClothingPhysicalMeshParameters_0p10.h"

namespace nvidia {
namespace apex {
namespace legacy {


typedef NvParameterized::ParamConversionTemplate<nvidia::parameterized::ClothingPhysicalMeshParameters_0p9, 
						nvidia::parameterized::ClothingPhysicalMeshParameters_0p10, 
						nvidia::parameterized::ClothingPhysicalMeshParameters_0p9::ClassVersion, 
						nvidia::parameterized::ClothingPhysicalMeshParameters_0p10::ClassVersion>
						ConversionClothingPhysicalMeshParameters_0p9_0p10Parent;

class ConversionClothingPhysicalMeshParameters_0p9_0p10: public ConversionClothingPhysicalMeshParameters_0p9_0p10Parent
{
public:
	static NvParameterized::Conversion* Create(NvParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionClothingPhysicalMeshParameters_0p9_0p10));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionClothingPhysicalMeshParameters_0p9_0p10)(t) : 0;
	}

protected:
	ConversionClothingPhysicalMeshParameters_0p9_0p10(NvParameterized::Traits* t) : ConversionClothingPhysicalMeshParameters_0p9_0p10Parent(t) {}

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

		float meshThickness = mLegacyData->transitionDownThickness;
		NvParameterized::Handle transitionDownHandle(*mNewData, "transitionDown");
		transitionDownHandle.resizeArray(mLegacyData->transitionDownC.arraySizes[0]);
		for (int32_t i = 0; i < mLegacyData->transitionDownC.arraySizes[0]; ++i)
		{
			const parameterized::ClothingPhysicalMeshParameters_0p9NS::SkinClothMapC_Type& mapC = mLegacyData->transitionDownC.buf[i];
			parameterized::ClothingPhysicalMeshParameters_0p10NS::SkinClothMapD_Type& mapD = mNewData->transitionDown.buf[i];

			mapD.vertexBary = mapC.vertexBary;
			mapD.vertexBary.z *= meshThickness;
			mapD.normalBary = mapC.normalBary;
			mapD.normalBary.z *= meshThickness;
			mapD.tangentBary = physx::PxVec3(PX_MAX_F32); // mark tangents as invalid
			mapD.vertexIndexPlusOffset = mapC.vertexIndexPlusOffset;

			// temporarily store face index to update in the ClothingAsset update
			mapD.vertexIndex0 = mapC.faceIndex0;
		}



		meshThickness = mLegacyData->transitionUpThickness;
		NvParameterized::Handle transitionUpHandle(*mNewData, "transitionUp");
		transitionUpHandle.resizeArray(mLegacyData->transitionUpC.arraySizes[0]);
		for (int32_t i = 0; i < mLegacyData->transitionUpC.arraySizes[0]; ++i)
		{
			const parameterized::ClothingPhysicalMeshParameters_0p9NS::SkinClothMapC_Type& mapC = mLegacyData->transitionUpC.buf[i];
			parameterized::ClothingPhysicalMeshParameters_0p10NS::SkinClothMapD_Type& mapD = mNewData->transitionUp.buf[i];

			mapD.vertexBary = mapC.vertexBary;
			mapD.vertexBary.z *= meshThickness;
			mapD.normalBary = mapC.normalBary;
			mapD.normalBary.z *= meshThickness;
			mapD.tangentBary = physx::PxVec3(PX_MAX_F32); // mark tangents as invalid
			mapD.vertexIndexPlusOffset = mapC.vertexIndexPlusOffset;

			// temporarily store face index to update in the ClothingAsset update
			mapD.vertexIndex0 = mapC.faceIndex0;
		}

		return true;
	}
};


}
}
} //nvidia::apex::legacy

#endif
