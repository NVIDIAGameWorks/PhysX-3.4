/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef APEX_SNIPPET_COMMON_H
#define APEX_SNIPPET_COMMON_H


#include "PxPhysicsAPI.h"
#include "Apex.h"


using namespace physx;
using namespace nvidia;


PxDefaultAllocator		 gAllocator;
PxDefaultErrorCallback	 gErrorCallback;

PxFoundation*			 gFoundation = NULL;
PxPhysics*				 gPhysics = NULL;
PxCooking*				 gCooking = NULL;

PxDefaultCpuDispatcher*	 gDispatcher = NULL;
PxScene*				 gPhysicsScene = NULL;

PxMaterial*				 gMaterial = NULL;

PxPvd*           gPvd = NULL;

#define PVD_TO_FILE 0

void initPhysX()
{
	gFoundation = PxCreateFoundation(PX_FOUNDATION_VERSION, gAllocator, gErrorCallback);

	gPvd = PxCreatePvd(*gFoundation);

	PxTolerancesScale scale;

	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, scale, true, gPvd);

	PxCookingParams cookingParams(scale);
	gCooking = PxCreateCooking(PX_PHYSICS_VERSION, gPhysics->getFoundation(), cookingParams);

	PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	gDispatcher = PxDefaultCpuDispatcherCreate(2);
	sceneDesc.cpuDispatcher = gDispatcher;
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;
	gPhysicsScene = gPhysics->createScene(sceneDesc);

	PxPvdSceneClient* pvdClient = gPhysicsScene->getScenePvdClient();
	if (pvdClient)
	{
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}
	gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);
}


void releasePhysX()
{
	gPhysicsScene->release();
	gDispatcher->release();
	gPhysics->release();
	gPvd->release();
	gCooking->release();
	gFoundation->release();
}


class DummyRenderResourceManager : public UserRenderResourceManager
{
public:
	virtual UserRenderVertexBuffer*   createVertexBuffer(const UserRenderVertexBufferDesc&) { return NULL;  }
	virtual void                        releaseVertexBuffer(UserRenderVertexBuffer&) {}

	virtual UserRenderIndexBuffer*    createIndexBuffer(const UserRenderIndexBufferDesc&) { return NULL; }
	virtual void                        releaseIndexBuffer(UserRenderIndexBuffer&) {}

	virtual UserRenderBoneBuffer*     createBoneBuffer(const UserRenderBoneBufferDesc&) { return NULL; }
	virtual void                        releaseBoneBuffer(UserRenderBoneBuffer&) {}

	virtual UserRenderInstanceBuffer* createInstanceBuffer(const UserRenderInstanceBufferDesc&) { return NULL; }
	virtual void                        releaseInstanceBuffer(UserRenderInstanceBuffer&) {}

	virtual UserRenderSpriteBuffer*   createSpriteBuffer(const UserRenderSpriteBufferDesc&) { return NULL; }
	virtual void                        releaseSpriteBuffer(UserRenderSpriteBuffer&) {}

	virtual UserRenderSurfaceBuffer*  createSurfaceBuffer(const UserRenderSurfaceBufferDesc&) { return NULL; }
	virtual void                        releaseSurfaceBuffer(UserRenderSurfaceBuffer&) {}

	virtual UserRenderResource*       createResource(const UserRenderResourceDesc&) { return NULL; }
	virtual void                        releaseResource(UserRenderResource&) {}

	virtual uint32_t                       getMaxBonesForMaterial(void*) { return 0; }

	virtual bool getSpriteLayoutData(uint32_t, uint32_t, UserRenderSpriteBufferDesc*)  { return false; }

	virtual bool getInstanceLayoutData(uint32_t, uint32_t, UserRenderInstanceBufferDesc*) { return false; }
};

#endif