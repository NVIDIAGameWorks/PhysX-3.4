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
#ifndef PVD_TYPE_NAMES_H
#define PVD_TYPE_NAMES_H
#if PX_SUPPORT_PVD
#include "PxPhysicsAPI.h"
#include "PxPvdObjectModelBaseTypes.h"
#include "PxMetaDataObjects.h"
#include "PxHeightFieldSample.h"

namespace physx
{
namespace Vd
{
struct PvdSqHit;
struct PvdRaycast;
struct PvdOverlap;
struct PvdSweep;

struct PvdHullPolygonData
{
	PxU16 mNumVertices;
	PxU16 mIndexBase;
	PvdHullPolygonData(PxU16 numVert, PxU16 idxBase) : mNumVertices(numVert), mIndexBase(idxBase)
	{
	}
};


struct PxArticulationLinkUpdateBlock
{
	PxTransform GlobalPose;
	PxVec3 LinearVelocity;
	PxVec3 AngularVelocity;
};
struct PxRigidDynamicUpdateBlock : public PxArticulationLinkUpdateBlock
{
	bool IsSleeping;
};

struct PvdContact
{
	PxVec3 point;
	PxVec3 axis;
	const void* shape0;
	const void* shape1;
	PxF32 separation;
	PxF32 normalForce;
	PxU32 internalFaceIndex0;
	PxU32 internalFaceIndex1;
	bool normalForceAvailable;
};

struct PvdPositionAndRadius
{
	PxVec3 position;
	PxF32 radius;
};

} //Vd
} //physx

namespace physx
{
namespace pvdsdk
{

#define DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(type) DEFINE_PVD_TYPE_NAME_MAP(physx::type, "physx3", #type)
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxPhysics)
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxScene)
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxTolerancesScale)
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxTolerancesScaleGeneratedValues)
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxSceneDescGeneratedValues)
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxSceneDesc)
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxSimulationStatistics)
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxSimulationStatisticsGeneratedValues)
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxMaterial)
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxMaterialGeneratedValues)
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxHeightField)
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxHeightFieldDesc)
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxHeightFieldDescGeneratedValues)
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxTriangleMesh)
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxConvexMesh)
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxActor)
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxRigidActor)
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxRigidBody)
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxRigidDynamic)
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxRigidDynamicGeneratedValues)
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxRigidStatic)
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxRigidStaticGeneratedValues)
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxShape)
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxShapeGeneratedValues)
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxGeometry)
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxBoxGeometry)
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxPlaneGeometry)
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxCapsuleGeometry)
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxSphereGeometry)
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxHeightFieldGeometry)
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxTriangleMeshGeometry)
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxConvexMeshGeometry)
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxBoxGeometryGeneratedValues)
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxPlaneGeometryGeneratedValues)
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxCapsuleGeometryGeneratedValues)
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxSphereGeometryGeneratedValues)
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxHeightFieldGeometryGeneratedValues)
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxTriangleMeshGeometryGeneratedValues)
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxConvexMeshGeometryGeneratedValues)
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxHeightFieldSample)
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxConstraint)
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxConstraintGeneratedValues)
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxArticulation)
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxArticulationGeneratedValues)
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxArticulationLink)
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxArticulationLinkGeneratedValues)
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxArticulationJoint)
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxArticulationJointGeneratedValues)
#if PX_USE_PARTICLE_SYSTEM_API
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxParticleBase)
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxParticleSystem)
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxParticleSystemGeneratedValues)
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxParticleFluid)
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxParticleFluidGeneratedValues)
#endif

#if PX_USE_CLOTH_API
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxClothFabric)
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxClothFabricPhase)
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxClothFabricPhaseGeneratedValues)
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxClothFabricGeneratedValues)
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxCloth)
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxClothGeneratedValues)
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxClothParticle)
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxClothStretchConfig)
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxClothTetherConfig)
#endif
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxAggregate)
DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxAggregateGeneratedValues)

#undef DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP

#define DEFINE_NATIVE_PVD_TYPE_MAP(type) DEFINE_PVD_TYPE_NAME_MAP(physx::Vd::type, "physx3", #type)
DEFINE_NATIVE_PVD_TYPE_MAP(PvdHullPolygonData)
DEFINE_NATIVE_PVD_TYPE_MAP(PxRigidDynamicUpdateBlock)
DEFINE_NATIVE_PVD_TYPE_MAP(PxArticulationLinkUpdateBlock)
DEFINE_NATIVE_PVD_TYPE_MAP(PvdContact)
DEFINE_NATIVE_PVD_TYPE_MAP(PvdRaycast)
DEFINE_NATIVE_PVD_TYPE_MAP(PvdSweep)
DEFINE_NATIVE_PVD_TYPE_MAP(PvdOverlap)
DEFINE_NATIVE_PVD_TYPE_MAP(PvdSqHit)
DEFINE_NATIVE_PVD_TYPE_MAP(PvdPositionAndRadius)

#undef DEFINE_NATIVE_PVD_TYPE_MAP


DEFINE_PVD_TYPE_ALIAS(physx::PxFilterData, U32Array4)


}
}

#endif

#endif
