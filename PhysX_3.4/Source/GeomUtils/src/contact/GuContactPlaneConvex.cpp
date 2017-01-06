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

#include "GuConvexMeshData.h"
#include "GuContactMethodImpl.h"
#include "GuContactBuffer.h"
#include "GuGeometryUnion.h"
#include "CmScaling.h"


namespace physx
{
namespace Gu
{
bool contactPlaneConvex(GU_CONTACT_METHOD_ARGS)
{
	PX_UNUSED(renderOutput);
	PX_UNUSED(cache);
	PX_UNUSED(shape0);

	// Get actual shape data
	//const PxPlaneGeometry& shapePlane = shape.get<const PxPlaneGeometry>();
	const PxConvexMeshGeometryLL& shapeConvex = shape1.get<const PxConvexMeshGeometryLL>();

	const PxVec3* PX_RESTRICT hullVertices = shapeConvex.hullData->getHullVertices();
	PxU32 numHullVertices = shapeConvex.hullData->mNbHullVertices;
//	Ps::prefetch128(hullVertices);

	// Plane is implicitly <1,0,0> 0 in localspace
	Cm::Matrix34 convexToPlane (transform0.transformInv(transform1));
	PxMat33 convexToPlane_rot(convexToPlane[0], convexToPlane[1], convexToPlane[2] );

	bool idtScale = shapeConvex.scale.isIdentity();
	Cm::FastVertex2ShapeScaling convexScaling;	// PT: TODO: remove default ctor
	if(!idtScale)
		convexScaling.init(shapeConvex.scale);

	convexToPlane = Cm::Matrix34( convexToPlane_rot * convexScaling.getVertex2ShapeSkew(), convexToPlane[3] );

	//convexToPlane = context.mVertex2ShapeSkew[1].getVertex2WorldSkew(convexToPlane);

	const Cm::Matrix34 planeToW (transform0);

	// This is rather brute-force
	
	bool status = false;

	const PxVec3 contactNormal = -planeToW.m.column0;

	while(numHullVertices--)
	{
		const PxVec3& vertex = *hullVertices++;
//		if(numHullVertices)
//			Ps::prefetch128(hullVertices);

		const PxVec3 pointInPlane = convexToPlane.transform(vertex);		//TODO: this multiply could be factored out!
		if(pointInPlane.x <= params.mContactDistance)
		{
//			const PxVec3 pointInW = planeToW.transform(pointInPlane);
//			contactBuffer.contact(pointInW, -planeToW.m.column0, pointInPlane.x);
			status = true;
			Gu::ContactPoint* PX_RESTRICT pt = contactBuffer.contact();
			if(pt)
			{
				pt->normal				= contactNormal;
				pt->point				= planeToW.transform(pointInPlane);
				pt->separation			= pointInPlane.x;
				pt->internalFaceIndex1	= PXC_CONTACT_NO_FACE_INDEX;
			}
		}
	}
	return status;
}
}//Gu
}//physx
