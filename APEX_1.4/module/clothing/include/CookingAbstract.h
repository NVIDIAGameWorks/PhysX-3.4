/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef COOKING_ABSTRACT_H
#define COOKING_ABSTRACT_H

#include "PsUserAllocated.h"
#include "PxVec3.h"
#include "PsArray.h"
#include "ApexUsingNamespace.h"

namespace NvParameterized
{
class Interface;
}

namespace nvidia
{
namespace clothing
{

namespace ClothingAssetParametersNS
{
struct BoneEntry_Type;
struct ActorEntry_Type;
}

typedef ClothingAssetParametersNS::BoneEntry_Type BoneEntry;
typedef ClothingAssetParametersNS::ActorEntry_Type BoneActorEntry;

using namespace physx::shdfnd;

class CookingAbstract : public nvidia::UserAllocated
{
public:
	CookingAbstract() : mBoneActors(NULL), mNumBoneActors(0), mBoneEntries(NULL), mNumBoneEntries(0), mBoneVertices(NULL), mMaxConvexVertices(256),
		mFreeTempMemoryWhenDone(NULL), mScale(1.0f), mVirtualParticleDensity(0.0f), mSelfcollisionRadius(0.0f)
	{
	}

	virtual ~CookingAbstract()
	{
		if (mFreeTempMemoryWhenDone != NULL)
		{
			PX_FREE(mFreeTempMemoryWhenDone);
			mFreeTempMemoryWhenDone = NULL;
		}
	}

	struct PhysicalMesh
	{
		PhysicalMesh() : meshID(0), isTetrahedral(false), vertices(NULL), numVertices(0), numSimulatedVertices(0), numMaxDistance0Vertices(0), 
						indices(NULL), numIndices(0), numSimulatedIndices(0), largestTriangleArea(0.0f), smallestTriangleArea(0.0f) {}

		uint32_t	meshID;
		bool		isTetrahedral;

		PxVec3*		vertices;
		uint32_t	numVertices;
		uint32_t	numSimulatedVertices;
		uint32_t	numMaxDistance0Vertices;

		uint32_t*	indices;
		uint32_t	numIndices;
		uint32_t	numSimulatedIndices;

		void computeTriangleAreas();
		float largestTriangleArea;
		float smallestTriangleArea;
	};

	void addPhysicalMesh(const PhysicalMesh& physicalMesh);
	void setConvexBones(const BoneActorEntry* boneActors, uint32_t numBoneActors, const BoneEntry* boneEntries, uint32_t numBoneEntries, const PxVec3* boneVertices, uint32_t maxConvexVertices);

	void freeTempMemoryWhenDone(void* memory)
	{
		mFreeTempMemoryWhenDone = memory;
	}
	void setScale(float scale)
	{
		mScale = scale;
	}
	void setVirtualParticleDensity(float density)
	{
		PX_ASSERT(density >= 0.0f);
		PX_ASSERT(density <= 1.0f);
		mVirtualParticleDensity = density;
	}
	void setSelfcollisionRadius(float radius)
	{
		PX_ASSERT(radius >= 0.0f);
		mSelfcollisionRadius = radius;
	}
	void setGravityDirection(const PxVec3& gravityDir)
	{
		mGravityDirection = gravityDir;
		mGravityDirection.normalize();
	}

	virtual NvParameterized::Interface* execute() = 0;

	bool isValid() const;

protected:
	Array<PhysicalMesh>		mPhysicalMeshes;

	const BoneActorEntry*	mBoneActors;
	uint32_t				mNumBoneActors;
	const BoneEntry*		mBoneEntries;
	uint32_t				mNumBoneEntries;
	const PxVec3*			mBoneVertices;
	uint32_t				mMaxConvexVertices;

	void*					mFreeTempMemoryWhenDone;
	float					mScale;

	float					mVirtualParticleDensity;
	float					mSelfcollisionRadius;

	PxVec3					mGravityDirection;
};

}
} // namespace nvidia

#endif // COOKING_ABSTRACT_H
