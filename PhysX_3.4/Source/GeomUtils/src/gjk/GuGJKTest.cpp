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


#include "GuGJK.h"
#include "GuGJKRaycast.h"
#include "GuGJKPenetration.h"
#include "GuGJKTest.h"

namespace physx
{
namespace Gu
{

using namespace Ps::aos;

GjkStatus testGjk(GjkConvex& a, GjkConvex& b,  const Vec3VArg initialSearchDir, const FloatVArg contactDist, Vec3V& closestA, Vec3V& closestB, Vec3V& normal, FloatV& dist)
{
	return gjk<GjkConvex, GjkConvex>(a, b, initialSearchDir, contactDist, closestA, closestB, normal, dist);
}
	

bool testGjkRaycast(GjkConvex& a, GjkConvex& b, const Vec3VArg initialSearchDir, const Ps::aos::FloatVArg initialLambda, const Ps::aos::Vec3VArg s, const Ps::aos::Vec3VArg r, Ps::aos::FloatV& lambda, 
		Ps::aos::Vec3V& normal, Ps::aos::Vec3V& closestA, const PxReal inflation)
{
	return gjkRaycast(a, b, initialSearchDir, initialLambda, s, r, lambda, normal, closestA, inflation);
}

GjkStatus testGjkPenetration (GjkConvex& a, GjkConvex& b,  const Vec3VArg initialSearchDir, const FloatVArg contactDist, Vec3V& closestA, Vec3V& closestB, Vec3V& normal, FloatV& sqDist)
{
	const bool takeCoreShape = a.getMarginIsRadius() || b.getMarginIsRadius();
	PxU8 aIndices[4];
	PxU8 bIndices[4];
	PxU8 size=0;
	return gjkPenetration<GjkConvex, GjkConvex>(a, b, initialSearchDir, contactDist, closestA, closestB, normal, 
		sqDist, aIndices, bIndices, size, takeCoreShape);
}

GjkStatus testGjkPenetration(GjkConvex& a, GjkConvex& b, const Vec3VArg initialSearchDir, const FloatVArg contactDist, Vec3V& closestA, Vec3V& closestB, Vec3V& normal, FloatV& sqDist,
	PxU8* aIndices, PxU8* bIndices, PxU8 size)
{
	const bool takeCoreShape = a.getMarginIsRadius() || b.getMarginIsRadius();
	return gjkPenetration<GjkConvex, GjkConvex>(a, b, initialSearchDir, contactDist, closestA, closestB, normal,
		sqDist, aIndices, bIndices, size, takeCoreShape);
}

GjkStatus testEpaPenetration(GjkConvex& a, GjkConvex& b, Ps::aos::Vec3V& contactA, Ps::aos::Vec3V& contactB, Ps::aos::Vec3V& normal, 
		Ps::aos::FloatV& penetrationDepth)
{
	const bool takeCoreShape = a.getMarginIsRadius() || b.getMarginIsRadius();
	PxU8 aIndices[4];
	PxU8 bIndices[4];
	PxU8 size=0;
	return epaPenetration(a, b, aIndices, bIndices, size, contactA, contactB, normal, penetrationDepth, takeCoreShape);
}

GjkStatus testEpaPenetration(GjkConvex& a, GjkConvex& b, Ps::aos::Vec3V& contactA, Ps::aos::Vec3V& contactB, Ps::aos::Vec3V& normal, 
		Ps::aos::FloatV& penetrationDepth, PxU8* aIndices, PxU8* bIndices, PxU8 size)
{
	const bool takeCoreShape = a.getMarginIsRadius() || b.getMarginIsRadius();
	return epaPenetration(a, b, aIndices, bIndices, size, contactA, contactB, normal, penetrationDepth, takeCoreShape);
}

}
}

