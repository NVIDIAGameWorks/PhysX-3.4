/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "ApexDefs.h"

#include "SimulationAbstract.h"

// for NUM_VERTICES_PER_CACHE_BLOCK
#include "ClothingAssetImpl.h"


namespace nvidia
{
namespace clothing
{

void SimulationAbstract::init(uint32_t numVertices, uint32_t numIndices, bool writebackNormals)
{
	sdkNumDeformableVertices = numVertices;
	sdkNumDeformableIndices = numIndices;

	const uint32_t alignedNumVertices = (numVertices + 15) & 0xfffffff0;
	const uint32_t writeBackDataSize = (sizeof(PxVec3) * alignedNumVertices) * (writebackNormals ? 2 : 1);

	PX_ASSERT(sdkWritebackPosition == NULL);
	PX_ASSERT(sdkWritebackNormal == NULL);
	sdkWritebackPosition = (PxVec3*)PX_ALLOC(writeBackDataSize, PX_DEBUG_EXP("SimulationAbstract::writebackData"));
	sdkWritebackNormal = writebackNormals ? sdkWritebackPosition + alignedNumVertices : NULL;

	const uint32_t allocNumVertices = (((numVertices + NUM_VERTICES_PER_CACHE_BLOCK - 1) / NUM_VERTICES_PER_CACHE_BLOCK)) * NUM_VERTICES_PER_CACHE_BLOCK;
	PX_ASSERT(skinnedPhysicsPositions == NULL);
	PX_ASSERT(skinnedPhysicsNormals == NULL);
	skinnedPhysicsPositions = (PxVec3*)PX_ALLOC(sizeof(PxVec3) * allocNumVertices * 2, PX_DEBUG_EXP("SimulationAbstract::skinnedPhysicsPositions"));
	skinnedPhysicsNormals = skinnedPhysicsPositions + allocNumVertices;
}



void SimulationAbstract::initSimulation(const tSimParams& s)
{
	simulation = s;
}


}
} // namespace nvidia

