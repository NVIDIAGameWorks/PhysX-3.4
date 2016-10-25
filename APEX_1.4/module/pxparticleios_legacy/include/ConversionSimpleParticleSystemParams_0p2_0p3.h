/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODULE_CONVERSIONSIMPLEPARTICLESYSTEMPARAMS_0P2_0P3H_H
#define MODULE_CONVERSIONSIMPLEPARTICLESYSTEMPARAMS_0P2_0P3H_H

#include "NvParamConversionTemplate.h"
#include "SimpleParticleSystemParams_0p2.h"
#include "SimpleParticleSystemParams_0p3.h"

namespace nvidia {
namespace apex {
namespace legacy {


typedef NvParameterized::ParamConversionTemplate<nvidia::parameterized::SimpleParticleSystemParams_0p2, 
						nvidia::parameterized::SimpleParticleSystemParams_0p3, 
						nvidia::parameterized::SimpleParticleSystemParams_0p2::ClassVersion, 
						nvidia::parameterized::SimpleParticleSystemParams_0p3::ClassVersion>
						ConversionSimpleParticleSystemParams_0p2_0p3Parent;

class ConversionSimpleParticleSystemParams_0p2_0p3: public ConversionSimpleParticleSystemParams_0p2_0p3Parent
{
public:
	static NvParameterized::Conversion* Create(NvParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionSimpleParticleSystemParams_0p2_0p3));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionSimpleParticleSystemParams_0p2_0p3)(t) : 0;
	}

protected:
	ConversionSimpleParticleSystemParams_0p2_0p3(NvParameterized::Traits* t) : ConversionSimpleParticleSystemParams_0p2_0p3Parent(t) {}

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
		mNewData->GridDensity.Enabled = mLegacyData->GridDensityGrid.Enabled;
		//mNewData->GridDensity.Resolution = mLegacyData->GridDensityGrid.GridResolution;
		mNewData->GridDensity.GridSize = mLegacyData->GridDensityGrid.FrustumParams.GridSize;
		mNewData->GridDensity.MaxCellCount = mLegacyData->GridDensityGrid.FrustumParams.GridMaxCellCount;

		// enums are strings, better do it the safe way
		NvParameterized::Handle hEnumNew(*mNewData, "GridDensity.Resolution");
		NvParameterized::Handle hEnumOld(*mLegacyData, "GridDensityGrid.GridResolution");
		PX_ASSERT(hEnumNew.isValid());
		PX_ASSERT(hEnumOld.isValid());

		const NvParameterized::Definition* paramDefOld;
		paramDefOld = hEnumOld.parameterDefinition();
		int32_t index = paramDefOld->enumValIndex(mLegacyData->GridDensityGrid.GridResolution);

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
