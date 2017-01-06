/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODULE_CONVERSIONDESTRUCTIBLEASSETPARAMETERS_0P13_0P14H_H
#define MODULE_CONVERSIONDESTRUCTIBLEASSETPARAMETERS_0P13_0P14H_H

#include "NvParamConversionTemplate.h"
#include "DestructibleAssetParameters_0p13.h"
#include "DestructibleAssetParameters_0p14.h"

namespace nvidia {
namespace apex {
namespace legacy {


typedef NvParameterized::ParamConversionTemplate<nvidia::parameterized::DestructibleAssetParameters_0p13, 
						nvidia::parameterized::DestructibleAssetParameters_0p14, 
						nvidia::parameterized::DestructibleAssetParameters_0p13::ClassVersion, 
						nvidia::parameterized::DestructibleAssetParameters_0p14::ClassVersion>
						ConversionDestructibleAssetParameters_0p13_0p14Parent;

class ConversionDestructibleAssetParameters_0p13_0p14: public ConversionDestructibleAssetParameters_0p13_0p14Parent
{
public:
	static NvParameterized::Conversion* Create(NvParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionDestructibleAssetParameters_0p13_0p14));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionDestructibleAssetParameters_0p13_0p14)(t) : 0;
	}

protected:
	ConversionDestructibleAssetParameters_0p13_0p14(NvParameterized::Traits* t) : ConversionDestructibleAssetParameters_0p13_0p14Parent(t) {}

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
		const int8_t DEFAULT_GROUP = -1;
		mNewData->RTFractureBehaviorGroup = DEFAULT_GROUP;
		mNewData->defaultBehaviorGroup.damageThreshold = mLegacyData->destructibleParameters.damageThreshold;
		mNewData->defaultBehaviorGroup.damageToRadius = mLegacyData->destructibleParameters.damageToRadius;
		mNewData->defaultBehaviorGroup.materialStrength = mLegacyData->destructibleParameters.materialStrength;
		
		PX_ASSERT(mLegacyData->chunks.arraySizes[0] == mNewData->chunks.arraySizes[0]);
		for (int32_t chunkIndex = 0; chunkIndex < mLegacyData->chunks.arraySizes[0]; ++chunkIndex)
		{
			parameterized::DestructibleAssetParameters_0p14NS::Chunk_Type& newChunk = mNewData->chunks.buf[chunkIndex];
			newChunk.behaviorGroupIndex = DEFAULT_GROUP;
		}

		return true;
	}
};


}
}
} //nvidia::apex::legacy

#endif
