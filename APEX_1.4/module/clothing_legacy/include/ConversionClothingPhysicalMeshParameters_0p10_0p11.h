// This code contains NVIDIA Confidential Information and is disclosed to you
// under a form of NVIDIA software license agreement provided separately to you.
//
// Notice
// NVIDIA Corporation and its licensors retain all intellectual property and
// proprietary rights in and to this software and related documentation and
// any modifications thereto. Any use, reproduction, disclosure, or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA Corporation is strictly prohibited.
//
// ALL NVIDIA DESIGN SPECIFICATIONS, CODE ARE PROVIDED "AS IS.". NVIDIA MAKES
// NO WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES OF NONINFRINGEMENT,
// MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Information and code furnished is believed to be accurate and reliable.
// However, NVIDIA Corporation assumes no responsibility for the consequences of use of such
// information or for any infringement of patents or other rights of third parties that may
// result from its use. No license is granted by implication or otherwise under any patent
// or patent rights of NVIDIA Corporation. Details are subject to change without notice.
// This code supersedes and replaces all information previously supplied.
// NVIDIA Corporation products are not authorized for use as critical
// components in life support devices or systems without express written approval of
// NVIDIA Corporation.
//
// Copyright (c) 2008-2013 NVIDIA Corporation. All rights reserved.

#ifndef CONVERSIONCLOTHINGPHYSICALMESHPARAMETERS_0P10_0P11H_H
#define CONVERSIONCLOTHINGPHYSICALMESHPARAMETERS_0P10_0P11H_H

#include "NvParamConversionTemplate.h"
#include "ClothingPhysicalMeshParameters_0p10.h"
#include "ClothingPhysicalMeshParameters_0p11.h"

namespace nvidia
{
namespace apex
{
namespace legacy 
{


typedef NvParameterized::ParamConversionTemplate<nvidia::parameterized::ClothingPhysicalMeshParameters_0p10, 
						nvidia::parameterized::ClothingPhysicalMeshParameters_0p11, 
						nvidia::parameterized::ClothingPhysicalMeshParameters_0p10::ClassVersion, 
						nvidia::parameterized::ClothingPhysicalMeshParameters_0p11::ClassVersion>
						ConversionClothingPhysicalMeshParameters_0p10_0p11Parent;

class ConversionClothingPhysicalMeshParameters_0p10_0p11: public ConversionClothingPhysicalMeshParameters_0p10_0p11Parent
{
public:
	static NvParameterized::Conversion* Create(NvParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionClothingPhysicalMeshParameters_0p10_0p11));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionClothingPhysicalMeshParameters_0p10_0p11)(t) : 0;
	}

protected:
	ConversionClothingPhysicalMeshParameters_0p10_0p11(NvParameterized::Traits* t) : ConversionClothingPhysicalMeshParameters_0p10_0p11Parent(t) {}

	const NvParameterized::PrefVer* getPreferredVersions() const
	{
		static NvParameterized::PrefVer prefVers[] =
		{
			//TODO:
			//	Add your preferred versions for included references here.
			//	Entry format is
			//		{ (const char*)longName, (PxU32)preferredVersion }

			{ 0, 0 } // Terminator (do not remove!)
		};

		return prefVers;
	}

	bool convert()
	{
		if (mLegacyData->submeshes.buf != NULL)
		{
			mNewData->physicalMesh.numSimulatedVertices = mLegacyData->submeshes.buf[0].numVertices;
			mNewData->physicalMesh.numMaxDistance0Vertices = mLegacyData->submeshes.buf[0].numMaxDistance0Vertices;

			mNewData->physicalMesh.numSimulatedIndices = mLegacyData->submeshes.buf[0].numIndices;
		}
		else
		{
			mNewData->physicalMesh.numMaxDistance0Vertices = mLegacyData->physicalMesh.numVertices;
		}

		return true;
	}
};

}
}
} //nvidia::apex::legacy

#endif
