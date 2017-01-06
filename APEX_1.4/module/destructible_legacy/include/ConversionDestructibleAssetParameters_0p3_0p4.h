/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODULE_CONVERSIONDESTRUCTIBLEASSETPARAMETERS_0P3_0P4H_H
#define MODULE_CONVERSIONDESTRUCTIBLEASSETPARAMETERS_0P3_0P4H_H

#include "NvParamConversionTemplate.h"
#include "DestructibleAssetParameters_0p3.h"
#include "DestructibleAssetParameters_0p4.h"

namespace nvidia {
namespace apex {
namespace legacy {


typedef NvParameterized::ParamConversionTemplate<nvidia::parameterized::DestructibleAssetParameters_0p3, 
						nvidia::parameterized::DestructibleAssetParameters_0p4, 
						nvidia::parameterized::DestructibleAssetParameters_0p3::ClassVersion, 
						nvidia::parameterized::DestructibleAssetParameters_0p4::ClassVersion>
						ConversionDestructibleAssetParameters_0p3_0p4Parent;

class ConversionDestructibleAssetParameters_0p3_0p4: public ConversionDestructibleAssetParameters_0p3_0p4Parent
{
public:
	static NvParameterized::Conversion* Create(NvParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionDestructibleAssetParameters_0p3_0p4));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionDestructibleAssetParameters_0p3_0p4)(t) : 0;
	}

protected:
	ConversionDestructibleAssetParameters_0p3_0p4(NvParameterized::Traits* t) : ConversionDestructibleAssetParameters_0p3_0p4Parent(t) {}

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
		NvParameterized::Handle handle(*mNewData);
		mNewData->getParameterHandle("chunkConvexHullStartIndices", handle);
		mNewData->resizeArray(handle, mNewData->chunks.arraySizes[0] + 1);
		for (int32_t i = 0; i <= mNewData->chunks.arraySizes[0]; ++i)
		{
			mNewData->chunkConvexHullStartIndices.buf[i] = (uint32_t)i;
		}

		return true;
	}
};


}
}
} //nvidia::apex::legacy

#endif
