/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODULE_CONVERSIONDESTRUCTIBLEASSETPARAMETERS_0P19_0P20H_H
#define MODULE_CONVERSIONDESTRUCTIBLEASSETPARAMETERS_0P19_0P20H_H

#include "NvParamConversionTemplate.h"
#include "DestructibleAssetParameters_0p19.h"
#include "DestructibleAssetParameters_0p20.h"

namespace nvidia {
namespace apex {
namespace legacy {


typedef NvParameterized::ParamConversionTemplate<nvidia::parameterized::DestructibleAssetParameters_0p19, 
						nvidia::parameterized::DestructibleAssetParameters_0p20, 
						nvidia::parameterized::DestructibleAssetParameters_0p19::ClassVersion, 
						nvidia::parameterized::DestructibleAssetParameters_0p20::ClassVersion>
						ConversionDestructibleAssetParameters_0p19_0p20Parent;

class ConversionDestructibleAssetParameters_0p19_0p20: public ConversionDestructibleAssetParameters_0p19_0p20Parent
{
public:
	static NvParameterized::Conversion* Create(NvParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionDestructibleAssetParameters_0p19_0p20));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionDestructibleAssetParameters_0p19_0p20)(t) : 0;
	}

protected:
	ConversionDestructibleAssetParameters_0p19_0p20(NvParameterized::Traits* t) : ConversionDestructibleAssetParameters_0p19_0p20Parent(t) {}

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
		// Convert to new behaviorGroup index convention

		const int32_t chunkCount = mNewData->chunks.arraySizes[0];
		for (int32_t chunkIndex = 0; chunkIndex < chunkCount; ++chunkIndex)
		{
			parameterized::DestructibleAssetParameters_0p20NS::Chunk_Type& chunk = mNewData->chunks.buf[chunkIndex];
			if (chunk.parentIndex < chunkCount && chunk.behaviorGroupIndex < 0)
			{
				chunk.behaviorGroupIndex = mNewData->chunks.buf[chunk.parentIndex].behaviorGroupIndex;
			}
		}

		return true;
	}
};


}
}
} //nvidia::apex::legacy

#endif
