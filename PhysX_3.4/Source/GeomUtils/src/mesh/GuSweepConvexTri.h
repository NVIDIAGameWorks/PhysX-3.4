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

#ifndef GU_SWEEP_CONVEX_TRI
#define GU_SWEEP_CONVEX_TRI

#include "GuVecTriangle.h"
#include "GuVecConvexHull.h"
#include "GuConvexMesh.h"
#include "PxConvexMeshGeometry.h"
#include "GuGJKRaycast.h"

// return true if hit, false if no hit
static PX_FORCE_INLINE bool sweepConvexVsTriangle(
	const PxVec3& v0, const PxVec3& v1, const PxVec3& v2,
	ConvexHullV& convexHull, const Ps::aos::PsMatTransformV& meshToConvex, const Ps::aos::PsTransformV& convexTransfV,
	const Ps::aos::Vec3VArg convexSpaceDir, const PxVec3& unitDir, const PxVec3& meshSpaceUnitDir,
	const Ps::aos::FloatVArg fullDistance, PxReal shrunkDistance,
	PxSweepHit& hit, bool isDoubleSided, const PxReal inflation, bool& initialOverlap, PxU32 faceIndex)
{
	using namespace Ps::aos;
	// Create triangle normal
	const PxVec3 denormalizedNormal = (v1 - v0).cross(v2 - v1);

	// Backface culling
	// PT: WARNING, the test is reversed compared to usual because we pass -unitDir to this function
	const bool culled = !isDoubleSided && (denormalizedNormal.dot(meshSpaceUnitDir) <= 0.0f);
	if(culled)
		return false;

	const Vec3V zeroV = V3Zero();
	const FloatV zero = FZero();

	const Vec3V p0 = V3LoadU(v0); // in mesh local space
	const Vec3V	p1 = V3LoadU(v1);
	const Vec3V p2 = V3LoadU(v2);

	// transform triangle verts from mesh local to convex local space
	TriangleV triangleV(meshToConvex.transform(p0), meshToConvex.transform(p1), meshToConvex.transform(p2));

	FloatV toi;
	Vec3V closestA,normal;

	LocalConvex<TriangleV> convexA(triangleV);
	LocalConvex<ConvexHullV> convexB(convexHull);
	const Vec3V initialSearchDir = V3Sub(triangleV.getCenter(), convexHull.getCenter());
	// run GJK raycast
	// sweep triangle in convex local space vs convex, closestA will be the impact point in convex local space
	const bool gjkHit = gjkRaycastPenetration<LocalConvex<TriangleV>, LocalConvex<ConvexHullV> >(
		convexA, convexB, initialSearchDir, zero, zeroV, convexSpaceDir, toi, normal, closestA, inflation, false);
	if(!gjkHit)
		return false;

	const FloatV minDist = FLoad(shrunkDistance);
	const Vec3V destWorldPointA = convexTransfV.transform(closestA);
	const Vec3V destNormal = V3Normalize(convexTransfV.rotate(normal));

	if(FAllGrtrOrEq(zero, toi))
	{
		initialOverlap	= true;	// PT: TODO: redundant with hit distance, consider removing
		return setInitialOverlapResults(hit, unitDir, faceIndex);
	}

	const FloatV dist = FMul(toi, fullDistance); // scale the toi to original full sweep distance
	if(FAllGrtr(minDist, dist)) // is current dist < minDist?
	{
		hit.faceIndex	= faceIndex;
		hit.flags		= PxHitFlag::eDISTANCE | PxHitFlag::ePOSITION | PxHitFlag::eNORMAL | PxHitFlag::eFACE_INDEX;
		V3StoreU(destWorldPointA, hit.position);
		V3StoreU(destNormal, hit.normal);
		FStore(dist, &hit.distance);
		return true; // report a hit
	}
	return false; // report no hit
}

#endif
