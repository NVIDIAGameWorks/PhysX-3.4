/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODULE_CONVERSIONBASICIOSASSETPARAM_0P0_0P1H_H
#define MODULE_CONVERSIONBASICIOSASSETPARAM_0P0_0P1H_H

#include "NvParamConversionTemplate.h"
#include "BasicIOSAssetParam_0p0.h"
#include "BasicIOSAssetParam_0p1.h"

namespace nvidia {
namespace apex {
namespace legacy {


typedef NvParameterized::ParamConversionTemplate<nvidia::parameterized::BasicIOSAssetParam_0p0, 
						nvidia::parameterized::BasicIOSAssetParam_0p1, 
						nvidia::parameterized::BasicIOSAssetParam_0p0::ClassVersion, 
						nvidia::parameterized::BasicIOSAssetParam_0p1::ClassVersion>
						ConversionBasicIOSAssetParam_0p0_0p1Parent;

class ConversionBasicIOSAssetParam_0p0_0p1: public ConversionBasicIOSAssetParam_0p0_0p1Parent
{
public:
	static NvParameterized::Conversion* Create(NvParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionBasicIOSAssetParam_0p0_0p1));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionBasicIOSAssetParam_0p0_0p1)(t) : 0;
	}

protected:
	ConversionBasicIOSAssetParam_0p0_0p1(NvParameterized::Traits* t) : ConversionBasicIOSAssetParam_0p0_0p1Parent(t) {}

	bool convert()
	{
		// just take the default values (scaleSceneGravity = 1.0, externalAcceleration=0,0,0)
		return true;
	}
};

}
}
} //nvidia::apex::legacy

#endif
