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

// ****************************************************************************
// This snippet illustrates simple use of joints in physx
//
// It creates a chain of objects joined by limited spherical joints, a chain
// joined by fixed joints which is breakable, and a chain of damped D6 joints
// ****************************************************************************

#include <ctype.h>

#include "PxPhysicsAPI.h"

#include "../SnippetCommon/SnippetPrint.h"
#include "../SnippetCommon/SnippetPVD.h"
#include "../SnippetUtils/SnippetUtils.h"


using namespace physx;

PxDefaultAllocator		gAllocator;
PxDefaultErrorCallback	gErrorCallback;

PxFoundation*			gFoundation = NULL;
PxPhysics*				gPhysics	= NULL;

PxDefaultCpuDispatcher*	gDispatcher = NULL;
PxScene*				gScene		= NULL;

PxMaterial*				gMaterial	= NULL;

PxPvd*                  gPvd        = NULL;

PxReal chainZ = 10.0f;

PxRigidDynamic* createDynamic(const PxTransform& t, const PxGeometry& geometry, const PxVec3& velocity=PxVec3(0))
{
	PxRigidDynamic* dynamic = PxCreateDynamic(*gPhysics, t, geometry, *gMaterial, 10.0f);
	dynamic->setAngularDamping(0.5f);
	dynamic->setLinearVelocity(velocity);
	gScene->addActor(*dynamic);
	return dynamic;
}

// spherical joint limited to an angle of at most pi/4 radians (45 degrees)
PxJoint* createLimitedSpherical(PxRigidActor* a0, const PxTransform& t0, PxRigidActor* a1, const PxTransform& t1)
{
	PxSphericalJoint* j = PxSphericalJointCreate(*gPhysics, a0, t0, a1, t1);
	j->setLimitCone(PxJointLimitCone(PxPi/4, PxPi/4, 0.05f));
	j->setSphericalJointFlag(PxSphericalJointFlag::eLIMIT_ENABLED, true);
	return j;
}

// revolute joint limited to an angle of at most pi/4 radians (45 degrees)

// fixed, breakable joint
PxJoint* createBreakableFixed(PxRigidActor* a0, const PxTransform& t0, PxRigidActor* a1, const PxTransform& t1)
{
	PxFixedJoint* j = PxFixedJointCreate(*gPhysics, a0, t0, a1, t1);
	j->setBreakForce(1000, 100000);	
	j->setConstraintFlag(PxConstraintFlag::eDRIVE_LIMITS_ARE_FORCES, true);
	j->setConstraintFlag(PxConstraintFlag::eDISABLE_PREPROCESSING, true);
	return j;
}

// D6 joint with a spring maintaining its position
PxJoint* createDampedD6(PxRigidActor* a0, const PxTransform& t0, PxRigidActor* a1, const PxTransform& t1)
{
	PxD6Joint* j = PxD6JointCreate(*gPhysics, a0, t0, a1, t1);
	j->setMotion(PxD6Axis::eSWING1, PxD6Motion::eFREE);
	j->setMotion(PxD6Axis::eSWING2, PxD6Motion::eFREE);
	j->setMotion(PxD6Axis::eTWIST, PxD6Motion::eFREE);
	j->setDrive(PxD6Drive::eSLERP, PxD6JointDrive(0, 1000, FLT_MAX, true));
	return j;
}

typedef PxJoint* (*JointCreateFunction)(PxRigidActor* a0, const PxTransform& t0, PxRigidActor* a1, const PxTransform& t1);

// create a chain rooted at the origin and extending along the x-axis, all transformed by the argument t.

void createChain(const PxTransform& t, PxU32 length, const PxGeometry& g, PxReal separation, JointCreateFunction createJoint)
{
	PxVec3 offset(separation/2, 0, 0);
	PxTransform localTm(offset);
	PxRigidDynamic* prev = NULL;

	for(PxU32 i=0;i<length;i++)
	{
		PxRigidDynamic* current = PxCreateDynamic(*gPhysics, t*localTm, g, *gMaterial, 1.0f);
		(*createJoint)(prev, prev ? PxTransform(offset) : t, current, PxTransform(-offset));
		gScene->addActor(*current);
		prev = current;
		localTm.p.x += separation;
	}
}

void initPhysics(bool interactive)
{
	PX_UNUSED(interactive);
	gFoundation = PxCreateFoundation(PX_FOUNDATION_VERSION, gAllocator, gErrorCallback);
	gPvd = PxCreatePvd(*gFoundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
	gPvd->connect(*transport,PxPvdInstrumentationFlag::eALL);

	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(),true, gPvd);
	PxInitExtensions(*gPhysics, gPvd);

	PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	gDispatcher = PxDefaultCpuDispatcherCreate(2);
	sceneDesc.cpuDispatcher	= gDispatcher;
	sceneDesc.filterShader	= PxDefaultSimulationFilterShader;
	gScene = gPhysics->createScene(sceneDesc);

	PxPvdSceneClient* pvdClient = gScene->getScenePvdClient();
	if(pvdClient)
	{
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}

	gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);

	PxRigidStatic* groundPlane = PxCreatePlane(*gPhysics, PxPlane(0,1,0,0), *gMaterial);
	gScene->addActor(*groundPlane);

	createChain(PxTransform(PxVec3(0.0f, 20.0f, 0.0f)), 5, PxBoxGeometry(2.0f, 0.5f, 0.5f), 4.0f, createLimitedSpherical);
	createChain(PxTransform(PxVec3(0.0f, 20.0f, -10.0f)), 5, PxBoxGeometry(2.0f, 0.5f, 0.5f), 4.0f, createBreakableFixed);
	createChain(PxTransform(PxVec3(0.0f, 20.0f, -20.0f)), 5, PxBoxGeometry(2.0f, 0.5f, 0.5f), 4.0f, createDampedD6);
}

void stepPhysics(bool interactive)
{
	PX_UNUSED(interactive);
	gScene->simulate(1.0f/60.0f);
	gScene->fetchResults(true);
}
	
void cleanupPhysics(bool interactive)
{
	PX_UNUSED(interactive);
	gScene->release();
	gDispatcher->release();
	gPhysics->release();
	PxPvdTransport* transport = gPvd->getTransport();
	gPvd->release();
	transport->release();
	PxCloseExtensions();
	gFoundation->release();
	
	printf("SnippetJoint done.\n");
}

void keyPress(unsigned char key, const PxTransform& camera)
{
	switch(toupper(key))
	{
	case ' ':	createDynamic(camera, PxSphereGeometry(3.0f), camera.rotate(PxVec3(0,0,-1))*200);	break;
	}
}

int snippetMain(int, const char*const*)
{
#ifdef RENDER_SNIPPET
	extern void renderLoop();
	renderLoop();
#else
	static const PxU32 frameCount = 100;
	initPhysics(false);
	for(PxU32 i=0; i<frameCount; i++)
		stepPhysics(false);
	cleanupPhysics(false);
#endif

	return 0;
}
