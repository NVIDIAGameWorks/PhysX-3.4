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

#include "GuIntersectionCapsuleTriangle.h"
#include "GuDistancePointSegment.h"

using namespace physx;
using namespace Gu;

bool Gu::intersectCapsuleTriangle(const PxVec3& N, const PxVec3& p0, const PxVec3& p1, const PxVec3& p2, const Gu::Capsule& capsule, const CapsuleTriangleOverlapData& params)
{
	PX_ASSERT(capsule.p0!=capsule.p1);

	{
		const PxReal d2 = distancePointSegmentSquaredInternal(capsule.p0, params.mCapsuleDir, p0);
		if(d2<=capsule.radius*capsule.radius)
			return true;
	}

//	const PxVec3 N = (p0 - p1).cross(p0 - p2);

	if(!testAxis(p0, p1, p2, capsule, N))
		return false;

	if(!testAxis(p0, p1, p2, capsule, computeEdgeAxis(p0, p1 - p0, capsule.p0, params.mCapsuleDir, params.mBDotB, params.mOneOverBDotB)))
		return false;

	if(!testAxis(p0, p1, p2, capsule, computeEdgeAxis(p1, p2 - p1, capsule.p0, params.mCapsuleDir, params.mBDotB, params.mOneOverBDotB)))
		return false;

	if(!testAxis(p0, p1, p2, capsule, computeEdgeAxis(p2, p0 - p2, capsule.p0, params.mCapsuleDir, params.mBDotB, params.mOneOverBDotB)))
		return false;

	return true;
}
