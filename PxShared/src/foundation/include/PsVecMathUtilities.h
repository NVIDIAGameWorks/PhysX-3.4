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

#ifndef PSFOUNDATION_PSVECMATHUTILITIES_H
#define PSFOUNDATION_PSVECMATHUTILITIES_H

#include "PsVecMath.h"

namespace physx
{
namespace shdfnd
{
namespace aos
{
/*!
    Extend an edge along its length by a factor
    */
PX_FORCE_INLINE void makeFatEdge(Vec3V& p0, Vec3V& p1, const FloatVArg fatCoeff)
{
	const Vec3V delta = V3Sub(p1, p0);
	const FloatV m = V3Length(delta);
	const BoolV con = FIsGrtr(m, FZero());
	const Vec3V fatDelta = V3Scale(V3ScaleInv(delta, m), fatCoeff);
	p0 = V3Sel(con, V3Sub(p0, fatDelta), p0);
	p1 = V3Sel(con, V3Add(p1, fatDelta), p1);
}
}
}
}

#endif
