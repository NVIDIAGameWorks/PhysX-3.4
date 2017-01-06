/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODULE_CONVERSIONCLOTHINGASSETPARAMETERS_0P13_0P14H_H
#define MODULE_CONVERSIONCLOTHINGASSETPARAMETERS_0P13_0P14H_H

#include "NvParamConversionTemplate.h"
#include "ClothingAssetParameters_0p13.h"
#include "ClothingAssetParameters_0p14.h"

namespace nvidia {
namespace apex {
namespace legacy {


typedef NvParameterized::ParamConversionTemplate<nvidia::parameterized::ClothingAssetParameters_0p13, 
						nvidia::parameterized::ClothingAssetParameters_0p14, 
						nvidia::parameterized::ClothingAssetParameters_0p13::ClassVersion, 
						nvidia::parameterized::ClothingAssetParameters_0p14::ClassVersion>
						ConversionClothingAssetParameters_0p13_0p14Parent;

class ConversionClothingAssetParameters_0p13_0p14: public ConversionClothingAssetParameters_0p13_0p14Parent
{
public:
	static NvParameterized::Conversion* Create(NvParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionClothingAssetParameters_0p13_0p14));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionClothingAssetParameters_0p13_0p14)(t) : 0;
	}

protected:
	ConversionClothingAssetParameters_0p13_0p14(NvParameterized::Traits* t) : ConversionClothingAssetParameters_0p13_0p14Parent(t) {}

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
		
		// look up the vertex indices for the skinClothMap update
		for (int32_t i = 0; i < mLegacyData->bones.arraySizes[0]; ++i)
		{
			mNewData->bones.buf[i].bindPose.column0 = PxVec4(mLegacyData->bones.buf[i].bindPose[0], mLegacyData->bones.buf[i].bindPose[1], mLegacyData->bones.buf[i].bindPose[2], 0);
			mNewData->bones.buf[i].bindPose.column1 = PxVec4(mLegacyData->bones.buf[i].bindPose[3], mLegacyData->bones.buf[i].bindPose[4], mLegacyData->bones.buf[i].bindPose[5], 0);
			mNewData->bones.buf[i].bindPose.column2 = PxVec4(mLegacyData->bones.buf[i].bindPose[6], mLegacyData->bones.buf[i].bindPose[7], mLegacyData->bones.buf[i].bindPose[8], 0);
			mNewData->bones.buf[i].bindPose.column3 = PxVec4(mLegacyData->bones.buf[i].bindPose[9], mLegacyData->bones.buf[i].bindPose[10], mLegacyData->bones.buf[i].bindPose[11], 1);
		}
		for (int32_t i = 0; i < mLegacyData->boneActors.arraySizes[0]; ++i)
		{
			mNewData->boneActors.buf[i].localPose.column0 = PxVec4(mLegacyData->boneActors.buf[i].localPose[0], mLegacyData->boneActors.buf[i].localPose[1], mLegacyData->boneActors.buf[i].localPose[2], 0);
			mNewData->boneActors.buf[i].localPose.column1 = PxVec4(mLegacyData->boneActors.buf[i].localPose[3], mLegacyData->boneActors.buf[i].localPose[4], mLegacyData->boneActors.buf[i].localPose[5], 0);
			mNewData->boneActors.buf[i].localPose.column2 = PxVec4(mLegacyData->boneActors.buf[i].localPose[6], mLegacyData->boneActors.buf[i].localPose[7], mLegacyData->boneActors.buf[i].localPose[8], 0);
			mNewData->boneActors.buf[i].localPose.column3 = PxVec4(mLegacyData->boneActors.buf[i].localPose[9], mLegacyData->boneActors.buf[i].localPose[10], mLegacyData->boneActors.buf[i].localPose[11], 1);
		}
		return true;
	}
};


}
}
} //nvidia::apex::legacy

#endif
