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

#include "GuGeometryUnion.h"

#include "GuContactBuffer.h"
#include "GuContactMethodImpl.h"
#include "PsVecTransform.h"

namespace physx
{
namespace Gu
{
bool pcmContactSphereSphere(GU_CONTACT_METHOD_ARGS)
{
	PX_UNUSED(cache);
	PX_UNUSED(renderOutput);

	using namespace Ps::aos;
	const PxSphereGeometry& shapeSphere0 = shape0.get<const PxSphereGeometry>();
	const PxSphereGeometry& shapeSphere1 = shape1.get<const PxSphereGeometry>();
	
	const FloatV cDist	= FLoad(params.mContactDistance);
	const Vec3V p0 =  V3LoadA(&transform0.p.x);
	const Vec3V p1 =  V3LoadA(&transform1.p.x);

	const FloatV r0		= FLoad(shapeSphere0.radius);
	const FloatV r1		= FLoad(shapeSphere1.radius);
	

	const Vec3V _delta = V3Sub(p0, p1);
	const FloatV distanceSq = V3Dot(_delta, _delta);
	const FloatV radiusSum = FAdd(r0, r1);
	const FloatV inflatedSum = FAdd(radiusSum, cDist);
	
	if(FAllGrtr(FMul(inflatedSum, inflatedSum), distanceSq))
	{
		const FloatV eps	=  FLoad(0.00001f);
		const FloatV nhalf	= FLoad(-0.5f);
		const FloatV magn = FSqrt(distanceSq);
		const BoolV bCon = FIsGrtrOrEq(eps, magn);
		const Vec3V normal = V3Sel(bCon, V3UnitX(), V3ScaleInv(_delta, magn)); 
		const FloatV scale = FMul(FSub(FAdd(r0, magn), r1), nhalf);
		const Vec3V point = V3ScaleAdd(normal, scale, p0);
		const FloatV dist =  FSub(magn, radiusSum);
		
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
