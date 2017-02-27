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

#ifndef GU_SIMD_HELPERS_H
#define GU_SIMD_HELPERS_H

#include "PxPhysXCommonConfig.h"
#include "CmPhysXCommon.h"
#include "geometry/PxTriangle.h"
#include "foundation/PxMat33.h"
#include "PsVecMath.h"

namespace physx
{
namespace Gu
{
	//! A padded version of PxTriangle, to safely load its data using SIMD
	class TrianglePadded : public PxTriangle
	{
	public:
		PX_FORCE_INLINE TrianglePadded()	{}
		PX_FORCE_INLINE ~TrianglePadded()	{}
		PxU32	padding;
	};

	// PT: wrapper helper class to make sure we can safely load a PxVec3 using SIMD loads
	// PT: TODO: refactor with PxVec3Pad
	class Vec3p : public PxVec3
	{
		public:
		PX_FORCE_INLINE	Vec3p()								{}
		PX_FORCE_INLINE	~Vec3p()							{}
		PX_FORCE_INLINE	Vec3p(const PxVec3& p) : PxVec3(p)	{}
		PX_FORCE_INLINE	Vec3p(float f) : PxVec3(f)			{}
		PxU32	padding;
	};
	PX_COMPILE_TIME_ASSERT(sizeof(Vec3p) == 16);

	//! A padded version of PxMat33, to safely load its data using SIMD
	class PxMat33Padded : public PxMat33
	{
	public:
		explicit PX_FORCE_INLINE PxMat33Padded(const PxQuat& q)
		{
			using namespace Ps::aos;
			const QuatV qV = V4LoadU(&q.x);
			Vec3V column0V, column1V, column2V;
			QuatGetMat33V(qV, column0V, column1V, column2V);
#if defined(PX_SIMD_DISABLED) || PX_ANDROID || (PX_LINUX && (PX_ARM || PX_A64))
			V3StoreU(column0V, column0);
			V3StoreU(column1V, column1);
			V3StoreU(column2V, column2);
#else
			V4StoreU(column0V, &column0.x);
			V4StoreU(column1V, &column1.x);
			V4StoreU(column2V, &column2.x);
#endif
		}
		PX_FORCE_INLINE ~PxMat33Padded()				{}
		PX_FORCE_INLINE void operator=(const PxMat33& other)
		{
			column0 = other.column0;
			column1 = other.column1;
			column2 = other.column2;
		}
		PxU32	padding;
	};

} // namespace Gu
}

#endif
