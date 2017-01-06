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

#ifndef RENDERER_CLOTH_SHAPE_H
#define RENDERER_CLOTH_SHAPE_H

#include <RendererShape.h>
#include "gpu/PxGpu.h"

// fwd to avoid including cuda.h
#if defined(__x86_64) || defined(AMD64) || defined(_M_AMD64)
typedef unsigned long long CUdeviceptr;
#else
typedef unsigned int CUdeviceptr;
#endif


namespace SampleRenderer
{
	class RendererVertexBuffer;
	class RendererIndexBuffer;

	class RendererClothShape : public RendererShape
	{
		enum VertexBufferEnum
		{
			STATIC_VB,
			DYNAMIC_VB,
			NUM_VERTEX_BUFFERS
		};
	public:
		RendererClothShape(Renderer& renderer, 
			const PxVec3* verts, PxU32 numVerts,
			const PxVec3* normals,
			const PxReal* uvs,
			const PxU16* faces, PxU32 numFaces, bool flipWinding=false,
			PxCudaContextManager* ctxMgr=NULL);
        
		// must acquire the cuda context and map buffers before calling
		bool update(CUdeviceptr srcVerts, PxU32 numVerts);
        void update(const PxVec3* verts, PxU32 numVerts, const PxVec3* normals);

		// helper functions to map the buffers for multiple shapes 
		// at once because interop mapping has a large per-call overhead
		// thread must acquire the cuda context before calling
		static void mapShapes(RendererClothShape** shapes, PxU32 n);
		static void unmapShapes(RendererClothShape** shapes, PxU32 n);

		virtual ~RendererClothShape();

		bool isInteropEnabled() const { return m_ctxMgr != NULL; }

	private:
		RendererVertexBuffer*	m_vertexBuffer[NUM_VERTEX_BUFFERS];
		RendererIndexBuffer*	m_indexBuffer;

		PxU32 m_numFaces;

		PxCudaContextManager* m_ctxMgr;
	};

} // namespace SampleRenderer

#endif
