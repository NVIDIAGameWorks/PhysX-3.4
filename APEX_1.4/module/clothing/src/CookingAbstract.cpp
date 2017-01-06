/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "CookingAbstract.h"



namespace nvidia
{
namespace clothing
{

using namespace nvidia;

void CookingAbstract::PhysicalMesh::computeTriangleAreas()
{
	smallestTriangleArea = largestTriangleArea = 0.0f;

	if (indices == NULL || vertices == NULL)
	{
		return;
	}

	smallestTriangleArea = PX_MAX_F32;

	for (uint32_t i = 0; i < numIndices; i += 3)
	{
		const PxVec3 edge1 = vertices[indices[i + 1]] - vertices[indices[i]];
		const PxVec3 edge2 = vertices[indices[i + 2]] - vertices[indices[i]];
		const float triangleArea = edge1.cross(edge2).magnitude();

		largestTriangleArea = PxMax(largestTriangleArea, triangleArea);
		smallestTriangleArea = PxMin(smallestTriangleArea, triangleArea);
	}
}



void CookingAbstract::addPhysicalMesh(const PhysicalMesh& physicalMesh)
{
	PhysicalMesh physicalMeshCopy = physicalMesh;
	physicalMeshCopy.computeTriangleAreas();
	mPhysicalMeshes.pushBack(physicalMeshCopy);
}



void CookingAbstract::setConvexBones(const BoneActorEntry* boneActors, uint32_t numBoneActors, const BoneEntry* boneEntries,
                                     uint32_t numBoneEntries, const PxVec3* boneVertices, uint32_t maxConvexVertices)
{
	mBoneActors = boneActors;
	mNumBoneActors = numBoneActors;
	mBoneEntries = boneEntries;
	mNumBoneEntries = numBoneEntries;
	mBoneVertices = boneVertices;

	PX_ASSERT(maxConvexVertices <= 256);
	mMaxConvexVertices = maxConvexVertices;
}


bool CookingAbstract::isValid() const
{
	return mPhysicalMeshes.size() > 0;
}

}
}