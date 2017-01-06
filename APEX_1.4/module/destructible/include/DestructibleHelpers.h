/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __DESTRUCTIBLEHELPERS_H__
#define __DESTRUCTIBLEHELPERS_H__

#include "Apex.h"
#include "ApexSDKHelpers.h"
#include "PsUserAllocated.h"
#include "PsMemoryBuffer.h"

#include "DestructibleAsset.h"
#include "NxPhysicsSDK.h"

/*
	For managing mesh cooking at various scales
 */

namespace nvidia
{
namespace destructible
{

class ConvexHullAtScale
{
public:
	ConvexHullAtScale() : scale(1), convexMesh(NULL) {}
	ConvexHullAtScale(const PxVec3& inScale) : scale(inScale), convexMesh(NULL) {}

	PxVec3		scale;
	PxConvexMesh* 		convexMesh;
	physx::Array<uint8_t>	cookedHullData;
};

class MultiScaledConvexHull : public UserAllocated
{
public:
	ConvexHullAtScale* 	getConvexHullAtScale(const PxVec3& scale, float tolerance = 0.0001f)
	{
		// Find mesh at scale.  If not found, create one.
		for (uint32_t index = 0; index < meshes.size(); ++index)
		{
			if (PxVec3equals(meshes[index].scale, scale, tolerance))
			{
				return &meshes[index];
			}
		}
		meshes.insert();
		return new(&meshes.back()) ConvexHullAtScale(scale);
	}

	void	releaseConvexMeshes()
	{
		for (uint32_t index = 0; index < meshes.size(); ++index)
		{
			if (meshes[index].convexMesh != NULL)
			{
				GetApexSDK()->getPhysXSDK()->releaseConvexMesh(*meshes[index].convexMesh);
				meshes[index].convexMesh = NULL;
			}
		}
	}

	Array<ConvexHullAtScale> meshes;
};


}
} // end namespace nvidia

#endif	// __DESTRUCTIBLEHELPERS_H__
