/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODULE_CONVERSIONCLOTHINGASSETPARAMETERS_0P1_0P2H_H
#define MODULE_CONVERSIONCLOTHINGASSETPARAMETERS_0P1_0P2H_H

#include "NvParamConversionTemplate.h"
#include "ClothingAssetParameters_0p1.h"
#include "ClothingAssetParameters_0p2.h"

#include "ClothingPhysicalMeshParameters_0p2.h"
#include "ClothingCookedParam_0p1.h"
#include <ParamArray.h>

namespace nvidia {
namespace apex {
namespace legacy {


typedef NvParameterized::ParamConversionTemplate<nvidia::parameterized::ClothingAssetParameters_0p1, 
						nvidia::parameterized::ClothingAssetParameters_0p2, 
						nvidia::parameterized::ClothingAssetParameters_0p1::ClassVersion, 
						nvidia::parameterized::ClothingAssetParameters_0p2::ClassVersion>
						ConversionClothingAssetParameters_0p1_0p2Parent;

class ConversionClothingAssetParameters_0p1_0p2: public ConversionClothingAssetParameters_0p1_0p2Parent
{
public:
	static NvParameterized::Conversion* Create(NvParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionClothingAssetParameters_0p1_0p2));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionClothingAssetParameters_0p1_0p2)(t) : 0;
	}

protected:
	ConversionClothingAssetParameters_0p1_0p2(NvParameterized::Traits* t) : ConversionClothingAssetParameters_0p1_0p2Parent(t) {}

	const NvParameterized::PrefVer* getPreferredVersions() const
	{
		static NvParameterized::PrefVer preferredVers[] =
		{
			{ "physicalMeshes[]", 2 },
			{ 0, 0}
		};

		return preferredVers;
	}

	bool convert()
	{
		//TODO:
		//	write custom conversion code here using mNewData and mLegacyData members
		//	note that members with same names were already copied in parent converter
		// and mNewData was already initialized with default values

		parameterized::ClothingCookedParam_0p1* cookedScale1 = (parameterized::ClothingCookedParam_0p1*)mNewData->getTraits()->createNvParameterized("ClothingCookedParam", 1);

		ParamArray<uint8_t> deformableCookedData(cookedScale1, "deformableCookedData", reinterpret_cast<ParamDynamicArrayStruct*>(&cookedScale1->deformableCookedData));
		ParamArray<parameterized::ClothingCookedParam_0p1NS::CookedPhysicalSubmesh_Type> cookedPhysicalSubmeshes(cookedScale1, "cookedPhysicalSubmeshes", reinterpret_cast<ParamDynamicArrayStruct*>(&cookedScale1->cookedPhysicalSubmeshes));


		for (int32_t physicalMeshID = 0; physicalMeshID < mNewData->physicalMeshes.arraySizes[0]; physicalMeshID++)
		{
			NvParameterized::Interface* iface = mNewData->physicalMeshes.buf[physicalMeshID];
			if (iface->version() == 2)
			{
				parameterized::ClothingPhysicalMeshParameters_0p2* physicalMesh = static_cast<parameterized::ClothingPhysicalMeshParameters_0p2*>(iface);

				uint32_t offset = deformableCookedData.size();
				deformableCookedData.resize(offset + physicalMesh->deformableCookedData.arraySizes[0]);
				memcpy(deformableCookedData.begin() + offset, physicalMesh->deformableCookedData.buf, (size_t)physicalMesh->deformableCookedData.arraySizes[0]);

				PX_ASSERT(cookedScale1->cookedDataVersion == 0 || physicalMesh->deformableCookedDataVersion == 0 || cookedScale1->cookedDataVersion == physicalMesh->deformableCookedDataVersion);
				cookedScale1->cookedDataVersion = physicalMesh->deformableCookedDataVersion;

				for (int32_t submeshID = 0; submeshID < physicalMesh->submeshes.arraySizes[0]; submeshID++)
				{
					parameterized::ClothingPhysicalMeshParameters_0p2NS::PhysicalSubmesh_Type& oldSubmesh = physicalMesh->submeshes.buf[submeshID];

					parameterized::ClothingCookedParam_0p1NS::CookedPhysicalSubmesh_Type submesh;
					submesh.cookedDataLength = oldSubmesh.cookedDataLength;
					submesh.cookedDataOffset = oldSubmesh.cookedDataOffset + offset;
					submesh.deformableMeshPointer = NULL;
					submesh.physicalMeshId = (uint32_t)physicalMeshID;
					submesh.submeshId = (uint32_t)submeshID;

					cookedPhysicalSubmeshes.pushBack(submesh);
				}
			}
		}

		// copy convex cooked data
		if (cookedScale1 != NULL)
		{
			NvParameterized::Handle handle(*cookedScale1, "convexCookedData");
			PX_ASSERT(handle.isValid());
			handle.resizeArray(mLegacyData->convexesCookedData.arraySizes[0]);
			memcpy(cookedScale1->convexCookedData.buf, mLegacyData->convexesCookedData.buf, sizeof(uint8_t) * mLegacyData->convexesCookedData.arraySizes[0]);
		}

		if (deformableCookedData.isEmpty())
		{
			cookedScale1->destroy();
			cookedScale1 = NULL;
		}

		if (cookedScale1 != NULL)
		{
			if (cookedScale1->cookedDataVersion == 0)
			{
				cookedScale1->cookedDataVersion = 300;
			}

			PX_ASSERT(mNewData->cookedData.arraySizes[0] == 0);
			NvParameterized::Handle cookedData(*mNewData, "cookedData");
			PX_ASSERT(cookedData.isValid());
			cookedData.resizeArray(1);
			mNewData->cookedData.buf[0].scale = 1.0f;
			mNewData->cookedData.buf[0].cookedData = cookedScale1;
		}

		return true;
	}
};


}
}
} //nvidia::apex::legacy

#endif
