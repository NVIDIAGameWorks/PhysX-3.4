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
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.
#include <PsUtilities.h>

#include <RendererSimpleParticleSystemShape.h>

#include <Renderer.h>

#include <RendererMemoryMacros.h>
#include <RendererIndexBuffer.h>
#include <RendererIndexBufferDesc.h>
#include <RendererVertexBuffer.h>
#include <RendererVertexBufferDesc.h>
#include <RendererMesh.h>
#include <RendererMeshDesc.h>
#include "PsUtilities.h"
#include "PsBitUtils.h"

using namespace SampleRenderer;
using namespace physx::shdfnd;

RendererSimpleParticleSystemShape::RendererSimpleParticleSystemShape(Renderer &renderer, 
	PxU32 _mMaxParticles) : 
											RendererShape(renderer), 
											mMaxParticles(_mMaxParticles),
											mVertexBuffer(NULL),
											mIndexBuffer(NULL)											
{
	RendererVertexBufferDesc vbdesc;

	vbdesc.maxVertices = mMaxParticles*4;
	vbdesc.hint = RendererVertexBuffer::HINT_DYNAMIC;
	vbdesc.semanticFormats[RendererVertexBuffer::SEMANTIC_POSITION] = RendererVertexBuffer::FORMAT_FLOAT3;
	vbdesc.semanticFormats[RendererVertexBuffer::SEMANTIC_COLOR] = RendererVertexBuffer::FORMAT_COLOR_NATIVE;
	vbdesc.semanticFormats[RendererVertexBuffer::SEMANTIC_TEXCOORD0] = RendererVertexBuffer::FORMAT_FLOAT2;
	mVertexBuffer = m_renderer.createVertexBuffer(vbdesc);
	RENDERER_ASSERT(mVertexBuffer, "Failed to create Vertex Buffer.");
	// if enabled mInstanced meshes -> create instance buffer
	PxU32 color = renderer.convertColor(RendererColor(255, 255, 255, 255));
	RendererMesh::Primitive primitive = RendererMesh::PRIMITIVE_TRIANGLES;
	initializeVertexBuffer(color);	

	if(mVertexBuffer)
	{
		RendererMeshDesc meshdesc;
		meshdesc.primitives			= primitive;
		meshdesc.vertexBuffers		= &mVertexBuffer;
		meshdesc.numVertexBuffers	= 1;
		meshdesc.firstVertex		= 0;
		meshdesc.numVertices		= mVertexBuffer->getMaxVertices();
		meshdesc.indexBuffer		= mIndexBuffer;
		meshdesc.firstIndex			= 0;
		if(mIndexBuffer) 
		{
			meshdesc.numIndices		= mIndexBuffer->getMaxIndices();
		} 
		else 
		{
			meshdesc.numIndices		= 0;
		}
		meshdesc.instanceBuffer		= NULL;
		meshdesc.firstInstance		= 0;
		meshdesc.numInstances		= 0;
		m_mesh = m_renderer.createMesh(meshdesc);
		RENDERER_ASSERT(m_mesh, "Failed to create Mesh.");
	}
}

RendererSimpleParticleSystemShape::~RendererSimpleParticleSystemShape(void)
{
	SAFE_RELEASE(mIndexBuffer);
	SAFE_RELEASE(mVertexBuffer);
	SAFE_RELEASE(m_mesh);
}

void RendererSimpleParticleSystemShape::initializeVertexBuffer(PxU32 color) 
{
	PxU32 positionStride = 0, colorStride = 0, uvStride = 0;
	PxU8* locked_positions = static_cast<PxU8*>(mVertexBuffer->lockSemantic(RendererVertexBuffer::SEMANTIC_POSITION, positionStride));	
	PxU8* locked_colors = static_cast<PxU8*>(mVertexBuffer->lockSemantic(RendererVertexBuffer::SEMANTIC_COLOR, colorStride));
	PxU8* locked_uvs_base = static_cast<PxU8*>(mVertexBuffer->lockSemantic(RendererVertexBuffer::SEMANTIC_TEXCOORD0, uvStride));	
	for(PxU32 i = 0; i < mMaxParticles; ++i) 
	{		

		for (PxU32 j = 0;j < 4; j++)
		{
			memset(locked_colors, color, sizeof(PxU32));
			memset(locked_positions, 0, sizeof(PxReal) * 3);
			locked_colors += colorStride;
			locked_positions += positionStride;
		}
		
		PxReal* locked_uvs = reinterpret_cast<PxReal*> (locked_uvs_base);
        *locked_uvs = 0.0f;
		locked_uvs++;
		*locked_uvs = 0.0f;
		locked_uvs_base += uvStride;

		locked_uvs = reinterpret_cast<PxReal*> (locked_uvs_base);
        *locked_uvs = 1.0f;
		locked_uvs++;
		*locked_uvs = 0.0f;
		locked_uvs_base += uvStride;

		locked_uvs = reinterpret_cast<PxReal*> (locked_uvs_base);
        *locked_uvs = 0.0f;
		locked_uvs++;
		*locked_uvs = 1.0f;
		locked_uvs_base += uvStride;

		locked_uvs = reinterpret_cast<PxReal*> (locked_uvs_base);
        *locked_uvs = 1.0f;
		locked_uvs++;
		*locked_uvs = 1.0f;
		locked_uvs_base += uvStride;
	}
	mVertexBuffer->unlockSemantic(RendererVertexBuffer::SEMANTIC_COLOR);
	mVertexBuffer->unlockSemantic(RendererVertexBuffer::SEMANTIC_POSITION);
	mVertexBuffer->unlockSemantic(RendererVertexBuffer::SEMANTIC_TEXCOORD0);

	RendererIndexBufferDesc inbdesc;
	inbdesc.hint = RendererIndexBuffer::HINT_STATIC;
	inbdesc.format = RendererIndexBuffer::FORMAT_UINT16;
	inbdesc.maxIndices = mMaxParticles*6;
	mIndexBuffer = m_renderer.createIndexBuffer(inbdesc);
	
	PxU16* ib = (PxU16*) mIndexBuffer->lock();
	for (PxU16 i = 0; i<mMaxParticles; i++)
	{
        ib[i * 6 + 0] = i * 4 + 0;
        ib[i * 6 + 1] = i * 4 + 1;
        ib[i * 6 + 2] = i * 4 + 2;
        ib[i * 6 + 3] = i * 4 + 1;
        ib[i * 6 + 4] = i * 4 + 3;
        ib[i * 6 + 5] = i * 4 + 2;
	}
	mIndexBuffer->unlock();
}

void RendererSimpleParticleSystemShape::updateBillboard(PxU32 validParticleRange, 
										 const PxVec3* positions, 
										 const PxU32* validParticleBitmap,
										 const PxReal* lifetime) 
{
	PxU32 positionStride = 0;
	PxU8 *locked_positions = NULL;
	locked_positions = static_cast<PxU8*>(mVertexBuffer->lockSemantic(
							RendererVertexBuffer::SEMANTIC_POSITION, positionStride));
	// update vertex buffer here
	PxU32 numParticles = 0;
	if(validParticleRange > 0)
	{
		for (PxU32 w = 0; w <= (validParticleRange-1) >> 5; w++)
		{
			for (PxU32 b = validParticleBitmap[w]; b; b &= b-1) 
			{
				PxU32 index = (w << 5 | physx::shdfnd::lowestSetBit(b));
				const PxVec3& basePos = positions[index];
				const float particleSize = 1.0f;
				PxVec3 offsets[4] =
				{
					PxVec3(-particleSize,  particleSize, 0.0f),
					PxVec3( particleSize,  particleSize, 0.0f),
					PxVec3(-particleSize, -particleSize, 0.0f),
					PxVec3( particleSize, -particleSize, 0.0f)
				};

				for (int p = 0; p < 4; p++)
				{
					*reinterpret_cast<PxVec3*>(locked_positions) = basePos + offsets[p];
					locked_positions += positionStride;
				}
				numParticles++;
			}
		}
	}
	
	mVertexBuffer->unlockSemantic(RendererVertexBuffer::SEMANTIC_POSITION);
	m_mesh->setVertexBufferRange(0, numParticles*4);
}
