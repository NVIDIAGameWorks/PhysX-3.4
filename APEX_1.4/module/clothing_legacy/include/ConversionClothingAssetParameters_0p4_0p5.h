/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODULE_CONVERSIONCLOTHINGASSETPARAMETERS_0P4_0P5H_H
#define MODULE_CONVERSIONCLOTHINGASSETPARAMETERS_0P4_0P5H_H

#include "NvParamConversionTemplate.h"
#include "ClothingAssetParameters_0p4.h"
#include "ClothingAssetParameters_0p5.h"

namespace nvidia {
namespace apex {
namespace legacy {


typedef NvParameterized::ParamConversionTemplate<nvidia::parameterized::ClothingAssetParameters_0p4, 
						nvidia::parameterized::ClothingAssetParameters_0p5, 
						nvidia::parameterized::ClothingAssetParameters_0p4::ClassVersion, 
						nvidia::parameterized::ClothingAssetParameters_0p5::ClassVersion>
						ConversionClothingAssetParameters_0p4_0p5Parent;

class ConversionClothingAssetParameters_0p4_0p5: public ConversionClothingAssetParameters_0p4_0p5Parent
{
public:
	static NvParameterized::Conversion* Create(NvParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionClothingAssetParameters_0p4_0p5));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionClothingAssetParameters_0p4_0p5)(t) : 0;
	}

protected:
	ConversionClothingAssetParameters_0p4_0p5(NvParameterized::Traits* t) : ConversionClothingAssetParameters_0p4_0p5Parent(t) {}

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


		mNewData->rootBoneIndex = 0;

		uint32_t numBones = (uint32_t)mLegacyData->bones.arraySizes[0];
		uint32_t minDepth = numBones;
		for (uint32_t i = 0; i < mNewData->bonesReferenced; i++)
		{
			uint32_t depth = 0;
			int32_t parent = mLegacyData->bones.buf[i].parentIndex;
			while (parent != -1 && depth < numBones)
			{
				parent = mLegacyData->bones.buf[parent].parentIndex;
				depth++;
			}

			if (depth < minDepth)
			{
				minDepth = depth;
				mNewData->rootBoneIndex = i;
			}
		}

		return true;
	}
};


}
}
} //nvidia::apex::legacy

#endif
