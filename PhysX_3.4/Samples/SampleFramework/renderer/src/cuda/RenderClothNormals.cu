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

#include "PxPhysics.h"
#include "PxVec4.h"
#include "PxVec3.h"
#include "PxVec2.h"

#include "cloth/PxClothTypes.h"

namespace physx
{

// interleaved format must match that used by RendererClothShape
struct Vertex
{
	PxVec3 position;
	PxVec3 normal;
};

namespace
{
	__device__ inline void PxAtomicFloatAdd(float* dest, float x)
	{
#if __CUDA_ARCH__ >= 200
		atomicAdd(dest, x);
#else
		union bits { float f; unsigned int i; };
		bits oldVal, newVal;

		do
		{
			// emulate atomic float add on 1.1 arch
			oldVal.f = *dest;
			newVal.f = oldVal.f + x;
		}
		while (atomicCAS((unsigned int*)dest, oldVal.i, newVal.i) != oldVal.i);
#endif
	}


	__device__ void PxAtomicVec3Add(PxVec3& dest, PxVec3 inc)
	{
		PxAtomicFloatAdd(&dest.x, inc.x);
		PxAtomicFloatAdd(&dest.y, inc.y);
		PxAtomicFloatAdd(&dest.z, inc.z);
	}
}

extern "C" __global__ void computeSmoothNormals(
	const PxClothParticle* particles,
	const PxU16* indices,
	Vertex* vertices,
	PxU32 numTris,
	PxU32 numParticles)
{
	// zero old normals
	for (PxU32 i=threadIdx.x; i < numParticles; i += blockDim.x)
		vertices[i].normal = PxVec3(0.0f);

	__syncthreads();

	for (PxU32 i=threadIdx.x; i < numTris; i += blockDim.x)
	{
		PxU16 a = indices[i*3];
		PxU16 b = indices[i*3+1];
		PxU16 c = indices[i*3+2];

		// calculate face normal
		PxVec3 e1 = particles[b].pos-particles[a].pos;
		PxVec3 e2 = particles[c].pos-particles[a].pos;
		PxVec3 n = e2.cross(e1);

		PxAtomicVec3Add(vertices[a].normal, n);
		PxAtomicVec3Add(vertices[b].normal, n);
		PxAtomicVec3Add(vertices[c].normal, n);
	}

	__syncthreads();

	// update vertex buffer
	for (PxU32 i=threadIdx.x; i < numParticles; i += blockDim.x)
	{
		vertices[i].position = particles[i].pos;
		vertices[i].normal = vertices[i].normal.getNormalized();
	}	
}

}