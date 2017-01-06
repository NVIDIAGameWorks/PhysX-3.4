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



#ifndef DY_ARTICULATION_DEBUG_FNS_H
#define DY_ARTICULATION_DEBUG_FNS_H

#include "DyArticulationFnsScalar.h"
#include "DyArticulationFnsSimd.h"

namespace physx
{
namespace Dy
{
#if 0
	void printMomentum(const char* id, PxTransform* pose, Cm::SpatialVector* velocity, FsInertia* inertia, PxU32 linkCount)
	{
		typedef ArticulationFnsScalar Fns;

		Cm::SpatialVector m = Cm::SpatialVector::zero();
		for(PxU32 i=0;i<linkCount;i++)
			m += Fns::translateForce(pose[i].p - pose[0].p, Fns::multiply(inertia[i], velocity[i]));
		printf("momentum (%20s): (%f, %f, %f), (%f, %f, %f)\n", id, m.linear.x, m.linear.y, m.linear.z, m.angular.x, m.angular.y, m.angular.z);
	}
#endif

class ArticulationFnsDebug
{
	typedef ArticulationFnsSimdBase SimdBase;
	typedef ArticulationFnsSimd<ArticulationFnsDebug> Simd;
	typedef ArticulationFnsScalar Scalar;

public:

	static PX_FORCE_INLINE	FsInertia addInertia(const FsInertia& in1, const FsInertia& in2)
	{
		return FsInertia(M33Add(in1.ll, in2.ll),
							M33Add(in1.la, in2.la),
							M33Add(in1.aa, in2.aa));
	}

	static PX_FORCE_INLINE	FsInertia subtractInertia(const FsInertia& in1, const FsInertia& in2)
	{
		return FsInertia(M33Sub(in1.ll, in2.ll),
							M33Sub(in1.la, in2.la),
							M33Sub(in1.aa, in2.aa));
	}

	static Mat33V invertSym33(const Mat33V &m)
	{
		PxMat33 n_ = Scalar::invertSym33(unsimdify(m));
		Mat33V n = SimdBase::invertSym33(m);
		compare33(n_, unsimdify(n));

		return n;
	}

	static Mat33V invSqrt(const Mat33V &m)
	{
		PxMat33 n_ = Scalar::invSqrt(unsimdify(m));
		Mat33V n = SimdBase::invSqrt(m);
		compare33(n_, unsimdify(n));

		return n;
	}



	static FsInertia invertInertia(const FsInertia &I)
	{
		SpInertia J_ = Scalar::invertInertia(unsimdify(I));
		FsInertia J = SimdBase::invertInertia(I);
		compareInertias(J_,unsimdify(J));

		return J;
	}

	static Mat33V computeSIS(const FsInertia &I, const Cm::SpatialVectorV S[3], Cm::SpatialVectorV*PX_RESTRICT IS)
	{
		Cm::SpatialVector IS_[3];
		Scalar::multiply(IS_, unsimdify(I), unsimdify(&S[0]));
		PxMat33 D_ = Scalar::multiplySym(IS_, unsimdify(&S[0]));

		Mat33V D = SimdBase::computeSIS(I, S, IS);

		compare33(unsimdify(D), D_);

		return D;
	}


	static FsInertia multiplySubtract(const FsInertia &I, const Mat33V &D, const Cm::SpatialVectorV IS[3], Cm::SpatialVectorV*PX_RESTRICT DSI)
	{
		Cm::SpatialVector DSI_[3];

		Scalar::multiply(DSI_, unsimdify(IS), unsimdify(D));
		SpInertia J_ = Scalar::multiplySubtract(unsimdify(I), DSI_, unsimdify(IS));

		FsInertia J = SimdBase::multiplySubtract(I, D, IS, DSI);

		compareInertias(unsimdify(J), J_);

		return J;
	} 


	static FsInertia multiplySubtract(const FsInertia &I, const Cm::SpatialVectorV S[3])
	{
		SpInertia J_ = Scalar::multiplySubtract(unsimdify(I), unsimdify(S), unsimdify(S));
		FsInertia J = SimdBase::multiplySubtract(I, S);
		compareInertias(unsimdify(J), J_);
		return J;
	} 


	static FsInertia translateInertia(Vec3V offset, const FsInertia &I)
	{
		PxVec3 offset_;
		V3StoreU(offset, offset_);
		SpInertia J_ = Scalar::translate(offset_, unsimdify(I));
		FsInertia J = SimdBase::translateInertia(offset, I);
		compareInertias(J_, unsimdify(J));

		return J;
	}


	static PX_FORCE_INLINE FsInertia propagate(const FsInertia &I,
												  const Cm::SpatialVectorV S[3],
												  const Mat33V &load,
												  const FloatV isf)
	{
		SpInertia J_ = Scalar::propagate(unsimdify(I), unsimdify(&S[0]), unsimdify(load), unsimdify(isf));
		FsInertia J = Simd::propagate(I, S, load, isf);

		compareInertias(J_, unsimdify(J));
		return J;
	}


	static PX_FORCE_INLINE Mat33V computeDriveInertia(const FsInertia &I0, 
													  const	FsInertia &I1, 
													  const Cm::SpatialVectorV S[3])
	{
		PxMat33 m_ = Scalar::computeDriveInertia(unsimdify(I0), unsimdify(I1), unsimdify(&S[0]));
		Mat33V m = Simd::computeDriveInertia(I0, I1, S);

		compare33(m_, unsimdify(m));
		return m;
	}

	static const PxMat33 unsimdify(const Mat33V &m)
	{
		PX_ALIGN(16, PxMat33) m_;
		PxMat33_From_Mat33V(m, m_);
		return m_;
	}

	static PxReal unsimdify(const FloatV &m)
	{
		PxF32 f;
		FStore(m, &f);
		return f;
	}

	static SpInertia unsimdify(const FsInertia &I)
	{
		return SpInertia (unsimdify(I.ll),
						  unsimdify(I.la),
						  unsimdify(I.aa));
	}

	static const Cm::SpatialVector* unsimdify(const Cm::SpatialVectorV *S)
	{
		return reinterpret_cast<const Cm::SpatialVector*>(S);
	}


private:

	static PxReal absmax(const PxVec3& n)
	{
		return PxMax(PxAbs(n.x), PxMax(PxAbs(n.y),PxAbs(n.z)));
	}

	static PxReal norm(const PxMat33& n)
	{
		return PxMax(absmax(n.column0), PxMax(absmax(n.column1), absmax(n.column2)));
	}

	static void compare33(const PxMat33& ref, const PxMat33& n)
	{
		PxReal errNorm = norm(ref-n);	
		PX_UNUSED(errNorm);
		PX_ASSERT(errNorm <= PxMax(norm(ref)*1e-3f, 1e-4f));
	}

	static void compareInertias(const SpInertia& a, const SpInertia& b)
	{
		compare33(a.mLL, b.mLL);
		compare33(a.mLA, b.mLA);
		compare33(a.mAA, b.mAA);
	}


};

#if DY_ARTICULATION_DEBUG_VERIFY
static bool isPositiveDefinite(const Mat33V& m)
{
	PX_ALIGN_PREFIX(16) PxMat33 m1 PX_ALIGN_SUFFIX(16);
	PxMat33_From_Mat33V(m, m1);
	return isPositiveDefinite(m1);
}


static bool isPositiveDefinite(const FsInertia& s)
{
	return isPositiveDefinite(ArticulationFnsDebug::unsimdify(s));
}

static PxReal magnitude(const Cm::SpatialVectorV &v)
{
	return PxSqrt(FStore(V3Dot(v.linear, v.linear)) + FStore(V3Dot(v.angular, v.angular)));
}

static bool almostEqual(const Cm::SpatialVectorV &ref, const Cm::SpatialVectorV& test, PxReal tolerance)
{
	return magnitude(ref-test)<=tolerance*magnitude(ref);
}
#endif
}
}

#endif //DY_ARTICULATION_DEBUG_FNS_H
