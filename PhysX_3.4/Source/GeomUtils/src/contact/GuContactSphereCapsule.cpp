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

#include "GuDistancePointSegment.h"
#include "GuContactMethodImpl.h"
#include "GuContactBuffer.h"
#include "GuInternal.h"
#include "GuGeometryUnion.h"

namespace physx
{
namespace Gu
{
bool contactSphereCapsule(GU_CONTACT_METHOD_ARGS)
{
	PX_UNUSED(renderOutput);
	PX_UNUSED(cache);

	const PxSphereGeometry& sphereGeom = shape0.get<const PxSphereGeometry>();
	const PxCapsuleGeometry& capsuleGeom = shape1.get<const PxCapsuleGeometry>();

	// PT: get capsule in local space
	const PxVec3 capsuleLocalSegment = getCapsuleHalfHeightVector(transform1, capsuleGeom);
	const Segment localSegment(capsuleLocalSegment, -capsuleLocalSegment);

	// PT: get sphere in capsule space
	const PxVec3 sphereCenterInCapsuleSpace = transform0.p - transform1.p;

	const PxReal radiusSum = sphereGeom.radius + capsuleGeom.radius;
	const PxReal inflatedSum = radiusSum + params.mContactDistance;

	// PT: compute distance between sphere center & capsule's segment
	PxReal u;
	const PxReal squareDist = distancePointSegmentSquared(localSegment, sphereCenterInCapsuleSpace, &u);
	if(squareDist >= inflatedSum*inflatedSum)
		return false;

	// PT: compute contact normal
	PxVec3 normal = sphereCenterInCapsuleSpace - localSegment.getPointAt(u);
		
	// We do a *manual* normalization to check for singularity condition
	const PxReal lenSq = normal.magnitudeSquared();
	if(lenSq==0.0f) 
		normal = PxVec3(1.0f, 0.0f, 0.0f);	// PT: zero normal => pick up random one
	else
		normal *= PxRecipSqrt(lenSq);

	// PT: compute contact point
	const PxVec3 point = sphereCenterInCapsuleSpace + transform1.p - normal * sphereGeom.radius;

	// PT: output unique contact
	contactBuffer.contact(point, normal, PxSqrt(squareDist) - radiusSum);
	return true;
}
}//Gu
}//physx
