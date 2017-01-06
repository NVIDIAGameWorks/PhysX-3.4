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


#include "PxConvexMeshExt.h"
#include "PxConvexMeshGeometry.h"
#include "PxConvexMesh.h"
#include "foundation/PxTransform.h"

using namespace physx;

static const PxReal gEpsilon = .01f;

PxU32 physx::PxFindFaceIndex(const PxConvexMeshGeometry& convexGeom, const PxTransform& pose, 
	const PxVec3& impactPos, const PxVec3& unitDir)
{
	PX_ASSERT(unitDir.isFinite());
	PX_ASSERT(unitDir.isNormalized());
	PX_ASSERT(impactPos.isFinite());
	PX_ASSERT(pose.isFinite());

	const PxVec3 impact = impactPos - unitDir * gEpsilon;

	const PxVec3 localPoint = pose.transformInv(impact);
	const PxVec3 localDir = pose.rotateInv(unitDir);

	// Create shape to vertex scale transformation matrix
	const PxMeshScale& meshScale = convexGeom.scale;
	const PxMat33 rot(meshScale.rotation);
	PxMat33 shape2VertexSkew = rot.getTranspose();
	const PxMat33 diagonal = PxMat33::createDiagonal(PxVec3(1.0f / meshScale.scale.x, 1.0f / meshScale.scale.y, 1.0f / meshScale.scale.z));
	shape2VertexSkew = shape2VertexSkew * diagonal;
	shape2VertexSkew = shape2VertexSkew * rot;

	const PxU32 nbPolys = convexGeom.convexMesh->getNbPolygons();
	PxU32 minIndex = 0;
	PxReal minD = PX_MAX_REAL;
	for (PxU32 j = 0; j < nbPolys; j++)
	{
		PxHullPolygon hullPolygon;
		convexGeom.convexMesh->getPolygonData(j, hullPolygon);
		
		// transform hull plane into shape space
		PxPlane plane;
		const PxVec3 tmp = shape2VertexSkew.transformTranspose(PxVec3(hullPolygon.mPlane[0],hullPolygon.mPlane[1],hullPolygon.mPlane[2]));
		const PxReal denom = 1.0f / tmp.magnitude();
		plane.n = tmp * denom;
		plane.d = hullPolygon.mPlane[3] * denom;

		PxReal d = plane.distance(localPoint);
		if (d < 0.0f)
			continue;

		const PxReal tweak = plane.n.dot(localDir) * gEpsilon;
		d += tweak;

		if (d < minD)
		{
			minIndex = j;
			minD = d;
		}
	}
	return minIndex;
}

