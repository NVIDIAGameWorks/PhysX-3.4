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

#ifndef PT_COLLISION_METHODS_H
#define PT_COLLISION_METHODS_H

#include "PxPhysXConfig.h"
#if PX_USE_PARTICLE_SYSTEM_API

#include "foundation/PxVec3.h"
#include "PtConfig.h"
#include "PtCollisionData.h"
#include "PtSpatialHash.h"
#include "PtParticleOpcodeCache.h"
#include "GuGeometryUnion.h"

namespace physx
{

namespace Pt
{

/*!
Collision routines for fluid particles
*/

void collideWithPlane(ParticleCollData* particleCollData, PxU32 numCollData, const Gu::GeometryUnion& planeShape,
                      PxReal proxRadius);

void collideWithConvexPlanes(ParticleCollData& collData, const PxPlane* planes, const PxU32 numPlanes,
                             const PxReal proxRadius);
void collideWithConvexPlanesSIMD(ParticleCollDataV4& collDataV4, const PxPlane* convexPlanes, PxU32 numPlanes,
                                 const PxReal proxRadius);

/**
input scaledPlaneBuf needs a capacity of the number of planes in convexShape
*/
void collideWithConvex(PxPlane* scaledPlaneBuf, ParticleCollData* particleCollData, PxU32 numCollData,
                       const Gu::GeometryUnion& convexShape, const PxReal proxRadius);

void collideWithBox(ParticleCollData* particleCollData, PxU32 numCollData, const Gu::GeometryUnion& boxShape,
                    PxReal proxRadius);

void collideWithCapsule(ParticleCollData* particleCollData, PxU32 numCollData, const Gu::GeometryUnion& capsuleShape,
                        PxReal proxRadius);

void collideWithSphere(ParticleCollData* particleCollData, PxU32 numCollData, const Gu::GeometryUnion& sphereShape,
                       PxReal proxRadius);

void collideCellsWithStaticMesh(ParticleCollData* particleCollData, const LocalCellHash& localCellHash,
                                const Gu::GeometryUnion& meshShape, const PxTransform& world2Shape,
                                const PxTransform& shape2World, PxReal cellSize, PxReal collisionRange,
                                PxReal proxRadius, const PxVec3& packetCorner);

void collideWithStaticMesh(PxU32 numParticles, ParticleCollData* particleCollData, ParticleOpcodeCache* opcodeCaches,
                           const Gu::GeometryUnion& meshShape, const PxTransform& world2Shape,
                           const PxTransform& shape2World, PxReal cellSize, PxReal collisionRange, PxReal proxRadius);

void collideWithStaticHeightField(ParticleCollData* particleCollData, PxU32 numCollData,
                                  const Gu::GeometryUnion& heightFieldShape, PxReal proxRadius,
                                  const PxTransform& shape2World);

} // namespace Pt
} // namespace physx

#endif // PX_USE_PARTICLE_SYSTEM_API
#endif // PT_COLLISION_METHODS_H
