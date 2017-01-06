/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODULE_CONVERSIONGROUNDEMITTERASSETPARAMETERS_0P1_0P2H_H
#define MODULE_CONVERSIONGROUNDEMITTERASSETPARAMETERS_0P1_0P2H_H

#include "NvParamConversionTemplate.h"
#include "GroundEmitterAssetParameters_0p1.h"
#include "GroundEmitterAssetParameters_0p2.h"

#include "nvparameterized/NvParamUtils.h"

#define PARAM_RET(x) if( (x) != NvParameterized::ERROR_NONE ) \
						{ PX_ASSERT(0 && "INVALID Parameter"); return false; }

namespace nvidia {
namespace apex {
namespace legacy {


typedef NvParameterized::ParamConversionTemplate<nvidia::parameterized::GroundEmitterAssetParameters_0p1, 
						nvidia::parameterized::GroundEmitterAssetParameters_0p2, 
						nvidia::parameterized::GroundEmitterAssetParameters_0p1::ClassVersion, 
						nvidia::parameterized::GroundEmitterAssetParameters_0p2::ClassVersion>
						ConversionGroundEmitterAssetParameters_0p1_0p2Parent;

class ConversionGroundEmitterAssetParameters_0p1_0p2: public ConversionGroundEmitterAssetParameters_0p1_0p2Parent
{
public:
	static NvParameterized::Conversion* Create(NvParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionGroundEmitterAssetParameters_0p1_0p2));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionGroundEmitterAssetParameters_0p1_0p2)(t) : 0;
	}

protected:
	ConversionGroundEmitterAssetParameters_0p1_0p2(NvParameterized::Traits* t) : ConversionGroundEmitterAssetParameters_0p1_0p2Parent(t) {}

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

		NvParameterized::Handle hOld(*mLegacyData, "materialFactoryMapList");
		PX_ASSERT(hOld.isValid());

		NvParameterized::Handle hNew(*mNewData, "materialFactoryMapList");
		PX_ASSERT(hNew.isValid());

		for (int32_t i = 0; i < mLegacyData->materialFactoryMapList.arraySizes[0]; i++)
		{
			hOld.set(i);
			hNew.set(i);

			NvParameterized::Handle hChild(mLegacyData);
			PARAM_RET(hOld.getChildHandle(mLegacyData, "iosAssetName", hChild));
			PX_ASSERT(hOld.isValid());

			NvParameterized::Interface* oldRef = NULL;

			PARAM_RET(hChild.getParamRef(oldRef));
			PARAM_RET(hChild.setParamRef(0));

			if (oldRef)
			{
				if (!physx::shdfnd::strcmp("NxBasicIosAsset", oldRef->className()))
				{
					oldRef->setClassName("BasicIosAsset");
				}
				PARAM_RET(hNew.getChildHandle(mNewData, "iosAssetName", hChild));
				PARAM_RET(hChild.setParamRef(oldRef));
			}
		
			hNew.popIndex();
			hOld.popIndex();
		}

		return true;
	}
};


}
}
} //nvidia::apex::legacy

#endif
