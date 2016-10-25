// This code contains NVIDIA Confidential Information and is disclosed to you
// under a form of NVIDIA software license agreement provided separately to you.
//
// Notice
// NVIDIA Corporation and its licensors retain all intellectual property and
// proprietary rights in and to this software and related documentation and
// any modifications thereto. Any use, reproduction, disclosure, or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA Corporation is strictly prohibited.
//
// ALL NVIDIA DESIGN SPECIFICATIONS, CODE ARE PROVIDED "AS IS.". NVIDIA MAKES
// NO WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES OF NONINFRINGEMENT,
// MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Information and code furnished is believed to be accurate and reliable.
// However, NVIDIA Corporation assumes no responsibility for the consequences of use of such
// information or for any infringement of patents or other rights of third parties that may
// result from its use. No license is granted by implication or otherwise under any patent
// or patent rights of NVIDIA Corporation. Details are subject to change without notice.
// This code supersedes and replaces all information previously supplied.
// NVIDIA Corporation products are not authorized for use as critical
// components in life support devices or systems without express written approval of
// NVIDIA Corporation.
//
// Copyright (c) 2008-2013 NVIDIA Corporation. All rights reserved.

#ifndef CONVERSIONCLOTHINGCOOKEDPARAM_0P2_0P3H_H
#define CONVERSIONCLOTHINGCOOKEDPARAM_0P2_0P3H_H

#include "NvParamConversionTemplate.h"
#include "ClothingCookedParam_0p2.h"
#include "ClothingCookedParam_0p3.h"

namespace nvidia
{
namespace apex
{
namespace legacy 
{

typedef NvParameterized::ParamConversionTemplate<nvidia::parameterized::ClothingCookedParam_0p2, 
						nvidia::parameterized::ClothingCookedParam_0p3, 
						nvidia::parameterized::ClothingCookedParam_0p2::ClassVersion, 
						nvidia::parameterized::ClothingCookedParam_0p3::ClassVersion>
						ConversionClothingCookedParam_0p2_0p3Parent;

class ConversionClothingCookedParam_0p2_0p3: public ConversionClothingCookedParam_0p2_0p3Parent
{
public:
	static NvParameterized::Conversion* Create(NvParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionClothingCookedParam_0p2_0p3));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionClothingCookedParam_0p2_0p3)(t) : 0;
	}

protected:
	ConversionClothingCookedParam_0p2_0p3(NvParameterized::Traits* t) : ConversionClothingCookedParam_0p2_0p3Parent(t) {}

	const NvParameterized::PrefVer* getPreferredVersions() const
	{
		static NvParameterized::PrefVer prefVers[] =
		{
			//TODO:
			//	Add your preferred versions for included references here.
			//	Entry format is
			//		{ (const char*)longName, (PxU32)preferredVersion }

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

		return true;
	}
};

}
}
} //nvidia::apex::legacy

#endif
