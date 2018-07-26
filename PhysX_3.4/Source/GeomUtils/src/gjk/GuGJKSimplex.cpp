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
// Copyright (c) 2008-2018 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

#include "GuGJKSimplex.h"

namespace physx
{
namespace Gu
{

	using namespace Ps::aos;

	static Vec3V getClosestPtPointTriangle(Vec3V* PX_RESTRICT Q, const BoolVArg bIsOutside4, PxU32* indices, PxU32& size)
	{
		FloatV bestSqDist = FMax();
		
		PxU32 _indices[3] = {0, 1, 2};

		Vec3V closestPt = V3Zero();
	
		if(BAllEqTTTT(BGetX(bIsOutside4)))
		{
			//use the original indices, size, v and w
			bestSqDist = closestPtPointTriangleBaryCentric(Q[0], Q[1], Q[2], indices, size, closestPt);
		}

		if(BAllEqTTTT(BGetY(bIsOutside4)))
		{

			PxU32 _size = 3;
			_indices[0] = 0; _indices[1] = 2; _indices[2] = 3; 
			Vec3V tClosestPt;
			const FloatV sqDist = closestPtPointTriangleBaryCentric(Q[0], Q[2], Q[3], _indices, _size, tClosestPt);

			const BoolV con = FIsGrtr(bestSqDist, sqDist);
			if(BAllEqTTTT(con))
			{
				closestPt = tClosestPt;
				bestSqDist = sqDist;

				indices[0] = _indices[0];
				indices[1] = _indices[1];
				indices[2] = _indices[2];

				size = _size;
			}
		}

		if(BAllEqTTTT(BGetZ(bIsOutside4)))
		{
			PxU32 _size = 3;
			
			_indices[0] = 0; _indices[1] = 3; _indices[2] = 1; 

			Vec3V tClosestPt;
			const FloatV sqDist = closestPtPointTriangleBaryCentric(Q[0], Q[3], Q[1], _indices, _size, tClosestPt);
			
			const BoolV con = FIsGrtr(bestSqDist, sqDist);
			if(BAllEqTTTT(con))
			{
				closestPt = tClosestPt;
				bestSqDist = sqDist;

				indices[0] = _indices[0];
				indices[1] = _indices[1];
				indices[2] = _indices[2];

				size = _size;
			}

		}

		if(BAllEqTTTT(BGetW(bIsOutside4)))
		{
	

			PxU32 _size = 3;
			_indices[0] = 1; _indices[1] = 3; _indices[2] = 2; 
			Vec3V tClosestPt;
			const FloatV sqDist = closestPtPointTriangleBaryCentric(Q[1], Q[3], Q[2], _indices, _size, tClosestPt);

			const BoolV con = FIsGrtr(bestSqDist, sqDist);

			if(BAllEqTTTT(con))
			{
				closestPt = tClosestPt;
				bestSqDist = sqDist;

				indices[0] = _indices[0];
				indices[1] = _indices[1];
				indices[2] = _indices[2];

				size = _size;
			}
		}

		return closestPt;
	}

	PX_NOALIAS Vec3V closestPtPointTetrahedron(Vec3V* PX_RESTRICT Q, Vec3V* PX_RESTRICT A, Vec3V* PX_RESTRICT B, PxU32& size)
	{
		
		const FloatV eps = FLoad(1e-4f);
		const Vec3V a = Q[0];
		const Vec3V b = Q[1];
		const Vec3V c = Q[2];  
		const Vec3V d = Q[3];

		//degenerated
		const Vec3V ab = V3Sub(b, a);
		const Vec3V ac = V3Sub(c, a);
		const Vec3V n = V3Normalize(V3Cross(ab, ac));
		const FloatV signDist = V3Dot(n, V3Sub(d, a));
		if(FAllGrtr(eps, FAbs(signDist)))
		{
			size = 3;
			return closestPtPointTriangle(Q, A, B, size);
		}

		const BoolV bIsOutside4 = PointOutsideOfPlane4(a, b, c, d);

		if(BAllEqFFFF(bIsOutside4))
		{
			//All inside
			return V3Zero();
		}

		PxU32 indices[3] = {0, 1, 2};
		
		const Vec3V closest = getClosestPtPointTriangle(Q, bIsOutside4, indices, size);

		const Vec3V q0 = Q[indices[0]]; const Vec3V q1 = Q[indices[1]]; const Vec3V q2 = Q[indices[2]];
		const Vec3V a0 = A[indices[0]]; const Vec3V a1 = A[indices[1]]; const Vec3V a2 = A[indices[2]];
		const Vec3V b0 = B[indices[0]]; const Vec3V b1 = B[indices[1]]; const Vec3V b2 = B[indices[2]];
		Q[0] = q0; Q[1] = q1; Q[2] = q2;
		A[0] = a0; A[1] = a1; A[2] = a2;
		B[0] = b0; B[1] = b1; B[2] = b2; 

		return closest;
	}

	PX_NOALIAS Vec3V closestPtPointTetrahedron(Vec3V* PX_RESTRICT Q, Vec3V* PX_RESTRICT A, Vec3V* PX_RESTRICT B, PxI32* PX_RESTRICT aInd,  PxI32* PX_RESTRICT bInd, PxU32& size)
	{
		
		const FloatV eps = FLoad(1e-4f);
		const Vec3V zeroV = V3Zero();
		
		const Vec3V a = Q[0];
		const Vec3V b = Q[1];
		const Vec3V c = Q[2];
		const Vec3V d = Q[3];

		//degenerated
		const Vec3V ab = V3Sub(b, a);
		const Vec3V ac = V3Sub(c, a);
		const Vec3V n = V3Normalize(V3Cross(ab, ac));
		const FloatV signDist = V3Dot(n, V3Sub(d, a));
		if(FAllGrtr(eps, FAbs(signDist)))
		{
			size = 3;
			return closestPtPointTriangle(Q, A, B, aInd, bInd, size);
		}

		const BoolV bIsOutside4 = PointOutsideOfPlane4(a, b, c, d);

		if(BAllEqFFFF(bIsOutside4))
		{
			//All inside
			return zeroV;
		}

		PxU32 indices[3] = {0, 1, 2};
		const Vec3V closest = getClosestPtPointTriangle(Q, bIsOutside4, indices, size);

		const Vec3V q0 = Q[indices[0]]; const Vec3V q1 = Q[indices[1]]; const Vec3V q2 = Q[indices[2]];
		const Vec3V a0 = A[indices[0]]; const Vec3V a1 = A[indices[1]]; const Vec3V a2 = A[indices[2]];
		const Vec3V b0 = B[indices[0]]; const Vec3V b1 = B[indices[1]]; const Vec3V b2 = B[indices[2]];
		const PxI32 _aInd0 = aInd[indices[0]]; const PxI32 _aInd1 = aInd[indices[1]]; const PxI32 _aInd2 = aInd[indices[2]];
		const PxI32 _bInd0 = bInd[indices[0]]; const PxI32 _bInd1 = bInd[indices[1]]; const PxI32 _bInd2 = bInd[indices[2]];
		Q[0] = q0; Q[1] = q1; Q[2] = q2;
		A[0] = a0; A[1] = a1; A[2] = a2;
		B[0] = b0; B[1] = b1; B[2] = b2; 
		aInd[0] = _aInd0; aInd[1] = _aInd1; aInd[2] = _aInd2;
		bInd[0] = _bInd0; bInd[1] = _bInd1; bInd[2] = _bInd2;

		return closest;
	}
}

}
