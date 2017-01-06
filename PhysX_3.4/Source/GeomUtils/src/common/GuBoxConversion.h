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

#ifndef GU_BOX_CONVERSION_H
#define GU_BOX_CONVERSION_H

#include "GuBox.h"
#include "PsMathUtils.h"
#include "CmMatrix34.h"
#include "PsVecMath.h"

namespace physx
{
	// PT: builds rot from quat. WARNING: writes 4 bytes after 'dst.rot'.
	PX_FORCE_INLINE void buildFrom(Gu::Box& dst, const PxQuat& q)
	{
		using namespace Ps::aos;
		const QuatV qV = V4LoadU(&q.x);
		Vec3V column0, column1, column2;
		QuatGetMat33V(qV, column0, column1, column2);
		// PT: TODO: investigate if these overlapping stores are a problem
		V4StoreU(Vec4V_From_Vec3V(column0), &dst.rot.column0.x);
		V4StoreU(Vec4V_From_Vec3V(column1), &dst.rot.column1.x);
		V4StoreU(Vec4V_From_Vec3V(column2), &dst.rot.column2.x);
	}

	PX_FORCE_INLINE void buildFrom(Gu::Box& dst, const PxVec3& center, const PxVec3& extents, const PxQuat& q)
	{
		using namespace Ps::aos;
		// PT: writes 4 bytes after 'rot' but it's safe since we then write 'center' just afterwards
		buildFrom(dst, q);
		dst.center	= center;
		dst.extents	= extents;
	}

	PX_FORCE_INLINE void buildMatrixFromBox(Cm::Matrix34& mat34, const Gu::Box& box)
	{
		mat34.m	= box.rot;
		mat34.p	= box.center;
	}

	// SD: function is now the same as FastVertex2ShapeScaling::transformQueryBounds
	// PT: lots of LHS in that one. TODO: revisit...
	PX_INLINE Gu::Box transform(const Cm::Matrix34& transfo, const Gu::Box& box)
	{
		Gu::Box ret;
		PxMat33& obbBasis = ret.rot;

		obbBasis.column0 = transfo.rotate(box.rot.column0 * box.extents.x);
		obbBasis.column1 = transfo.rotate(box.rot.column1 * box.extents.y);
		obbBasis.column2 = transfo.rotate(box.rot.column2 * box.extents.z);

		ret.center = transfo.transform(box.center);
		ret.extents = Ps::optimizeBoundingBox(obbBasis);
		return ret;
	}

	PX_INLINE Gu::Box transformBoxOrthonormal(const Gu::Box& box, const PxTransform& t)
	{
		Gu::Box ret;
		PxMat33& obbBasis = ret.rot;
		obbBasis.column0 = t.rotate(box.rot.column0);
		obbBasis.column1 = t.rotate(box.rot.column1);
		obbBasis.column2 = t.rotate(box.rot.column2);
		ret.center = t.transform(box.center);
		ret.extents = box.extents;
		return ret;
	}

	/**
	\brief recomputes the OBB after an arbitrary transform by a 4x4 matrix.
	\param	mtx		[in] the transform matrix
	\param	obb		[out] the transformed OBB
	*/
	PX_INLINE	void rotate(const Gu::Box& src, const Cm::Matrix34& mtx, Gu::Box& obb)
	{
		// The extents remain constant
		obb.extents = src.extents;
		// The center gets x-formed
		obb.center = mtx.transform(src.center);
		// Combine rotations
		obb.rot = mtx.m * src.rot;
	}

// PT: TODO: move this to a better place
	PX_FORCE_INLINE void getInverse(PxMat33& dstRot, PxVec3& dstTrans, const PxMat33& srcRot, const PxVec3& srcTrans)
	{
		const PxMat33 invRot = srcRot.getInverse();
		dstTrans = invRot.transform(-srcTrans);
		dstRot = invRot;
	}

}

#endif
