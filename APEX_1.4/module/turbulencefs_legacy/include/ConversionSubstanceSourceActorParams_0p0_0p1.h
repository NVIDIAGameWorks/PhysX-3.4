/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODULE_CONVERSIONSUBSTANCESOURCEACTORPARAMS_0P0_0P1H_H
#define MODULE_CONVERSIONSUBSTANCESOURCEACTORPARAMS_0P0_0P1H_H

#include "NvParamConversionTemplate.h"
#include "SubstanceSourceActorParams_0p0.h"
#include "SubstanceSourceActorParams_0p1.h"

namespace nvidia {
namespace apex {
namespace legacy {


typedef NvParameterized::ParamConversionTemplate<nvidia::parameterized::SubstanceSourceActorParams_0p0, 
						nvidia::parameterized::SubstanceSourceActorParams_0p1, 
						nvidia::parameterized::SubstanceSourceActorParams_0p0::ClassVersion, 
						nvidia::parameterized::SubstanceSourceActorParams_0p1::ClassVersion>
						ConversionSubstanceSourceActorParams_0p0_0p1Parent;

class ConversionSubstanceSourceActorParams_0p0_0p1: public ConversionSubstanceSourceActorParams_0p0_0p1Parent
{
public:
	static NvParameterized::Conversion* Create(NvParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionSubstanceSourceActorParams_0p0_0p1));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionSubstanceSourceActorParams_0p0_0p1)(t) : 0;
	}

protected:
	ConversionSubstanceSourceActorParams_0p0_0p1(NvParameterized::Traits* t) : ConversionSubstanceSourceActorParams_0p0_0p1Parent(t) {}

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
		{
			physx::PxMat33 tm(physx::PxVec3(mLegacyData->initialPose[0],mLegacyData->initialPose[1],mLegacyData->initialPose[2]),
								physx::PxVec3(mLegacyData->initialPose[3],mLegacyData->initialPose[4],mLegacyData->initialPose[5]),
								physx::PxVec3(mLegacyData->initialPose[6],mLegacyData->initialPose[7],mLegacyData->initialPose[8]));

			mNewData->initialPose.q = physx::PxQuat(tm);
			mNewData->initialPose.p = physx::PxVec3(mLegacyData->initialPose[9],mLegacyData->initialPose[10],mLegacyData->initialPose[11]);
		}
		return true;
	}
};


}
}
} //nvidia::apex::legacy

#endif
