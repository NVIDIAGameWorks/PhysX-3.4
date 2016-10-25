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

#ifndef CONVERSIONCLOTHINGGRAPHICALLODPARAMETERS_0P4_0P5H_H
#define CONVERSIONCLOTHINGGRAPHICALLODPARAMETERS_0P4_0P5H_H

#include "NvParamConversionTemplate.h"
#include "ClothingGraphicalLodParameters_0p4.h"
#include "ClothingGraphicalLodParameters_0p5.h"

namespace nvidia
{
namespace apex
{
namespace legacy 
{

typedef NvParameterized::ParamConversionTemplate<parameterized::ClothingGraphicalLodParameters_0p4, 
						parameterized::ClothingGraphicalLodParameters_0p5, 
						nvidia::parameterized::ClothingGraphicalLodParameters_0p4::ClassVersion, 
						nvidia::parameterized::ClothingGraphicalLodParameters_0p5::ClassVersion>
						ConversionClothingGraphicalLodParameters_0p4_0p5Parent;

class ConversionClothingGraphicalLodParameters_0p4_0p5: public ConversionClothingGraphicalLodParameters_0p4_0p5Parent
{
public:
	static NvParameterized::Conversion* Create(NvParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionClothingGraphicalLodParameters_0p4_0p5));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionClothingGraphicalLodParameters_0p4_0p5)(t) : 0;
	}

protected:
	ConversionClothingGraphicalLodParameters_0p4_0p5(NvParameterized::Traits* t) : ConversionClothingGraphicalLodParameters_0p4_0p5Parent(t) {}

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
		int32_t partNum = mLegacyData->physicsSubmeshPartitioning.arraySizes[0];

		NvParameterized::Handle physicsMeshPartitioningHandle(*mNewData, "physicsMeshPartitioning");
		PX_ASSERT(physicsMeshPartitioningHandle.isValid());
		physicsMeshPartitioningHandle.resizeArray((int32_t)(partNum));

		for (int32_t p = 0; p < partNum; p++)
		{
			mNewData->physicsMeshPartitioning.buf[p].graphicalSubmesh = mLegacyData->physicsSubmeshPartitioning.buf[p].graphicalSubmesh;
			mNewData->physicsMeshPartitioning.buf[p].numSimulatedIndices = mLegacyData->physicsSubmeshPartitioning.buf[p].numSimulatedIndices;
			mNewData->physicsMeshPartitioning.buf[p].numSimulatedVertices = mLegacyData->physicsSubmeshPartitioning.buf[p].numSimulatedVertices;
			mNewData->physicsMeshPartitioning.buf[p].numSimulatedVerticesAdditional = mLegacyData->physicsSubmeshPartitioning.buf[p].numSimulatedVerticesAdditional;
		}

		return true;
	}
};

}
}
} //nvidia::apex::legacy

#endif
