/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODULE_CONVERSIONCLOTHINGASSETPARAMETERS_0P10_0P11H_H
#define MODULE_CONVERSIONCLOTHINGASSETPARAMETERS_0P10_0P11H_H

#include "NvParamConversionTemplate.h"
#include "ClothingAssetParameters_0p10.h"
#include "ClothingAssetParameters_0p11.h"

#include "ApexSharedUtils.h"

namespace nvidia {
namespace apex {
namespace legacy {


typedef NvParameterized::ParamConversionTemplate<nvidia::parameterized::ClothingAssetParameters_0p10, 
						nvidia::parameterized::ClothingAssetParameters_0p11, 
						nvidia::parameterized::ClothingAssetParameters_0p10::ClassVersion, 
						nvidia::parameterized::ClothingAssetParameters_0p11::ClassVersion>
						ConversionClothingAssetParameters_0p10_0p11Parent;

class ConversionClothingAssetParameters_0p10_0p11: public ConversionClothingAssetParameters_0p10_0p11Parent
{
public:
	static NvParameterized::Conversion* Create(NvParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionClothingAssetParameters_0p10_0p11));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionClothingAssetParameters_0p10_0p11)(t) : 0;
	}

protected:
	ConversionClothingAssetParameters_0p10_0p11(NvParameterized::Traits* t) : ConversionClothingAssetParameters_0p10_0p11Parent(t) {}

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

		// extract planes from convex collision boneActors and store in bonePlanes
		Array<parameterized::ClothingAssetParameters_0p11NS::BonePlane_Type> bonePlanes;
		Array<uint32_t> convexes;
		for (int32_t i = 0; i < mLegacyData->boneActors.arraySizes[0]; ++i)
		{
			parameterized::ClothingAssetParameters_0p10NS::ActorEntry_Type& boneActor = mLegacyData->boneActors.buf[i];
			if (boneActor.convexVerticesCount > 0)
			{
				// extract planes from points
				ConvexHullImpl convexHull;
				convexHull.init();

				convexHull.buildFromPoints(&mLegacyData->boneVertices.buf[boneActor.convexVerticesStart], boneActor.convexVerticesCount, sizeof(physx::PxVec3));
				uint32_t planeCount = convexHull.getPlaneCount();
				if (planeCount + bonePlanes.size() > 32)
				{
					APEX_DEBUG_WARNING("The asset contains more than 32 planes for collision convexes, some of them will not be simulated with 3.x cloth.");
					break;
				}
				else
				{
					uint32_t convex = 0; // each bit references a plane
					for (uint32_t i = 0; i < planeCount; ++i)
					{
						physx::PxPlane plane = convexHull.getPlane(i);
						convex |= 1 << bonePlanes.size();

						parameterized::ClothingAssetParameters_0p11NS::BonePlane_Type newEntry;
						memset(&newEntry, 0, sizeof(parameterized::ClothingAssetParameters_0p11NS::BonePlane_Type));
						newEntry.boneIndex = boneActor.boneIndex;
						newEntry.n = plane.n;
						newEntry.d = plane.d;

						bonePlanes.pushBack(newEntry);
					}

					convexes.pushBack(convex);
				}
			}
		}

		NvParameterized::Handle bonePlanesHandle(*mNewData, "bonePlanes");
		mNewData->resizeArray(bonePlanesHandle, (int32_t)bonePlanes.size());
		for (uint32_t i = 0; i < bonePlanes.size(); ++i)
		{
			mNewData->bonePlanes.buf[i] = bonePlanes[i];
		}

		NvParameterized::Handle convexesHandle(*mNewData, "collisionConvexes");
		mNewData->resizeArray(convexesHandle, (int32_t)convexes.size());
		for (uint32_t i = 0; i < convexes.size(); ++i)
		{
			mNewData->collisionConvexes.buf[i] = convexes[i];
		}
		return true;
	}
};


}
}
} //nvidia::apex::legacy

#endif
