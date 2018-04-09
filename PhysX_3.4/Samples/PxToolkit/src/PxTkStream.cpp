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

#include "PxTkStream.h"
#include "foundation/PxAssert.h"
#include "PxTkFile.h"
#include "PxTkNamespaceMangle.h"
#include "PsIntrinsics.h"
#include "foundation/PxMath.h"
#include "PxPhysics.h"
#include "geometry/PxConvexMesh.h"
#include "cooking/PxCooking.h"
#include "foundation/PxBounds3.h"
#include "extensions/PxDefaultStreams.h"

using namespace PxToolkit;
///////////////////////////////////////////////////////////////////////////////

PxTriangleMesh* PxToolkit::createTriangleMesh32(PxPhysics& physics, PxCooking& cooking, const PxVec3* verts, PxU32 vertCount, const PxU32* indices32, PxU32 triCount, bool insert)
{
	PxTriangleMeshDesc meshDesc;
	meshDesc.points.count			= vertCount;
	meshDesc.points.stride			= sizeof(PxVec3);
	meshDesc.points.data			= verts;

	meshDesc.triangles.count		= triCount;
	meshDesc.triangles.stride		= 3*sizeof(PxU32);
	meshDesc.triangles.data			= indices32;

	if(!insert)
	{
		PxDefaultMemoryOutputStream writeBuffer;
		bool status = cooking.cookTriangleMesh(meshDesc, writeBuffer);
		if(!status)
			return NULL;

		PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
		return physics.createTriangleMesh(readBuffer);
	}
	else
	{
		return cooking.createTriangleMesh(meshDesc,physics.getPhysicsInsertionCallback());
	}
}

PxTriangleMesh* PxToolkit::createTriangleMesh32(PxPhysics& physics, PxCooking& cooking, PxTriangleMeshDesc* meshDesc, bool insert)
{
	if(!insert)
	{
		PxDefaultMemoryOutputStream writeBuffer;
		bool status = cooking.cookTriangleMesh(*meshDesc, writeBuffer);
		if(!status)
			return NULL;

		PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
		return physics.createTriangleMesh(readBuffer);
	}
	else
	{
		return cooking.createTriangleMesh(*meshDesc,physics.getPhysicsInsertionCallback());
	}
}

PxConvexMesh* PxToolkit::createConvexMesh(PxPhysics& physics, PxCooking& cooking, const PxVec3* verts, PxU32 vertCount, PxConvexFlags flags)
{
	PxConvexMeshDesc convexDesc;
	convexDesc.points.count		= vertCount;
	convexDesc.points.stride	= sizeof(PxVec3);
	convexDesc.points.data		= verts;
	convexDesc.flags			= flags;
	return cooking.createConvexMesh(convexDesc, physics.getPhysicsInsertionCallback());
}

PxConvexMesh* PxToolkit::createConvexMeshSafe(PxPhysics& physics, PxCooking& cooking, const PxVec3* verts, PxU32 vertCount, PxConvexFlags flags, PxU32 vLimit)
{
	PxConvexMeshDesc convexDesc;
	convexDesc.points.count			= vertCount;
	convexDesc.points.stride		= sizeof(PxVec3);
	convexDesc.points.data			= verts;
	convexDesc.flags				= flags;
	convexDesc.vertexLimit			= vLimit;

	PxDefaultMemoryOutputStream buf;
	bool retVal = cooking.cookConvexMesh(convexDesc, buf);	
	if(!retVal)
	{
		// create AABB
		PxBounds3 aabb;
		aabb.setEmpty();
		for (PxU32 i = 0; i < vertCount; i++)
		{
			aabb.include(verts[i]);
		}

		PxVec3  aabbVerts[8];
		aabbVerts[0] = PxVec3(aabb.minimum.x,aabb.minimum.y,aabb.minimum.z);
		aabbVerts[1] = PxVec3(aabb.maximum.x,aabb.minimum.y,aabb.minimum.z);
		aabbVerts[2] = PxVec3(aabb.maximum.x,aabb.maximum.y,aabb.minimum.z);
		aabbVerts[3] = PxVec3(aabb.minimum.x,aabb.maximum.y,aabb.minimum.z);

		aabbVerts[4] = PxVec3(aabb.minimum.x,aabb.minimum.y,aabb.maximum.z);
		aabbVerts[5] = PxVec3(aabb.maximum.x,aabb.minimum.y,aabb.maximum.z);
		aabbVerts[6] = PxVec3(aabb.maximum.x,aabb.maximum.y,aabb.maximum.z);
		aabbVerts[7] = PxVec3(aabb.minimum.x,aabb.maximum.y,aabb.maximum.z);

		convexDesc.points.count			= 8;
		convexDesc.points.stride		= sizeof(PxVec3);
		convexDesc.points.data			= &aabbVerts[0];
		convexDesc.flags				= flags;

		retVal = cooking.cookConvexMesh(convexDesc, buf);
	}

	if(!retVal)
	{
		return NULL;
	}

	PxDefaultMemoryInputData input(buf.getData(), buf.getSize());
	return physics.createConvexMesh(input);
}


