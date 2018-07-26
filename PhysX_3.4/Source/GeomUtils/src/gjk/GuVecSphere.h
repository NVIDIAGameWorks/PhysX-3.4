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

#ifndef GU_VEC_SPHERE_H
#define GU_VEC_SPHERE_H
/** \addtogroup geomutils
@{
*/

#include "GuVecConvex.h"
#include "GuConvexSupportTable.h"
#include "PxSphereGeometry.h"

/**
\brief Represents a sphere defined by its center point and radius.
*/
namespace physx
{
namespace Gu
{
	class SphereV : public ConvexV
	{
	public:
		/**
		\brief Constructor
		*/
		PX_INLINE SphereV(): ConvexV(ConvexType::eSPHERE)
		{
			radius = Ps::aos::FZero();
			bMarginIsRadius = true;
		}

		PX_INLINE SphereV(const Ps::aos::Vec3VArg _center, const Ps::aos::FloatV _radius) : ConvexV(ConvexType::eSPHERE, _center)
		{
			using namespace Ps::aos;
			radius = _radius;
			FStore(radius, &margin);
			FStore(radius, &minMargin);
			FStore(radius, &sweepMargin);
			bMarginIsRadius = true;
		}


		/**
		\brief Copy constructor
		*/
		PX_INLINE SphereV(const SphereV& sphere) : ConvexV(ConvexType::eSPHERE), radius(sphere.radius)
		{

			margin = sphere.margin;
			minMargin = sphere.minMargin;
			sweepMargin = sphere.sweepMargin;
			bMarginIsRadius = true;
		}

		PX_INLINE SphereV(const PxGeometry& geom) : ConvexV(ConvexType::eSPHERE, Ps::aos::V3Zero())
		{
			using namespace Ps::aos;
			const PxSphereGeometry& sphereGeom = static_cast<const PxSphereGeometry&>(geom);
			const FloatV r = FLoad(sphereGeom.radius);
			radius = r;
			margin = sphereGeom.radius;
			minMargin = sphereGeom.radius;
			sweepMargin = sphereGeom.radius;
			bMarginIsRadius = true;
		}

		/**
		\brief Destructor
		*/
		PX_INLINE ~SphereV()
		{
		}

		PX_INLINE	void	setV(const Ps::aos::Vec3VArg _center, const Ps::aos::FloatVArg _radius)		
		{ 
			center = _center;
			radius = _radius;
		}

		/**
		\brief Checks the sphere is valid.

		\return		true if the sphere is valid
		*/
		PX_INLINE bool isValid() const
		{
			// Consistency condition for spheres: Radius >= 0.0f
			using namespace Ps::aos;
			return BAllEqTTTT(FIsGrtrOrEq(radius, FZero())) != 0;
		}

		/**
		\brief Tests if a point is contained within the sphere.

		\param[in] p the point to test
		\return	true if inside the sphere
		*/
		PX_INLINE bool contains(const Ps::aos::Vec3VArg p) const
		{
			using namespace Ps::aos;
			const FloatV rr = FMul(radius, radius);
			const FloatV cc =  V3LengthSq(V3Sub(center, p));
			return FAllGrtrOrEq(rr, cc) != 0;
		}

		/**
		\brief Tests if a sphere is contained within the sphere.

		\param		sphere	[in] the sphere to test
		\return		true if inside the sphere
		*/
		PX_INLINE bool contains(const SphereV& sphere)	const
		{
			using namespace Ps::aos;
			
			const Vec3V centerDif= V3Sub(center, sphere.center);
			const FloatV radiusDif = FSub(radius, sphere.radius);
			const FloatV cc = V3Dot(centerDif, centerDif);
			const FloatV rr = FMul(radiusDif, radiusDif); 

			const BoolV con0 = FIsGrtrOrEq(radiusDif, FZero());//might contain
			const BoolV con1 = FIsGrtr(rr, cc);//return true
			return BAllEqTTTT(BAnd(con0, con1))==1;
		}

		/**
		\brief Tests if a box is contained within the sphere.

		\param		minimum		[in] minimum value of the box
		\param		maximum		[in] maximum value of the box
		\return		true if inside the sphere
		*/
		PX_INLINE bool contains(const Ps::aos::Vec3VArg minimum, const Ps::aos::Vec3VArg maximum) const
		{
		
			//compute the sphere which wrap around the box
			using namespace Ps::aos;
			const FloatV zero = FZero();
			const FloatV half = FHalf();

			const Vec3V boxSphereCenter = V3Scale(V3Add(maximum, minimum), half);
			const Vec3V v = V3Scale(V3Sub(maximum, minimum), half);
			const FloatV boxSphereR = V3Length(v);

			const Vec3V w = V3Sub(center, boxSphereCenter);
			const FloatV wLength = V3Length(w);
			const FloatV dif = FSub(FSub(radius, wLength), boxSphereR); 

			return FAllGrtrOrEq(dif, zero) != 0;
		}           
                  
		/**
		\brief Tests if the sphere intersects another sphere

		\param		sphere	[in] the other sphere
		\return		true if spheres overlap
		*/
		PX_INLINE bool intersect(const SphereV& sphere) const
		{
			using namespace Ps::aos;
			const Vec3V centerDif = V3Sub(center, sphere.center);
			const FloatV cc = V3Dot(centerDif, centerDif);
			const FloatV r = FAdd(radius, sphere.radius);
			const FloatV rr = FMul(r, r);
			return FAllGrtrOrEq(rr, cc) != 0;
		}

		//return point in local space
		PX_FORCE_INLINE Ps::aos::Vec3V getPoint(const PxU8)
		{
			return Ps::aos::V3Zero();
		}
  //  
		//sweep code need to have full version
		PX_FORCE_INLINE Ps::aos::Vec3V supportSweep(const Ps::aos::Vec3VArg dir)const
		{
			using namespace Ps::aos;
			const Vec3V _dir = V3Normalize(dir);
			return V3ScaleAdd(_dir, radius, center);  
		}

		//make the support function the same as support margin
		PX_FORCE_INLINE Ps::aos::Vec3V support(const Ps::aos::Vec3VArg)const
		{
			return center;//_margin is the same as radius
		}
  

		PX_FORCE_INLINE Ps::aos::Vec3V supportMargin(const Ps::aos::Vec3VArg dir, const Ps::aos::FloatVArg _margin, Ps::aos::Vec3V& support)const
		{
			PX_UNUSED(_margin);
			PX_UNUSED(dir);

			support = center;
			return center;//_margin is the same as radius
		}

		PX_FORCE_INLINE Ps::aos::BoolV isMarginEqRadius()const
		{
			return Ps::aos::BTTTT();
		}

		PX_FORCE_INLINE Ps::aos::FloatV getSweepMargin() const
		{
			return Ps::aos::FZero();
		}


		Ps::aos::FloatV radius;		//!< Sphere's center, w component is radius

	};
}

}

#endif
