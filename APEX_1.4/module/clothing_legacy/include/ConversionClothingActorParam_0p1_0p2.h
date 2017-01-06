/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODULE_CONVERSIONCLOTHINGACTORPARAM_0P1_0P2H_H
#define MODULE_CONVERSIONCLOTHINGACTORPARAM_0P1_0P2H_H

#include "NvParamConversionTemplate.h"
#include "ClothingActorParam_0p1.h"
#include "ClothingActorParam_0p2.h"

namespace nvidia {
namespace apex {
namespace legacy {


typedef NvParameterized::ParamConversionTemplate<nvidia::parameterized::ClothingActorParam_0p1, 
						nvidia::parameterized::ClothingActorParam_0p2, 
						nvidia::parameterized::ClothingActorParam_0p1::ClassVersion, 
						nvidia::parameterized::ClothingActorParam_0p2::ClassVersion>
						ConversionClothingActorParam_0p1_0p2Parent;

class ConversionClothingActorParam_0p1_0p2: public ConversionClothingActorParam_0p1_0p2Parent
{
public:
	static NvParameterized::Conversion* Create(NvParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionClothingActorParam_0p1_0p2));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionClothingActorParam_0p1_0p2)(t) : 0;
	}

protected:
	ConversionClothingActorParam_0p1_0p2(NvParameterized::Traits* t) : ConversionClothingActorParam_0p1_0p2Parent(t) {}

	bool convert()
	{
		//TODO:
		//	write custom conversion code here using mNewData and mLegacyData members
		//	note that members with same names were already copied in parent converter
		// and mNewData was already initialized with default values

		return true;
	}
};


}
}
} //nvidia::apex::legacy

#endif
