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

#include "GuContactBuffer.h"
#include "PsVecTransform.h"

#include "GuGeometryUnion.h"
#include "GuContactMethodImpl.h"

namespace physx
{

namespace Gu
{
bool pcmContactSpherePlane(GU_CONTACT_METHOD_ARGS)
{
	using namespace Ps::aos;
	PX_UNUSED(renderOutput);
	PX_UNUSED(cache);
	PX_UNUSED(shape1);

	// Get actual shape data
	const PxSphereGeometry& shapeSphere = shape0.get<const PxSphereGeometry>();

	//sphere transform
	const Vec3V p0 = V3LoadU_SafeReadW(transform0.p);	// PT: safe because 'mRefCount' follows 'mTransform' in PxsTransform

	//plane transform
	const Vec3V p1 = V3LoadU_SafeReadW(transform1.p);	// PT: safe because 'mRefCount' follows 'mTransform' in PxsTransform
	const QuatV q1 = QuatVLoadU(&transform1.q.x);

	const FloatV radius = FLoad(shapeSphere.radius);
	const FloatV contactDist = FLoad(params.mContactDistance);

	const PsTransformV transf1(p1, q1);
	//Sphere in plane space
	const Vec3V sphereCenterInPlaneSpace = transf1.transformInv(p0);
	

	//Separation
	const FloatV separation = FSub(V3GetX(sphereCenterInPlaneSpace), radius);

	if(FAllGrtrOrEq(contactDist, separation))
	{
		//get the plane normal
		const Vec3V worldNormal = QuatGetBasisVector0(q1);
		const Vec3V worldPoint = V3NegScaleSub(worldNormal, radius, p0);
		Gu::ContactPoint& contact = contactBuffer.contacts[contactBuffer.count++];
		//Fast allign store
		V4StoreA(Vec4V_From_Vec3V(worldNormal), &contact.normal.x);
		V4StoreA(Vec4V_From_Vec3V(worldPoint), &contact.point.x);
		FStore(separation, &contact.separation);
		contact.internalFaceIndex1 = PXC_CONTACT_NO_FACE_INDEX;
	
		return true;
	}
	return false;
}
}//Gu
}//physx
