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
// This snippet creates convex meshes with different cooking settings 
// and shows how these settings affect the convex mesh creation performance and 
// the size of the resulting cooked meshes.
// ****************************************************************************

#include <ctype.h>

#include "PxPhysicsAPI.h"

#include "../SnippetUtils/SnippetUtils.h"

using namespace physx;

PxDefaultAllocator		gAllocator;
PxDefaultErrorCallback	gErrorCallback;

PxFoundation*			gFoundation = NULL;
PxPhysics*				gPhysics	= NULL;
PxCooking*				gCooking	= NULL;

float rand(float loVal, float hiVal)
{
	return loVal + (float(rand())/RAND_MAX)*(hiVal - loVal);
}

template<PxConvexMeshCookingType::Enum convexMeshCookingType, bool directInsertion, PxU32 gaussMapLimit>
void createRandomConvex(PxU32 numVerts, const PxVec3* verts)
{
	PxCookingParams params = gCooking->getParams();

	// If inflation is used, the legacy incremental hull creation algorithm is picked.
	// Without inflation the new default quickhull algorithm is used.

	// Use the new (default) PxConvexMeshCookingType::eQUICKHULL or the legacy PxConvexMeshCookingType::eINFLATION_INCREMENTAL_HULL.
	params.convexMeshCookingType = convexMeshCookingType;

	// If the gaussMapLimit is chosen higher than the number of output vertices, no gauss map is added to the convex mesh data (here 256).
	// If the gaussMapLimit is chosen lower than the number of output vertices, a gauss map is added to the convex mesh data (here 16).
	params.gaussMapLimit = gaussMapLimit;
	gCooking->setParams(params);

	// Setup the convex mesh descriptor
	PxConvexMeshDesc desc;

	// We provide points only, therefore the PxConvexFlag::eCOMPUTE_CONVEX flag must be specified
	desc.points.data = verts;
	desc.points.count = numVerts;
	desc.points.stride = sizeof(PxVec3);
	desc.flags = PxConvexFlag::eCOMPUTE_CONVEX;

	PxU32 meshSize = 0;
	PxConvexMesh* convex = NULL;

	PxU64 startTime = SnippetUtils::getCurrentTimeCounterValue();

	if(directInsertion)
	{
		// Directly insert mesh into PhysX
		convex = gCooking->createConvexMesh(desc, gPhysics->getPhysicsInsertionCallback());
		PX_ASSERT(convex);
	}
	else
	{
		// Serialize the cooked mesh into a stream.
		PxDefaultMemoryOutputStream outStream;
		bool res = gCooking->cookConvexMesh(desc, outStream);
		PX_UNUSED(res);
		PX_ASSERT(res);
		meshSize = outStream.getSize();

		// Create the mesh from a stream.
		PxDefaultMemoryInputData inStream(outStream.getData(), outStream.getSize());
		convex = gPhysics->createConvexMesh(inStream);
		PX_ASSERT(convex);
	}

	// Print the elapsed time for comparison
	PxU64 stopTime = SnippetUtils::getCurrentTimeCounterValue();
	float elapsedTime = SnippetUtils::getElapsedTimeInMilliseconds(stopTime - startTime);
	printf("\t -----------------------------------------------\n");
	printf("\t Create convex mesh with %d triangles: \n", numVerts);
	directInsertion ? printf("\t\t Direct mesh insertion enabled\n") : printf("\t\t Direct mesh insertion disabled\n");
	printf("\t\t Gauss map limit: %d \n", gaussMapLimit);
	printf("\t\t Created hull number of vertices: %d \n", convex->getNbVertices());
	printf("\t\t Created hull number of polygons: %d \n", convex->getNbPolygons());
	printf("\t Elapsed time in ms: %f \n", double(elapsedTime));
	if (!directInsertion)
	{
		printf("\t Mesh size: %d \n", meshSize);
	}

	convex->release();
}

void createConvexMeshes()
{
	const PxU32 numVerts = 64;
	PxVec3* vertices = new PxVec3[numVerts];

	// Prepare random verts
	for(PxU32 i = 0; i < numVerts; i++)
	{
		vertices[i] = PxVec3(rand(-20.0f, 20.0f), rand(-20.0f, 20.0f), rand(-20.0f, 20.0f));
	}

	//// Create convex mesh using the inflation incremental hull algorithm with different settings
	printf("-----------------------------------------------\n");
	printf("Create convex mesh using the inflation incremental hull algorithm: \n\n");

	// The default convex mesh creation serializing to a stream, useful for offline cooking.
	createRandomConvex<PxConvexMeshCookingType::eINFLATION_INCREMENTAL_HULL, false, 16>(numVerts, vertices);

	// The default convex mesh creation without the additional gauss map data.
	createRandomConvex<PxConvexMeshCookingType::eINFLATION_INCREMENTAL_HULL, false, 256>(numVerts, vertices);

	// Convex mesh creation inserting the mesh directly into PhysX. 
	// Useful for runtime cooking.
	createRandomConvex<PxConvexMeshCookingType::eINFLATION_INCREMENTAL_HULL, true, 16>(numVerts, vertices);

	// Convex mesh creation inserting the mesh directly into PhysX, without gauss map data.
	// Useful for runtime cooking.
	createRandomConvex<PxConvexMeshCookingType::eINFLATION_INCREMENTAL_HULL, true, 256>(numVerts, vertices);

	// Create convex mesh using the quickhull algorithm with different settings
	printf("-----------------------------------------------\n");
	printf("Create convex mesh using the quickhull algorithm: \n\n");

	// The default convex mesh creation serializing to a stream, useful for offline cooking.
	createRandomConvex<PxConvexMeshCookingType::eQUICKHULL, false, 16>(numVerts, vertices);

	// The default convex mesh creation without the additional gauss map data.
	createRandomConvex<PxConvexMeshCookingType::eQUICKHULL, false, 256>(numVerts, vertices);

	// Convex mesh creation inserting the mesh directly into PhysX. 
	// Useful for runtime cooking.
	createRandomConvex<PxConvexMeshCookingType::eQUICKHULL, true, 16>(numVerts, vertices);

	// Convex mesh creation inserting the mesh directly into PhysX, without gauss map data.
	// Useful for runtime cooking.
	createRandomConvex<PxConvexMeshCookingType::eQUICKHULL, true, 256>(numVerts, vertices);

	delete [] vertices;
}

void initPhysics()
{
	gFoundation = PxCreateFoundation(PX_FOUNDATION_VERSION, gAllocator, gErrorCallback);
	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(),true);
	gCooking = PxCreateCooking(PX_PHYSICS_VERSION, *gFoundation, PxCookingParams(PxTolerancesScale()));
}
	
void cleanupPhysics()
{
	gPhysics->release();
	gCooking->release();
	gFoundation->release();
	
	printf("SnippetConvexMeshCreate done.\n");
}


int snippetMain(int, const char*const*)
{	
	initPhysics();
	createConvexMeshes();
	cleanupPhysics();

	return 0;
}
