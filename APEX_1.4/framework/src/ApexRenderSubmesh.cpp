/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "ApexRenderSubmesh.h"

//#include "ApexStream.h"
//#include "ApexSharedSerialization.h"
#include "ApexSDKIntl.h"


namespace nvidia
{
namespace apex
{

PX_INLINE uint32_t findIndexedNeighbors(uint32_t indexedNeighbors[3], uint32_t triangleIndex,
        const uint32_t* indexBuffer, const uint32_t* vertexTriangleRefs, const uint32_t* vertexToTriangleMap)
{
	uint32_t indexedNeighborCount = 0;
	const uint32_t* triangleVertexIndices = indexBuffer + 3 * triangleIndex;
	for (uint32_t v = 0; v < 3; ++v)
	{
		const uint32_t vertexIndex = triangleVertexIndices[v];
		const uint32_t prevVertexIndex = triangleVertexIndices[(3 >> v) ^ 1];
		// Find all other triangles which have this vertex
		const uint32_t mapStart = vertexTriangleRefs[vertexIndex];
		const uint32_t mapStop = vertexTriangleRefs[vertexIndex + 1];
		for (uint32_t i = mapStart; i < mapStop; ++i)
		{
			const uint32_t neighborTriangleIndex = vertexToTriangleMap[i];
			// See if the previous vertex on the triangle matches the next vertex on the neighbor.  (This will
			// automatically exclude the triangle itself, so no check to exclude a self-check is made.)
			const uint32_t* neighborTriangleVertexIndices = indexBuffer + 3 * neighborTriangleIndex;
			const uint8_t indexMatch = (uint8_t)((uint8_t)(neighborTriangleVertexIndices[0] == vertexIndex) |
													     (uint8_t)(neighborTriangleVertexIndices[1] == vertexIndex) << 1 |
													     (uint8_t)(neighborTriangleVertexIndices[2] == vertexIndex) << 2);
			const uint32_t nextNeighborVertexIndex = neighborTriangleVertexIndices[indexMatch & 3];
			if (nextNeighborVertexIndex == prevVertexIndex)
			{
				// Found a neighbor
				indexedNeighbors[indexedNeighborCount++] = neighborTriangleIndex;
			}
		}
	}

	return indexedNeighborCount;
}



void ApexRenderSubmesh::applyPermutation(const Array<uint32_t>& old2new, const Array<uint32_t>& new2old)
{
	if (mParams->vertexPartition.arraySizes[0] == 2)
	{
		mVertexBuffer.applyPermutation(new2old);
	}

	const uint32_t numIndices = (uint32_t)mParams->indexBuffer.arraySizes[0];
	for (uint32_t i = 0; i < numIndices; i++)
	{
		PX_ASSERT(mParams->indexBuffer.buf[i] < old2new.size());
		mParams->indexBuffer.buf[i] = old2new[mParams->indexBuffer.buf[i]];
	}
}



bool ApexRenderSubmesh::createFromParameters(SubmeshParameters* params)
{
	mParams = params;

	if (mParams->vertexBuffer == NULL)
	{
		NvParameterized::Traits* traits = GetInternalApexSDK()->getParameterizedTraits();
		mParams->vertexBuffer = traits->createNvParameterized(VertexBufferParameters::staticClassName());
	}
	mVertexBuffer.setParams(static_cast<VertexBufferParameters*>(mParams->vertexBuffer));

	return true;
}



void ApexRenderSubmesh::setParams(SubmeshParameters* submeshParams, VertexBufferParameters* vertexBufferParams)
{

	if (vertexBufferParams == NULL && submeshParams != NULL)
	{
		vertexBufferParams = static_cast<VertexBufferParameters*>(submeshParams->vertexBuffer);
		PX_ASSERT(vertexBufferParams != NULL);
	}
	else if (submeshParams != NULL && submeshParams->vertexBuffer == NULL)
	{
		submeshParams->vertexBuffer = vertexBufferParams;
	}
	else if (mParams == NULL)
	{
		// Only emit this warning if mParams is empty yet (not on destruction of the object)
		APEX_INTERNAL_ERROR("Confliciting parameterized objects!");
	}
	mParams = submeshParams;

	mVertexBuffer.setParams(vertexBufferParams);
}



void ApexRenderSubmesh::addStats(RenderMeshAssetStats& stats) const
{
	stats.vertexCount += mVertexBuffer.getVertexCount();
	stats.indexCount += mParams->indexBuffer.arraySizes[0];

	const uint32_t submeshVertexBytes = mVertexBuffer.getAllocationSize();
	stats.vertexBufferBytes += submeshVertexBytes;
	stats.totalBytes += submeshVertexBytes;

	const uint32_t submeshIndexBytes = mParams->indexBuffer.arraySizes[0] * sizeof(uint32_t);
	stats.indexBufferBytes += submeshIndexBytes;
	stats.totalBytes += submeshIndexBytes;

	stats.totalBytes += mParams->smoothingGroups.arraySizes[0] * sizeof(uint32_t);
}



void ApexRenderSubmesh::buildVertexBuffer(const VertexFormat& format, uint32_t vertexCount)
{
	mVertexBuffer.build(format, vertexCount);
}


} // namespace apex
} // namespace nvidia
