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

#ifndef GU_VEC_SHRUNK_CONVEX_HULL_NO_SCALE_H
#define GU_VEC_SHRUNK_CONVEX_HULL_NO_SCALE_H

#include "foundation/PxUnionCast.h"
#include "GuVecShrunkConvexHull.h"


namespace physx
{
namespace Gu
{


	/*
		ML:
		ShrinkedConvexHull is used in GJK code but not EPA code
	*/
	class ShrunkConvexHullNoScaleV : public ShrunkConvexHullV
	{


		public:
		/**
		\brief Constructor
		*/
		PX_SUPPORT_INLINE ShrunkConvexHullNoScaleV(): ShrunkConvexHullV()
		{
		}

		PX_SUPPORT_INLINE ShrunkConvexHullNoScaleV(const PxGeometry& geom) : ShrunkConvexHullV(geom)
		{
		}

		PX_SUPPORT_INLINE ShrunkConvexHullNoScaleV(const Gu::ConvexHullData* hullData_, const Ps::aos::Vec3VArg center_, const Ps::aos::Vec3VArg scale, 
			const Ps::aos::QuatVArg scaleRot):
			ShrunkConvexHullV(hullData_, center_, scale, scaleRot, true)
		{
		}


	
		PX_FORCE_INLINE Ps::aos::Vec3V supportPoint(const PxI32 index, Ps::aos::FloatV* marginDif) const
		{
			using namespace Ps::aos;

			if (getMarginF() > 0)
			{
				//p is in the shape space
				return planeShift(PxU32(index), getMargin(), marginDif);
			}
			else
			{
				return V3LoadU_SafeReadW(verts[index]);
			}
		}

		//get the support point in vertex space
		PX_SUPPORT_INLINE Ps::aos::Vec3V planeShift(const PxU32 index, const Ps::aos::FloatVArg margin_, Ps::aos::FloatV* marginDif) const
		{
			using namespace Ps::aos;

			//calculate the support point for the core(shrunk) shape
			const PxU8* PX_RESTRICT polyInds = hullData->getFacesByVertices8();

			const Vec3V p = V3LoadU_SafeReadW(verts[index]);	// PT: safe because of the way vertex memory is allocated in ConvexHullData (and 'verts' is initialized with ConvexHullData::getHullVertices())

			const PxU32 ind = index*3;

			const PxPlane& data1 = hullData->mPolygons[polyInds[ind]].mPlane;
			const PxPlane& data2 = hullData->mPolygons[polyInds[ind+1]].mPlane;
			const PxPlane& data3 = hullData->mPolygons[polyInds[ind+2]].mPlane;

			//ML: because we don't have scale in this type of convex hull so that normal in vertex space will be the same as shape space normal
			//This is only required if the scale is not uniform
			const Vec3V n1 = V3LoadU_SafeReadW(data1.n);	// PT: safe because 'd' follows 'n' in the plane class
			const Vec3V n2 = V3LoadU_SafeReadW(data2.n);	// PT: safe because 'd' follows 'n' in the plane class
			const Vec3V n3 = V3LoadU_SafeReadW(data3.n);	// PT: safe because 'd' follows 'n' in the plane class

			//This is only required if the scale is not 1
			const FloatV d1 = FSub(margin_, V3Dot(p, n1));
			const FloatV d2 = FSub(margin_, V3Dot(p, n2));
			const FloatV d3 = FSub(margin_, V3Dot(p, n3));

			//This is unavoidable unless we pre-calc the core shape
			const Vec3V intersectPoints = intersectPlanes(n1, d1, n2, d2, n3, d3);
			const Vec3V v =V3Sub(p, intersectPoints); 
			(*marginDif) = V3Length(v);
			return intersectPoints;
		}


		PX_SUPPORT_INLINE Ps::aos::Vec3V supportLocal(const Ps::aos::Vec3VArg dir) const
		{
			PxI32 index;
			Ps::aos::FloatV marginDif;
			return supportLocal(dir, index, &marginDif);
		}

		PX_SUPPORT_INLINE Ps::aos::Vec3V supportRelative(const Ps::aos::Vec3VArg dir, const Ps::aos::PsMatTransformV& aTob, const Ps::aos::PsMatTransformV& aTobT) const
		{
			PxI32 index;
			Ps::aos::FloatV marginDif;
			return supportRelative(dir, aTob, aTobT, index, &marginDif);
		}

		//This function is used in gjk penetration
		//dir in the shape space
		PX_SUPPORT_INLINE Ps::aos::Vec3V supportLocal(const Ps::aos::Vec3VArg dir,  PxI32& index, Ps::aos::FloatV* marginDif) const
		{
			using namespace Ps::aos;
			//get the extreme point index
			const PxU32 maxIndex = supportVertexIndex(dir);
			index = PxI32(maxIndex);
			if (getMarginF() > 0)
			{
				//p is in the shape space
				return planeShift(maxIndex, getMargin(), marginDif);
			}
			else
			{
				return V3LoadU_SafeReadW(verts[index]);
			}
		}

		PX_SUPPORT_INLINE Ps::aos::Vec3V supportRelative(	const Ps::aos::Vec3VArg dir, const Ps::aos::PsMatTransformV& aTob,
															const Ps::aos::PsMatTransformV& aTobT, PxI32& index, Ps::aos::FloatV* marginDif) const
		{
			using namespace Ps::aos;

			//transform dir from b space to the shape space of a space
//			const Vec3V dir_ = aTob.rotateInv(dir);//relTra.rotateInv(dir);//M33MulV3(skewInvRot, dir);
			const Vec3V dir_ = aTobT.rotate(dir);//relTra.rotateInv(dir);//M33MulV3(skewInvRot, dir);
			const Vec3V p = supportLocal(dir_, index, marginDif);
			//transfrom from a to b space
			return aTob.transform(p);
		}
	};

	#define PX_SCONVEX_TO_NOSCALECONVEX(x)			(static_cast<ShrunkConvexHullNoScaleV*>(x))
}

}

#endif	// 
