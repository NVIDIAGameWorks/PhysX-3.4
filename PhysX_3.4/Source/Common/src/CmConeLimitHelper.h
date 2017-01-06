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


#ifndef PX_PHYSICS_COMMON_CONELIMITHELPER
#define PX_PHYSICS_COMMON_CONELIMITHELPER

// This class contains methods for supporting the tan-quarter swing limit - that
// is the, ellipse defined by tanQ(theta)^2/tanQ(thetaMax)^2 + tanQ(phi)^2/tanQ(phiMax)^2 = 1
// 
// Angles are passed as an PxVec3 swing vector with x = 0 and y and z the swing angles
// around the y and z axes

#include "CmPhysXCommon.h"
#include "PsMathUtils.h"

namespace physx
{
namespace Cm
{
	PX_FORCE_INLINE PxReal tanAdd(PxReal tan1, PxReal tan2)
	{
		PX_ASSERT(PxAbs(1-tan1*tan2)>1e-6f);
		return (tan1+tan2)/(1-tan1*tan2);
	}

	// this is here because it's used in both LL and Extensions. However, it
	// should STAY IN THE SDK CODE BASE because it's SDK-specific

	class ConeLimitHelper
	{
	public:
		ConeLimitHelper(PxReal tanQSwingY, PxReal tanQSwingZ, PxReal tanQPadding)
			: mTanQYMax(tanQSwingY), mTanQZMax(tanQSwingZ), mTanQPadding(tanQPadding) {}

		// whether the point is inside the (inwardly) padded cone - if it is, there's no limit
		// constraint

		PX_FORCE_INLINE bool contains(const PxVec3& tanQSwing)
		{
			PxReal tanQSwingYPadded = tanAdd(PxAbs(tanQSwing.y),mTanQPadding);
			PxReal tanQSwingZPadded = tanAdd(PxAbs(tanQSwing.z),mTanQPadding);
			return Ps::sqr(tanQSwingYPadded/mTanQYMax)+Ps::sqr(tanQSwingZPadded/mTanQZMax) <= 1;
		}

		PX_FORCE_INLINE PxVec3 clamp(const PxVec3& tanQSwing, 
									 PxVec3& normal)
		{
			PxVec3 p = Ps::ellipseClamp(tanQSwing, PxVec3(0,mTanQYMax,mTanQZMax));
			normal = PxVec3(0, p.y/Ps::sqr(mTanQYMax), p.z/Ps::sqr(mTanQZMax));
#ifdef PX_PARANOIA_ELLIPSE_CHECK
			PxReal err = PxAbs(Ps::sqr(p.y/mTanQYMax) + Ps::sqr(p.z/mTanQZMax) - 1);
			PX_ASSERT(err<1e-3);
#endif

			return p;
		}


		// input is a swing quat, such that swing.x = twist.y = twist.z = 0, q = swing * twist
		// The routine is agnostic to the sign of q.w (i.e. we don't need the minimal-rotation swing)

		// output is an axis such that positive rotation increases the angle outward from the
		// limit (i.e. the image of the x axis), the error is the sine of the angular difference,
		// positive if the twist axis is inside the cone 

		bool getLimit(const PxQuat& swing, PxVec3& axis, PxReal& error)
		{
			PX_ASSERT(swing.w>0);
			PxVec3 twistAxis = swing.getBasisVector0();
			PxVec3 tanQSwing = PxVec3(0, Ps::tanHalf(swing.z,swing.w), -Ps::tanHalf(swing.y,swing.w));
			if(contains(tanQSwing))
				return false;

			PxVec3 normal, clamped = clamp(tanQSwing, normal);

			// rotation vector and ellipse normal
			PxVec3 r(0,-clamped.z,clamped.y), d(0, -normal.z, normal.y);

			// the point on the cone defined by the tanQ swing vector r
			PxVec3 p(1.f,0,0);
			PxReal r2 = r.dot(r), a = 1-r2, b = 1/(1+r2), b2 = b*b;
			PxReal v1 = 2*a*b2;
			PxVec3 v2(a, 2*r.z, -2*r.y);		// a*p + 2*r.cross(p);
			PxVec3 coneLine = v1 * v2 - p;		// already normalized

			// the derivative of coneLine in the direction d	
			PxReal rd = r.dot(d);
			PxReal dv1 = -4*rd*(3-r2)*b2*b;
			PxVec3 dv2(-2*rd, 2*d.z, -2*d.y);
	
			PxVec3 coneNormal = v1 * dv2 + dv1 * v2;

			axis = coneLine.cross(coneNormal)/coneNormal.magnitude();
			error = coneLine.cross(axis).dot(twistAxis);

			PX_ASSERT(PxAbs(axis.magnitude()-1)<1e-5f);

#ifdef PX_PARANOIA_ELLIPSE_CHECK
			bool inside = Ps::sqr(tanQSwing.y/mTanQYMax) + Ps::sqr(tanQSwing.z/mTanQZMax) <= 1;
			PX_ASSERT(inside && error>-1e-4f || !inside && error<1e-4f);
#endif

			return true;
		}

	private:


		PxReal mTanQYMax, mTanQZMax, mTanQPadding;
	};


} // namespace Cm

}

#endif
