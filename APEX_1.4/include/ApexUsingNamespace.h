/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef APEX_USING_NAMESPACE_H
#define APEX_USING_NAMESPACE_H

#include "foundation/Px.h"
#include "ApexDefs.h"

#define FORWARD_DECLARATION_AND_USING(T, X) \
	namespace physx \
	{ \
		T X ; \
	}; \

FORWARD_DECLARATION_AND_USING(class,	PxActor);
FORWARD_DECLARATION_AND_USING(class,	PxBase);
FORWARD_DECLARATION_AND_USING(class,	PxBoxGeometry);
FORWARD_DECLARATION_AND_USING(class,	PxCapsuleGeometry);
FORWARD_DECLARATION_AND_USING(class,	PxCloth);
FORWARD_DECLARATION_AND_USING(class,	PxCooking);
FORWARD_DECLARATION_AND_USING(struct,	PxConvexFlag);
FORWARD_DECLARATION_AND_USING(class,	PxConvexMesh);
FORWARD_DECLARATION_AND_USING(class,	PxConvexMeshGeometry);
FORWARD_DECLARATION_AND_USING(struct,	PxDebugPoint);
FORWARD_DECLARATION_AND_USING(struct,	PxDebugLine);
FORWARD_DECLARATION_AND_USING(struct,	PxDebugTriangle);
FORWARD_DECLARATION_AND_USING(struct,	PxDebugText);
FORWARD_DECLARATION_AND_USING(struct,	PxFilterData);
FORWARD_DECLARATION_AND_USING(struct,	PxFilterFlag);
FORWARD_DECLARATION_AND_USING(class,	PxGeometry);
FORWARD_DECLARATION_AND_USING(struct,	PxGeometryType);
FORWARD_DECLARATION_AND_USING(class,	PxJoint);
FORWARD_DECLARATION_AND_USING(class,	PxMaterial);
FORWARD_DECLARATION_AND_USING(struct,	PxPairFlag);
FORWARD_DECLARATION_AND_USING(class,	PxParticleBase);
FORWARD_DECLARATION_AND_USING(class,	PxParticleFluid);
FORWARD_DECLARATION_AND_USING(class,	PxParticleSystem);
FORWARD_DECLARATION_AND_USING(class,	PxPhysics);
FORWARD_DECLARATION_AND_USING(struct,	PxQueryFilterData);
FORWARD_DECLARATION_AND_USING(struct,	PxQueryFlag);
FORWARD_DECLARATION_AND_USING(struct,	PxQueryHit);
FORWARD_DECLARATION_AND_USING(struct,	PxQueryHitType);
FORWARD_DECLARATION_AND_USING(class,	PxRenderBuffer);
FORWARD_DECLARATION_AND_USING(class,	PxRigidActor);
FORWARD_DECLARATION_AND_USING(class,	PxRigidBody);
FORWARD_DECLARATION_AND_USING(struct,	PxRigidBodyFlag);
FORWARD_DECLARATION_AND_USING(class,	PxRigidDynamic);
FORWARD_DECLARATION_AND_USING(class,	PxSimulationEventCallback);
FORWARD_DECLARATION_AND_USING(class,	PxContactModifyCallback);
FORWARD_DECLARATION_AND_USING(class,	PxScene);
FORWARD_DECLARATION_AND_USING(class,	PxShape);
FORWARD_DECLARATION_AND_USING(struct,	PxShapeFlag);
FORWARD_DECLARATION_AND_USING(class,	PxSphereGeometry);
FORWARD_DECLARATION_AND_USING(class,	PxTriangleMesh);
FORWARD_DECLARATION_AND_USING(class,	PxTriangleMeshGeometry);

namespace physx
{
	namespace shdfnd {}
	using namespace shdfnd;

	namespace general_PxIOStream2
	{
		class PxFileBuf;
	}
}

namespace nvidia
{
	namespace apex {}
	using namespace apex;

	using namespace physx;
	using namespace physx::shdfnd;
	using namespace physx::general_PxIOStream2;
};



#endif // APEX_USING_NAMESPACE_H
