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


#include "ExtD6Joint.h"
#include "ExtConstraintHelper.h"
#include "CmConeLimitHelper.h"

namespace physx
{
namespace Ext
{
	PxU32 D6JointSolverPrep(Px1DConstraint* constraints,
		PxVec3& body0WorldOffset,
		PxU32 maxConstraints,
		PxConstraintInvMassScale& invMassScale,
		const void* constantBlock,
		const PxTransform& bA2w,
		const PxTransform& bB2w)
	{
		PX_UNUSED(maxConstraints);

		using namespace joint;

		PX_UNUSED(maxConstraints);
		const D6JointData& data = 
			*reinterpret_cast<const D6JointData*>(constantBlock);

		invMassScale = data.invMassScale;

		const PxU32 SWING1_FLAG = 1<<PxD6Axis::eSWING1, 
			SWING2_FLAG = 1<<PxD6Axis::eSWING2, 
			TWIST_FLAG  = 1<<PxD6Axis::eTWIST;

		const PxU32 ANGULAR_MASK = SWING1_FLAG | SWING2_FLAG | TWIST_FLAG;
		const PxU32 LINEAR_MASK = 1<<PxD6Axis::eX | 1<<PxD6Axis::eY | 1<<PxD6Axis::eZ;

		const PxD6JointDrive* drives = data.drive;
		PxU32 locked = data.locked, limited = data.limited, driving = data.driving;

		PxTransform cA2w = bA2w.transform(data.c2b[0]);
		PxTransform cB2w = bB2w.transform(data.c2b[1]);

		body0WorldOffset = cB2w.p-bA2w.p;
		ConstraintHelper g(constraints, cA2w.p-bA2w.p, cB2w.p-bB2w.p);

		if(cA2w.q.dot(cB2w.q)<0)	// minimum dist quat (equiv to flipping cB2bB.q, which we don't use anywhere)
			cB2w.q = -cB2w.q;

		PxTransform cB2cA = cA2w.transformInv(cB2w);	

		PX_ASSERT(data.c2b[0].isValid());
		PX_ASSERT(data.c2b[1].isValid());
		PX_ASSERT(cA2w.isValid());
		PX_ASSERT(cB2w.isValid());
		PX_ASSERT(cB2cA.isValid());

		PxMat33 cA2w_m(cA2w.q), cB2w_m(cB2w.q);

		// handy for swing computation
		PxVec3 bX = cB2w_m[0], aY = cA2w_m[1], aZ = cA2w_m[2];

		if(driving & ((1<<PxD6Drive::eX)|(1<<PxD6Drive::eY)|(1<<PxD6Drive::eZ)))
		{
			// TODO: make drive unilateral if we are outside the limit
			PxVec3 posErr = data.drivePosition.p - cB2cA.p;
			for(PxU32 i = 0; i < 3; i++)
			{
				// -driveVelocity because velTarget is child (body1) - parent (body0) and Jacobian is 1 for body0 and -1 for parent
				if(driving & (1<<(PxD6Drive::eX+i)))
					g.linear(cA2w_m[i], -data.driveLinearVelocity[i], posErr[i], drives[PxD6Drive::eX+i]); 
			}
		}

		if(driving & ((1<<PxD6Drive::eSLERP)|(1<<PxD6Drive::eSWING)|(1<<PxD6Drive::eTWIST)))
		{
			PxQuat d2cA_q = cB2cA.q.dot(data.drivePosition.q)>0 ? data.drivePosition.q : -data.drivePosition.q; 

			const PxVec3& v = data.driveAngularVelocity;
			PxQuat delta = d2cA_q.getConjugate() * cB2cA.q;

			if(driving & (1<<PxD6Drive::eSLERP))
			{
				PxVec3 velTarget = -cA2w.rotate(data.driveAngularVelocity);

				PxVec3 axis[3] = { PxVec3(1.f,0,0), PxVec3(0,1.f,0), PxVec3(0,0,1.f) };
				
				if(drives[PxD6Drive::eSLERP].stiffness!=0)
					computeJacobianAxes(axis, cA2w.q * d2cA_q, cB2w.q);	// converges faster if there is only velocity drive

				for(PxU32 i = 0; i < 3; i++)
					g.angular(axis[i], axis[i].dot(velTarget), -delta.getImaginaryPart()[i], drives[PxD6Drive::eSLERP], PxConstraintSolveHint::eSLERP_SPRING);
			}
			else 
			{
				if(driving & (1<<PxD6Drive::eTWIST))
					g.angular(bX, v.x, -2.0f * delta.x, drives[PxD6Drive::eTWIST]); 

				if(driving & (1<<PxD6Drive::eSWING))
				{
					PxVec3 err = delta.rotate(PxVec3(1.f,0,0));

					if(!(locked & SWING1_FLAG))
						g.angular(cB2w_m[1], v.y, err.z, drives[PxD6Drive::eSWING]);

					if(!(locked & SWING2_FLAG))
						g.angular(cB2w_m[2], v.z, -err.y, drives[PxD6Drive::eSWING]);
				}
			}
		}

		if (limited & ANGULAR_MASK)
		{
			PxQuat swing, twist;
			Ps::separateSwingTwist(cB2cA.q,swing,twist);

			// swing limits: if just one is limited: if the other is free, we support 
			// (-pi/2, +pi/2) limit, using tan of the half-angle as the error measure parameter. 
			// If the other is locked, we support (-pi, +pi) limits using the tan of the quarter-angle
			// Notation: th == Ps::tanHalf, tq = tanQuarter

			if(limited & SWING1_FLAG && limited & SWING2_FLAG)
			{
				Cm::ConeLimitHelper coneHelper(data.tqSwingZ, data.tqSwingY, data.tqSwingPad);

				PxVec3 axis;
				PxReal error;
				if(coneHelper.getLimit(swing, axis, error))
					g.angularLimit(cA2w.rotate(axis),error,data.swingLimit);
			}
			else
			{
				const PxJointLimitCone &limit = data.swingLimit;

				PxReal tqPad = data.tqSwingPad, thPad = data.thSwingPad;

				if(limited & SWING1_FLAG)
				{
					if(locked & SWING2_FLAG)
					{
						g.quarterAnglePair(Ps::tanHalf(swing.y, swing.w), -data.tqSwingY, data.tqSwingY, tqPad, aY, limit);
					}
					else
					{
						PxReal dot=-aZ.dot(bX);
						g.halfAnglePair(Ps::tanHalf(dot, 1-dot*dot), -data.thSwingY, data.thSwingY, thPad, aZ.cross(bX), limit);
					}
				}
				if(limited & SWING2_FLAG)
				{
					if(locked & SWING1_FLAG)
					{
						g.quarterAnglePair(Ps::tanHalf(swing.z, swing.w), -data.tqSwingZ, data.tqSwingZ, tqPad, aZ, limit);
					}
					else
					{
						PxReal dot=aY.dot(bX);
						g.halfAnglePair(Ps::tanHalf(dot, 1-dot*dot), -data.thSwingZ, data.thSwingZ, thPad, -aY.cross(bX), limit);
					}
				}
			}

			if(limited & TWIST_FLAG)
			{
				g.quarterAnglePair(Ps::tanHalf(twist.x, twist.w), data.tqTwistLow, data.tqTwistHigh, data.tqTwistPad,
					cB2w_m[0], data.twistLimit);
			}
		}

		if(limited & LINEAR_MASK)
		{
			PxVec3 limitDir = PxVec3(0);

			for(PxU32 i = 0; i < 3; i++)
			{
				if(limited & (1<<(PxD6Axis::eX+i)))
					limitDir += cA2w_m[i] * cB2cA.p[i];
			}

			PxReal distance = limitDir.magnitude();
			if(distance > data.linearMinDist)
				g.linearLimit(limitDir * (1.0f/distance), distance, data.linearLimit.value, data.linearLimit);
		}

		// we handle specially the case of just one swing dof locked

		PxU32 angularLocked = locked & ANGULAR_MASK;

		if(angularLocked == SWING1_FLAG)
		{
			g.angularHard(bX.cross(aZ), -bX.dot(aZ));
			locked &= ~SWING1_FLAG;
		}
		else if(angularLocked == SWING2_FLAG)
		{
			locked &= ~SWING2_FLAG;
			g.angularHard(bX.cross(aY), -bX.dot(aY));
		}

		g.prepareLockedAxes(cA2w.q, cB2w.q, cB2cA.p,locked&7, locked>>3);

		return g.getCount();
	}
}//namespace

}
