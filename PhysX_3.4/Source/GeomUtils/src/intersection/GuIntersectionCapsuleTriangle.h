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

#ifndef GU_INTERSECTION_CAPSULE_TRIANGLE_H
#define GU_INTERSECTION_CAPSULE_TRIANGLE_H

#include "CmPhysXCommon.h"
#include "GuCapsule.h"
#include "PsUtilities.h"

namespace physx
{
namespace Gu
{
	// PT: precomputed data for capsule-triangle test. Useful when testing the same capsule vs several triangles.
	struct CapsuleTriangleOverlapData
	{
		PxVec3		mCapsuleDir;
		float		mBDotB;
		float		mOneOverBDotB;

		void		init(const Capsule& capsule)
		{
			const PxVec3 dir = capsule.p1 - capsule.p0;
			const float BDotB = dir.dot(dir);
			mCapsuleDir		= dir;
			mBDotB			= BDotB;
			mOneOverBDotB	= BDotB!=0.0f ? 1.0f/BDotB : 0.0f;
		}
	};

	// PT: tests if projections of capsule & triangle overlap on given axis
	PX_FORCE_INLINE PxU32 testAxis(const PxVec3& p0, const PxVec3& p1, const PxVec3& p2, const Capsule& capsule, const PxVec3& axis)
	{
		// Project capsule
		float min0 = capsule.p0.dot(axis);
		float max0 = capsule.p1.dot(axis);
		if(min0>max0)
			Ps::swap(min0, max0);
		const float MR = axis.magnitude()*capsule.radius;
		min0 -= MR;
		max0 += MR;

		// Project triangle
		float min1, max1;
		{
			min1 = max1 = p0.dot(axis);
			float dp = p1.dot(axis);
			if(dp<min1)	min1 = dp;
			if(dp>max1)	max1 = dp;
			dp = p2.dot(axis);
			if(dp<min1)	min1 = dp;
			if(dp>max1)	max1 = dp;
		}

		// Test projections
		if(max0<min1 || max1<min0)
			return 0;

		return 1;
	}

	// PT: computes shortest vector going from capsule axis to triangle edge
	PX_FORCE_INLINE PxVec3 computeEdgeAxis(	const PxVec3& p, const PxVec3& a,
											const PxVec3& q, const PxVec3& b,
											float BDotB, float oneOverBDotB)	
	{
		const PxVec3 T = q - p;
		const float ADotA = a.dot(a);
		const float ADotB = a.dot(b);
		const float ADotT = a.dot(T);
		const float BDotT = b.dot(T);

		const float denom = ADotA*BDotB - ADotB*ADotB;

		float t = denom!=0.0f ? (ADotT*BDotB - BDotT*ADotB) / denom : 0.0f;
		t = PxClamp(t, 0.0f, 1.0f);

		float u = (t*ADotB - BDotT) * oneOverBDotB;

		if(u<0.0f)
		{
			u = 0.0f;
			t = ADotT / ADotA;
			t = PxClamp(t, 0.0f, 1.0f);
		}
		else if(u>1.0f)
		{
			u = 1.0f;
			t = (ADotB + ADotT) / ADotA;
			t = PxClamp(t, 0.0f, 1.0f);
		}
		return T + b*u - a*t;
	}

	/**
	*	Checks if a capsule intersects a triangle.
	*
	*	\param		normal	[in] triangle normal (orientation does not matter)
	*	\param		p0		[in] triangle's first point
	*	\param		p1		[in] triangle's second point
	*	\param		p2		[in] triangle's third point
	*	\param		capsule	[in] capsule
	*	\param		params	[in] precomputed capsule params
	*	\return		true if capsule overlaps triangle
	*/
	bool intersectCapsuleTriangle(const PxVec3& normal, const PxVec3& p0, const PxVec3& p1, const PxVec3& p2, const Gu::Capsule& capsule, const CapsuleTriangleOverlapData& params);
}
}

#endif
