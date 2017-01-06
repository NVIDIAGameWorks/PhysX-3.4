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

#ifndef RENDERER_PARTICLE_SYSTEM_SHAPE_H
#define RENDERER_PARTICLE_SYSTEM_SHAPE_H

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
	class RendererInstanceBuffer;
	class RendererIndexBuffer;

	class RendererParticleSystemShape : public RendererShape
	{
		enum VertexBufferEnum
		{
			STATIC_VB,
			DYNAMIC_POS_VB,
			DYNAMIC_COL_VB,
			NUM_VERTEX_BUFFERS
		};

	public:
								RendererParticleSystemShape(Renderer &renderer, 
									physx::PxU32 num_vertices, 
									bool _mInstanced,
									bool _mFading,
									PxReal fadingPeriod = 1.0f,
									PxReal debriScaleFactor = 1.0f,
									PxCudaContextManager* ctxMgr=NULL
									);
		virtual					~RendererParticleSystemShape(void);

		bool					updateBillboard(PxU32 validParticleRange, 
										CUdeviceptr positions, 
										CUdeviceptr validParticleBitmap,
										CUdeviceptr lifetimes,
										PxU32 numParticles);

		bool					updateInstanced(PxU32 validParticleRange, 
										CUdeviceptr positions, 
										CUdeviceptr validParticleBitmap,
										CUdeviceptr orientations,
										PxU32 numParticles);

		void					updateBillboard(PxU32 validParticleRange, 
										 const PxVec3* positions, 
										 const PxU32* validParticleBitmap,
										 const PxReal* lifetime = NULL);

		void					updateInstanced(PxU32 validParticleRange, 
										const PxVec3* positions, 
										const PxU32* validParticleBitmap,
										const PxMat33* orientation);

		bool					isInteropEnabled() { return mCtxMgr != NULL; }

	private:
		bool					mInstanced;
		bool					mFading;
		PxReal					mFadingPeriod;
		PxU32					mMaxParticles;
		RendererVertexBuffer*	mVertexBuffer[NUM_VERTEX_BUFFERS];
		RendererInstanceBuffer*	mInstanceBuffer;
		RendererIndexBuffer*	mIndexBuffer;

		PxCudaContextManager*	mCtxMgr;

		void					initializeVertexBuffer(PxU32 color);
		void					initializeBuffersAsSimpleConvex(PxU32 color, PxReal scaleFactor);
		void					initializeInstanceBuffer();
	};

} // namespace SampleRenderer

#endif
