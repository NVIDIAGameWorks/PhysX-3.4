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

#ifndef VEHICLE_COOKING_H
#define VEHICLE_COOKING_H

#include "PxPhysicsAPI.h"

class RAWMesh;
class GLMesh;

physx::PxConvexMesh* createConvexMesh(const physx::PxVec3* verts, const physx::PxU32 numVerts, physx::PxPhysics& physics, physx::PxCooking& cooking);
physx::PxConvexMesh* createCuboidConvexMesh(const physx::PxVec3& halfExtents, physx::PxPhysics& physics, physx::PxCooking& cooking);
physx::PxConvexMesh* createWedgeConvexMesh(const physx::PxVec3& halfExtents, physx::PxPhysics& physics, physx::PxCooking& cooking);
physx::PxConvexMesh* createCylinderConvexMesh(const physx::PxF32 width, const physx::PxF32 radius, const physx::PxU32 numCirclePoints, physx::PxPhysics& physics, physx::PxCooking& cooking);
physx::PxConvexMesh* createSquashedCuboidMesh(const physx::PxF32 baseLength, const physx::PxF32 baseDepth, const physx::PxF32 height1, const physx::PxF32 height2, physx::PxPhysics& physics, physx::PxCooking& cooking);
physx::PxConvexMesh* createPrismConvexMesh(const physx::PxF32 baseLength, const physx::PxF32 baseDepth, const physx::PxF32 height, physx::PxPhysics& physics, physx::PxCooking& cooking);
physx::PxConvexMesh* createWheelConvexMesh(const physx::PxVec3* verts, const physx::PxU32 numVerts, physx::PxPhysics& physics, physx::PxCooking& cooking);
physx::PxConvexMesh* createChassisConvexMesh(const physx::PxVec3* verts, const physx::PxU32 numVerts, physx::PxPhysics& physics, physx::PxCooking& cooking);

GLMesh* createRenderMesh(const RAWMesh& data);


#endif //VEHICLE_COOKING_H
