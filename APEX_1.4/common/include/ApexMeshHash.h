/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef APEX_MESH_HASH_H
#define APEX_MESH_HASH_H

#include "ApexDefs.h"

#include "ApexUsingNamespace.h"
#include "PsUserAllocated.h"
#include "PsArray.h"

#include "PxVec3.h"

namespace nvidia
{
namespace apex
{

struct MeshHashRoot
{
	int32_t first;
	uint32_t timeStamp;
};

struct MeshHashEntry
{
	int32_t next;
	uint32_t itemIndex;
};


class ApexMeshHash : public UserAllocated
{
public:
	ApexMeshHash();
	~ApexMeshHash();

	void   setGridSpacing(float spacing);
	float getGridSpacing()
	{
		return 1.0f / mInvSpacing;
	}
	void reset();
	void add(const PxBounds3& bounds, uint32_t itemIndex);
	void add(const PxVec3& pos, uint32_t itemIndex);

	void query(const PxBounds3& bounds, physx::Array<uint32_t>& itemIndices, int32_t maxIndices = -1);
	void queryUnique(const PxBounds3& bounds, physx::Array<uint32_t>& itemIndices, int32_t maxIndices = -1);

	void query(const PxVec3& pos, physx::Array<uint32_t>& itemIndices, int32_t maxIndices = -1);
	void queryUnique(const PxVec3& pos, physx::Array<uint32_t>& itemIndices, int32_t maxIndices = -1);

	// applied functions, only work if inserted objects are points!
	int32_t getClosestPointNr(const PxVec3* points, uint32_t numPoints, uint32_t pointStride, const PxVec3& pos);

private:
	enum
	{
		HashIndexSize = 170111
	};

	void compressIndices(physx::Array<uint32_t>& itemIndices);
	float mSpacing;
	float mInvSpacing;
	uint32_t mTime;

	inline uint32_t  hashFunction(int32_t xi, int32_t yi, int32_t zi)
	{
		uint32_t h = (uint32_t)((xi * 92837111) ^(yi * 689287499) ^(zi * 283923481));
		return h % HashIndexSize;
	}

	inline void cellCoordOf(const PxVec3& v, int& xi, int& yi, int& zi)
	{
		xi = (int)(v.x * mInvSpacing);
		if (v.x < 0.0f)
		{
			xi--;
		}
		yi = (int)(v.y * mInvSpacing);
		if (v.y < 0.0f)
		{
			yi--;
		}
		zi = (int)(v.z * mInvSpacing);
		if (v.z < 0.0f)
		{
			zi--;
		}
	}

	MeshHashRoot* mHashIndex;
	physx::Array<MeshHashEntry> mEntries;

	physx::Array<uint32_t> mTempIndices;
};

}
} // end namespace nvidia::apex

#endif