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

#ifndef GU_INTERSECTION_RAY_CAPSULE_H
#define GU_INTERSECTION_RAY_CAPSULE_H

#include "CmPhysXCommon.h"
#include "GuCapsule.h"
#include "GuDistancePointSegment.h"
#include "GuIntersectionRay.h"

namespace physx
{
namespace Gu
{
	PxU32 intersectRayCapsuleInternal(const PxVec3& origin, const PxVec3& dir, const PxVec3& p0, const PxVec3& p1, float radius, PxReal s[2]);

	PX_FORCE_INLINE bool intersectRayCapsule(const PxVec3& origin, const PxVec3& dir, const PxVec3& p0, const PxVec3& p1, float radius, PxReal& t)
	{
		// PT: move ray origin close to capsule, to solve accuracy issues.
		// We compute the distance D between the ray origin and the capsule's segment.
		// Then E = D - radius = distance between the ray origin and the capsule.
		// We can move the origin freely along 'dir' up to E units before touching the capsule.
		PxReal l = distancePointSegmentSquaredInternal(p0, p1 - p0, origin);
		l = PxSqrt(l) - radius;

		// PT: if this becomes negative or null, the ray starts inside the capsule and we can early exit
		if(l<=0.0f)
		{
			t = 0.0f;
			return true;
		}

		// PT: we remove an arbitrary GU_RAY_SURFACE_OFFSET units to E, to make sure we don't go close to the surface.
		// If we're moving in the direction of the capsule, the origin is now about GU_RAY_SURFACE_OFFSET units from it.
		// If we're moving away from the capsule, the ray won't hit the capsule anyway.
		// If l is smaller than GU_RAY_SURFACE_OFFSET we're close enough, accuracy is good, there is nothing to do.
		if(l>GU_RAY_SURFACE_OFFSET)
			l -= GU_RAY_SURFACE_OFFSET;
		else
			l = 0.0f;

		// PT: move origin closer to capsule and do the raycast
		PxReal s[2];
		const PxU32 nbHits = Gu::intersectRayCapsuleInternal(origin + l*dir, dir, p0, p1, radius, s);
		if(!nbHits)
			return false;

		// PT: keep closest hit only
		if(nbHits == 1)
			t = s[0];
		else
			t = (s[0] < s[1]) ? s[0] : s[1];

		// PT: fix distance (smaller than expected after moving ray close to capsule)
		t += l;
		return true;
	}

	PX_FORCE_INLINE bool intersectRayCapsule(const PxVec3& origin, const PxVec3& dir, const Gu::Capsule& capsule, PxReal& t)
	{
		return Gu::intersectRayCapsule(origin, dir, capsule.p0, capsule.p1, capsule.radius, t);
	}
}
}

#endif
