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

#ifndef GU_VEC_SHRUNK_BOX_H
#define GU_VEC_SHRUNK_BOX_H

/** \addtogroup geomutils
@{
*/
#include "PxPhysXCommonConfig.h"
#include "GuVecConvex.h"
#include "PsVecTransform.h"
#include "GuConvexSupportTable.h"
#include "GuVecBox.h"



namespace physx
{

namespace Gu
{

	/**
	\brief Represents an oriented bounding box. 

	As a center point, extents(radii) and a rotation. i.e. the center of the box is at the center point, 
	the box is rotated around this point with the rotation and it is 2*extents in width, height and depth.
	*/

	/**
	Box geometry

	The rot member describes the world space orientation of the box.
	The center member gives the world space position of the box.
	The extents give the local space coordinates of the box corner in the positive octant.
	Dimensions of the box are: 2*extent.
	Transformation to world space is: worldPoint = rot * localPoint + center
	Transformation to local space is: localPoint = T(rot) * (worldPoint - center)
	Where T(M) denotes the transpose of M.
	*/

	class ShrunkBoxV : public BoxV
	{
	public:

		/**
		\brief Constructor
		*/
		PX_INLINE ShrunkBoxV() : BoxV()
		{
		}

		//this is used by CCD system
		PX_INLINE ShrunkBoxV(const PxGeometry& geom) : BoxV(geom)
		{
			margin = minExtent * BOX_MARGIN_CCD_RATIO;
			initialiseMarginDif();
		}

		/**
		\brief Constructor

		\param origin Center of the OBB
		\param extent Extents/radii of the obb.
		*/

		PX_FORCE_INLINE ShrunkBoxV(const Ps::aos::Vec3VArg origin, const Ps::aos::Vec3VArg extent) 
			: BoxV(origin, extent)
		{
			//calculate margin
			margin = minExtent * BOX_MARGIN_RATIO;
			initialiseMarginDif();
		}
		
		/**
		\brief Destructor
		*/
		PX_INLINE ~ShrunkBoxV()
		{
		}

		PX_FORCE_INLINE void resetMargin(const PxReal toleranceLength)
		{
			BoxV::resetMargin(toleranceLength);
			margin = PxMin(toleranceLength * BOX_MARGIN_RATIO, margin);
			initialiseMarginDif();
		}

		//! Assignment operator
		PX_INLINE const ShrunkBoxV& operator=(const ShrunkBoxV& other)
		{
			center	= other.center;
			extents	= other.extents;
			margin =  other.margin;
			minMargin = other.minMargin;
			return *this;
		}

		PX_FORCE_INLINE Ps::aos::Vec3V supportPoint(const PxI32 index, Ps::aos::FloatV* marginDif_)const
		{
			using namespace Ps::aos;
			const Vec3V extents_ = V3Sub(extents,  V3Splat(getMargin()));
			const BoolV con = boxVertexTable[index];
			(*marginDif_) = marginDif;
			return V3Sel(con, extents_, V3Neg(extents_));
		}  

		//local space point
		PX_FORCE_INLINE Ps::aos::Vec3V supportLocal(const Ps::aos::Vec3VArg dir)const
		{
			using namespace Ps::aos;
		
			const Vec3V zero = V3Zero();
			const Vec3V extents_ = V3Sub(extents,  V3Splat(getMargin()));
			const BoolV comp = V3IsGrtr(dir, zero);
			return V3Sel(comp, extents_, V3Neg(extents_));
		}

		//local space point
		PX_FORCE_INLINE Ps::aos::Vec3V supportLocal(const Ps::aos::Vec3VArg dir,  PxI32& index, Ps::aos::FloatV* marginDif_)const
		{
			using namespace Ps::aos;
		
			const Vec3V zero = V3Zero();
			const Vec3V extents_ = V3Sub(extents,  V3Splat(getMargin()));
			const BoolV comp = V3IsGrtr(dir, zero);
			getIndex(comp, index);
			(*marginDif_) = marginDif; 
			return V3Sel(comp, extents_, V3Neg(extents_));
		}

		PX_FORCE_INLINE Ps::aos::Vec3V supportRelative(const Ps::aos::Vec3VArg dir, const Ps::aos::PsMatTransformV& aTob, const Ps::aos::PsMatTransformV& aTobT) const
		{
			using namespace Ps::aos;
		
			//transfer dir into the local space of the box
//			const Vec3V dir_ =aTob.rotateInv(dir);//relTra.rotateInv(dir);
			const Vec3V dir_ = aTobT.rotate(dir);//relTra.rotateInv(dir);
			const Vec3V p = supportLocal(dir_);
			return aTob.transform(p);//relTra.transform(p);//V3Add(center, M33MulV3(rot, p));
		}

		PX_FORCE_INLINE Ps::aos::Vec3V supportRelative(	const Ps::aos::Vec3VArg dir, const Ps::aos::PsMatTransformV& aTob,
														const Ps::aos::PsMatTransformV& aTobT, PxI32& index,  Ps::aos::FloatV* marginDif_)const
		{
			using namespace Ps::aos;
	
			//transfer dir into the local space of the box
//			const Vec3V dir_ =aTob.rotateInv(dir);//relTra.rotateInv(dir);
			const Vec3V dir_ = aTobT.rotate(dir);//relTra.rotateInv(dir);
			const Vec3V p = supportLocal(dir_, index, marginDif_);
			return aTob.transform(p);//relTra.transform(p);//V3Add(center, M33MulV3(rot, p));
		}

	private:

		PX_FORCE_INLINE void initialiseMarginDif()
		{
			using namespace Ps::aos;
			//calculate the marginDif
			const PxReal sqMargin = margin * margin;
			const PxReal tempMarginDif = sqrtf(sqMargin * 3.f);
			const PxReal marginDif_ = tempMarginDif - margin;
			marginDif = FLoad(marginDif_);
		}
	};
}	

}

/** @} */
#endif
