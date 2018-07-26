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
// Copyright (c) 2008-2018 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

// ****************************************************************************
// This snippet illustrates the use of simple contact reports in combination
// with continuous collision detection (CCD). Furthermore, extra contact report
// data will be requested.
//
// The snippet defines a filter shader function that enables CCD and requests
// touch reports for all pairs, and a contact callback function that saves the 
// contact points and the actor positions at time of impact.
// It configures the scene to use this filter and callback, enables CCD and 
// prints the number of contact points found. If rendering, it renders each 
// contact as a line whose length and direction are defined by the contact 
// impulse (the line points in the opposite direction of the impulse). In
// addition, the path of the fast moving dynamic object is drawn with lines.
// 
// ****************************************************************************

#include <vector>

#include "PxPhysicsAPI.h"

#include "../SnippetUtils/SnippetUtils.h"
#include "../SnippetCommon/SnippetPrint.h"
#include "../SnippetCommon/SnippetPVD.h"


using namespace physx;

PxDefaultAllocator		gAllocator;
PxDefaultErrorCallback	gErrorCallback;

PxFoundation*			gFoundation			= NULL;
PxPhysics*				gPhysics			= NULL;
PxCooking*				gCooking			= NULL;

PxDefaultCpuDispatcher*	gDispatcher			= NULL;
PxScene*				gScene				= NULL;
PxMaterial*				gMaterial			= NULL;

PxTriangleMesh*			gTriangleMesh		= NULL;
PxRigidStatic*			gTriangleMeshActor	= NULL;
PxRigidDynamic*			gSphereActor		= NULL;
PxPvd*                  gPvd                = NULL;

PxU32					gSimStepCount		= 0;

std::vector<PxVec3> gContactPositions;
std::vector<PxVec3> gContactImpulses;
std::vector<PxVec3> gContactSphereActorPositions;


PxFilterFlags contactReportFilterShader(PxFilterObjectAttributes attributes0, PxFilterData filterData0, 
										PxFilterObjectAttributes attributes1, PxFilterData filterData1,
										PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
{
	PX_UNUSED(attributes0);
	PX_UNUSED(attributes1);
	PX_UNUSED(filterData0);
	PX_UNUSED(filterData1);
	PX_UNUSED(constantBlockSize);
	PX_UNUSED(constantBlock);

	//
	// Enable CCD for the pair, request contact reports for initial and CCD contacts.
	// Additionally, provide information per contact point and provide the actor
	// pose at the time of contact.
	//

	pairFlags = PxPairFlag::eCONTACT_DEFAULT
			  | PxPairFlag::eDETECT_CCD_CONTACT
			  | PxPairFlag::eNOTIFY_TOUCH_CCD
			  |	PxPairFlag::eNOTIFY_TOUCH_FOUND
			  | PxPairFlag::eNOTIFY_CONTACT_POINTS
			  | PxPairFlag::eCONTACT_EVENT_POSE;
	return PxFilterFlag::eDEFAULT;
}

class ContactReportCallback: public PxSimulationEventCallback
{
	void onConstraintBreak(PxConstraintInfo* constraints, PxU32 count)	{ PX_UNUSED(constraints); PX_UNUSED(count); }
	void onWake(PxActor** actors, PxU32 count)							{ PX_UNUSED(actors); PX_UNUSED(count); }
	void onSleep(PxActor** actors, PxU32 count)							{ PX_UNUSED(actors); PX_UNUSED(count); }
	void onTrigger(PxTriggerPair* pairs, PxU32 count)					{ PX_UNUSED(pairs); PX_UNUSED(count); }
	void onAdvance(const PxRigidBody*const*, const PxTransform*, const PxU32) {}
	void onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs) 
	{
		std::vector<PxContactPairPoint> contactPoints;

		PxTransform spherePose(PxIdentity);
		PxU32 nextPairIndex = 0xffffffff;

		PxContactPairExtraDataIterator iter(pairHeader.extraDataStream, pairHeader.extraDataStreamSize);
		bool hasItemSet = iter.nextItemSet();
		if (hasItemSet)
			nextPairIndex = iter.contactPairIndex;

		for(PxU32 i=0; i < nbPairs; i++)
		{
			//
			// Get the pose of the dynamic object at time of impact.
			//
			if (nextPairIndex == i)
			{
				if (pairHeader.actors[0]->is<PxRigidDynamic>())
					spherePose = iter.eventPose->globalPose[0];
				else
					spherePose = iter.eventPose->globalPose[1];

				gContactSphereActorPositions.push_back(spherePose.p);

				hasItemSet = iter.nextItemSet();
				if (hasItemSet)
					nextPairIndex = iter.contactPairIndex;
			}

			//
			// Get the contact points for the pair.
			//
			const PxContactPair& cPair = pairs[i];
			if (cPair.events & (PxPairFlag::eNOTIFY_TOUCH_FOUND | PxPairFlag::eNOTIFY_TOUCH_CCD))
			{
				PxU32 contactCount = cPair.contactCount;
				contactPoints.resize(contactCount);
				cPair.extractContacts(&contactPoints[0], contactCount);

				for(PxU32 j=0; j < contactCount; j++)
				{
					gContactPositions.push_back(contactPoints[j].position);
					gContactImpulses.push_back(contactPoints[j].impulse);
				}
			}
		}
	}
};

ContactReportCallback gContactReportCallback;

void initScene()
{
	//
	// Create a static triangle mesh
	//

	PxVec3 vertices[] = {	PxVec3(-8.0f, 0.0f, -3.0f),
							PxVec3(-8.0f, 0.0f, 3.0f),
							PxVec3(0.0f, 0.0f, 3.0f),
							PxVec3(0.0f, 0.0f, -3.0f),
							PxVec3(-8.0f, 10.0f, -3.0f),
							PxVec3(-8.0f, 10.0f, 3.0f),
							PxVec3(0.0f, 10.0f, 3.0f),
							PxVec3(0.0f, 10.0f, -3.0f),
						};

	PxU32 vertexCount = sizeof(vertices) / sizeof(vertices[0]);

	PxU32 triangleIndices[] = {	0, 1, 2,
								0, 2, 3,
								0, 5, 1,
								0, 4, 5,
								4, 6, 5,
								4, 7, 6
							};
	PxU32 triangleCount = (sizeof(triangleIndices) / sizeof(triangleIndices[0])) / 3;

	PxTriangleMeshDesc triangleMeshDesc;
	triangleMeshDesc.points.count = vertexCount;
	triangleMeshDesc.points.data = vertices;
	triangleMeshDesc.points.stride = sizeof(PxVec3);
	triangleMeshDesc.triangles.count = triangleCount;
	triangleMeshDesc.triangles.data = triangleIndices;
	triangleMeshDesc.triangles.stride = 3 * sizeof(PxU32);

	gTriangleMesh = gCooking->createTriangleMesh(triangleMeshDesc, gPhysics->getPhysicsInsertionCallback());

	if (!gTriangleMesh)
		return;

	gTriangleMeshActor = gPhysics->createRigidStatic(PxTransform(PxVec3(0.0f, 1.0f, 0.0f), PxQuat(PxHalfPi / 60.0f, PxVec3(0.0f, 1.0f, 0.0f))));

	if (!gTriangleMeshActor)
		return;

	PxTriangleMeshGeometry triGeom(gTriangleMesh);
	PxShape* triangleMeshShape = PxRigidActorExt::createExclusiveShape(*gTriangleMeshActor, triGeom, *gMaterial);

	if (!triangleMeshShape)
		return;

	gScene->addActor(*gTriangleMeshActor);

	
	//
	// Create a fast moving sphere that will hit and bounce off the static triangle mesh 3 times
	// in one simulation step.
	//

	PxTransform spherePose(PxVec3(0.0f, 5.0f, 1.0f));
	gContactSphereActorPositions.push_back(spherePose.p);
	gSphereActor = gPhysics->createRigidDynamic(spherePose);
	gSphereActor->setRigidBodyFlag(PxRigidBodyFlag::eENABLE_CCD, true);

	if (!gSphereActor)
		return;

	PxSphereGeometry sphereGeom(1.0f);
	PxShape* sphereShape = PxRigidActorExt::createExclusiveShape(*gSphereActor, sphereGeom, *gMaterial);

	if (!sphereShape)
		return;

	PxRigidBodyExt::updateMassAndInertia(*gSphereActor, 1.0f);

	PxReal velMagn = 900.0f;
	PxVec3 vel = PxVec3(-1.0f, -1.0f, 0.0f);
	vel.normalize();
	vel *= velMagn;
	gSphereActor->setLinearVelocity(vel);

	gScene->addActor(*gSphereActor);
}

void initPhysics(bool interactive)
{
	PX_UNUSED(interactive);

	gFoundation = PxCreateFoundation(PX_FOUNDATION_VERSION, gAllocator, gErrorCallback);
	gPvd = PxCreatePvd(*gFoundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
	gPvd->connect(*transport,PxPvdInstrumentationFlag::eALL);

	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(),true,gPvd);
	gCooking = PxCreateCooking(PX_PHYSICS_VERSION, *gFoundation, PxCookingParams(PxTolerancesScale()));
	PxInitExtensions(*gPhysics, gPvd);

	PxU32 numCores = SnippetUtils::getNbPhysicalCores();
	gDispatcher = PxDefaultCpuDispatcherCreate(numCores == 0 ? 0 : numCores - 1);
	PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
	sceneDesc.cpuDispatcher = gDispatcher;
	sceneDesc.gravity = PxVec3(0, 0, 0);
	sceneDesc.filterShader	= contactReportFilterShader;			
	sceneDesc.simulationEventCallback = &gContactReportCallback;
	sceneDesc.flags |= PxSceneFlag::eENABLE_CCD;
	sceneDesc.ccdMaxPasses = 4;

	gScene = gPhysics->createScene(sceneDesc);
	PxPvdSceneClient* pvdClient = gScene->getScenePvdClient();
	if(pvdClient)
	{
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
	}
	gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 1.0f);

	initScene();
}

void stepPhysics(bool interactive)
{
	PX_UNUSED(interactive);

	if (!gSimStepCount)
	{
		gScene->simulate(1.0f/60.0f);
		gScene->fetchResults(true);
		printf("%d contact points\n", PxU32(gContactPositions.size()));

		if (gSphereActor)
			gContactSphereActorPositions.push_back(gSphereActor->getGlobalPose().p);

		gSimStepCount = 1;
	}
}

void cleanupPhysics(bool interactive)
{
	if (gSphereActor)
		gSphereActor->release();

	if (gTriangleMeshActor)
		gTriangleMeshActor->release();

	if (gTriangleMesh)
		gTriangleMesh->release();

	PX_UNUSED(interactive);
	gScene->release();
	gDispatcher->release();
	PxCloseExtensions();
	gCooking->release();
	gPhysics->release();
	PxPvdTransport* transport = gPvd->getTransport();
	gPvd->release();
	transport->release();
	gFoundation->release();
	
	printf("SnippetContactReportCCD done.\n");
}

int snippetMain(int, const char*const*)
{
#ifdef RENDER_SNIPPET
	extern void renderLoop();
	renderLoop();
#else
	initPhysics(false);

	stepPhysics(false);

	cleanupPhysics(false);
#endif

	return 0;
}

