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

#ifndef GU_VEC_BOX_H
#define GU_VEC_BOX_H

/** \addtogroup geomutils
@{
*/
#include "foundation/PxTransform.h"
#include "PxPhysXCommonConfig.h"
#include "GuVecConvex.h"
#include "PsVecTransform.h"
#include "GuConvexSupportTable.h"
#include "PxBoxGeometry.h"

namespace physx
{
PX_PHYSX_COMMON_API extern const Ps::aos::BoolV boxVertexTable[8];

namespace Gu
{

#define BOX_MARGIN_RATIO		0.15f
#define	BOX_MIN_MARGIN_RATIO	0.05f
#define	BOX_SWEEP_MARGIN_RATIO	0.05f

#define BOX_MARGIN_CCD_RATIO		0.01f
#define	BOX_MIN_MARGIN_CCD_RATIO	0.005f


	class CapsuleV;


	PX_FORCE_INLINE void CalculateBoxMargin(const Ps::aos::Vec3VArg extent, PxReal& minExtent, PxReal& minMargin, PxReal& sweepMargin,
		const PxReal minMarginR = BOX_MIN_MARGIN_RATIO)
	{
		using namespace Ps::aos;

		const FloatV min = V3ExtractMin(extent);
		FStore(min, &minExtent);

		minMargin = minExtent * minMarginR;
		sweepMargin = minExtent * BOX_SWEEP_MARGIN_RATIO;
	}

	PX_FORCE_INLINE Ps::aos::FloatV CalculateBoxTolerance(const Ps::aos::Vec3VArg extent)
	{
		using namespace Ps::aos;
		
		const FloatV r0 = FLoad(0.01f);
		const FloatV min = V3ExtractMin(extent);//FMin(V3GetX(extent), FMin(V3GetY(extent), V3GetZ(extent)));
		return FMul(min, r0);
	}

	//This method is called in the PCM contact gen for the refreshing contacts 
	PX_FORCE_INLINE Ps::aos::FloatV CalculatePCMBoxMargin(const Ps::aos::Vec3VArg extent, const PxReal toleranceLength, const PxReal toleranceMarginRatio = BOX_MARGIN_RATIO)
	{
		using namespace Ps::aos;
	
		const FloatV min = V3ExtractMin(extent);//FMin(V3GetX(extent), FMin(V3GetY(extent), V3GetZ(extent)));
		const FloatV toleranceMargin = FLoad(toleranceLength * toleranceMarginRatio);
		return FMin(FMul(min, FLoad(BOX_MARGIN_RATIO)), toleranceMargin);
	}

	PX_FORCE_INLINE Ps::aos::FloatV CalculateMTDBoxMargin(const Ps::aos::Vec3VArg extent)
	{
		using namespace Ps::aos;

		const FloatV min = V3ExtractMin(extent);//FMin(V3GetX(extent), FMin(V3GetY(extent), V3GetZ(extent)));
		return FMul(min, FLoad(BOX_MARGIN_RATIO));
	}

	class BoxV : public ConvexV
	{
	public:

		/**
		\brief Constructor
		*/
		PX_INLINE BoxV() : ConvexV(ConvexType::eBOX)
		{
		}

		PX_FORCE_INLINE BoxV(const Ps::aos::Vec3VArg origin, const Ps::aos::Vec3VArg extent) : 
			ConvexV(ConvexType::eBOX, origin), extents(extent)
		{
			CalculateBoxMargin(extent, minExtent, minMargin, sweepMargin);
			marginDif = Ps::aos::FZero();
			margin = 0.f;
		}
		
		//this constructor is used by the CCD system
		PX_FORCE_INLINE BoxV(const PxGeometry& geom) : ConvexV(ConvexType::eBOX, Ps::aos::V3Zero())
		{
			using namespace Ps::aos;
			const PxBoxGeometry& boxGeom = static_cast<const PxBoxGeometry&>(geom);
			const Vec3V extent = Ps::aos::V3LoadU(boxGeom.halfExtents);
			extents = extent;
			CalculateBoxMargin(extent, minExtent, minMargin, sweepMargin, BOX_MIN_MARGIN_CCD_RATIO);
			marginDif = Ps::aos::FZero();
			margin = 0.f;
		}

		/**
		\brief Destructor
		*/
		PX_INLINE ~BoxV()
		{
		}

		PX_FORCE_INLINE void resetMargin(const PxReal toleranceLength)
		{
			minMargin = PxMin(toleranceLength * BOX_MIN_MARGIN_RATIO, minMargin);
		}

		//! Assignment operator
		PX_FORCE_INLINE const BoxV& operator=(const BoxV& other)
		{
			center	= other.center;
			extents	= other.extents;
			margin =  other.margin;
			minMargin = other.minMargin;
			marginDif = other.marginDif;
			sweepMargin = other.sweepMargin;
			return *this;
		}

		PX_FORCE_INLINE void populateVerts(const PxU8* inds, PxU32 numInds, const PxVec3* originalVerts, Ps::aos::Vec3V* verts)const
		{
			using namespace Ps::aos;

			for(PxU32 i=0; i<numInds; ++i)
				verts[i] = V3LoadU_SafeReadW(originalVerts[inds[i]]);	// PT: safe because of the way vertex memory is allocated in ConvexHullData (and 'populateVerts' is always called with polyData.mVerts)
		}

		PX_FORCE_INLINE Ps::aos::Vec3V supportPoint(const PxI32 index, Ps::aos::FloatV* /*marginDif*/)const
		{
			using namespace Ps::aos;
			const BoolV con = boxVertexTable[index];
			return V3Sel(con, extents, V3Neg(extents));
		}  

		PX_FORCE_INLINE void getIndex(const Ps::aos::BoolV con, PxI32& index)const 
		{ 
			using namespace Ps::aos;
			index = PxI32(BGetBitMask(con) & 0x7); 
		}

		PX_FORCE_INLINE Ps::aos::Vec3V supportLocal(const Ps::aos::Vec3VArg dir)const  
		{
			using namespace Ps::aos;
			return V3Sel(V3IsGrtr(dir,  V3Zero()), extents, V3Neg(extents));
		}

		//this is used in the sat test for the full contact gen
		PX_SUPPORT_INLINE void supportLocal(const Ps::aos::Vec3VArg dir, Ps::aos::FloatV& min, Ps::aos::FloatV& max)const
		{
			using namespace Ps::aos;
			const Vec3V point = V3Sel(V3IsGrtr(dir,  V3Zero()), extents, V3Neg(extents));
			max = V3Dot(dir, point);
			min = FNeg(max);
		}

		PX_SUPPORT_INLINE Ps::aos::Vec3V supportRelative(const Ps::aos::Vec3VArg dir, const Ps::aos::PsMatTransformV& aTob, const Ps::aos::PsMatTransformV& aTobT) const  
		{
			//a is the current object, b is the other object, dir is in the local space of b
			using namespace Ps::aos;
//			const Vec3V _dir = aTob.rotateInv(dir);//relTra.rotateInv(dir);//from b to a
			const Vec3V _dir = aTobT.rotate(dir);//relTra.rotateInv(dir);//from b to a
			const Vec3V p =  supportLocal(_dir);
			//transfer p into the b space
			return aTob.transform(p);//relTra.transform(p);
		}

		PX_SUPPORT_INLINE Ps::aos::Vec3V supportLocal(const Ps::aos::Vec3VArg dir, PxI32& index, Ps::aos::FloatV* /*marginDif_*/)const  
		{
			using namespace Ps::aos;
			const BoolV comp = V3IsGrtr(dir, V3Zero());
			getIndex(comp, index);
			return  V3Sel(comp, extents, V3Neg(extents));
		}

		PX_SUPPORT_INLINE Ps::aos::Vec3V supportRelative(	const Ps::aos::Vec3VArg dir, const Ps::aos::PsMatTransformV& aTob,
															const Ps::aos::PsMatTransformV& aTobT, PxI32& index, Ps::aos::FloatV* marginDif_)const  
		{
			//a is the current object, b is the other object, dir is in the local space of b
			using namespace Ps::aos;
//			const Vec3V _dir = aTob.rotateInv(dir);//relTra.rotateInv(dir);//from b to a
			const Vec3V _dir = aTobT.rotate(dir);//relTra.rotateInv(dir);//from b to a
			const Vec3V p = supportLocal(_dir, index, marginDif_);
			//transfer p into the b space
			return aTob.transform(p);//relTra.transform(p);
		}

		Ps::aos::Vec3V  extents;
		Ps::aos::FloatV marginDif;
		PxReal			minExtent;
	};
}	//PX_COMPILE_TIME_ASSERT(sizeof(Gu::BoxV) == 96);

}

/** @} */
#endif
