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

#include "GuMidphaseInterface.h"
#include "CmScaling.h"
#include "GuSphere.h"
#include "GuInternal.h"
#include "GuConvexUtilsInternal.h"
#include "GuVecTriangle.h"
#include "GuVecConvexHull.h"
#include "GuConvexMesh.h"
#include "GuGJK.h"
#include "GuSweepSharedTests.h"

using namespace physx;
using namespace Cm;
using namespace Gu;
using namespace physx::shdfnd::aos;

// PT: TODO: remove this function, replace with Midphase:: call at calling sites (TA34704)
bool Gu::checkOverlapAABB_triangleGeom(const PxGeometry& geom, const PxTransform& pose, const PxBounds3& box)
{
	PX_ASSERT(geom.getType() == PxGeometryType::eTRIANGLEMESH);
	const PxTriangleMeshGeometry& meshGeom = static_cast<const PxTriangleMeshGeometry&>(geom);

	// PT: TODO: pass AABB directly to interface
	const Box obb(box.getCenter(), box.getExtents(), PxMat33(PxIdentity));

	TriangleMesh* meshData = static_cast<TriangleMesh*>(meshGeom.triangleMesh);
	return Midphase::intersectBoxVsMesh(obb, *meshData, pose, meshGeom.scale, NULL);
}

bool GeomOverlapCallback_SphereMesh(GU_OVERLAP_FUNC_PARAMS)
{
	PX_ASSERT(geom0.getType()==PxGeometryType::eSPHERE);
	PX_ASSERT(geom1.getType()==PxGeometryType::eTRIANGLEMESH);
	PX_UNUSED(cache);

	const PxSphereGeometry& sphereGeom = static_cast<const PxSphereGeometry&>(geom0);
	const PxTriangleMeshGeometry& meshGeom = static_cast<const PxTriangleMeshGeometry&>(geom1);	

	const Sphere worldSphere(pose0.p, sphereGeom.radius);

	TriangleMesh* meshData = static_cast<TriangleMesh*>(meshGeom.triangleMesh);
	return Midphase::intersectSphereVsMesh(worldSphere, *meshData, pose1, meshGeom.scale, NULL);
}

bool GeomOverlapCallback_CapsuleMesh(GU_OVERLAP_FUNC_PARAMS)
{
	PX_ASSERT(geom0.getType()==PxGeometryType::eCAPSULE);
	PX_ASSERT(geom1.getType()==PxGeometryType::eTRIANGLEMESH);
	PX_UNUSED(cache);

	const PxCapsuleGeometry& capsuleGeom = static_cast<const PxCapsuleGeometry&>(geom0);
	const PxTriangleMeshGeometry& meshGeom = static_cast<const PxTriangleMeshGeometry&>(geom1);

	TriangleMesh* meshData = static_cast<TriangleMesh*>(meshGeom.triangleMesh);

	Capsule capsule;
	getCapsule(capsule, capsuleGeom, pose0);
	return Midphase::intersectCapsuleVsMesh(capsule, *meshData, pose1, meshGeom.scale, NULL);
}

bool GeomOverlapCallback_BoxMesh(GU_OVERLAP_FUNC_PARAMS)
{
	PX_ASSERT(geom0.getType()==PxGeometryType::eBOX);
	PX_ASSERT(geom1.getType()==PxGeometryType::eTRIANGLEMESH);
	PX_UNUSED(cache);

	const PxBoxGeometry& boxGeom = static_cast<const PxBoxGeometry&>(geom0);
	const PxTriangleMeshGeometry& meshGeom = static_cast<const PxTriangleMeshGeometry&>(geom1);

	TriangleMesh* meshData = static_cast<TriangleMesh*>(meshGeom.triangleMesh);

	Box box;
	buildFrom(box, pose0.p, boxGeom.halfExtents, pose0.q);
	return Midphase::intersectBoxVsMesh(box, *meshData, pose1, meshGeom.scale, NULL);
}

///////////////////////////////////////////////////////////////////////////////
struct ConvexVsMeshOverlapCallback : MeshHitCallback<PxRaycastHit>
{
	PsMatTransformV MeshToBoxV;
	Vec3V boxExtents;

	ConvexVsMeshOverlapCallback(
		const ConvexMesh& cm, const PxMeshScale& convexScale, const FastVertex2ShapeScaling& meshScale,
		const PxTransform& tr0, const PxTransform& tr1, bool identityScale, const Box& meshSpaceOBB)
		:
			MeshHitCallback<PxRaycastHit>(CallbackMode::eMULTIPLE),
			mAnyHit			(false),
			mIdentityScale	(identityScale)
	{
		if (!mIdentityScale) // not done in initializer list for performance
			mMeshScale = Ps::aos::Mat33V(
				V3LoadU(meshScale.getVertex2ShapeSkew().column0),
				V3LoadU(meshScale.getVertex2ShapeSkew().column1),
				V3LoadU(meshScale.getVertex2ShapeSkew().column2) );
		using namespace Ps::aos;

		const ConvexHullData* hullData = &cm.getHull();

		const Vec3V vScale0 = V3LoadU_SafeReadW(convexScale.scale);	// PT: safe because 'rotation' follows 'scale' in PxMeshScale
		const QuatV vQuat0 = QuatVLoadU(&convexScale.rotation.x);

		mConvex =  ConvexHullV(hullData, V3Zero(), vScale0, vQuat0, convexScale.isIdentity());
		aToB = PsMatTransformV(tr0.transformInv(tr1));
		
		mIdentityScale = identityScale;

		{
			// Move to AABB space
			Matrix34 MeshToBox;
			computeWorldToBoxMatrix(MeshToBox, meshSpaceOBB);

			const Vec3V base0 = V3LoadU(MeshToBox.m.column0);
			const Vec3V base1 = V3LoadU(MeshToBox.m.column1);
			const Vec3V base2 = V3LoadU(MeshToBox.m.column2);
			const Mat33V matV(base0, base1, base2);
			const Vec3V p  = V3LoadU(MeshToBox.p);
			MeshToBoxV = PsMatTransformV(p, matV);
			boxExtents = V3LoadU(meshSpaceOBB.extents+PxVec3(0.001f));
		}
	}
	virtual ~ConvexVsMeshOverlapCallback()	{}

	virtual PxAgain processHit( // all reported coords are in mesh local space including hit.position
		const PxRaycastHit&, const PxVec3& v0a, const PxVec3& v1a, const PxVec3& v2a, PxReal&, const PxU32*)
	{
		using namespace Ps::aos;
		Vec3V v0 = V3LoadU(v0a), v1 = V3LoadU(v1a), v2 = V3LoadU(v2a);

		// test triangle AABB in box space vs box AABB in box local space
		const Vec3V triV0 = MeshToBoxV.transform(v0); // AP: MeshToBoxV already includes mesh scale so we have to use unscaled verts here
		const Vec3V triV1 = MeshToBoxV.transform(v1);
		const Vec3V triV2 = MeshToBoxV.transform(v2);
		Vec3V triMn = V3Min(V3Min(triV0, triV1), triV2);
		Vec3V triMx = V3Max(V3Max(triV0, triV1), triV2);
		Vec3V negExtents = V3Neg(boxExtents);
		BoolV minSeparated = V3IsGrtr(triMn, boxExtents), maxSeparated = V3IsGrtr(negExtents, triMx);
		BoolV bSeparated = BAnyTrue3(BOr(minSeparated, maxSeparated));
		if (BAllEqTTTT(bSeparated))
			return true; // continue traversal

		if (!mIdentityScale)
		{
			v0 = M33MulV3(mMeshScale, v0);
			v1 = M33MulV3(mMeshScale, v1);
			v2 = M33MulV3(mMeshScale, v2);
		}

		TriangleV triangle(v0, v1, v2);
		Vec3V contactA, contactB, normal;
		FloatV dist;
		GjkStatus status;
		RelativeConvex<TriangleV> convexA(triangle, aToB);
		LocalConvex<ConvexHullV> convexB(mConvex);
		status = gjk(convexA, convexB, aToB.p, FZero(), contactA, contactB, normal, dist);
		if (status == GJK_CONTACT)// || FAllGrtrOrEq(mSqTolerance, sqDist))
		{
			mAnyHit = true;
			return false; // abort traversal
		}
		return true; // continue traversal
	}
	
	ConvexHullV			mConvex;
	PsMatTransformV		aToB;
	Ps::aos::Mat33V		mMeshScale;
	bool				mAnyHit;
	bool				mIdentityScale;

private:
	ConvexVsMeshOverlapCallback& operator=(const ConvexVsMeshOverlapCallback&);
};

// PT: TODO: refactor bits of this with convex-vs-mesh code
bool GeomOverlapCallback_ConvexMesh(GU_OVERLAP_FUNC_PARAMS)
{
	PX_ASSERT(geom0.getType()==PxGeometryType::eCONVEXMESH);
	PX_ASSERT(geom1.getType()==PxGeometryType::eTRIANGLEMESH);
	PX_UNUSED(cache);

	const PxConvexMeshGeometry& convexGeom = static_cast<const PxConvexMeshGeometry&>(geom0);
	const PxTriangleMeshGeometry& meshGeom = static_cast<const PxTriangleMeshGeometry&>(geom1);

	ConvexMesh* cm = static_cast<ConvexMesh*>(convexGeom.convexMesh);
	TriangleMesh* meshData = static_cast<TriangleMesh*>(meshGeom.triangleMesh);

	const bool idtScaleConvex = convexGeom.scale.isIdentity();
	const bool idtScaleMesh = meshGeom.scale.isIdentity();

	FastVertex2ShapeScaling convexScaling;
	if (!idtScaleConvex)
		convexScaling.init(convexGeom.scale);

	FastVertex2ShapeScaling meshScaling;
	if (!idtScaleMesh)
		meshScaling.init(meshGeom.scale);

	const Matrix34 world0(pose0);
	const Matrix34 world1(pose1);

	PX_ASSERT(!cm->getLocalBoundsFast().isEmpty());
	const PxBounds3 hullAABB = cm->getLocalBoundsFast().transformFast(convexScaling.getVertex2ShapeSkew());

	Box hullOBB;
	computeHullOBB(hullOBB, hullAABB, 0.0f, world0, world1, meshScaling, idtScaleMesh);

	ConvexVsMeshOverlapCallback cb(*cm, convexGeom.scale, meshScaling, pose0, pose1, idtScaleMesh, hullOBB);
	Midphase::intersectOBB(meshData, hullOBB, cb, true, false);

	return cb.mAnyHit;
}

