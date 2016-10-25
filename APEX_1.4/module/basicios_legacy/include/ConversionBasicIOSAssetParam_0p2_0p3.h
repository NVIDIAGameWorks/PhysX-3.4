/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODULE_CONVERSIONBASICIOSASSETPARAM_0P2_0P3H_H
#define MODULE_CONVERSIONBASICIOSASSETPARAM_0P2_0P3H_H

#include "NvParamConversionTemplate.h"
#include "BasicIOSAssetParam_0p2.h"
#include "BasicIOSAssetParam_0p3.h"

namespace nvidia {
namespace apex {
namespace legacy {


typedef NvParameterized::ParamConversionTemplate<nvidia::parameterized::BasicIOSAssetParam_0p2, 
						nvidia::parameterized::BasicIOSAssetParam_0p3, 
						nvidia::parameterized::BasicIOSAssetParam_0p2::ClassVersion, 
						nvidia::parameterized::BasicIOSAssetParam_0p3::ClassVersion>
						ConversionBasicIOSAssetParam_0p2_0p3Parent;

class ConversionBasicIOSAssetParam_0p2_0p3: public ConversionBasicIOSAssetParam_0p2_0p3Parent
{
public:
	static NvParameterized::Conversion* Create(NvParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionBasicIOSAssetParam_0p2_0p3));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionBasicIOSAssetParam_0p2_0p3)(t) : 0;
	}

protected:
	ConversionBasicIOSAssetParam_0p2_0p3(NvParameterized::Traits* t) : ConversionBasicIOSAssetParam_0p2_0p3Parent(t) {}

	bool convert()
	{
		// Inherit median from legacy asset
		mNewData->particleMass.center = mLegacyData->particleMass;

		return true;
	}
};


}
}
} //nvidia::apex::legacy

#endif
