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
#include "PxMat33.h"
#include "PxStrideIterator.h"

namespace physx
{

template <typename T>
__device__ T* ptrOffset(T* p, PxU32 byteOffset)
{
	return (T*)((unsigned char*)(p) + byteOffset);
}

#if __CUDA_ARCH__ < 200
__device__ PxU32 gOffset;
#else
__device__ __shared__ PxU32 gOffset;
#endif


// copies orientations and positions to the destination vertex
// buffer based on the validityBitmap state
extern "C" __global__ void updateInstancedVB(
	PxVec3* destPositions,
	PxVec3* destRotation0,
	PxVec3* destRotation1,
	PxVec3* destRotation2,
	PxU32 destStride,
	const PxVec4* srcPositions,
	const PxMat33* srcRotations,
	const PxU32* validParticleBitmap,
	PxU32 validParticleRange)
{
	if (!threadIdx.x)
		gOffset = 0;

	__syncthreads();

	if (validParticleRange)
	{
		for (PxU32 w=threadIdx.x; w <= (validParticleRange) >> 5; w+=blockDim.x)
		{
			const PxU32 srcBaseIndex = w << 5;

			// reserve space in the output vertex buffer based on
			// population count of validity bitmap (avoids excess atomic ops)
			PxU32 destIndex = atomicAdd(&gOffset, __popc(validParticleBitmap[w]));
			
			for (PxU32 b=validParticleBitmap[w]; b; b &= b-1) 
			{
				const PxU32 index = srcBaseIndex | __ffs(b)-1;

				const PxU32 offset = destIndex*destStride;

				*ptrOffset(destRotation0, offset) = srcRotations[index].column0;
				*ptrOffset(destRotation1, offset) = srcRotations[index].column1;
				*ptrOffset(destRotation2, offset) = srcRotations[index].column2;

				PxVec3* p = ptrOffset(destPositions, offset);
				p->x = srcPositions[index].x;
				p->y = srcPositions[index].y;
				p->z = srcPositions[index].z;

				++destIndex;
			}
		}
	}
}


// copies positions and alpha to the destination vertex buffer based on 
// validity bitmap and particle life times
extern "C" __global__ void updateBillboardVB(
	PxVec3* destPositions,
	PxU8* destAlphas,
	PxU32 destStride,
	PxF32 fadingPeriod,
	const PxVec4* srcPositions, 
	const PxReal* srcLifetimes,
	const PxU32* validParticleBitmap,
	PxU32 validParticleRange)
{
	if (!threadIdx.x)
		gOffset = 0;

	__syncthreads();

	if (validParticleRange)
	{
		for (PxU32 w=threadIdx.x; w <= (validParticleRange) >> 5; w+=blockDim.x)
		{
			const PxU32 srcBaseIndex = w << 5;

			// reserve space in the output vertex buffer based on
			// population count of validity bitmap (avoids excess atomic ops)
			PxU32 destIndex = atomicAdd(&gOffset, __popc(validParticleBitmap[w]));

			for (PxU32 b=validParticleBitmap[w]; b; b &= b-1) 
			{
				PxU32 index = srcBaseIndex | __ffs(b)-1;

				const PxU32 offset = destIndex*destStride;

				// copy position
				PxVec3* p = ptrOffset(destPositions, offset);
				p->x = srcPositions[index].x;
				p->y = srcPositions[index].y;
				p->z = srcPositions[index].z;

				// update alpha
				if (srcLifetimes)
				{
					PxU8 lifetime = 0;
					if(srcLifetimes[index] >= fadingPeriod)
						lifetime = 255;
					else
					{
						if(srcLifetimes[index] <= 0.0f)
							lifetime = 0; 
						else
							lifetime = static_cast<PxU8>(srcLifetimes[index] * 255 / fadingPeriod);
					}

					destAlphas[destIndex*4] = lifetime;
				}

				++destIndex;
			}
		}
	}
}

}