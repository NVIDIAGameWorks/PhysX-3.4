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
// This snippet demonstrates the use of articulations.
// ****************************************************************************

#include <ctype.h>
#include <vector>

#include "PxPhysicsAPI.h"

#include "../SnippetUtils/SnippetUtils.h"
#include "../SnippetCommon/SnippetPrint.h"
#include "../SnippetCommon/SnippetPVD.h"

using namespace physx;

PxDefaultAllocator		gAllocator;
PxDefaultErrorCallback	gErrorCallback;

PxFoundation*			gFoundation		= NULL;
PxPhysics*				gPhysics		= NULL;

PxDefaultCpuDispatcher*	gDispatcher		= NULL;
PxScene*				gScene			= NULL;

PxMaterial*				gMaterial		= NULL;

PxPvd*                  gPvd			= NULL;

PxArticulation*			gArticulation	= NULL;

void initPhysics(bool /*interactive*/)
{
	gFoundation = PxCreateFoundation(PX_FOUNDATION_VERSION, gAllocator, gErrorCallback);
	gPvd = PxCreatePvd(*gFoundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
	gPvd->connect(*transport,PxPvdInstrumentationFlag::eALL);

	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), true, gPvd);
	PxInitExtensions(*gPhysics, gPvd);

	PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	
	PxU32 numCores = SnippetUtils::getNbPhysicalCores();
	gDispatcher = PxDefaultCpuDispatcherCreate(numCores == 0 ? 0 : numCores - 1);
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

	gArticulation = gPhysics->createArticulation();

	// Stabilization can create artefacts on jointed objects so we just disable it
	gArticulation->setStabilizationThreshold(0.0f);

	gArticulation->setMaxProjectionIterations(16);
	gArticulation->setSeparationTolerance(0.001f);

	const float scale = 0.25f;
	const float radius = 0.5f*scale;
	const float halfHeight = 1.0f*scale;
	const PxU32 nbCapsules = 40;
	const float capsuleMass = 1.0f;

	const PxVec3 initPos(0.0f, 24.0f, 0.0f);
	PxVec3 pos = initPos;
	PxShape* capsuleShape = gPhysics->createShape(PxCapsuleGeometry(radius, halfHeight), *gMaterial);
	PxArticulationLink* firstLink = NULL;
	PxArticulationLink* parent = NULL;

	const bool overlappingLinks = true;	// Change this for another kind of rope

	// Create rope
	for(PxU32 i=0;i<nbCapsules;i++)
	{
		PxArticulationLink* link = gArticulation->createLink(parent, PxTransform(pos));
		if(!firstLink)
			firstLink = link;

		link->attachShape(*capsuleShape);
		PxRigidBodyExt::setMassAndUpdateInertia(*link, capsuleMass);

		PxArticulationJoint* joint = link->getInboundJoint();
		if(joint)	// Will be null for root link
		{
			if(overlappingLinks)
			{
				joint->setParentPose(PxTransform(PxVec3(halfHeight, 0.0f, 0.0f)));
				joint->setChildPose(PxTransform(PxVec3(-halfHeight, 0.0f, 0.0f)));
			}
			else
			{
				joint->setParentPose(PxTransform(PxVec3(radius + halfHeight, 0.0f, 0.0f)));
				joint->setChildPose(PxTransform(PxVec3(-radius - halfHeight, 0.0f, 0.0f)));
			}
		}

		if(overlappingLinks)
			pos.x += (radius + halfHeight*2.0f);
		else
			pos.x += (radius + halfHeight) * 2.0f;
		parent = link;
	}

	// Attach large & heavy box at the end of the rope
	{
		const float boxMass = 50.0f;
		const float boxSize = 1.0f;
		PxShape* boxShape = gPhysics->createShape(PxBoxGeometry(boxSize, boxSize, boxSize), *gMaterial);

		pos.x -= (radius + halfHeight) * 2.0f;
		pos.x += (radius + halfHeight) + boxSize;

		PxArticulationLink* link = gArticulation->createLink(parent, PxTransform(pos));

		link->attachShape(*boxShape);
		PxRigidBodyExt::setMassAndUpdateInertia(*link, boxMass);

		PxArticulationJoint* joint = link->getInboundJoint();
		if(joint)	// Will be null for root link
		{
			joint->setParentPose(PxTransform(PxVec3(radius + halfHeight, 0.0f, 0.0f)));
			joint->setChildPose(PxTransform(PxVec3(-boxSize, 0.0f, 0.0f)));
		}
	}
	gScene->addArticulation(*gArticulation);

	// Attach articulation to static world
	{
		PxShape* anchorShape = gPhysics->createShape(PxSphereGeometry(0.05f), *gMaterial);
		PxRigidStatic* anchor = PxCreateStatic(*gPhysics, PxTransform(initPos), *anchorShape);
		gScene->addActor(*anchor);
		PxSphericalJoint* j = PxSphericalJointCreate(*gPhysics, anchor, PxTransform(PxVec3(0.0f)), firstLink, PxTransform(PxVec3(0.0f)));
		PX_UNUSED(j);
	}

	// Create obstacle
	{
		PxShape* boxShape = gPhysics->createShape(PxBoxGeometry(1.0f, 0.1f, 2.0f), *gMaterial);
		PxRigidStatic* obstacle = PxCreateStatic(*gPhysics, PxTransform(initPos+PxVec3(10.0f, -3.0f, 0.0f)), *boxShape);
		gScene->addActor(*obstacle);
	}
}

void stepPhysics(bool /*interactive*/)
{
	gScene->simulate(1.0f/60.0f);
	gScene->fetchResults(true);
}
	
void cleanupPhysics(bool /*interactive*/)
{
	gArticulation->release();
	gScene->release();
	gDispatcher->release();
	gPhysics->release();	
	PxPvdTransport* transport = gPvd->getTransport();
	gPvd->release();
	transport->release();
	PxCloseExtensions();  
	gFoundation->release();

	printf("SnippetArticulation done.\n");
}

void keyPress(unsigned char /*key*/, const PxTransform& /*camera*/)
{
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
