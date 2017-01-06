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

#ifndef GU_DISTANCE_POINT_TRIANGLE_H
#define GU_DISTANCE_POINT_TRIANGLE_H

#include "foundation/PxVec3.h"
#include "PxPhysXCommonConfig.h"
#include "CmPhysXCommon.h"

namespace physx
{
namespace Gu
{
	// PT: special version:
	// - inlined
	// - doesn't compute (s,t) output params
	// - expects precomputed edges in input
	PX_FORCE_INLINE PxVec3 closestPtPointTriangle2(const PxVec3& p, const PxVec3& a, const PxVec3& b, const PxVec3& c, const PxVec3& ab, const PxVec3& ac)
	{
		// Check if P in vertex region outside A
		//const PxVec3 ab = b - a;
		//const PxVec3 ac = c - a;
		const PxVec3 ap = p - a;
		const float d1 = ab.dot(ap);
		const float d2 = ac.dot(ap);
		if(d1<=0.0f && d2<=0.0f)
			return a;	// Barycentric coords 1,0,0

		// Check if P in vertex region outside B
		const PxVec3 bp = p - b;
		const float d3 = ab.dot(bp);
		const float d4 = ac.dot(bp);
		if(d3>=0.0f && d4<=d3)
			return b;	// Barycentric coords 0,1,0

		// Check if P in edge region of AB, if so return projection of P onto AB
		const float vc = d1*d4 - d3*d2;
		if(vc<=0.0f && d1>=0.0f && d3<=0.0f)
		{
			const float v = d1 / (d1 - d3);
			return a + v * ab;	// barycentric coords (1-v, v, 0)
		}

		// Check if P in vertex region outside C
		const PxVec3 cp = p - c;
		const float d5 = ab.dot(cp);
		const float d6 = ac.dot(cp);
		if(d6>=0.0f && d5<=d6)
			return c;	// Barycentric coords 0,0,1

		// Check if P in edge region of AC, if so return projection of P onto AC
		const float vb = d5*d2 - d1*d6;
		if(vb<=0.0f && d2>=0.0f && d6<=0.0f)
		{
			const float w = d2 / (d2 - d6);
			return a + w * ac;	// barycentric coords (1-w, 0, w)
		}

		// Check if P in edge region of BC, if so return projection of P onto BC
		const float va = d3*d6 - d5*d4;
		if(va<=0.0f && (d4-d3)>=0.0f && (d5-d6)>=0.0f)
		{
			const float w = (d4-d3) / ((d4 - d3) + (d5-d6));
			return b + w * (c-b);	// barycentric coords (0, 1-w, w)
		}

		// P inside face region. Compute Q through its barycentric coords (u,v,w)
		const float denom = 1.0f / (va + vb + vc);
		const float v = vb * denom;
		const float w = vc * denom;
		return a + ab*v + ac*w;
	}

	PX_PHYSX_COMMON_API PxVec3 closestPtPointTriangle(const PxVec3& p, const PxVec3& a, const PxVec3& b, const PxVec3& c, float& s, float& t);

	PX_FORCE_INLINE PxReal distancePointTriangleSquared(const PxVec3& point, 
														const PxVec3& triangleOrigin, 
														const PxVec3& triangleEdge0, 
														const PxVec3& triangleEdge1,
														PxReal* param0=NULL, 
														PxReal* param1=NULL)
	{
		const PxVec3 pt0 = triangleEdge0 + triangleOrigin;
		const PxVec3 pt1 = triangleEdge1 + triangleOrigin;
		float s,t;
		const PxVec3 cp = closestPtPointTriangle(point, triangleOrigin, pt0, pt1, s, t);
		if(param0)
			*param0 = s;
		if(param1)
			*param1 = t;
		return (cp - point).magnitudeSquared();
	}

} // namespace Gu

}

#endif
