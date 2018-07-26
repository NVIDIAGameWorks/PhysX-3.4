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
// Copyright (c) 2008-2018 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

#ifndef GU_RAWQUERY_TESTS_SIMD_H
#define GU_RAWQUERY_TESTS_SIMD_H

#include "foundation/PxTransform.h"
#include "foundation/PxBounds3.h"
#include "CmPhysXCommon.h"
#include "PxBoxGeometry.h"
#include "PxSphereGeometry.h"
#include "PxCapsuleGeometry.h"
#include "PsVecMath.h"

namespace physx
{
namespace Gu
{

struct RayAABBTest
{
	PX_FORCE_INLINE RayAABBTest(const PxVec3& origin_, const PxVec3& unitDir_, const PxReal maxDist, const PxVec3& inflation_)
	: mOrigin(V3LoadU(origin_))
	, mDir(V3LoadU(unitDir_))
	, mDirYZX(V3PermYZX(mDir))
	, mInflation(V3LoadU(inflation_))
	, mAbsDir(V3Abs(mDir))
	, mAbsDirYZX(V3PermYZX(mAbsDir))
	{
		const PxVec3 ext = maxDist >= PX_MAX_F32 ? PxVec3(	unitDir_.x == 0 ? origin_.x : PxSign(unitDir_.x)*PX_MAX_F32,
															unitDir_.y == 0 ? origin_.y : PxSign(unitDir_.y)*PX_MAX_F32,
															unitDir_.z == 0 ? origin_.z : PxSign(unitDir_.z)*PX_MAX_F32)
										   : origin_ + unitDir_ * maxDist;
		mRayMin = V3Min(mOrigin, V3LoadU(ext));
		mRayMax = V3Max(mOrigin, V3LoadU(ext));
	}

	PX_FORCE_INLINE void setDistance(PxReal distance)
	{
		const Vec3V ext = V3ScaleAdd(mDir, FLoad(distance), mOrigin);
		mRayMin = V3Min(mOrigin, ext);
		mRayMax = V3Max(mOrigin, ext);
	}

	template<bool TInflate>
	PX_FORCE_INLINE PxU32 check(const Vec3V center, const Vec3V extents) const
	{
		const Vec3V iExt = TInflate ? V3Add(extents, mInflation) : extents;

		// coordinate axes
		const Vec3V nodeMax = V3Add(center, iExt);
		const Vec3V nodeMin = V3Sub(center, iExt);

		// cross axes
		const Vec3V offset = V3Sub(mOrigin, center);
		const Vec3V offsetYZX = V3PermYZX(offset);
		const Vec3V iExtYZX = V3PermYZX(iExt);
	
		const Vec3V f = V3NegMulSub(mDirYZX, offset, V3Mul(mDir, offsetYZX));
		const Vec3V g = V3MulAdd(iExt, mAbsDirYZX, V3Mul(iExtYZX, mAbsDir));

		const BoolV
			maskA = V3IsGrtrOrEq(nodeMax, mRayMin),
			maskB = V3IsGrtrOrEq(mRayMax, nodeMin),
			maskC = V3IsGrtrOrEq(g, V3Abs(f));
		const BoolV andABCMasks = BAnd(BAnd(maskA, maskB), maskC);

		return BAllEqTTTT(andABCMasks);
	}

	const Vec3V mOrigin, mDir, mDirYZX, mInflation, mAbsDir, mAbsDirYZX;
	Vec3V mRayMin, mRayMax;
protected:
	RayAABBTest& operator=(const RayAABBTest&);
};

// probably not worth having a SIMD version of this unless the traversal passes Vec3Vs
struct AABBAABBTest
{
	PX_FORCE_INLINE AABBAABBTest(const PxTransform&t, const PxBoxGeometry&b)
	: mCenter(V3LoadU(t.p))
	, mExtents(V3LoadU(b.halfExtents))
	{ }

	PX_FORCE_INLINE AABBAABBTest(const PxBounds3& b)
	: mCenter(V3LoadU(b.getCenter()))
	, mExtents(V3LoadU(b.getExtents()))
	{ }

	PX_FORCE_INLINE Ps::IntBool operator()(const Vec3V center, const Vec3V extents) const		
	{	
		//PxVec3 c; PxVec3_From_Vec3V(center, c);
		//PxVec3 e; PxVec3_From_Vec3V(extents, e);
		//if(PxAbs(c.x - mCenter.x) > mExtents.x + e.x) return Ps::IntFalse;
		//if(PxAbs(c.y - mCenter.y) > mExtents.y + e.y) return Ps::IntFalse;
		//if(PxAbs(c.z - mCenter.z) > mExtents.z + e.z) return Ps::IntFalse;
		//return Ps::IntTrue;
		return Ps::IntBool(V3AllGrtrOrEq(V3Add(mExtents, extents), V3Abs(V3Sub(center, mCenter))));
	}

private:
	AABBAABBTest& operator=(const AABBAABBTest&);
	const Vec3V mCenter, mExtents;
};

struct SphereAABBTest
{
	PX_FORCE_INLINE SphereAABBTest(const PxTransform& t, const PxSphereGeometry& s)
	: mCenter(V3LoadU(t.p))
	, mRadius2(FLoad(s.radius * s.radius))
	{}

	PX_FORCE_INLINE SphereAABBTest(const PxVec3& center, PxF32 radius)
	: mCenter(V3LoadU(center))
	, mRadius2(FLoad(radius * radius))
	{}
	
	PX_FORCE_INLINE Ps::IntBool operator()(const Vec3V boxCenter, const Vec3V boxExtents) const		
	{	
		const Vec3V offset = V3Sub(mCenter, boxCenter);
		const Vec3V closest = V3Clamp(offset, V3Neg(boxExtents), boxExtents);
		const Vec3V d = V3Sub(offset, closest);
		return Ps::IntBool(BAllEqTTTT(FIsGrtrOrEq(mRadius2, V3Dot(d, d))));
	}

private:
	SphereAABBTest& operator=(const SphereAABBTest&);
	const Vec3V mCenter;
	const FloatV mRadius2;
};

// The Opcode capsule-AABB traversal test seems to be *exactly* the same as the ray-box test inflated by the capsule radius (so not a true capsule/box test)
// and the code for the ray-box test is better. TODO: check the zero length case and use the sphere traversal if this one fails.
// (OTOH it's not that hard to adapt the Ray-AABB test to a capsule test)

struct CapsuleAABBTest: private RayAABBTest
{
	PX_FORCE_INLINE CapsuleAABBTest(const PxVec3& origin, const PxVec3& unitDir, const PxReal length, const PxVec3& inflation)
		: RayAABBTest(origin, unitDir, length, inflation)
	{}

	PX_FORCE_INLINE Ps::IntBool operator()(const Vec3VArg center, const Vec3VArg extents) const
	{
		return Ps::IntBool(RayAABBTest::check<true>(center, extents));
	}
};

template<bool fullTest>
struct OBBAABBTests
{
	OBBAABBTests(const PxVec3& pos, const PxMat33& rot, const PxVec3& halfExtentsInflated)
	{
		const Vec3V eps = V3Load(1e-6f);

		mT = V3LoadU(pos);
		mExtents = V3LoadU(halfExtentsInflated);

		// storing the transpose matrices yields a simpler SIMD test
		mRT = Mat33V_From_PxMat33(rot.getTranspose());
		mART = Mat33V(V3Add(V3Abs(mRT.col0), eps), V3Add(V3Abs(mRT.col1), eps), V3Add(V3Abs(mRT.col2), eps));
		mBB_xyz = M33TrnspsMulV3(mART, mExtents);

		if(fullTest)
		{
			const Vec3V eYZX = V3PermYZX(mExtents), eZXY = V3PermZXY(mExtents);

			mBB_123 = V3MulAdd(eYZX, V3PermZXY(mART.col0), V3Mul(eZXY, V3PermYZX(mART.col0)));
			mBB_456 = V3MulAdd(eYZX, V3PermZXY(mART.col1), V3Mul(eZXY, V3PermYZX(mART.col1)));
			mBB_789 = V3MulAdd(eYZX, V3PermZXY(mART.col2), V3Mul(eZXY, V3PermYZX(mART.col2)));
		}
	}

	// TODO: force inline it?
	Ps::IntBool operator()(const Vec3V center, const Vec3V extents) const
	{	
		const Vec3V t = V3Sub(mT, center);

		// class I - axes of AABB
		if(V3OutOfBounds(t, V3Add(extents, mBB_xyz)))
			return Ps::IntFalse;

		const Vec3V rX = mRT.col0, rY = mRT.col1, rZ = mRT.col2;
		const Vec3V arX = mART.col0, arY = mART.col1, arZ = mART.col2;

		const FloatV eX = V3GetX(extents), eY = V3GetY(extents), eZ = V3GetZ(extents);
		const FloatV tX = V3GetX(t), tY = V3GetY(t), tZ = V3GetZ(t);

		// class II - axes of OBB
		{
			const Vec3V v = V3ScaleAdd(rZ, tZ, V3ScaleAdd(rY, tY, V3Scale(rX, tX)));
			const Vec3V v2 = V3ScaleAdd(arZ, eZ, V3ScaleAdd(arY, eY, V3ScaleAdd(arX, eX, mExtents)));
			if(V3OutOfBounds(v, v2))
				return Ps::IntFalse;
		}

		if(!fullTest)
			return Ps::IntTrue;

		// class III - edge cross products. Almost all OBB tests early-out with type I or type II,
		// so early-outs here probably aren't useful (TODO: profile)

		const Vec3V va = V3NegScaleSub(rZ, tY, V3Scale(rY, tZ));
		const Vec3V va2 = V3ScaleAdd(arY, eZ, V3ScaleAdd(arZ, eY, mBB_123));
		const BoolV ba = BOr(V3IsGrtr(va, va2), V3IsGrtr(V3Neg(va2), va));
	
		const Vec3V vb = V3NegScaleSub(rX, tZ, V3Scale(rZ, tX));
		const Vec3V vb2 = V3ScaleAdd(arX, eZ, V3ScaleAdd(arZ, eX, mBB_456));
		const BoolV bb = BOr(V3IsGrtr(vb, vb2), V3IsGrtr(V3Neg(vb2), vb));
		
		const Vec3V vc = V3NegScaleSub(rY, tX, V3Scale(rX, tY));
		const Vec3V vc2 = V3ScaleAdd(arX, eY, V3ScaleAdd(arY, eX, mBB_789));
		const BoolV bc = BOr(V3IsGrtr(vc, vc2), V3IsGrtr(V3Neg(vc2), vc));

		return Ps::IntBool(BAllEqFFFF(BOr(ba, BOr(bb,bc))));
	}

	Vec3V		mExtents;	// extents of OBB
	Vec3V		mT;			// translation of OBB
	Mat33V		mRT;		// transpose of rotation matrix of OBB
	Mat33V		mART;		// transpose of mRT, padded by epsilon

	Vec3V		mBB_xyz;	// extents of OBB along coordinate axes
	Vec3V		mBB_123;	// projections of extents onto edge-cross axes
	Vec3V		mBB_456;
	Vec3V		mBB_789;
};

typedef OBBAABBTests<true> OBBAABBTest;

}
}
#endif
