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
#include "GuDistancePointTriangle.h"
#include "GuDistancePointTriangleSIMD.h"

using namespace physx;

// Based on Christer Ericson's book
PxVec3 Gu::closestPtPointTriangle(const PxVec3& p, const PxVec3& a, const PxVec3& b, const PxVec3& c, float& s, float& t)
{
	// Check if P in vertex region outside A
	const PxVec3 ab = b - a;
	const PxVec3 ac = c - a;
	const PxVec3 ap = p - a;
	const float d1 = ab.dot(ap);
	const float d2 = ac.dot(ap);
	if(d1<=0.0f && d2<=0.0f)
	{
		s = 0.0f;
		t = 0.0f;
		return a;	// Barycentric coords 1,0,0
	}

	// Check if P in vertex region outside B
	const PxVec3 bp = p - b;
	const float d3 = ab.dot(bp);
	const float d4 = ac.dot(bp);
	if(d3>=0.0f && d4<=d3)
	{
		s = 1.0f;
		t = 0.0f;
		return b;	// Barycentric coords 0,1,0
	}

	// Check if P in edge region of AB, if so return projection of P onto AB
	const float vc = d1*d4 - d3*d2;
	if(vc<=0.0f && d1>=0.0f && d3<=0.0f)
	{
		const float v = d1 / (d1 - d3);
		s = v;
		t = 0.0f;
		return a + v * ab;	// barycentric coords (1-v, v, 0)
	}

	// Check if P in vertex region outside C
	const PxVec3 cp = p - c;
	const float d5 = ab.dot(cp);
	const float d6 = ac.dot(cp);
	if(d6>=0.0f && d5<=d6)
	{
		s = 0.0f;
		t = 1.0f;
		return c;	// Barycentric coords 0,0,1
	}

	// Check if P in edge region of AC, if so return projection of P onto AC
	const float vb = d5*d2 - d1*d6;
	if(vb<=0.0f && d2>=0.0f && d6<=0.0f)
	{
		const float w = d2 / (d2 - d6);
		s = 0.0f;
		t = w;
		return a + w * ac;	// barycentric coords (1-w, 0, w)
	}

	// Check if P in edge region of BC, if so return projection of P onto BC
	const float va = d3*d6 - d5*d4;
	if(va<=0.0f && (d4-d3)>=0.0f && (d5-d6)>=0.0f)
	{
		const float w = (d4-d3) / ((d4 - d3) + (d5-d6));
		s = 1.0f-w;
		t = w;
		return b + w * (c-b);	// barycentric coords (0, 1-w, w)
	}

	// P inside face region. Compute Q through its barycentric coords (u,v,w)
	const float denom = 1.0f / (va + vb + vc);
	const float v = vb * denom;
	const float w = vc * denom;
	s = v;
	t = w;
	return a + ab*v + ac*w;
}

//Ps::aos::FloatV Gu::distancePointTriangleSquared(	const Ps::aos::Vec3VArg p, 
//													const Ps::aos::Vec3VArg a, 
//													const Ps::aos::Vec3VArg b, 
//													const Ps::aos::Vec3VArg c,
//													Ps::aos::FloatV& u,
//													Ps::aos::FloatV& v,
//													Ps::aos::Vec3V& closestP)
//{
//	using namespace Ps::aos;
//
//	const FloatV zero = FZero();
//	const FloatV one = FOne();
//	//const Vec3V zero = V3Zero();
//	const Vec3V ab = V3Sub(b, a);
//	const Vec3V ac = V3Sub(c, a);
//	const Vec3V bc = V3Sub(c, b);
//	const Vec3V ap = V3Sub(p, a);
//	const Vec3V bp = V3Sub(p, b);
//	const Vec3V cp = V3Sub(p, c);
//
//	const FloatV d1 = V3Dot(ab, ap); //  snom
//	const FloatV d2 = V3Dot(ac, ap); //  tnom
//	const FloatV d3 = V3Dot(ab, bp); // -sdenom
//	const FloatV d4 = V3Dot(ac, bp); //  unom = d4 - d3
//	const FloatV d5 = V3Dot(ab, cp); //  udenom = d5 - d6
//	const FloatV d6 = V3Dot(ac, cp); // -tdenom
//	const FloatV unom = FSub(d4, d3);
//	const FloatV udenom = FSub(d5, d6);
//	
//	//check if p in vertex region outside a
//	const BoolV con00 = FIsGrtr(zero, d1); // snom <= 0
//	const BoolV con01 = FIsGrtr(zero, d2); // tnom <= 0
//	const BoolV con0 = BAnd(con00, con01); // vertex region a
//	const FloatV u0 = zero;
//	const FloatV v0 = zero;
//
//	//check if p in vertex region outside b
//	const BoolV con10 = FIsGrtrOrEq(d3, zero);
//	const BoolV con11 = FIsGrtrOrEq(d3, d4);
//	const BoolV con1 = BAnd(con10, con11); // vertex region b
//	const FloatV u1 = one;
//	const FloatV v1 = zero;
//
//	//check if p in vertex region outside c
//	const BoolV con20 = FIsGrtrOrEq(d6, zero);
//	const BoolV con21 = FIsGrtrOrEq(d6, d5); 
//	const BoolV con2 = BAnd(con20, con21); // vertex region c
//	const FloatV u2 = zero;
//	const FloatV v2 = one;
//
//	//check if p in edge region of AB
//	const FloatV vc = FSub(FMul(d1, d4), FMul(d3, d2));
//	
//	const BoolV con30 = FIsGrtr(zero, vc);
//	const BoolV con31 = FIsGrtrOrEq(d1, zero);
//	const BoolV con32 = FIsGrtr(zero, d3);
//	const BoolV con3 = BAnd(con30, BAnd(con31, con32));
//	const FloatV sScale = FDiv(d1, FSub(d1, d3));
//	const Vec3V closest3 = V3Add(a, V3Scale(ab, sScale));
//	const FloatV u3 = sScale;
//	const FloatV v3 = zero;
//
//	//check if p in edge region of BC
//	const FloatV va = FSub(FMul(d3, d6),FMul(d5, d4));
//	const BoolV con40 = FIsGrtr(zero, va);
//	const BoolV con41 = FIsGrtrOrEq(d4, d3);
//	const BoolV con42 = FIsGrtrOrEq(d5, d6);
//	const BoolV con4 = BAnd(con40, BAnd(con41, con42)); 
//	const FloatV uScale = FDiv(unom, FAdd(unom, udenom));
//	const Vec3V closest4 = V3Add(b, V3Scale(bc, uScale));
//	const FloatV u4 = FSub(one, uScale);
//	const FloatV v4 = uScale;
//
//	//check if p in edge region of AC
//	const FloatV vb = FSub(FMul(d5, d2), FMul(d1, d6));
//	const BoolV con50 = FIsGrtr(zero, vb);
//	const BoolV con51 = FIsGrtrOrEq(d2, zero);
//	const BoolV con52 = FIsGrtr(zero, d6);
//	const BoolV con5 = BAnd(con50, BAnd(con51, con52));
//	const FloatV tScale = FDiv(d2, FSub(d2, d6));
//	const Vec3V closest5 = V3Add(a, V3Scale(ac, tScale));
//	const FloatV u5 = zero;
//	const FloatV v5 = tScale;
//
//	//P must project inside face region. Compute Q using Barycentric coordinates
//	const FloatV denom = FRecip(FAdd(va, FAdd(vb, vc)));
//	const FloatV t = FMul(vb, denom);
//	const FloatV w = FMul(vc, denom);
//	const Vec3V bCom = V3Scale(ab, t);
//	const Vec3V cCom = V3Scale(ac, w);
//	const Vec3V closest6 = V3Add(a, V3Add(bCom, cCom));
//	const FloatV u6 = t;
//	const FloatV v6 = w;
//	
//	const Vec3V closest= V3Sel(con0, a, V3Sel(con1, b, V3Sel(con2, c, V3Sel(con3, closest3, V3Sel(con4, closest4, V3Sel(con5, closest5, closest6))))));
//	u = FSel(con0, u0, FSel(con1, u1, FSel(con2, u2, FSel(con3, u3, FSel(con4, u4, FSel(con5, u5, u6))))));
//	v = FSel(con0, v0, FSel(con1, v1, FSel(con2, v2, FSel(con3, v3, FSel(con4, v4, FSel(con5, v5, v6))))));
//	closestP = closest;
//
//	const Vec3V vv = V3Sub(p, closest);
//
//	return V3Dot(vv, vv);
//}


Ps::aos::FloatV Gu::distancePointTriangleSquared(	const Ps::aos::Vec3VArg p, 
													const Ps::aos::Vec3VArg a, 
													const Ps::aos::Vec3VArg b, 
													const Ps::aos::Vec3VArg c,
													Ps::aos::FloatV& u,
													Ps::aos::FloatV& v,
													Ps::aos::Vec3V& closestP)
{
	using namespace Ps::aos;

	const FloatV zero = FZero();
	const FloatV one = FOne();
	//const Vec3V zero = V3Zero();
	const Vec3V ab = V3Sub(b, a);
	const Vec3V ac = V3Sub(c, a);
	const Vec3V bc = V3Sub(c, b);
	const Vec3V ap = V3Sub(p, a);
	const Vec3V bp = V3Sub(p, b);
	const Vec3V cp = V3Sub(p, c);

	const FloatV d1 = V3Dot(ab, ap); //  snom
	const FloatV d2 = V3Dot(ac, ap); //  tnom
	const FloatV d3 = V3Dot(ab, bp); // -sdenom
	const FloatV d4 = V3Dot(ac, bp); //  unom = d4 - d3
	const FloatV d5 = V3Dot(ab, cp); //  udenom = d5 - d6
	const FloatV d6 = V3Dot(ac, cp); // -tdenom
	const FloatV unom = FSub(d4, d3);
	const FloatV udenom = FSub(d5, d6);
	
	//check if p in vertex region outside a
	const BoolV con00 = FIsGrtr(zero, d1); // snom <= 0
	const BoolV con01 = FIsGrtr(zero, d2); // tnom <= 0
	const BoolV con0 = BAnd(con00, con01); // vertex region a

	if(BAllEqTTTT(con0))
	{
		u = zero;
		v = zero;
		const Vec3V vv = V3Sub(p, a);
		closestP = a;
		return V3Dot(vv, vv);
	}

	//check if p in vertex region outside b
	const BoolV con10 = FIsGrtrOrEq(d3, zero);
	const BoolV con11 = FIsGrtrOrEq(d3, d4);
	const BoolV con1 = BAnd(con10, con11); // vertex region b
	if(BAllEqTTTT(con1))
	{
		u = one;
		v = zero;
		const Vec3V vv = V3Sub(p, b);
		closestP = b;
		return V3Dot(vv, vv);
	}

	//check if p in vertex region outside c
	const BoolV con20 = FIsGrtrOrEq(d6, zero);
	const BoolV con21 = FIsGrtrOrEq(d6, d5); 
	const BoolV con2 = BAnd(con20, con21); // vertex region c
	if(BAllEqTTTT(con2))
	{
		u = zero;
		v = one;
		const Vec3V vv = V3Sub(p, c);
		closestP = c;
		return V3Dot(vv, vv);
	}

	//check if p in edge region of AB
	const FloatV vc = FSub(FMul(d1, d4), FMul(d3, d2));
	
	const BoolV con30 = FIsGrtr(zero, vc);
	const BoolV con31 = FIsGrtrOrEq(d1, zero);
	const BoolV con32 = FIsGrtr(zero, d3);
	const BoolV con3 = BAnd(con30, BAnd(con31, con32));
	if(BAllEqTTTT(con3))
	{
		const FloatV sScale = FDiv(d1, FSub(d1, d3));
		const Vec3V closest3 = V3Add(a, V3Scale(ab, sScale));
		u = sScale;
		v = zero;
		const Vec3V vv = V3Sub(p, closest3);
		closestP = closest3;
		return V3Dot(vv, vv);
	}

	//check if p in edge region of BC
	const FloatV va = FSub(FMul(d3, d6),FMul(d5, d4));
	const BoolV con40 = FIsGrtr(zero, va);
	const BoolV con41 = FIsGrtrOrEq(d4, d3);
	const BoolV con42 = FIsGrtrOrEq(d5, d6);
	const BoolV con4 = BAnd(con40, BAnd(con41, con42)); 
	if(BAllEqTTTT(con4))
	{
		const FloatV uScale = FDiv(unom, FAdd(unom, udenom));
		const Vec3V closest4 = V3Add(b, V3Scale(bc, uScale));
		u = FSub(one, uScale);
		v = uScale;
		const Vec3V vv = V3Sub(p, closest4);
		closestP = closest4;
		return V3Dot(vv, vv);
	}

	//check if p in edge region of AC
	const FloatV vb = FSub(FMul(d5, d2), FMul(d1, d6));
	const BoolV con50 = FIsGrtr(zero, vb);
	const BoolV con51 = FIsGrtrOrEq(d2, zero);
	const BoolV con52 = FIsGrtr(zero, d6);
	const BoolV con5 = BAnd(con50, BAnd(con51, con52));
	if(BAllEqTTTT(con5))
	{
		const FloatV tScale = FDiv(d2, FSub(d2, d6));
		const Vec3V closest5 = V3Add(a, V3Scale(ac, tScale));
		u = zero;
		v = tScale;
		const Vec3V vv = V3Sub(p, closest5);
		closestP = closest5;
		return V3Dot(vv, vv);
	}

	//P must project inside face region. Compute Q using Barycentric coordinates
	const FloatV denom = FRecip(FAdd(va, FAdd(vb, vc)));
	const FloatV t = FMul(vb, denom);
	const FloatV w = FMul(vc, denom);
	const Vec3V bCom = V3Scale(ab, t);
	const Vec3V cCom = V3Scale(ac, w);
	const Vec3V closest6 = V3Add(a, V3Add(bCom, cCom));
	u = t;
	v = w;
	closestP = closest6;

	const Vec3V vv = V3Sub(p, closest6);

	return V3Dot(vv, vv);
}
