/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODULE_CONVERSIONBASICIOSASSETPARAM_0P3_0P4H_H
#define MODULE_CONVERSIONBASICIOSASSETPARAM_0P3_0P4H_H

#include "NvParamConversionTemplate.h"
#include "BasicIOSAssetParam_0p3.h"
#include "BasicIOSAssetParam_0p4.h"

namespace nvidia {
namespace apex {
namespace legacy {


typedef NvParameterized::ParamConversionTemplate<nvidia::parameterized::BasicIOSAssetParam_0p3, 
						nvidia::parameterized::BasicIOSAssetParam_0p4, 
						nvidia::parameterized::BasicIOSAssetParam_0p3::ClassVersion, 
						nvidia::parameterized::BasicIOSAssetParam_0p4::ClassVersion>
						ConversionBasicIOSAssetParam_0p3_0p4Parent;

class ConversionBasicIOSAssetParam_0p3_0p4: public ConversionBasicIOSAssetParam_0p3_0p4Parent
{
public:
	static NvParameterized::Conversion* Create(NvParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionBasicIOSAssetParam_0p3_0p4));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionBasicIOSAssetParam_0p3_0p4)(t) : 0;
	}

protected:
	ConversionBasicIOSAssetParam_0p3_0p4(NvParameterized::Traits* t) : ConversionBasicIOSAssetParam_0p3_0p4Parent(t) {}

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
		// copy 'collisionGroupMaskName' to 'collisionFilterDataName'
		NvParameterized::Handle handle(*mNewData, "collisionFilterDataName");
		handle.setParamString(mLegacyData->collisionGroupMaskName);

		return true;
	}
};


}
}
} //nvidia::apex::legacy

#endif
