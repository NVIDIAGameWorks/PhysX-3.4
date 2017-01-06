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

#ifndef GU_BARYCENTRIC_COORDINATES_H
#define GU_BARYCENTRIC_COORDINATES_H

#include "PxPhysXCommonConfig.h"
#include "CmPhysXCommon.h"
#include "PsVecMath.h"

namespace physx
{
namespace Gu
{
	//calculate the barycentric coorinates for a point in a segment
	void barycentricCoordinates(const Ps::aos::Vec3VArg p, 
		const Ps::aos::Vec3VArg a, 
		const Ps::aos::Vec3VArg b, 
		Ps::aos::FloatV& v);

	//calculate the barycentric coorinates for a point in a triangle
	void barycentricCoordinates(const Ps::aos::Vec3VArg p, 
		const Ps::aos::Vec3VArg a, 
		const Ps::aos::Vec3VArg b, 
		const Ps::aos::Vec3VArg c, 
		Ps::aos::FloatV& v, 
		Ps::aos::FloatV& w);

	void barycentricCoordinates(const Ps::aos::Vec3VArg v0, 
		const Ps::aos::Vec3VArg v1, 
		const Ps::aos::Vec3VArg v2,  
		Ps::aos::FloatV& v, 
		Ps::aos::FloatV& w);

	PX_INLINE Ps::aos::BoolV isValidTriangleBarycentricCoord(const Ps::aos::FloatVArg v, const Ps::aos::FloatVArg w)
	{
		using namespace Ps::aos;
		const FloatV zero = FNeg(FEps());
		const FloatV one = FAdd(FOne(), FEps());

		const BoolV con0 = BAnd(FIsGrtrOrEq(v, zero), FIsGrtrOrEq(one, v));
		const BoolV con1 = BAnd(FIsGrtrOrEq(w, zero), FIsGrtrOrEq(one, w));
		const BoolV con2 = FIsGrtr(one, FAdd(v, w));
		return BAnd(con0, BAnd(con1, con2));
	}

	PX_INLINE Ps::aos::BoolV isValidTriangleBarycentricCoord2(const Ps::aos::Vec4VArg vwvw)
	{
		using namespace Ps::aos;
		const Vec4V eps = V4Splat(FEps());
		const Vec4V zero =V4Neg(eps);
		const Vec4V one = V4Add(V4One(), eps);

		const Vec4V v0v1v0v1 = V4PermXZXZ(vwvw);
		const Vec4V w0w1w0w1 = V4PermYWYW(vwvw);

		const BoolV con0 = BAnd(V4IsGrtrOrEq(v0v1v0v1, zero), V4IsGrtrOrEq(one, v0v1v0v1));
		const BoolV con1 = BAnd(V4IsGrtrOrEq(w0w1w0w1, zero), V4IsGrtrOrEq(one, w0w1w0w1));
		const BoolV con2 = V4IsGrtr(one, V4Add(v0v1v0v1, w0w1w0w1));
		return BAnd(con0, BAnd(con1, con2));
	}

} // namespace Gu

}

#endif
