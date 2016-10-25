/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODULE_CONVERSIONSIMPLEPARTICLESYSTEMPARAMS_0P1_0P2H_H
#define MODULE_CONVERSIONSIMPLEPARTICLESYSTEMPARAMS_0P1_0P2H_H

#include "NvParamConversionTemplate.h"
#include "SimpleParticleSystemParams_0p1.h"
#include "SimpleParticleSystemParams_0p2.h"

namespace nvidia {
namespace apex {
namespace legacy {


typedef NvParameterized::ParamConversionTemplate<nvidia::parameterized::SimpleParticleSystemParams_0p1, 
						nvidia::parameterized::SimpleParticleSystemParams_0p2, 
						nvidia::parameterized::SimpleParticleSystemParams_0p1::ClassVersion, 
						nvidia::parameterized::SimpleParticleSystemParams_0p2::ClassVersion>
						ConversionSimpleParticleSystemParams_0p1_0p2Parent;

class ConversionSimpleParticleSystemParams_0p1_0p2: public ConversionSimpleParticleSystemParams_0p1_0p2Parent
{
public:
	static NvParameterized::Conversion* Create(NvParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionSimpleParticleSystemParams_0p1_0p2));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionSimpleParticleSystemParams_0p1_0p2)(t) : 0;
	}

protected:
	ConversionSimpleParticleSystemParams_0p1_0p2(NvParameterized::Traits* t) : ConversionSimpleParticleSystemParams_0p1_0p2Parent(t) {}

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

		mNewData->GridDensityGrid.Enabled = mLegacyData->FakeDensityGrid.Enabled;
		mNewData->GridDensityGrid.FrustumParams.GridMaxCellCount = mLegacyData->FakeDensityGrid.FrustumParams.GridMaxCellCount;
		mNewData->GridDensityGrid.FrustumParams.GridSize = mLegacyData->FakeDensityGrid.FrustumParams.GridSize;

		NvParameterized::Handle hEnumNew(*mNewData, "GridDensityGrid.GridResolution");  //convert FDG -> GDG
		NvParameterized::Handle hEnumOld(*mLegacyData, "FakeDensityGrid.GridResolution");  
		PX_ASSERT(hEnumNew.isValid());
		PX_ASSERT(hEnumOld.isValid());

		const NvParameterized::Definition* paramDefOld;
		paramDefOld = hEnumOld.parameterDefinition();
		int32_t index = paramDefOld->enumValIndex(mLegacyData->FakeDensityGrid.GridResolution);

		const NvParameterized::Definition* paramDefNew;
		paramDefNew = hEnumNew.parameterDefinition();
		hEnumNew.setParamEnum(paramDefNew->enumVal(index));

		return true;
	}
};


}
}
} //nvidia::apex::legacy

#endif
