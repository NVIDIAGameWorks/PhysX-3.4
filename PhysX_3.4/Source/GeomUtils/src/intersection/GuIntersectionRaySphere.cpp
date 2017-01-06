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

#include "foundation/PxVec3.h"
#include "GuIntersectionRaySphere.h"
#include "GuIntersectionRay.h"

using namespace physx;

// Based on GD Mag code, but now works correctly when origin is inside the sphere.
// This version has limited accuracy.
bool Gu::intersectRaySphereBasic(const PxVec3& origin, const PxVec3& dir, PxReal length, const PxVec3& center, PxReal radius, PxReal& dist, PxVec3* hit_pos)
{
	// get the offset vector
	const PxVec3 offset = center - origin;

	// get the distance along the ray to the center point of the sphere
	const PxReal ray_dist = dir.dot(offset);

	// get the squared distances
	const PxReal off2 = offset.dot(offset);
	const PxReal rad_2 = radius * radius;
	if(off2 <= rad_2)
	{
		// we're in the sphere
		if(hit_pos)
			*hit_pos	= origin;
		dist	= 0.0f;
		return true;
	}

	if(ray_dist <= 0 || (ray_dist - length) > radius)
	{
		// moving away from object or too far away
		return false;
	}

	// find hit distance squared
	const PxReal d = rad_2 - (off2 - ray_dist * ray_dist);
	if(d<0.0f)
	{
		// ray passes by sphere without hitting
		return false;
	}

	// get the distance along the ray
	dist = ray_dist - PxSqrt(d);
	if(dist > length)
	{
		// hit point beyond length
		return false;
	}

	// sort out the details
	if(hit_pos)
		*hit_pos = origin + dir * dist;
	return true;
}

// PT: modified version calls the previous function, but moves the ray origin closer to the sphere. The test accuracy is
// greatly improved as a result. This is an idea proposed on the GD-Algorithms list by Eddie Edwards.
// See: http://www.codercorner.com/blog/?p=321
bool Gu::intersectRaySphere(const PxVec3& origin, const PxVec3& dir, PxReal length, const PxVec3& center, PxReal radius, PxReal& dist, PxVec3* hit_pos)
{
	const PxVec3 x = origin - center;
	PxReal l = PxSqrt(x.dot(x)) - radius - GU_RAY_SURFACE_OFFSET;

//	if(l<0.0f)
//		l=0.0f;
	l = physx::intrinsics::selectMax(l, 0.0f);

	bool status = intersectRaySphereBasic(origin + l*dir, dir, length - l, center, radius, dist, hit_pos);
	if(status)
	{
//		dist += l/length;
		dist += l;
	}
	return status;
}
