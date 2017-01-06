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

#ifndef GU_VEC_CAPSULE_H
#define GU_VEC_CAPSULE_H

/** \addtogroup geomutils
@{
*/

#include "GuVecConvex.h"   
#include "GuConvexSupportTable.h"
#include "PxCapsuleGeometry.h"


namespace physx
{
namespace Gu
{

	PX_FORCE_INLINE Ps::aos::FloatV CalculateCapsuleMinMargin(const Ps::aos::FloatVArg radius)
	{
		using namespace Ps::aos;
		const FloatV ratio = Ps::aos::FLoad(0.05f);
		return FMul(radius, ratio);
	}

	class CapsuleV : public ConvexV  
	{
	public:
		/**
		\brief Constructor
		*/

		PX_INLINE CapsuleV():ConvexV(ConvexType::eCAPSULE)
		{
			bMarginIsRadius = true;
		}

		//constructor for sphere
		PX_INLINE CapsuleV(const Ps::aos::Vec3VArg p, const Ps::aos::FloatVArg radius_) : ConvexV(ConvexType::eCAPSULE)
		{
			using namespace Ps::aos;
			center = p;
			radius = radius_;
			p0 = p;
			p1 = p;
			FStore(radius, &margin);
			FStore(radius, &minMargin);
			FStore(radius, &sweepMargin);
			bMarginIsRadius = true; 
		}

		PX_INLINE CapsuleV(const Ps::aos::Vec3VArg center_, const Ps::aos::Vec3VArg v_, const Ps::aos::FloatVArg radius_) : 
			ConvexV(ConvexType::eCAPSULE, center_)
		{
			using namespace Ps::aos;
			radius = radius_;
			p0 = V3Add(center_, v_);
			p1 = V3Sub(center_, v_);
			FStore(radius, &margin);
			FStore(radius, &minMargin);
			FStore(radius, &sweepMargin);
			bMarginIsRadius = true;
		}

		PX_INLINE CapsuleV(const PxGeometry& geom) : ConvexV(ConvexType::eCAPSULE, Ps::aos::V3Zero())
		{
			using namespace Ps::aos;
			const PxCapsuleGeometry& capsuleGeom = static_cast<const PxCapsuleGeometry&>(geom);

			const Vec3V axis = V3Scale(V3UnitX(), FLoad(capsuleGeom.halfHeight));
			const FloatV r = FLoad(capsuleGeom.radius);
			p0 = axis;
			p1 = V3Neg(axis);
			radius = r;
			FStore(radius, &margin);
			FStore(radius, &minMargin);
			FStore(radius, &sweepMargin);
			bMarginIsRadius = true;
		}

		/**
		\brief Constructor

		\param _radius Radius of the capsule.
		*/

		/**
		\brief Destructor
		*/
		PX_INLINE ~CapsuleV()
		{
		}

		PX_FORCE_INLINE void initialize(const Ps::aos::Vec3VArg _p0, const Ps::aos::Vec3VArg _p1, const Ps::aos::FloatVArg _radius)
		{
			using namespace Ps::aos;
			radius = _radius;
			p0 = _p0;
			p1 = _p1;
			FStore(radius, &margin);
			FStore(radius, &minMargin);
			FStore(radius, &sweepMargin);
			center = V3Scale(V3Add(_p0, _p1), FHalf());
		}   

		PX_INLINE Ps::aos::Vec3V computeDirection() const
		{
			return Ps::aos::V3Sub(p1, p0);
		}

		PX_FORCE_INLINE	Ps::aos::FloatV	getRadius()	const
		{
			return radius;
		}

		PX_FORCE_INLINE Ps::aos::Vec3V supportPoint(const PxI32 index, Ps::aos::FloatV* /*marginDif*/)const
		{
			return (&p0)[1-index];
		}

		PX_FORCE_INLINE void getIndex(const Ps::aos::BoolV con, PxI32& index)const
		{
			using namespace Ps::aos;
			const VecI32V v = VecI32V_From_BoolV(con);
			const VecI32V t = VecI32V_And(v, VecI32V_One());
			PxI32_From_VecI32V(t, &index);
		}

		PX_FORCE_INLINE void setCenter(const Ps::aos::Vec3VArg _center)
		{
			using namespace Ps::aos;
			Vec3V offset = V3Sub(_center, center);
			center = _center;

			p0 = V3Add(p0, offset);
			p1 = V3Add(p1, offset);
		}

		//dir, p0 and p1 are in the local space of dir
		PX_FORCE_INLINE Ps::aos::Vec3V supportLocal(const Ps::aos::Vec3VArg dir)const
		{
			using namespace Ps::aos;
			//const Vec3V _dir = V3Normalize(dir);
			const FloatV dist0 = V3Dot(p0, dir);
			const FloatV dist1 = V3Dot(p1, dir);
			return V3Sel(FIsGrtr(dist0, dist1), p0, p1);
		}
	
		PX_FORCE_INLINE Ps::aos::Vec3V supportRelative(const Ps::aos::Vec3VArg dir, const Ps::aos::PsMatTransformV& aToB, const Ps::aos::PsMatTransformV& aTobT) const
		{
			using namespace Ps::aos;
			//transform dir into the local space of a
//			const Vec3V _dir = aToB.rotateInv(dir);
			const Vec3V _dir = aTobT.rotate(dir);
			const Vec3V p = supportLocal(_dir);
			//transform p back to the local space of b
			return aToB.transform(p);
		}

		//dir, p0 and p1 are in the local space of dir
		PX_FORCE_INLINE Ps::aos::Vec3V supportLocal(const Ps::aos::Vec3VArg dir, PxI32& index, Ps::aos::FloatV* /*marginDif*/)const
		{
			using namespace Ps::aos;
			
			const FloatV dist0 = V3Dot(p0, dir);
			const FloatV dist1 = V3Dot(p1, dir);
			const BoolV comp = FIsGrtr(dist0, dist1);
			getIndex(comp, index);
			return V3Sel(comp, p0, p1);
		}
	
		PX_FORCE_INLINE Ps::aos::Vec3V supportRelative(	const Ps::aos::Vec3VArg dir, const Ps::aos::PsMatTransformV& aToB,
														const Ps::aos::PsMatTransformV& aTobT, PxI32& index, Ps::aos::FloatV* marginDif)const
		{
			using namespace Ps::aos;
			//transform dir into the local space of a
//			const Vec3V _dir = aToB.rotateInv(dir);
			const Vec3V _dir = aTobT.rotate(dir);

			const Vec3V p = supportLocal(_dir, index, marginDif);
			//transform p back to the local space of b
			return aToB.transform(p);
		}

		//PX_FORCE_INLINE Ps::aos::BoolV supportLocalIndex(const Ps::aos::Vec3VArg dir, PxI32& index)const
		//{
		//	using namespace Ps::aos;
		//	//scale dir and put it in the vertex space
		//	const FloatV dist0 = V3Dot(p0, dir);
		//	const FloatV dist1 = V3Dot(p1, dir);
		//	const BoolV comp = FIsGrtr(dist0, dist1);
		//	getIndex(comp, index);
		//	return comp;
		//}

		PX_FORCE_INLINE Ps::aos::Vec3V supportLocal(Ps::aos::Vec3V& support, const PxI32& index, const Ps::aos::BoolV comp)const
		{
			PX_UNUSED(index);

			using namespace Ps::aos;
			const Vec3V p = V3Sel(comp, p0, p1);
			support = p;
			return p;
		}

		//PX_FORCE_INLINE Ps::aos::BoolV supportRelativeIndex(const Ps::aos::Vec3VArg dir, const Ps::aos::PsMatTransformV& aTob, PxI32& index)const
		//{
		//	using namespace Ps::aos;
		//	//scale dir and put it in the vertex space
		//	const Vec3V _dir =aTob.rotateInv(dir);//relTra.rotateInv(dir);
		//	return supportLocalIndex(_dir, index);
		//}

		PX_FORCE_INLINE Ps::aos::FloatV getSweepMargin() const
		{
			return Ps::aos::FZero();
		}

		//don't change the order of p0 and p1, the getPoint function depend on the order
		Ps::aos::Vec3V	p0;		//!< Start of segment
		Ps::aos::Vec3V	p1;		//!< End of segment
		Ps::aos::FloatV	radius;
	};
}

}

#endif
