/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODULE_CONVERSIONCLOTHINGASSETPARAMETERS_0P7_0P8H_H
#define MODULE_CONVERSIONCLOTHINGASSETPARAMETERS_0P7_0P8H_H

#include "NvParamConversionTemplate.h"
#include "ClothingAssetParameters_0p7.h"
#include "ClothingAssetParameters_0p8.h"

namespace nvidia {
namespace apex {
namespace legacy {


typedef NvParameterized::ParamConversionTemplate<nvidia::parameterized::ClothingAssetParameters_0p7, 
						nvidia::parameterized::ClothingAssetParameters_0p8, 
						nvidia::parameterized::ClothingAssetParameters_0p7::ClassVersion, 
						nvidia::parameterized::ClothingAssetParameters_0p8::ClassVersion>
						ConversionClothingAssetParameters_0p7_0p8Parent;

class ConversionClothingAssetParameters_0p7_0p8: public ConversionClothingAssetParameters_0p7_0p8Parent
{
public:
	static NvParameterized::Conversion* Create(NvParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionClothingAssetParameters_0p7_0p8));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionClothingAssetParameters_0p7_0p8)(t) : 0;
	}

protected:
	ConversionClothingAssetParameters_0p7_0p8(NvParameterized::Traits* t) : ConversionClothingAssetParameters_0p7_0p8Parent(t) {}

	const NvParameterized::PrefVer* getPreferredVersions() const
	{
		static NvParameterized::PrefVer prefVers[] =
		{
			//TODO:
			//	Add your preferred versions for included references here.
			//	Entry format is
			//		{ (const char*)longName, (uint32_t)preferredVersion }

			{ "physicalMeshes[]", 8 },
			{ 0, 0 } // Terminator (do not remove!)
		};

		//PX_UNUSED(prefVers[0]); // Make compiler happy

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

#if 0
		// PH: It seems this was a bad idea. So now old assets will not have any virtual particles.

		float smallestTriangleArea = PX_MAX_F32;
		float largestTriangleArea = 0.0f;

		const int32_t numPhysicalMeshes = mNewData->physicalMeshes.arraySizes[0];
		for (int32_t i = 0; i < numPhysicalMeshes; i++)
		{
			PX_ASSERT(mNewData->physicalMeshes.buf[i]->getMajorVersion() == 0);
			PX_ASSERT(mNewData->physicalMeshes.buf[i]->getMinorVersion() == 8);

			ClothingPhysicalMeshParameters_0p8& physicalMesh = static_cast<ClothingPhysicalMeshParameters_0p8&>(*mNewData->physicalMeshes.buf[i]);

			const uint32_t numIndices = physicalMesh.physicalMesh.numIndices;
			//const uint32_t numVertices = physicalMesh.physicalMesh.numVertices;
			const uint32_t* indices = physicalMesh.physicalMesh.indices.buf;
			const physx::PxVec3* vertices = physicalMesh.physicalMesh.vertices.buf;

			for (uint32_t j = 0; j < numIndices; j += 3)
			{
				const physx::PxVec3 edge1 = vertices[indices[j + 1]] - vertices[indices[j]];
				const physx::PxVec3 edge2 = vertices[indices[j + 2]] - vertices[indices[j]];
				const float triangleArea = edge1.cross(edge2).magnitude();

				smallestTriangleArea = PxMin(smallestTriangleArea, triangleArea);
				largestTriangleArea = PxMax(largestTriangleArea, triangleArea);
			}
		}

		const float sphereArea = mNewData->simulation.thickness * mNewData->simulation.thickness * PxPi;
		const float coveredTriangleArea = 1.5f * sphereArea;

		float globalMinDensity = 0.0f;

		for (int32_t i = 0; i < numPhysicalMeshes; i++)
		{
			PX_ASSERT(mNewData->physicalMeshes.buf[i]->getMajorVersion() == 0);
			PX_ASSERT(mNewData->physicalMeshes.buf[i]->getMinorVersion() == 8);

			ClothingPhysicalMeshParameters_0p8& physicalMesh = static_cast<ClothingPhysicalMeshParameters_0p8&>(*mNewData->physicalMeshes.buf[i]);

			const uint32_t numIndices = physicalMesh.physicalMesh.numIndices;
			const uint32_t* indices = physicalMesh.physicalMesh.indices.buf;
			const physx::PxVec3* vertices = physicalMesh.physicalMesh.vertices.buf;

			for (uint32_t j = 0; j < numIndices; j += 3)
			{
				const physx::PxVec3 edge1 = vertices[indices[j + 1]] - vertices[indices[j]];
				const physx::PxVec3 edge2 = vertices[indices[j + 2]] - vertices[indices[j]];
				const float triangleArea = edge1.cross(edge2).magnitude();

				const float oldNumSpheres = (triangleArea - coveredTriangleArea) / sphereArea;
				if (oldNumSpheres > 0.8f)
				{
					const float reverseMinTriangleArea = triangleArea / oldNumSpheres;
					const float minDensity = physx::PxClamp((reverseMinTriangleArea - largestTriangleArea) / (smallestTriangleArea / 2.0f - largestTriangleArea), 0.0f, 1.0f);

					globalMinDensity = PxMax(globalMinDensity, minDensity);
				}
			}
		}

		mNewData->simulation.virtualParticleDensity = globalMinDensity;
#endif

		return true;
	}
};


}
}
} //nvidia::apex::legacy

#endif
