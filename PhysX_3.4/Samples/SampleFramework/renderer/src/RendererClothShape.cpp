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
// Copyright (c) 2008-2017 NVIDIA Corporation. All rights reserved.

#include <RendererClothShape.h>

#include <Renderer.h>

#include <RendererVertexBuffer.h>
#include <RendererVertexBufferDesc.h>

#include <RendererIndexBuffer.h>
#include <RendererIndexBufferDesc.h>

#include <RendererMesh.h>
#include <RendererMeshDesc.h>

#include <RendererMemoryMacros.h>

#if defined(RENDERER_ENABLE_CUDA_INTEROP)
#include <task/PxTask.h>
#include <cuda.h>

// pre-compiled fatbin
#if defined(_M_X64)
#include "cuda/RenderClothNormals_x64.cuh"
#else
#include "cuda/RenderClothNormals_x86.cuh"
#endif

namespace
{
	CUmodule gClothRenderModule;
	CUfunction gClothRenderFunction;

	void checkSuccess(CUresult r)
	{
		PX_ASSERT(r == CUDA_SUCCESS);
	}
}

#endif

using namespace SampleRenderer;

RendererClothShape::RendererClothShape(	Renderer& renderer, 
	const PxVec3* verts, PxU32 numVerts, 
	const PxVec3* normals,
	const PxReal* uvs,
	const PxU16* faces, PxU32 numFaces, bool flipWinding,
	PxCudaContextManager* ctxMgr) :
RendererShape(renderer), m_numFaces(numFaces), m_ctxMgr(ctxMgr)
{
	PX_UNUSED(m_numFaces);
    
	{
		RendererVertexBufferDesc vbdesc;
		vbdesc.hint = RendererVertexBuffer::HINT_STATIC;
		vbdesc.semanticFormats[RendererVertexBuffer::SEMANTIC_TEXCOORD0] = RendererVertexBuffer::FORMAT_FLOAT2;
		vbdesc.maxVertices = numVerts;
		m_vertexBuffer[STATIC_VB] = m_renderer.createVertexBuffer(vbdesc);
		RENDERER_ASSERT(m_vertexBuffer[STATIC_VB], "Failed to create Vertex Buffer.");
	}
	{
		RendererVertexBufferDesc vbdesc;
		vbdesc.hint = RendererVertexBuffer::HINT_DYNAMIC;
		vbdesc.semanticFormats[RendererVertexBuffer::SEMANTIC_POSITION]  = RendererVertexBuffer::FORMAT_FLOAT3;
		vbdesc.semanticFormats[RendererVertexBuffer::SEMANTIC_NORMAL]    = RendererVertexBuffer::FORMAT_FLOAT3;
		vbdesc.maxVertices = numVerts;

#if defined(RENDERER_ENABLE_CUDA_INTEROP)
		if (ctxMgr)
		{
			vbdesc.registerInCUDA = true;
			vbdesc.interopContext = ctxMgr;

			ctxMgr->acquireContext();

			// find module containing our kernel
			CUresult success;

			success = cuModuleLoadDataEx(&gClothRenderModule, computeSmoothNormals, 0, NULL, NULL);
			RENDERER_ASSERT(success == CUDA_SUCCESS, "Could not load cloth render CUDA module.");

			success = cuModuleGetFunction(&gClothRenderFunction, gClothRenderModule, "computeSmoothNormals");
			RENDERER_ASSERT(success == CUDA_SUCCESS, "Could not find cloth render CUDA function.");

			ctxMgr->releaseContext();
		}
#endif //RENDERER_ENABLE_CUDA_INTEROP

		m_vertexBuffer[DYNAMIC_VB] = m_renderer.createVertexBuffer(vbdesc);
		RENDERER_ASSERT(m_vertexBuffer[DYNAMIC_VB], "Failed to create Vertex Buffer.");
	}
	
	if(m_vertexBuffer[DYNAMIC_VB] && m_vertexBuffer[STATIC_VB])
	{
		PxU32 positionStride = 0;
		void* vertPositions = m_vertexBuffer[DYNAMIC_VB]->lockSemantic(RendererVertexBuffer::SEMANTIC_POSITION, positionStride);

		PxU32 normalStride = 0;
		void* vertNormals = m_vertexBuffer[DYNAMIC_VB]->lockSemantic(RendererVertexBuffer::SEMANTIC_NORMAL, normalStride);

		PxU32 uvStride = 0;
		void* vertUVs = m_vertexBuffer[STATIC_VB]->lockSemantic(RendererVertexBuffer::SEMANTIC_TEXCOORD0, uvStride);

		if(vertPositions && vertNormals && vertUVs)
		{
			for(PxU32 i=0; i<numVerts; i++)
			{
				memcpy(vertPositions, verts+i, sizeof(PxVec3));
				if(normals)
					memcpy(vertNormals, normals+i, sizeof(PxVec3));
				else
					memset(vertNormals, 0, sizeof(PxVec3));

				if(uvs)
					memcpy(vertUVs, uvs+i*2, sizeof(PxReal)*2);
				else
					memset(vertUVs, 0, sizeof(PxReal)*2);

				vertPositions = (void*)(((PxU8*)vertPositions) + positionStride);
				vertNormals   = (void*)(((PxU8*)vertNormals)   + normalStride);
				vertUVs       = (void*)(((PxU8*)vertUVs)       + uvStride);
			}
		}
		m_vertexBuffer[DYNAMIC_VB]->unlockSemantic(RendererVertexBuffer::SEMANTIC_NORMAL);
		m_vertexBuffer[DYNAMIC_VB]->unlockSemantic(RendererVertexBuffer::SEMANTIC_POSITION);
		m_vertexBuffer[STATIC_VB]->unlockSemantic(RendererVertexBuffer::SEMANTIC_TEXCOORD0);
	}

	const PxU32 numIndices = numFaces*3;

	RendererIndexBufferDesc ibdesc;
	ibdesc.hint       = RendererIndexBuffer::HINT_STATIC;
	ibdesc.format     = RendererIndexBuffer::FORMAT_UINT16;
	ibdesc.maxIndices = numIndices;

#if defined(RENDERER_ENABLE_CUDA_INTEROP)
	if (ctxMgr)
	{
		ibdesc.registerInCUDA = true;
		ibdesc.interopContext = ctxMgr;
		ibdesc.hint = RendererIndexBuffer::HINT_DYNAMIC;
	}
#endif // RENDERER_ENABLE_CUDA_INTEROP

	m_indexBuffer = m_renderer.createIndexBuffer(ibdesc);
	RENDERER_ASSERT(m_indexBuffer, "Failed to create Index Buffer.");
	if(m_indexBuffer)
	{
		PxU16* indices = (PxU16*)m_indexBuffer->lock();
		if(indices)
		{
			if(flipWinding)
			{
				for(PxU32 i=0;i<numFaces;i++)
				{
					indices[i*3+0] = faces[i*3+0];
					indices[i*3+1] = faces[i*3+2];
					indices[i*3+2] = faces[i*3+1];
				}
			}
			else
			{
				memcpy(indices, faces, sizeof(*faces)*numFaces*3);
			}
		}
		m_indexBuffer->unlock();
	}

	if(m_vertexBuffer[DYNAMIC_VB] && m_vertexBuffer[STATIC_VB] && m_indexBuffer)
	{
		RendererMeshDesc meshdesc;
		meshdesc.primitives       = RendererMesh::PRIMITIVE_TRIANGLES;
		meshdesc.vertexBuffers    = m_vertexBuffer;
		meshdesc.numVertexBuffers = NUM_VERTEX_BUFFERS;
		meshdesc.firstVertex      = 0;
		meshdesc.numVertices      = numVerts;
		meshdesc.indexBuffer      = m_indexBuffer;
		meshdesc.firstIndex       = 0;
		meshdesc.numIndices       = numIndices;
		m_mesh = m_renderer.createMesh(meshdesc);
		RENDERER_ASSERT(m_mesh, "Failed to create Mesh.");
	}
}
namespace
{

#if defined(RENDERER_ENABLE_CUDA_INTEROP)

PX_INLINE void cuuAlignUp(int* offset, int alignment)
{
	*offset = ((*offset) + (alignment) - 1) & ~((alignment) - 1);
}

template <typename T, typename T2> CUresult
cuuParamSet(CUfunction hf, int* offset, const T2& var)
{
	cuuAlignUp(offset, __alignof(T));
	CUresult err = cuParamSetv(hf, *offset, (void*)&var, sizeof(T));
	*offset += sizeof(T);
	return err;
}

#endif // RENDERER_ENABLE_CUDA_INTEROP

} // anonymous namespace


bool RendererClothShape::update(CUdeviceptr srcVerts, PxU32 numVerts)
{
#if defined(RENDERER_ENABLE_CUDA_INTEROP)

	// note: CUDA context must be acquired, and buffers mapped at this point

	PX_ASSERT(m_ctxMgr);
	PX_ASSERT(srcVerts);

	// get vertex buffer
	CUgraphicsResource vbRes;
	CUdeviceptr destVerts;
	size_t destSize;
	m_vertexBuffer[DYNAMIC_VB]->getInteropResourceHandle(vbRes);
	cuGraphicsResourceGetMappedPointer(&destVerts, &destSize, vbRes);

	// get index buffer
	CUgraphicsResource ibRes;
	CUdeviceptr srcIndices;
	size_t indicesSize;
	m_indexBuffer->getInteropResourceHandle(ibRes);
	cuGraphicsResourceGetMappedPointer(&srcIndices, &indicesSize, ibRes);

	// if either buffers could not be mapped revert to CPU
	if (!destVerts || !srcIndices)
		return false;

	const size_t ptrSize = sizeof(CUdeviceptr);

	int offset = 0;
	checkSuccess(cuuParamSet<CUdeviceptr>(gClothRenderFunction, &offset, srcVerts));
	checkSuccess(cuuParamSet<CUdeviceptr>(gClothRenderFunction, &offset, srcIndices));
	checkSuccess(cuuParamSet<CUdeviceptr>(gClothRenderFunction, &offset, destVerts));
	checkSuccess(cuuParamSet<PxU32>(gClothRenderFunction, &offset, m_numFaces));
	checkSuccess(cuuParamSet<PxU32>(gClothRenderFunction, &offset, numVerts));
	checkSuccess(cuParamSetSize(gClothRenderFunction, offset));

	const PxU32 blockSize = 512;
	const PxU32 numBlocks = 1;

	checkSuccess(cuFuncSetBlockShape(gClothRenderFunction, blockSize, 1, 1));
	checkSuccess(cuLaunchGridAsync(gClothRenderFunction, numBlocks, 1, 0));

	m_mesh->setVertexBufferRange(0, numVerts);

#else
	PX_ASSERT(0);
#endif

	return true;
}

void RendererClothShape::mapShapes(RendererClothShape** shapes, PxU32 n)
{
	PX_UNUSED(shapes);
	PX_UNUSED(n);

#if defined(RENDERER_ENABLE_CUDA_INTEROP)

	if (n == 0)
		return;

	PX_PROFILE_ZONE("RendererClothShape::mapGraphicsResource",0);

	std::vector<CUgraphicsResource> resources;
	for (PxU32 i=0; i < n; ++i)
	{
		CUgraphicsResource vbRes, ibRes;
		shapes[i]->m_vertexBuffer[DYNAMIC_VB]->getInteropResourceHandle(vbRes);
		shapes[i]->m_indexBuffer->getInteropResourceHandle(ibRes);

		resources.push_back(vbRes);
		resources.push_back(ibRes);		
	}

	checkSuccess(cuGraphicsMapResources(unsigned int(resources.size()), &resources[0], 0));

#else
	PX_ASSERT(0);
#endif
}

void RendererClothShape::unmapShapes(RendererClothShape** shapes, PxU32 n)
{
	PX_UNUSED(shapes);
	PX_UNUSED(n);

#if defined(RENDERER_ENABLE_CUDA_INTEROP)

	if (n == 0)
		return;
	
	PX_PROFILE_ZONE("RendererClothShape::unmapGraphicsResource",0);

	std::vector<CUgraphicsResource> resources;
	for (PxU32 i=0; i < n; ++i)
	{
		CUgraphicsResource vbRes, ibRes;
		shapes[i]->m_vertexBuffer[DYNAMIC_VB]->getInteropResourceHandle(vbRes);
		shapes[i]->m_indexBuffer->getInteropResourceHandle(ibRes);

		resources.push_back(vbRes);
		resources.push_back(ibRes);		
	}

	checkSuccess(cuGraphicsUnmapResources(unsigned int(resources.size()), &resources[0], 0));

#else
	PX_ASSERT(0);
#endif
}

void RendererClothShape::update(const PxVec3* verts, PxU32 numVerts, const PxVec3* normals)
{	
    PxU32 positionStride = 0, normalStride = 0;

    PxU8 *locked_positions = static_cast<PxU8*>(m_vertexBuffer[DYNAMIC_VB]->lockSemantic(
                              RendererVertexBuffer::SEMANTIC_POSITION, positionStride));
    PxU8 *locked_normals = static_cast<PxU8*>(m_vertexBuffer[DYNAMIC_VB]->lockSemantic(
                              RendererVertexBuffer::SEMANTIC_NORMAL, normalStride));
    
    for(PxU32 i=0; i<numVerts; i++)
    {
        memcpy(locked_positions, verts + i, sizeof(PxVec3));
        memcpy(locked_normals,  normals + i, sizeof(PxVec3));

        locked_positions += positionStride;
        locked_normals += normalStride;
    }

    m_vertexBuffer[DYNAMIC_VB]->unlockSemantic(SampleRenderer::RendererVertexBuffer::SEMANTIC_NORMAL);
    m_vertexBuffer[DYNAMIC_VB]->unlockSemantic(SampleRenderer::RendererVertexBuffer::SEMANTIC_POSITION);

    m_mesh->setVertexBufferRange(0, numVerts);
}

RendererClothShape::~RendererClothShape(void)
{
	for(PxU32 i = 0; i < NUM_VERTEX_BUFFERS; i++ )
	{
		SAFE_RELEASE(m_vertexBuffer[i]);
	}
	SAFE_RELEASE(m_indexBuffer);
	SAFE_RELEASE(m_mesh);
}
