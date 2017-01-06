/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "RTdef.h"
#if RT_COMPILE
#include <RenderMeshAsset.h>

#include "Mesh.h"

namespace nvidia
{
namespace fracture
{

void Mesh::gatherPartMesh(Array<PxVec3>& vertices,
					nvidia::Array<uint32_t>&  indices,
					nvidia::Array<PxVec3>& normals,
					nvidia::Array<PxVec2>& texcoords,
					nvidia::Array<SubMesh>& subMeshes,
					const RenderMeshAsset& renderMeshAsset,
					uint32_t partIndex)
					
{
	if (partIndex >= renderMeshAsset.getPartCount())
	{
		vertices.resize(0);
		indices.resize(0);
		normals.resize(0);
		texcoords.resize(0);
		subMeshes.resize(0);
		return;
	}

	subMeshes.resize(renderMeshAsset.getSubmeshCount());

	// Pre-count vertices and indices so we can allocate once
	uint32_t vertexCount = 0;
	uint32_t indexCount = 0;
	for (uint32_t submeshIndex = 0; submeshIndex < renderMeshAsset.getSubmeshCount(); ++submeshIndex)
	{
		const RenderSubmesh& submesh = renderMeshAsset.getSubmesh(submeshIndex);
		vertexCount += submesh.getVertexCount(partIndex);
		indexCount += submesh.getIndexCount(partIndex);
	}

	vertices.resize(vertexCount);
	normals.resize(vertexCount);
	texcoords.resize(vertexCount);
	indices.resize(indexCount);

	vertexCount = 0;
	indexCount = 0;
	for (uint32_t submeshIndex = 0; submeshIndex < renderMeshAsset.getSubmeshCount(); ++submeshIndex)
	{
		const RenderSubmesh& submesh = renderMeshAsset.getSubmesh(submeshIndex);
		const uint32_t submeshVertexCount = submesh.getVertexCount(partIndex);
		if (submeshVertexCount > 0)
		{
			const VertexBuffer& vertexBuffer = submesh.getVertexBuffer();
			const VertexFormat& vertexFormat = vertexBuffer.getFormat();

			enum { MESH_SEMANTIC_COUNT = 3 };
			struct { 
				RenderVertexSemantic::Enum semantic; 
				RenderDataFormat::Enum format; 
				uint32_t sizeInBytes;
				void* dstBuffer;
			} semanticData[MESH_SEMANTIC_COUNT] = {
				{ RenderVertexSemantic::POSITION,  RenderDataFormat::FLOAT3, sizeof(PxVec3), &vertices[vertexCount]  },
				{ RenderVertexSemantic::NORMAL,    RenderDataFormat::FLOAT3, sizeof(PxVec3), &normals[vertexCount]   },
				{ RenderVertexSemantic::TEXCOORD0, RenderDataFormat::FLOAT2, sizeof(PxVec2), &texcoords[vertexCount] } 
			};

			for (uint32_t i = 0; i < MESH_SEMANTIC_COUNT; ++i)
			{
				const int32_t bufferIndex = vertexFormat.getBufferIndexFromID(vertexFormat.getSemanticID(semanticData[i].semantic));
				if (bufferIndex >= 0)
					vertexBuffer.getBufferData(semanticData[i].dstBuffer, 
											   semanticData[i].format, 
											   semanticData[i].sizeInBytes, 
											   (uint32_t)bufferIndex, 
											   submesh.getFirstVertexIndex(partIndex), 
											   submesh.getVertexCount(partIndex));
				else
					memset(semanticData[i].dstBuffer, 0, submesh.getVertexCount(partIndex)*semanticData[i].sizeInBytes);
			}

			/*
			const uint32_t firstVertexIndex       = submesh.getFirstVertexIndex(partIndex);
			fillBuffer<PxVec3>(vertexBuffer, vertexFormat, RenderVertexSemantic::POSITION, RenderDataFormat::FLOAT3,
			                   firstVertexIndex, submeshVertexCount, &vertices[vertexCount]);
			fillBuffer<PxVec3>(vertexBuffer, vertexFormat, RenderVertexSemantic::NORMAL, RenderDataFormat::FLOAT3,
			                   firstVertexIndex, submeshVertexCount, &normals[vertexCount]);
			fillBuffer<PxVec2>(vertexBuffer, vertexFormat, RenderVertexSemantic::TEXCOORD0, RenderDataFormat::FLOAT2,
			                   firstVertexIndex, submeshVertexCount, &texcoords[vertexCount]);*/

			const uint32_t* partIndexBuffer = submesh.getIndexBuffer(partIndex);
			const uint32_t partIndexCount = submesh.getIndexCount(partIndex);
			subMeshes[submeshIndex].firstIndex = (int32_t)partIndexCount;
			for (uint32_t indexNum = 0; indexNum < partIndexCount; ++indexNum)
			{
				indices[indexCount++] = partIndexBuffer[indexNum] + vertexCount - submesh.getFirstVertexIndex(partIndex);
			}
			vertexCount += submeshVertexCount;
		}
	}
}


void Mesh::loadFromRenderMesh(const RenderMeshAsset& mesh, uint32_t partIndex)
{
	gatherPartMesh(mVertices, mIndices, mNormals, mTexCoords, mSubMeshes, mesh, partIndex);
}

}
}
#endif