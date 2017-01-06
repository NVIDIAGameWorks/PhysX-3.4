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


//#include "GuGJKWrapper.h"
#include "GuVecSphere.h"
#include "GuVecCapsule.h"
#include "GuGeometryUnion.h"

#include "GuContactMethodImpl.h"
#include "GuContactBuffer.h"


namespace physx
{

namespace Gu
{
PX_FORCE_INLINE Ps::aos::FloatV PxcDistancePointSegmentSquared(const Ps::aos::Vec3VArg a, const Ps::aos::Vec3VArg b, const Ps::aos::Vec3VArg p, Ps::aos::FloatV& param)
{
	using namespace Ps::aos;
	const FloatV zero = FZero();
	const FloatV one = FOne();

	const Vec3V ap = V3Sub(p, a);
	const Vec3V ab = V3Sub(b, a);
	const FloatV nom = V3Dot(ap, ab);
	
	const FloatV denom = V3Dot(ab, ab);
	const FloatV tValue = FClamp(FMul(nom, FDiv(one, denom)), zero, one);

	const FloatV t = FSel(FIsEq(denom, zero), zero, tValue);
	const Vec3V v = V3NegScaleSub(ab, t, ap);
	param = t;
	return V3Dot(v, v);
}

bool pcmContactSphereCapsule(GU_CONTACT_METHOD_ARGS)
{
	PX_UNUSED(cache);
	PX_UNUSED(renderOutput);

	using namespace Ps::aos;
	const PxSphereGeometry& shapeSphere = shape0.get<const PxSphereGeometry>();
	const PxCapsuleGeometry& shapeCapsule = shape1.get<const PxCapsuleGeometry>();

	//Sphere in world space
	const Vec3V sphereCenter =  V3LoadA(&transform0.p.x);
	const QuatV q1 = QuatVLoadA(&transform1.q.x);
	const Vec3V p1 = V3LoadA(&transform1.p.x);

	const FloatV sphereRadius = FLoad(shapeSphere.radius);
	const FloatV capsuleRadius = FLoad(shapeCapsule.radius);
	const FloatV cDist = FLoad(params.mContactDistance);

	//const FloatV r0 = FloatV_From_F32(shapeCapsule.radius);
	const FloatV halfHeight0 = FLoad(shapeCapsule.halfHeight);
	const Vec3V basisVector0 = QuatGetBasisVector0(q1);
	const Vec3V tmp0 = V3Scale(basisVector0, halfHeight0);
	const Vec3V s = V3Add(p1, tmp0);
	const Vec3V e = V3Sub(p1, tmp0);

	
	const FloatV radiusSum = FAdd(sphereRadius, capsuleRadius);
	const FloatV inflatedSum = FAdd(radiusSum, cDist);

	// Collision detection
	FloatV t;
	const FloatV squareDist = PxcDistancePointSegmentSquared(s, e, sphereCenter, t);
	const FloatV sqInflatedSum = FMul(inflatedSum, inflatedSum);

	if(FAllGrtr(sqInflatedSum, squareDist))//BAllEq(con, bTrue))
	{
		const Vec3V p = V3ScaleAdd(V3Sub(e, s), t, s);
		const Vec3V dir = V3Sub(sphereCenter, p);
		const Vec3V normal = V3NormalizeSafe(dir, V3UnitX());
		const Vec3V point = V3NegScaleSub(normal, sphereRadius, sphereCenter);//transform back to the world space

		const FloatV dist = FSub(FSqrt(squareDist), radiusSum);
		//context.mContactBuffer.contact(point, normal, FSub(FSqrt(squareDist), radiusSum));
		PX_ASSERT(contactBuffer.count < ContactBuffer::MAX_CONTACTS);
		Gu::ContactPoint& contact = contactBuffer.contacts[contactBuffer.count++];

		V4StoreA(Vec4V_From_Vec3V(normal), &contact.normal.x);
		V4StoreA(Vec4V_From_Vec3V(point), &contact.point.x);
		FStore(dist, &contact.separation);

		contact.internalFaceIndex1 = PXC_CONTACT_NO_FACE_INDEX;

		return true;
	}
	return false;
}
}//Gu
}//physx
