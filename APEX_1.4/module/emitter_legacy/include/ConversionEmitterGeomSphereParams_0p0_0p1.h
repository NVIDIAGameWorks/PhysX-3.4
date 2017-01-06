/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODULE_CONVERSIONEMITTERGEOMSPHEREPARAMS_0P0_0P1H_H
#define MODULE_CONVERSIONEMITTERGEOMSPHEREPARAMS_0P0_0P1H_H

#include "NvParamConversionTemplate.h"
#include "EmitterGeomSphereParams_0p0.h"
#include "EmitterGeomSphereParams_0p1.h"

namespace nvidia {
namespace apex {
namespace legacy {


typedef NvParameterized::ParamConversionTemplate<nvidia::parameterized::EmitterGeomSphereParams_0p0, 
						nvidia::parameterized::EmitterGeomSphereParams_0p1, 
						nvidia::parameterized::EmitterGeomSphereParams_0p0::ClassVersion, 
						nvidia::parameterized::EmitterGeomSphereParams_0p1::ClassVersion>
						ConversionEmitterGeomSphereParams_0p0_0p1Parent;

class ConversionEmitterGeomSphereParams_0p0_0p1: public ConversionEmitterGeomSphereParams_0p0_0p1Parent
{
public:
	static NvParameterized::Conversion* Create(NvParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionEmitterGeomSphereParams_0p0_0p1));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionEmitterGeomSphereParams_0p0_0p1)(t) : 0;
	}

protected:
	ConversionEmitterGeomSphereParams_0p0_0p1(NvParameterized::Traits* t) : ConversionEmitterGeomSphereParams_0p0_0p1Parent(t) {}

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

		return true;
	}
};


}
}
} //nvidia::apex::legacy

#endif
