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

#ifndef GU_BV4_BOX_OVERLAP_INTERNAL_H
#define GU_BV4_BOX_OVERLAP_INTERNAL_H

#include "GuBV4_Common.h"

	template<class ParamsT>
	PX_FORCE_INLINE void precomputeData(ParamsT* PX_RESTRICT dst, PxMat33* PX_RESTRICT absRot, const PxMat33* PX_RESTRICT boxToModelR)
	{
		// Precompute absolute box-to-model rotation matrix
		dst->mPreca0_PaddedAligned.x = boxToModelR->column0.x;
		dst->mPreca0_PaddedAligned.y = boxToModelR->column1.y;
		dst->mPreca0_PaddedAligned.z = boxToModelR->column2.z;

		dst->mPreca1_PaddedAligned.x = boxToModelR->column0.y;
		dst->mPreca1_PaddedAligned.y = boxToModelR->column1.z;
		dst->mPreca1_PaddedAligned.z = boxToModelR->column2.x;

		dst->mPreca2_PaddedAligned.x = boxToModelR->column0.z;
		dst->mPreca2_PaddedAligned.y = boxToModelR->column1.x;
		dst->mPreca2_PaddedAligned.z = boxToModelR->column2.y;

		// Epsilon value prevents floating-point inaccuracies (strategy borrowed from RAPID)
		const PxReal epsilon = 1e-6f;
		absRot->column0.x = dst->mPreca0b_PaddedAligned.x = epsilon + fabsf(boxToModelR->column0.x);
		absRot->column0.y = dst->mPreca1b_PaddedAligned.x = epsilon + fabsf(boxToModelR->column0.y);
		absRot->column0.z = dst->mPreca2b_PaddedAligned.x = epsilon + fabsf(boxToModelR->column0.z);

		absRot->column1.x = dst->mPreca2b_PaddedAligned.y = epsilon + fabsf(boxToModelR->column1.x);
		absRot->column1.y = dst->mPreca0b_PaddedAligned.y = epsilon + fabsf(boxToModelR->column1.y);
		absRot->column1.z = dst->mPreca1b_PaddedAligned.y = epsilon + fabsf(boxToModelR->column1.z);

		absRot->column2.x = dst->mPreca1b_PaddedAligned.z = epsilon + fabsf(boxToModelR->column2.x);
		absRot->column2.y = dst->mPreca2b_PaddedAligned.z = epsilon + fabsf(boxToModelR->column2.y);
		absRot->column2.z = dst->mPreca0b_PaddedAligned.z = epsilon + fabsf(boxToModelR->column2.z);
	}

	template<class ParamsT>
	PX_FORCE_INLINE	void setupBoxData(ParamsT* PX_RESTRICT dst, const PxVec3& extents, const PxMat33* PX_RESTRICT mAR)
	{
		dst->mBoxExtents_PaddedAligned = extents;

		const float Ex = extents.x;
		const float Ey = extents.y;
		const float Ez = extents.z;
		dst->mBB_PaddedAligned.x = Ex*mAR->column0.x + Ey*mAR->column1.x + Ez*mAR->column2.x;
		dst->mBB_PaddedAligned.y = Ex*mAR->column0.y + Ey*mAR->column1.y + Ez*mAR->column2.y;
		dst->mBB_PaddedAligned.z = Ex*mAR->column0.z + Ey*mAR->column1.z + Ez*mAR->column2.z;
	}

	struct OBBTestParams	// Data needed to perform the OBB-OBB overlap test
	{
#ifdef GU_BV4_QUANTIZED_TREE
		BV4_ALIGN16(Vec3p	mCenterOrMinCoeff_PaddedAligned);
		BV4_ALIGN16(Vec3p	mExtentsOrMaxCoeff_PaddedAligned);
#endif
		BV4_ALIGN16(Vec3p	mTBoxToModel_PaddedAligned);		//!< Translation from obb space to model space
		BV4_ALIGN16(Vec3p	mBB_PaddedAligned);
		BV4_ALIGN16(Vec3p	mBoxExtents_PaddedAligned);

		BV4_ALIGN16(Vec3p	mPreca0_PaddedAligned);
		BV4_ALIGN16(Vec3p	mPreca1_PaddedAligned);
		BV4_ALIGN16(Vec3p	mPreca2_PaddedAligned);
		BV4_ALIGN16(Vec3p	mPreca0b_PaddedAligned);
		BV4_ALIGN16(Vec3p	mPreca1b_PaddedAligned);
		BV4_ALIGN16(Vec3p	mPreca2b_PaddedAligned);

		PX_FORCE_INLINE	void	precomputeBoxData(const PxVec3& extents, const PxMat33* PX_RESTRICT box_to_model)
		{
			PxMat33	absRot;	//!< Absolute rotation matrix
			precomputeData(this, &absRot, box_to_model);

			setupBoxData(this, extents, &absRot);
		}
	};

#endif // GU_BV4_BOX_OVERLAP_INTERNAL_H
