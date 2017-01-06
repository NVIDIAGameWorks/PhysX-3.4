/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODULE_CONVERSIONCONVEXHULLPARAMETERS_0P0_0P1H_H
#define MODULE_CONVERSIONCONVEXHULLPARAMETERS_0P0_0P1H_H

#include "NvParamConversionTemplate.h"
#include "ConvexHullParameters_0p0.h"
#include "ConvexHullParameters_0p1.h"

#include "ApexSharedUtils.h"

namespace nvidia {
namespace apex {
namespace legacy {


typedef NvParameterized::ParamConversionTemplate<nvidia::parameterized::ConvexHullParameters_0p0, 
						nvidia::parameterized::ConvexHullParameters_0p1, 
						nvidia::parameterized::ConvexHullParameters_0p0::ClassVersion, 
						nvidia::parameterized::ConvexHullParameters_0p1::ClassVersion>
						ConversionConvexHullParameters_0p0_0p1Parent;

class ConversionConvexHullParameters_0p0_0p1: public ConversionConvexHullParameters_0p0_0p1Parent
{
public:
	static NvParameterized::Conversion* Create(NvParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionConvexHullParameters_0p0_0p1));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionConvexHullParameters_0p0_0p1)(t) : 0;
	}

protected:
	ConversionConvexHullParameters_0p0_0p1(NvParameterized::Traits* t) : ConversionConvexHullParameters_0p0_0p1Parent(t) {}

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
		// The 0.0 format does not have the adjacentFaces array.  We will simply rebuild the convex hull from the
		// old version's vertices.

		nvidia::ConvexHullImpl hull;
		hull.init();

		hull.buildFromPoints(mLegacyData->vertices.buf, (uint32_t)mLegacyData->vertices.arraySizes[0], (uint32_t)mLegacyData->vertices.elementSize);
		mNewData->copy(*hull.mParams);

		return true;
	}
};


}
}
} //nvidia::apex::legacy

#endif
