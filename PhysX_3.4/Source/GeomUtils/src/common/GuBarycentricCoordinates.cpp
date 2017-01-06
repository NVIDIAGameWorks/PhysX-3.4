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


#include "GuBarycentricCoordinates.h"

using namespace physx;
using namespace Ps::aos;

void Gu::barycentricCoordinates(const Vec3VArg p, const Vec3VArg a, const Vec3VArg b, FloatV& v)
{
	const Vec3V v0 = V3Sub(a, p);
	const Vec3V v1 = V3Sub(b, p);
	const Vec3V d = V3Sub(v1, v0);
	const FloatV denominator = V3Dot(d, d);
	const FloatV numerator = V3Dot(V3Neg(v0), d);
	v = FDiv(numerator, denominator);
}

void Gu::barycentricCoordinates(const Ps::aos::Vec3VArg p, const Ps::aos::Vec3VArg a, const Ps::aos::Vec3VArg b, const Ps::aos::Vec3VArg c, Ps::aos::FloatV& v, Ps::aos::FloatV& w)
{
	const Vec3V ab = V3Sub(b, a);
	const Vec3V ac = V3Sub(c, a);

	const Vec3V n = V3Cross(ab, ac);

	const VecCrossV crossA = V3PrepareCross(V3Sub(a, p));
	const VecCrossV crossB = V3PrepareCross(V3Sub(b, p));
	const VecCrossV crossC = V3PrepareCross(V3Sub(c, p));
	const Vec3V bCrossC = V3Cross(crossB, crossC);
	const Vec3V cCrossA = V3Cross(crossC, crossA);
	const Vec3V aCrossB = V3Cross(crossA, crossB);

	const FloatV va = V3Dot(n, bCrossC);//edge region of BC, signed area rbc, u = S(rbc)/S(abc) for a
	const FloatV vb = V3Dot(n, cCrossA);//edge region of AC, signed area rac, v = S(rca)/S(abc) for b
	const FloatV vc = V3Dot(n, aCrossB);//edge region of AB, signed area rab, w = S(rab)/S(abc) for c
	const FloatV totalArea =FAdd(va, FAdd(vb, vc));
	const FloatV zero = FZero();
	const FloatV denom = FSel(FIsEq(totalArea, zero), zero, FRecip(totalArea));
	v = FMul(vb, denom);
	w = FMul(vc, denom);
	
}

/*
	v0 = b - a;
	v1 = c - a;
	v2 = p - a;
*/
void Gu::barycentricCoordinates(const Vec3VArg v0, const Vec3VArg v1, const Vec3VArg v2, FloatV& v, FloatV& w)
{
	const FloatV d00 = V3Dot(v0, v0);
	const FloatV d01 = V3Dot(v0, v1);
	const FloatV d11 = V3Dot(v1, v1);
	const FloatV d20 = V3Dot(v2, v0);
	const FloatV d21 = V3Dot(v2, v1);
	const FloatV denom = FRecip(FSub(FMul(d00,d11), FMul(d01, d01)));
	v = FMul(FSub(FMul(d11, d20), FMul(d01, d21)), denom);
	w = FMul(FSub(FMul(d00, d21), FMul(d01, d20)), denom);
}

