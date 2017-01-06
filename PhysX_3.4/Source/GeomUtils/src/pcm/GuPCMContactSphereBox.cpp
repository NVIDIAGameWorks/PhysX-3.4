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
#include "GuVecBox.h"
#include "GuVecSphere.h"
#include "GuGeometryUnion.h"
   
#include "GuContactMethodImpl.h"
#include "GuContactBuffer.h"



namespace physx
{

namespace Gu
{
bool pcmContactSphereBox(GU_CONTACT_METHOD_ARGS)
{
	PX_UNUSED(renderOutput);
	PX_UNUSED(cache);

	using namespace Ps::aos;
	// Get actual shape data
	const PxSphereGeometry& shapeSphere = shape0.get<const PxSphereGeometry>();
	const PxBoxGeometry& shapeBox = shape1.get<const PxBoxGeometry>();
	//

	//const PsTransformV transf0(transform0);
	const Vec3V sphereOrigin = V3LoadA(&transform0.p.x);
	//const PsTransformV transf1(transform1);

	const QuatV q1 = QuatVLoadA(&transform1.q.x);
	const Vec3V p1 = V3LoadA(&transform1.p.x);

	const FloatV radius = FLoad(shapeSphere.radius);
	
	const PsTransformV transf1(p1, q1);
	
	const FloatV cDist = FLoad(params.mContactDistance);

	const Vec3V boxExtents = V3LoadU(shapeBox.halfExtents);

	//translate sphere center into the box space
	const Vec3V sphereCenter = transf1.transformInv(sphereOrigin);

	const Vec3V nBoxExtents = V3Neg(boxExtents);

	//const FloatV radSq = FMul(radius, radius);

	const FloatV inflatedSum = FAdd(radius, cDist);
	const FloatV sqInflatedSum = FMul(inflatedSum, inflatedSum);

	const Vec3V p = V3Clamp(sphereCenter, nBoxExtents, boxExtents);
	const Vec3V v = V3Sub(sphereCenter, p);
	const FloatV lengthSq = V3Dot(v, v);

	PX_ASSERT(contactBuffer.count < ContactBuffer::MAX_CONTACTS);

	if(FAllGrtr(sqInflatedSum, lengthSq))//intersect
	{
		//check whether the spherCenter is inside the box
		const BoolV bInsideBox = V3IsGrtrOrEq(boxExtents, V3Abs(sphereCenter));
		// PT: TODO: ??? revisit this, why do we have both BAllEqTTTT and BAllTrue3?
		if(BAllEqTTTT(BAllTrue3(bInsideBox)))//sphere center inside the box
		{
			//Pick directions and sign
			const Vec3V absP = V3Abs(p);
			const Vec3V distToSurface = V3Sub(boxExtents, absP);//dist from embedded center to box surface along 3 dimensions.
			
			const FloatV x = V3GetX(distToSurface);
			const FloatV y = V3GetY(distToSurface);
			const FloatV z = V3GetZ(distToSurface);

			const Vec3V xV = V3Splat(x);
			const Vec3V zV = V3Splat(z);

			//find smallest element of distToSurface
			const BoolV con0 = BAllTrue3(V3IsGrtrOrEq(distToSurface, zV));
			const BoolV con1 = BAllTrue3(V3IsGrtrOrEq(distToSurface, xV));
			const Vec3V sign = V3Sign(p);
	
			const Vec3V tmpX = V3Mul(V3UnitX(), sign);
			const Vec3V tmpY = V3Mul(V3UnitY(), sign);
			const Vec3V tmpZ = V3Mul(V3UnitZ(), sign);
			
			const Vec3V locNorm= V3Sel(con0, tmpZ, V3Sel(con1, tmpX, tmpY));////local coords contact normal
			const FloatV dist = FNeg(FSel(con0, z, FSel(con1, x, y)));

			//separation so far is just the embedding of the center point; we still have to push out all of the radius.
			const Vec3V point = sphereOrigin;
			const Vec3V normal = transf1.rotate(locNorm);
			const FloatV penetration = FSub(dist, radius);


			Gu::ContactPoint& contact = contactBuffer.contacts[contactBuffer.count++];
			V4StoreA(Vec4V_From_Vec3V(normal), &contact.normal.x);
			V4StoreA(Vec4V_From_Vec3V(point), &contact.point.x);
			FStore(penetration, &contact.separation);

			contact.internalFaceIndex1 = PXC_CONTACT_NO_FACE_INDEX;
			//context.mContactBuffer.contact(point, normal, penetration);
		}
		else
		{
			//get the closest point from the center to the box surface
			const FloatV recipLength = FRsqrt(lengthSq);
			const FloatV length = FRecip(recipLength);
			const Vec3V locNorm = V3Scale(v, recipLength);
			const FloatV penetration = FSub(length, radius);
			const Vec3V normal = transf1.rotate(locNorm);
			const Vec3V point = transf1.transform(p);

			PX_ASSERT(contactBuffer.count < ContactBuffer::MAX_CONTACTS);
			Gu::ContactPoint& contact = contactBuffer.contacts[contactBuffer.count++];
			V4StoreA(Vec4V_From_Vec3V(normal), &contact.normal.x);
			V4StoreA(Vec4V_From_Vec3V(point), &contact.point.x);
			FStore(penetration, &contact.separation);

			contact.internalFaceIndex1 = PXC_CONTACT_NO_FACE_INDEX;

			//context.mContactBuffer.contact(point, normal, penetration);
		}
		return true;
	}
	return false;
}

}//Gu
}//physx
