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

#ifndef GU_VEC_PLANE_H
#define GU_VEC_PLANE_H
/** \addtogroup geomutils
@{
*/

#include "foundation/PxVec3.h"
#include "foundation/PxPlane.h"
#include "CmPhysXCommon.h"
#include "PsVecMath.h"

/**
\brief Representation of a plane.

Plane equation used: a*x + b*y + c*z + d = 0
*/
namespace physx
{
namespace Gu
{
	
	class PlaneV
	{
	public:
		/**
		\brief Constructor
		*/
		PX_FORCE_INLINE PlaneV()
		{
		}

		/**
		\brief Constructor from a normal and a distance
		*/
		PX_FORCE_INLINE PlaneV(const Ps::aos::FloatVArg nx, const Ps::aos::FloatVArg ny, const Ps::aos::FloatVArg nz, const Ps::aos::FloatVArg _d)
		{
			set(nx, ny, nz, _d);
		}

		PX_FORCE_INLINE PlaneV(const PxPlane& plane)
		{
			using namespace Ps::aos;
			const Vec3V _n = V3LoadU(plane.n); 
			const FloatV _d = FLoad(plane.d);
			nd = V4SetW(Vec4V_From_Vec3V(_n), _d);
		}


		/**
		\brief Constructor from three points
		*/
		PX_FORCE_INLINE PlaneV(const Ps::aos::Vec3VArg p0, const Ps::aos::Vec3VArg p1, const Ps::aos::Vec3VArg p2)
		{
			set(p0, p1, p2);
		}

		/**
		\brief Constructor from a normal and a distance
		*/ 
		PX_FORCE_INLINE PlaneV(const Ps::aos::Vec3VArg _n, const Ps::aos::FloatVArg _d) 
		{
			nd = Ps::aos::V4SetW(Ps::aos::Vec4V_From_Vec3V(_n), _d);
		}

		/**
		\brief Copy constructor
		*/
		PX_FORCE_INLINE PlaneV(const PlaneV& plane) : nd(plane.nd)
		{
		}

		/**
		\brief Destructor
		*/
		PX_FORCE_INLINE ~PlaneV()
		{
		}

		/**
		\brief Sets plane to zero.
		*/
		PX_FORCE_INLINE PlaneV& setZero()
		{
			nd = Ps::aos::V4Zero();
			return *this;
		}

		PX_FORCE_INLINE PlaneV& set(const Ps::aos::FloatVArg nx, const Ps::aos::FloatVArg ny, const Ps::aos::FloatVArg nz, const Ps::aos::FloatVArg _d)
		{

			using namespace Ps::aos;
			const Vec3V n= V3Merge(nx, ny, nz);
			nd = V4SetW(Vec4V_From_Vec3V(n), _d);
			return *this;
		}

		PX_FORCE_INLINE PlaneV& set(const Ps::aos::Vec3VArg _normal, Ps::aos::FloatVArg _d)
		{
			nd = Ps::aos::V4SetW(Ps::aos::Vec4V_From_Vec3V(_normal), _d);
			return *this;
		}

		/**
		\brief Computes the plane equation from 3 points.
		*/
		PlaneV& set(const Ps::aos::Vec3VArg p0, const Ps::aos::Vec3VArg p1, const Ps::aos::Vec3VArg p2)
		{
			using namespace Ps::aos;
			const Vec3V edge0 = V3Sub(p1, p0);
			const Vec3V edge1 = V3Sub(p2, p0);

			const Vec3V n = V3Normalize(V3Cross(edge0, edge1));
			// See comments in set() for computation of d
			const FloatV d = FNeg(V3Dot(p0, n));
			nd = V4SetW(Vec4V_From_Vec3V(n), d);
			return	*this;
		}

		/***
		\brief Computes distance, assuming plane is normalized
		\sa normalize
		*/
		PX_FORCE_INLINE Ps::aos::FloatV distance(const Ps::aos::Vec3VArg p) const
		{
			// Valid for plane equation a*x + b*y + c*z + d = 0
			using namespace Ps::aos;
			const Vec3V n = Vec3V_From_Vec4V(nd);
			return FAdd(V3Dot(p, n), V4GetW(nd));
		}

		PX_FORCE_INLINE Ps::aos::BoolV belongs(const Ps::aos::Vec3VArg p) const
		{
			using namespace Ps::aos;
			const FloatV eps = FLoad(1.0e-7f);
			return FIsGrtr(eps, FAbs(distance(p)));
		}

		/**
		\brief projects p into the plane
		*/
		PX_FORCE_INLINE Ps::aos::Vec3V project(const Ps::aos::Vec3VArg p) const
		{
			// Pretend p is on positive side of plane, i.e. plane.distance(p)>0.
			// To project the point we have to go in a direction opposed to plane's normal, i.e.:
			using namespace Ps::aos;
			const Vec3V n = Vec3V_From_Vec4V(nd);
			return V3Sub(p, V3Scale(n, V4GetW(nd)));

		}

		PX_FORCE_INLINE Ps::aos::FloatV signedDistanceHessianNormalForm(const Ps::aos::Vec3VArg point) const
		{
			using namespace Ps::aos;
			const Vec3V n = Vec3V_From_Vec4V(nd);
			return FAdd(V3Dot(n, point), V4GetW(nd));
		}

		PX_FORCE_INLINE Ps::aos::Vec3V getNormal() const
		{
			return Ps::aos::Vec3V_From_Vec4V(nd);
		}

		PX_FORCE_INLINE Ps::aos::FloatV getSignDist() const
		{
			return Ps::aos::V4GetW(nd);
		}

		/**
		\brief find an arbitrary point in the plane
		*/
		PX_FORCE_INLINE Ps::aos::Vec3V pointInPlane() const
		{
			// Project origin (0,0,0) to plane:
			// (0) - normal * distance(0) = - normal * ((p|(0)) + d) = -normal*d
			using namespace Ps::aos;
			const Vec3V n = Vec3V_From_Vec4V(nd);
			return V3Neg(V3Scale(n, V4GetW(nd)));
		}

		PX_FORCE_INLINE void normalize()
		{
			using namespace Ps::aos;
			const Vec3V n = Vec3V_From_Vec4V(nd);
			const FloatV denom = FRecip(V3Length(n));
			V4Scale(nd, denom);
		}

		Ps::aos::Vec4V	nd;		//!< The normal to the plan , w store the distance from the origin
	};
}

}

/** @} */
#endif
