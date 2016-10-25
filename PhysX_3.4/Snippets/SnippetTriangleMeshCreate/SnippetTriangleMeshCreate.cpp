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
// Copyright (c) 2008-2016 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

// ****************************************************************************
// This snippet demonstrates the possibilities of triangle mesh creation.
//
// The snippet creates triangle mesh with a different cooking settings 
// and shows how these settings affect the triangle mesh creation speed.
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

PxU32 rand(PxU32 loVal, PxU32 hiVal)
{
	return loVal + PxU32(rand()%(hiVal - loVal));
}

void indexToCoord(PxU32& x, PxU32& z, PxU32 index, PxU32 w)
{
	x = index % w;
	z = index / w;
}

// Creates a random terrain data.
void createRandomTerrain(const PxVec3& origin, PxU32 numRows, PxU32 numColumns,
	PxReal cellSizeRow, PxReal cellSizeCol, PxReal heightScale,
	PxVec3*& vertices, PxU32*& indices)
{
	PxU32 numX = (numColumns + 1);
	PxU32 numZ = (numRows + 1);
	PxU32 numVertices = numX*numZ;
	PxU32 numTriangles = numRows*numColumns * 2;

	if (vertices == NULL)
		vertices = new PxVec3[numVertices];
	if (indices == NULL)
		indices = new PxU32[numTriangles * 3];

	PxU32 currentIdx = 0;
	for (PxU32 i = 0; i <= numRows; i++)
	{
		for (PxU32 j = 0; j <= numColumns; j++)
		{
			PxVec3 v(origin.x + PxReal(j)*cellSizeRow, origin.y, origin.z + PxReal(i)*cellSizeCol);
			vertices[currentIdx++] = v;
		}
	}

	currentIdx = 0;
	for (PxU32 i = 0; i < numRows; i++)
	{
		for (PxU32 j = 0; j < numColumns; j++)
		{
			PxU32 base = (numColumns + 1)*i + j;
			indices[currentIdx++] = base + 1;
			indices[currentIdx++] = base;
			indices[currentIdx++] = base + numColumns + 1;
			indices[currentIdx++] = base + numColumns + 2;
			indices[currentIdx++] = base + 1;
			indices[currentIdx++] = base + numColumns + 1;
		}
	}

	for (PxU32 i = 0; i < numVertices; i++)
	{
		PxVec3& v = vertices[i];
		v.y += heightScale * rand(-1.0f, 1.0f);
	}
}

// Creates a triangle mesh with different settings.
template <bool skipMeshCleanup, bool skipEdgeData, bool inserted, bool cookingPerformance>
void createTriangleMesh(PxU32 numVertices, const PxVec3* vertices, PxU32 numTriangles, const PxU32* indices)
{
	PxU64 startTime = SnippetUtils::getCurrentTimeCounterValue();

	PxTriangleMeshDesc meshDesc;
	meshDesc.points.count = numVertices;
	meshDesc.points.data = vertices;
	meshDesc.points.stride = sizeof(PxVec3);
	meshDesc.triangles.count = numTriangles;
	meshDesc.triangles.data = indices;
	meshDesc.triangles.stride = 3 * sizeof(PxU32);

	PxCookingParams params = gCooking->getParams();
	params.midphaseDesc = PxMeshMidPhase::eBVH33;
	// we suppress the triangle mesh remap table computation to gain some speed, as we will not need it 
	// in this snippet
	params.suppressTriangleMeshRemapTable = true;

	// If DISABLE_CLEAN_MESH is set, the mesh is not cleaned during the cooking. The input mesh must be valid. 
	// The following conditions are true for a valid triangle mesh :
	//  1. There are no duplicate vertices(within specified vertexWeldTolerance.See PxCookingParams::meshWeldTolerance)
	//  2. There are no large triangles(within specified PxTolerancesScale.)
	// It is recommended to run a separate validation check in debug/checked builds, see below.

	if (!skipMeshCleanup)
		params.meshPreprocessParams &= ~static_cast<PxMeshPreprocessingFlags>(PxMeshPreprocessingFlag::eDISABLE_CLEAN_MESH);
	else
		params.meshPreprocessParams |= PxMeshPreprocessingFlag::eDISABLE_CLEAN_MESH;

	// If DISABLE_ACTIVE_EDGES_PREDOCOMPUTE is set, the cooking does not compute the active (convex) edges, and instead 
	// marks all edges as active. This makes cooking faster but can slow down contact generation. This flag may change 
	// the collision behavior, as all edges of the triangle mesh will now be considered active.
	if (!skipEdgeData)
		params.meshPreprocessParams &= ~static_cast<PxMeshPreprocessingFlags>(PxMeshPreprocessingFlag::eDISABLE_ACTIVE_EDGES_PRECOMPUTE);
	else
		params.meshPreprocessParams |= PxMeshPreprocessingFlag::eDISABLE_ACTIVE_EDGES_PRECOMPUTE;

	// The COOKING_PERFORMANCE flag for BVH33 midphase enables a fast cooking path at the expense of somewhat lower quality BVH construction.
	if(params.midphaseDesc.getType() == PxMeshMidPhase::eBVH33)
	{		
		if (cookingPerformance)
			params.midphaseDesc.mBVH33Desc.meshCookingHint = PxMeshCookingHint::eCOOKING_PERFORMANCE;
		else
			params.midphaseDesc.mBVH33Desc.meshCookingHint = PxMeshCookingHint::eSIM_PERFORMANCE;
	}

	gCooking->setParams(params);

#if defined(PX_CHECKED) || defined(PX_DEBUG)
	// If DISABLE_CLEAN_MESH is set, the mesh is not cleaned during the cooking. 
	// We should check the validity of provided triangles in debug/checked builds though.
	if (skipMeshCleanup)
	{
		PX_ASSERT(gCooking->validateTriangleMesh(meshDesc));
	}
#endif // DEBUG


	PxTriangleMesh* triMesh = NULL;

	// The cooked mesh may either be saved to a stream for later loading, or inserted directly into PxPhysics.
	if (inserted)
	{
		triMesh = gCooking->createTriangleMesh(meshDesc, gPhysics->getPhysicsInsertionCallback());
	}
	else
	{
		PxDefaultMemoryOutputStream outBuffer;
		gCooking->cookTriangleMesh(meshDesc, outBuffer);

		PxDefaultMemoryInputData stream(outBuffer.getData(), outBuffer.getSize());
		triMesh = gPhysics->createTriangleMesh(stream);
	}

	// Print the elapsed time for comparison
	PxU64 stopTime = SnippetUtils::getCurrentTimeCounterValue();
	float elapsedTime = SnippetUtils::getElapsedTimeInMilliseconds(stopTime - startTime);
	printf("-----------------------------------------------\n");
	printf("Create triangle mesh with %d triangles: \n",numTriangles);
	cookingPerformance ? printf("\t Cooking performance on\n") : printf("\t Cooking performance off\n");
	inserted ? printf("\t Mesh inserted on\n") : printf("\t Mesh inserted off\n");
	!skipEdgeData ? printf("\t Precompute edge data on\n") : printf("\t Precompute edge data off\n");
	!skipMeshCleanup ? printf("\t Mesh cleanup on\n") : printf("\t Mesh cleanup off\n");	
	printf("Elapsed time in ms: %f \n", double(elapsedTime));

	triMesh->release();
}

void createTriangleMeshes()
{	
	const PxU32 numColumns = 128;
	const PxU32 numRows = 128;
	const PxU32 numVertices = (numColumns + 1)*(numRows + 1);
	const PxU32 numTriangles = numColumns*numRows * 2;

	PxVec3* vertices = new PxVec3[numVertices];
	PxU32* indices = new PxU32[numTriangles * 3];

	createRandomTerrain(PxVec3(0.0f, 0.0f, 0.0f), numRows, numColumns, 1.0f, 1.0f, 1.f, vertices, indices);

	// Create triangle mesh with different settings

	// Favor runtime speed, cleaning the mesh and precomputing active edges. Store the mesh in a stream.
	// These are the default settings, suitable for offline cooking.
	createTriangleMesh<false,false,false,false>(numVertices,vertices,numTriangles,indices);

	// Favor cooking speed, skip mesh cleanup, but precompute active edges. Insert into PxPhysics.
	// These settings are suitable for runtime cooking, although selecting fast cooking may reduce
	// runtime performance of simulation and queries. We still need to ensure the triangles 
	// are valid, so we perform a validation check in debug/checked builds.
	createTriangleMesh<true,false,true,true>(numVertices,vertices,numTriangles,indices);

	// Favor cooking speed, skip mesh cleanup, and don't precompute the active edges. Insert into PxPhysics.
	// This is the fastest possible solution for runtime cooking, but all edges are marked as active, which can
	// further reduce runtime performance, and also affect behavior.
	createTriangleMesh<false,true,true,true>(numVertices,vertices,numTriangles,indices);

	delete [] vertices;
	delete [] indices;
}

void initPhysics()
{
	gFoundation = PxCreateFoundation(PX_FOUNDATION_VERSION, gAllocator, gErrorCallback);
	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(),true);
	gCooking = PxCreateCooking(PX_PHYSICS_VERSION, *gFoundation, PxCookingParams(PxTolerancesScale()));
	
	createTriangleMeshes();
}
	
void cleanupPhysics()
{
	gPhysics->release();
	gCooking->release();
	gFoundation->release();
	
	printf("SnippetTriangleMeshCreate done.\n");
}


int snippetMain(int, const char*const*)
{	
	initPhysics();
	cleanupPhysics();

	return 0;
}
