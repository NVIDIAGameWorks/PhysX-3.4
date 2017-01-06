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


#include "PxTriangleMeshExt.h"
#include "PxMeshQuery.h"
#include "PxGeometryQuery.h"
#include "PxTriangleMeshGeometry.h"
#include "PxHeightFieldGeometry.h"
#include "PxTriangleMesh.h"
#include "PsAllocator.h"

using namespace physx;

PxMeshOverlapUtil::PxMeshOverlapUtil() : mResultsMemory(mResults), mNbResults(0), mMaxNbResults(256)
{
}

PxMeshOverlapUtil::~PxMeshOverlapUtil()
{
	if(mResultsMemory != mResults)
		PX_FREE(mResultsMemory);
}

PxU32 PxMeshOverlapUtil::findOverlap(const PxGeometry& geom, const PxTransform& geomPose, const PxTriangleMeshGeometry& meshGeom, const PxTransform& meshPose)
{
	bool overflow;
	PxU32 nbTouchedTris = PxMeshQuery::findOverlapTriangleMesh(geom, geomPose, meshGeom, meshPose, mResultsMemory, mMaxNbResults, 0, overflow);

	if(overflow)
	{
		const PxU32 maxNbTris = meshGeom.triangleMesh->getNbTriangles();
		if(!maxNbTris)
		{
			mNbResults = 0;
			return 0;
		}

		if(mMaxNbResults<maxNbTris)
		{
			if(mResultsMemory != mResults)
				PX_FREE(mResultsMemory);

			mResultsMemory = reinterpret_cast<PxU32*>(PX_ALLOC(sizeof(PxU32)*maxNbTris, "PxMeshOverlapUtil::findOverlap"));
			mMaxNbResults = maxNbTris;
		}
		nbTouchedTris = PxMeshQuery::findOverlapTriangleMesh(geom, geomPose, meshGeom, meshPose, mResultsMemory, mMaxNbResults, 0, overflow);
		PX_ASSERT(nbTouchedTris);
		PX_ASSERT(!overflow);
	}
	mNbResults = nbTouchedTris;
	return nbTouchedTris;
}

PxU32 PxMeshOverlapUtil::findOverlap(const PxGeometry& geom, const PxTransform& geomPose, const PxHeightFieldGeometry& hfGeom, const PxTransform& hfPose)
{
	bool overflow = true;
	PxU32 nbTouchedTris = 0;
	do
	{
		nbTouchedTris = PxMeshQuery::findOverlapHeightField(geom, geomPose, hfGeom, hfPose, mResultsMemory, mMaxNbResults, 0, overflow);
		if(overflow)
		{
			const PxU32 maxNbTris = mMaxNbResults * 2;

			if(mResultsMemory != mResults)
				PX_FREE(mResultsMemory);

			mResultsMemory = reinterpret_cast<PxU32*>(PX_ALLOC(sizeof(PxU32)*maxNbTris, "PxMeshOverlapUtil::findOverlap"));
			mMaxNbResults = maxNbTris;
		}
	}while(overflow);

	mNbResults = nbTouchedTris;
	return nbTouchedTris;
}
namespace
{
template<typename MeshGeometry>
bool computeMeshPenetrationT(PxVec3& direction, 
						     PxReal& depth,
							 const PxGeometry& geom, 
							 const PxTransform& geomPose, 
							 const MeshGeometry& meshGeom, 
							 const PxTransform& meshPose, 
							 PxU32 maxIter,
							 PxU32* nbIterOut)
{
	PxU32 nbIter = 0;
	PxTransform pose = geomPose;
	for (;  nbIter < maxIter; nbIter++)
	{
		PxVec3 currentDir;
		PxF32 currentDepth;

		if (!PxGeometryQuery::computePenetration(currentDir, currentDepth, geom, pose, meshGeom, meshPose))
			break;

		pose.p += currentDir * currentDepth;
	}

	if(nbIterOut)
		*nbIterOut = nbIter;

	PxVec3 diff = pose.p - geomPose.p;
	depth = diff.magnitude();	

	if (depth>0)
		direction = diff / depth;

	return nbIter!=0;
}
}

bool physx::PxComputeTriangleMeshPenetration(PxVec3& direction, 
											PxReal& depth,
											const PxGeometry& geom, 
											const PxTransform& geomPose, 
											const PxTriangleMeshGeometry& meshGeom, 
											const PxTransform& meshPose, 
											PxU32 maxIter,
											PxU32* nbIter)
{
	return computeMeshPenetrationT(direction, depth, geom, geomPose, meshGeom, meshPose, maxIter, nbIter);
}


PxVec3 physx::PxComputeMeshPenetration(PxU32 maxIter, const PxGeometry& geom, const PxTransform& geomPose, const PxTriangleMeshGeometry& meshGeom, const PxTransform& meshPose, PxU32& nb)
{
	PxVec3 direction;
	PxReal depth;
	computeMeshPenetrationT(direction, depth, geom, geomPose, meshGeom, meshPose, maxIter, &nb);
	return direction * depth;
}


bool physx::PxComputeHeightFieldPenetration(PxVec3& direction, 
										    PxReal& depth,
											const PxGeometry& geom, 
											const PxTransform& geomPose, 
											const PxHeightFieldGeometry& hfGeom, 
											const PxTransform& meshPose, 
											PxU32 maxIter,
											PxU32* nbIter)
{
	return computeMeshPenetrationT(direction, depth, geom, geomPose, hfGeom, meshPose, maxIter, nbIter);
}


PxVec3 physx::PxComputeHeightFieldPenetration(PxU32 maxIter, const PxGeometry& geom, const PxTransform& geomPose, const PxHeightFieldGeometry& meshGeom, const PxTransform& meshPose, PxU32& nb)
{
	PxVec3 direction;
	PxReal depth;
	computeMeshPenetrationT(direction, depth, geom, geomPose, meshGeom, meshPose, maxIter, &nb);
	return direction * depth;
}

