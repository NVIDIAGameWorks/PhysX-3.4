/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODULE_CONVERSIONDESTRUCTIBLEASSETPARAMETERS_0P15_0P16H_H
#define MODULE_CONVERSIONDESTRUCTIBLEASSETPARAMETERS_0P15_0P16H_H

#include "NvParamConversionTemplate.h"
#include "DestructibleAssetParameters_0p15.h"
#include "DestructibleAssetParameters_0p16.h"

namespace nvidia {
namespace apex {
namespace legacy {


typedef NvParameterized::ParamConversionTemplate<nvidia::parameterized::DestructibleAssetParameters_0p15, 
						nvidia::parameterized::DestructibleAssetParameters_0p16, 
						nvidia::parameterized::DestructibleAssetParameters_0p15::ClassVersion, 
						nvidia::parameterized::DestructibleAssetParameters_0p16::ClassVersion>
						ConversionDestructibleAssetParameters_0p15_0p16Parent;

class ConversionDestructibleAssetParameters_0p15_0p16: public ConversionDestructibleAssetParameters_0p15_0p16Parent
{
public:
	static NvParameterized::Conversion* Create(NvParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionDestructibleAssetParameters_0p15_0p16));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionDestructibleAssetParameters_0p15_0p16)(t) : 0;
	}

protected:
	ConversionDestructibleAssetParameters_0p15_0p16(NvParameterized::Traits* t) : ConversionDestructibleAssetParameters_0p15_0p16Parent(t) {}

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
		// Convert default behavior group's damage spread function parameters
		mNewData->defaultBehaviorGroup.damageSpread.minimumRadius = mLegacyData->defaultBehaviorGroup.minimumDamageRadius;
		mNewData->defaultBehaviorGroup.damageSpread.radiusMultiplier = mLegacyData->defaultBehaviorGroup.damageRadiusMultiplier;
		mNewData->defaultBehaviorGroup.damageSpread.falloffExponent = mLegacyData->defaultBehaviorGroup.damageFalloffExponent;

		// Convert user-defined behavior group's damage spread function parameters
		const int32_t behaviorGroupCount = mLegacyData->behaviorGroups.arraySizes[0];
		PX_ASSERT(mNewData->behaviorGroups.arraySizes[0] == behaviorGroupCount);
		for (int32_t i = 0; i < physx::PxMin(behaviorGroupCount, mNewData->behaviorGroups.arraySizes[0]); ++i)
		{
			mNewData->behaviorGroups.buf[i].damageSpread.minimumRadius = mLegacyData->behaviorGroups.buf[i].minimumDamageRadius;
			mNewData->behaviorGroups.buf[i].damageSpread.radiusMultiplier = mLegacyData->behaviorGroups.buf[i].damageRadiusMultiplier;
			mNewData->behaviorGroups.buf[i].damageSpread.falloffExponent = mLegacyData->behaviorGroups.buf[i].damageFalloffExponent;
		}

		return true;
	}
};


}
}
} //nvidia::apex::legacy

#endif
