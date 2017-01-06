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
  

#include "PxParticleGpu.h"

#if PX_SUPPORT_GPU_PHYSX

#include "NpPhysics.h"
#include "NpPhysicsGpu.h"

using namespace physx;

//-------------------------------------------------------------------------------------------------------------------//

bool PxParticleGpu::createTriangleMeshMirror(const class PxTriangleMesh& triangleMesh, PxCudaContextManager& contextManager)
{
	return NpPhysics::getInstance().getNpPhysicsGpu().createTriangleMeshMirror(triangleMesh, contextManager);
}

void PxParticleGpu::releaseTriangleMeshMirror(const class PxTriangleMesh& triangleMesh, PxCudaContextManager* contextManager)
{
	NpPhysics::getInstance().getNpPhysicsGpu().releaseTriangleMeshMirror(triangleMesh, contextManager);
}

bool PxParticleGpu::createHeightFieldMirror(const class PxHeightField& heightField, PxCudaContextManager& contextManager)
{
	return NpPhysics::getInstance().getNpPhysicsGpu().createHeightFieldMirror(heightField, contextManager);
}

void PxParticleGpu::releaseHeightFieldMirror(const class PxHeightField& heightField, PxCudaContextManager* contextManager)
{
	NpPhysics::getInstance().getNpPhysicsGpu().releaseHeightFieldMirror(heightField, contextManager);
}

bool PxParticleGpu::createConvexMeshMirror(const class PxConvexMesh& convexMesh, PxCudaContextManager& contextManager)
{
	return NpPhysics::getInstance().getNpPhysicsGpu().createConvexMeshMirror(convexMesh, contextManager);
}

void PxParticleGpu::releaseConvexMeshMirror(const class PxConvexMesh& convexMesh, PxCudaContextManager* contextManager)
{
	NpPhysics::getInstance().getNpPhysicsGpu().releaseConvexMeshMirror(convexMesh, contextManager);
}

void PxParticleGpu::setExplicitCudaFlushCountHint(const class PxScene& scene, PxU32 cudaFlushCount)
{
	NpPhysics::getInstance().getNpPhysicsGpu().setExplicitCudaFlushCountHint(scene, cudaFlushCount);
}

bool PxParticleGpu::setTriangleMeshCacheSizeHint(const class PxScene& scene, PxU32 size)
{
	return NpPhysics::getInstance().getNpPhysicsGpu().setTriangleMeshCacheSizeHint(scene, size);
}

PxTriangleMeshCacheStatistics PxParticleGpu::getTriangleMeshCacheStatistics(const class PxScene& scene)
{
	return NpPhysics::getInstance().getNpPhysicsGpu().getTriangleMeshCacheStatistics(scene);
}

//-------------------------------------------------------------------------------------------------------------------//

#endif // PX_SUPPORT_GPU_PHYSX
