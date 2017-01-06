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

#ifndef SQ_BOUNDS_H
#define SQ_BOUNDS_H

#include "CmPhysXCommon.h"
#include "foundation/PxBounds3.h"
#include "PsVecMath.h"

namespace physx
{
	namespace Scb
	{
		class Shape;
		class Actor;
	}

namespace Sq
{
	void computeStaticWorldAABB(PxBounds3& bounds, const Scb::Shape& scbShape, const Scb::Actor& scbActor);
	void computeDynamicWorldAABB(PxBounds3& bounds, const Scb::Shape& scbShape, const Scb::Actor& scbActor);

	typedef void(*ComputeBoundsFunc)	(PxBounds3& bounds, const Scb::Shape& scbShape, const Scb::Actor& scbActor);

	extern const ComputeBoundsFunc gComputeBoundsTable[2];

	// PT: TODO: - check that this is compatible with Gu::computeBounds(..., SQ_PRUNER_INFLATION, ...)
	// PT: TODO: - refactor with "inflateBounds" in GuBounds.cpp if possible
	// PT: TODO: - use SQ_PRUNER_INFLATION instead of hardcoding "0.01f"
	PX_FORCE_INLINE void inflateBounds(PxBounds3& dst, const PxBounds3& src)
	{
		using namespace physx::shdfnd::aos;

		const Vec4V minV = V4LoadU(&src.minimum.x);
		const Vec4V maxV = V4LoadU(&src.maximum.x);
		const Vec4V eV = V4Scale(V4Sub(maxV, minV), FLoad(0.5f * 0.01f));

		V4StoreU(V4Sub(minV, eV), &dst.minimum.x);
		PX_ALIGN(16, PxVec4) max4;
		V4StoreA(V4Add(maxV, eV), &max4.x);
		dst.maximum = PxVec3(max4.x, max4.y, max4.z);
	}
}
}

#endif // SQ_BOUNDS_H
