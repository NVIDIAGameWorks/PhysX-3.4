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



#ifndef DY_ARTICULATION_REFERENCE_H
#define DY_ARTICULATION_REFERENCE_H

// a per-row struct where we put extra data for debug and setup - ultimately this will move to be just
// debug only



#include "DyArticulationUtils.h"
#include "DyArticulationScalar.h"
#include "DyArticulationFnsScalar.h"
#include "DySpatial.h"

#if DY_ARTICULATION_DEBUG_VERIFY

namespace physx
{

PX_FORCE_INLINE Cm::SpatialVector propagateVelocity(const FsRow& row, 
													const FsJointVectors& jv,
													const PxVec3& SZ, 
													const Cm::SpatialVector& v,
													const FsRowAux& aux)
{
	typedef ArticulationFnsScalar Fns;

	Cm::SpatialVector w = Fns::translateMotion(-getParentOffset(jv), v);
	PxVec3 DSZ = Fns::multiply(row.D, SZ);

	PxVec3 n = Fns::axisDot(getDSI(row), w) + DSZ;
	Cm::SpatialVector result = w - Cm::SpatialVector(getJointOffset(jv).cross(n),n);
#if DY_ARTICULATION_DEBUG_VERIFY
	Cm::SpatialVector check = ArticulationRef::propagateVelocity(row, jv, SZ, v, aux);
	PX_ASSERT((result-check).magnitude()<1e-5*PxMax(check.magnitude(), 1.0f));
#endif
	return result;
}

PX_FORCE_INLINE Cm::SpatialVector propagateImpulse(const FsRow& row, 
												   const FsJointVectors& jv,
												   PxVec3& SZ, 
												   const Cm::SpatialVector& Z,
												   const FsRowAux& aux)
{
	typedef ArticulationFnsScalar Fns;

	SZ = Z.angular + Z.linear.cross(getJointOffset(jv));
	Cm::SpatialVector result = Fns::translateForce(getParentOffset(jv), Z - Fns::axisMultiply(getDSI(row), SZ));
#if DY_ARTICULATION_DEBUG_VERIFY
	PxVec3 SZcheck;
	Cm::SpatialVector check = ArticulationRef::propagateImpulse(row, jv, SZcheck, Z, aux);
	PX_ASSERT((result-check).magnitude()<1e-5*PxMax(check.magnitude(), 1.0f));
	PX_ASSERT((SZ-SZcheck).magnitude()<1e-5*PxMax(SZcheck.magnitude(), 1.0f));
#endif
	return result;
}

}
#endif

#endif //DY_ARTICULATION_REFERENCE_H
