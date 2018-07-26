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
// This snippet illustrates the usage of PxPruningStructure.
//
// It creates a box stack, then prepares a pruning structure. This structure
// together with the actors is serialized into a collection. When the collection
// is added to the scene, the actor's scene query shape AABBs are directly merged
// into the current scene query AABB tree through the precomputed pruning structure.
// This may unbalance the AABB tree but should provide significant speedup in 
// case of large world scenarios where parts get streamed in on the fly.
// ****************************************************************************

#include <ctype.h>
#include <vector>

#include "PxPhysicsAPI.h"

#include "extensions/PxCollectionExt.h"

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

#define MAX_MEMBLOCKS 10
PxU8*					gMemBlocks[MAX_MEMBLOCKS];
PxU32					gMemBlockCount = 0;

PxReal stackZ = 10.0f;

/**
Allocates 128 byte aligned memory block for binary serialized data
Stores pointer to memory in gMemBlocks for later deallocation
*/
void* createAlignedBlock(PxU32 size)
{
	PX_ASSERT(gMemBlockCount < MAX_MEMBLOCKS);
	PxU8* baseAddr = static_cast<PxU8*>(malloc(size + PX_SERIAL_FILE_ALIGN - 1));
	gMemBlocks[gMemBlockCount++] = baseAddr;
	void* alignedBlock = reinterpret_cast<void*>((size_t(baseAddr) + PX_SERIAL_FILE_ALIGN - 1)&~(PX_SERIAL_FILE_ALIGN - 1));
	return alignedBlock;
}

// Create a regular stack, with actors added directly into a scene.
void createStack(const PxTransform& t, PxU32 size, PxReal halfExtent)
{	
	PxShape* shape = gPhysics->createShape(PxBoxGeometry(halfExtent, halfExtent, halfExtent), *gMaterial);
	for(PxU32 i=0; i<size;i++)
	{
		for(PxU32 j=0;j<size-i;j++)
		{
			PxTransform localTm(PxVec3(PxReal(j*2) - PxReal(size-i), PxReal(i*2+1), 0) * halfExtent);
			PxRigidDynamic* body = gPhysics->createRigidDynamic(t.transform(localTm));
			body->attachShape(*shape);
			PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
			gScene->addActor(*body);
		}
	}
	shape->release();
}

// Create a stack where pruning structure is build in runtime and used to merge
// the query shapes into the AABB tree.
void createStackWithRuntimePrunerStructure(const PxTransform& t, PxU32 size, PxReal halfExtent)
{
	std::vector<PxRigidActor*> actors;
	PxShape* shape = gPhysics->createShape(PxBoxGeometry(halfExtent, halfExtent, halfExtent), *gMaterial);
	for (PxU32 i = 0; i < size; i++)
	{
		for (PxU32 j = 0; j < size - i; j++)
		{
			PxTransform localTm(PxVec3(PxReal(j * 2) - PxReal(size - i), PxReal(i * 2 + 1), 0) * halfExtent);
			PxRigidDynamic* body = gPhysics->createRigidDynamic(t.transform(localTm));
			body->attachShape(*shape);
			PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
			// store the actors, will be added later
			actors.push_back(body);
		}
	}
	shape->release();

	// Create pruning structure from given actors.
	PxPruningStructure* ps = gPhysics->createPruningStructure(&actors[0], PxU32(actors.size()));
	// Add actors into a scene together with the precomputed pruning structure.	
	gScene->addActors(*ps);
	ps->release();
}

// Create a stack where pruning structure is build in runtime and then stored into a collection.
// The collection is stored into a stream and loaded into another stream. The loaded collection
// is added to a scene. While the collection is added to the scene the pruning structure is used.
void createStackWithSerializedPrunerStructure(const PxTransform& t, PxU32 size, PxReal halfExtent)
{
	PxCollection* collection = PxCreateCollection();		// collection for all the objects
	PxSerializationRegistry* sr = PxSerialization::createSerializationRegistry(*gPhysics);

	std::vector<PxRigidActor*> actors;
	PxShape* shape = gPhysics->createShape(PxBoxGeometry(halfExtent, halfExtent, halfExtent), *gMaterial);
	for (PxU32 i = 0; i < size; i++)
	{
		for (PxU32 j = 0; j < size - i; j++)
		{
			PxTransform localTm(PxVec3(PxReal(j * 2) - PxReal(size - i), PxReal(i * 2 + 1), 0) * halfExtent);
			PxRigidDynamic* body = gPhysics->createRigidDynamic(t.transform(localTm));
			body->attachShape(*shape);
			PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
			// store the actors, will be added later
			actors.push_back(body);
		}
	}
	collection->add(*shape);	

	// Create pruner structure from given actors.
	PxPruningStructure* ps = gPhysics->createPruningStructure(&actors[0], PxU32(actors.size()));
	// Add the pruning structure into the collection. Adding the pruning structure will automatically
	// add the actors from which the collection was build.
	collection->add(*ps);
	PxSerialization::complete(*collection, *sr);

	// Store the collection into a stream.
	PxDefaultMemoryOutputStream outStream;
	PxSerialization::serializeCollectionToBinary(outStream, *collection, *sr);
	collection->release();

	// Release the used items added to the collection.
	ps->release();
	for (size_t i = 0; i < actors.size(); i++)
	{
		actors[i]->release();
	}	
	shape->release();

	// Load collection from the stream into and input stream.
	PxDefaultMemoryInputData inputStream(outStream.getData(), outStream.getSize());
	void* alignedBlock = createAlignedBlock(inputStream.getLength());
	inputStream.read(alignedBlock, inputStream.getLength());
	PxCollection* collection1 = PxSerialization::createCollectionFromBinary(alignedBlock, *sr);

	// Add collection to the scene.
	gScene->addCollection(*collection1);

	// Release objects in collection, the pruning structure must be released before its actors
	// otherwise actors will still be part of pruning structure
	PxCollectionExt::releaseObjects(*collection1);

	collection1->release();
}

void initPhysics(bool )
{
	gFoundation = PxCreateFoundation(PX_FOUNDATION_VERSION, gAllocator, gErrorCallback);

	gPvd = PxCreatePvd(*gFoundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
	gPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);

	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(),true, gPvd);

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

	// Create a regular stack.
	createStack(PxTransform(PxVec3(0,0,stackZ-=10.0f)), 3, 2.0f);
	// Create a stack using the runtime pruner structure usage.
	createStackWithRuntimePrunerStructure(PxTransform(PxVec3(0,0,stackZ-=10.0f)), 3, 2.0f);
	// Create a stack using the serialized pruner structure usage.
	createStackWithSerializedPrunerStructure(PxTransform(PxVec3(0,0,stackZ-=10.0f)), 3, 2.0f);
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
	
	// Now that the objects have been released, it's safe to release the space they occupy.
	for (PxU32 i = 0; i < gMemBlockCount; i++)
	{
		free(gMemBlocks[i]);
	}
	gMemBlockCount = 0;

	gFoundation->release();
	
	printf("SnippetPrunerSerialization done.\n");
}


int snippetMain(int, const char*const*)
{
	static const PxU32 frameCount = 100;
	initPhysics(false);
	for(PxU32 i=0; i<frameCount; i++)
		stepPhysics(false);
	cleanupPhysics(false);

	return 0;
}
