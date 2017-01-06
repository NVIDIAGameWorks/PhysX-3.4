/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODULE_CONVERSIONEMITTERGEOMEXPLICITPARAMS_0P0_0P1H_H
#define MODULE_CONVERSIONEMITTERGEOMEXPLICITPARAMS_0P0_0P1H_H

#include "NvParamConversionTemplate.h"
#include "EmitterGeomExplicitParams_0p0.h"
#include "EmitterGeomExplicitParams_0p1.h"

namespace nvidia {
namespace apex {
namespace legacy {


typedef NvParameterized::ParamConversionTemplate<nvidia::parameterized::EmitterGeomExplicitParams_0p0, 
						nvidia::parameterized::EmitterGeomExplicitParams_0p1, 
						nvidia::parameterized::EmitterGeomExplicitParams_0p0::ClassVersion, 
						nvidia::parameterized::EmitterGeomExplicitParams_0p1::ClassVersion>
						ConversionEmitterGeomExplicitParams_0p0_0p1Parent;

class ConversionEmitterGeomExplicitParams_0p0_0p1: public ConversionEmitterGeomExplicitParams_0p0_0p1Parent
{
public:
	static NvParameterized::Conversion* Create(NvParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionEmitterGeomExplicitParams_0p0_0p1));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionEmitterGeomExplicitParams_0p0_0p1)(t) : 0;
	}

protected:
	ConversionEmitterGeomExplicitParams_0p0_0p1(NvParameterized::Traits* t) : ConversionEmitterGeomExplicitParams_0p0_0p1Parent(t) {}

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


#	define NX_ERR_CHECK_RETURN(x) { if( NvParameterized::ERROR_NONE != (x) ) return false; }

	bool convert()
	{
		NvParameterized::Handle h(*mNewData);
		int32_t size;

		size = mLegacyData->positions.arraySizes[0];
		NX_ERR_CHECK_RETURN(mNewData->getParameterHandle("points.positions", h));
		NX_ERR_CHECK_RETURN(h.resizeArray(size));
		for (int32_t i = 0; i < size; ++i)
		{
			mNewData->points.positions.buf[i].position = mLegacyData->positions.buf[i];
			mNewData->points.positions.buf[i].doDetectOverlaps = false;
		}

		size = mLegacyData->velocities.arraySizes[0];
		NX_ERR_CHECK_RETURN(mNewData->getParameterHandle("points.velocities", h));
		NX_ERR_CHECK_RETURN(h.resizeArray(size));
		NX_ERR_CHECK_RETURN(h.setParamVec3Array(&mLegacyData->velocities.buf[0], size));

		return true;
	}
};


}
}
} //nvidia::apex::legacy

#endif
