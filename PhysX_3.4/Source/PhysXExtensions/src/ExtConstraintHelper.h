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


#ifndef NP_CONSTRAINT_HELPER_H
#define NP_CONSTRAINT_HELPER_H

#include "foundation/PxAssert.h"
#include "foundation/PxTransform.h"
#include "foundation/PxMat33.h"
#include "PxJointLimit.h"
#include "ExtJoint.h"

namespace physx
{
namespace Ext
{
	namespace joint
	{
		PX_INLINE void computeDerived(const JointData& data, 
									  const PxTransform& bA2w, 
									  const PxTransform& bB2w,
									  PxTransform& cA2w,
									  PxTransform& cB2w,
									  PxTransform& cB2cA)
		{
			PX_ASSERT(bA2w.isValid() && bB2w.isValid());

			cA2w = bA2w.transform(data.c2b[0]);
			cB2w = bB2w.transform(data.c2b[1]);

			if(cA2w.q.dot(cB2w.q)<0)	// minimum error quat
				cB2w.q = -cB2w.q;

			cB2cA = cA2w.transformInv(cB2w);

			PX_ASSERT(cA2w.isValid() && cB2w.isValid() && cB2cA.isValid());
		}

		PX_INLINE PxVec3 truncateLinear(const PxVec3& in, PxReal tolerance, bool& truncated)
		{		
			PxReal m = in.magnitudeSquared();
			truncated = m>tolerance * tolerance;
			return truncated ? in * PxRecipSqrt(m) * tolerance : in;
		}

		PX_INLINE PxQuat truncateAngular(const PxQuat& in, PxReal sinHalfTol, PxReal cosHalfTol, bool& truncated)
		{
			truncated = false;

			if(sinHalfTol > 0.9999f)	// fixes numerical tolerance issue of projecting because quat is not exactly normalized
				return in;

			PxQuat q = in.w>=0 ? in : -in;
					
			const PxVec3& im = q.getImaginaryPart();
			PxReal m = im.magnitudeSquared();
			truncated =  m>sinHalfTol*sinHalfTol;
			if(!truncated)
				return in;

			PxVec3 outV = im * sinHalfTol * PxRecipSqrt(m);			
			return PxQuat(outV.x, outV.y, outV.z, cosHalfTol);
		}

		PX_FORCE_INLINE void projectTransforms(PxTransform& bA2w, PxTransform& bB2w, 
											   const PxTransform& cA2w, const PxTransform& cB2w, 
											   const PxTransform& cB2cA, const JointData& data, bool projectToA)
		{
			PX_ASSERT(cB2cA.isValid());

			// normalization here is unfortunate: long chains of projected constraints can result in
			// accumulation of error in the quaternion which eventually leaves the quaternion
			// magnitude outside the validation range. The approach here is slightly overconservative
			// in that we could just normalize the quaternions which are out of range, but since we
			// regard projection as an occasional edge case it shouldn't be perf-sensitive, and
			// this way we maintain the invariant (also maintained by the dynamics integrator) that
			// body quats are properly normalized up to FP error.

			if (projectToA)
			{
				bB2w = cA2w.transform(cB2cA.transform(data.c2b[1].getInverse()));
				bB2w.q.normalize();
			}
			else
			{
				bA2w = cB2w.transform(cB2cA.transformInv(data.c2b[0].getInverse()));
				bA2w.q.normalize();
			}


			PX_ASSERT(bA2w.isValid());
			PX_ASSERT(bB2w.isValid());
		}



		PX_INLINE void computeJacobianAxes(PxVec3 row[3], const PxQuat& qa, const PxQuat& qb)
		{
			// Compute jacobian matrix for (qa* qb)  [[* means conjugate in this expr]]
			// d/dt (qa* qb) = 1/2 L(qa*) R(qb) (omega_b - omega_a)
			// result is L(qa*) R(qb), where L(q) and R(q) are left/right q multiply matrix

			PxReal wa = qa.w, wb = qb.w;
			const PxVec3 va(qa.x,qa.y,qa.z), vb(qb.x,qb.y,qb.z);

			const PxVec3 c = vb*wa + va*wb;
			const PxReal d0 = wa*wb;
			const PxReal d1 = va.dot(vb);
			const PxReal d = d0 - d1;

			row[0] = (va * vb.x + vb * va.x + PxVec3(d,     c.z, -c.y)) * 0.5f;
			row[1] = (va * vb.y + vb * va.y + PxVec3(-c.z,  d,    c.x)) * 0.5f;
			row[2] = (va * vb.z + vb * va.z + PxVec3(c.y,   -c.x,   d)) * 0.5f;

			if ((d0 + d1) != 0.0f)  // check if relative rotation is 180 degrees which can lead to singular matrix
				return;
			else
			{
				row[0].x += PX_EPS_F32;
				row[1].y += PX_EPS_F32;
				row[2].z += PX_EPS_F32;
			}
		}

		class ConstraintHelper
		{
			Px1DConstraint* mConstraints;
			Px1DConstraint* mCurrent;
			PxVec3 mRa, mRb;

		public:
			ConstraintHelper(Px1DConstraint* c, const PxVec3& ra, const PxVec3& rb)
				: mConstraints(c), mCurrent(c), mRa(ra), mRb(rb)	{}


			// hard linear & angular
			PX_FORCE_INLINE void linearHard(const PxVec3& axis, PxReal posErr)
			{
				Px1DConstraint *c = linear(axis, posErr, PxConstraintSolveHint::eEQUALITY);
				c->flags |= Px1DConstraintFlag::eOUTPUT_FORCE;
			}

			PX_FORCE_INLINE void angularHard(const PxVec3& axis, PxReal posErr)
			{
				Px1DConstraint *c = angular(axis, posErr, PxConstraintSolveHint::eEQUALITY);
				c->flags |= Px1DConstraintFlag::eOUTPUT_FORCE;
			}

			// limited linear & angular
			PX_FORCE_INLINE void linearLimit(const PxVec3& axis, PxReal ordinate, PxReal limitValue, const PxJointLimitParameters& limit)
			{
				PxReal pad = limit.isSoft() ? 0 : limit.contactDistance;

				if(ordinate + pad > limitValue)
					addLimit(linear(axis,limitValue - ordinate, PxConstraintSolveHint::eNONE),limit);
			}

			PX_FORCE_INLINE void angularLimit(const PxVec3& axis, PxReal ordinate, PxReal limitValue, PxReal pad, const PxJointLimitParameters& limit)
			{
				if(limit.isSoft())
					pad = 0;

				if(ordinate + pad > limitValue)
					addLimit(angular(axis,limitValue - ordinate, PxConstraintSolveHint::eNONE),limit);
			}


			PX_FORCE_INLINE void angularLimit(const PxVec3& axis, PxReal error, const PxJointLimitParameters& limit)
			{
				addLimit(angular(axis,error, PxConstraintSolveHint::eNONE),limit);
			}

			PX_FORCE_INLINE void halfAnglePair(PxReal halfAngle, PxReal lower, PxReal upper, PxReal pad, const PxVec3& axis, const PxJointLimitParameters& limit)
			{
				PX_ASSERT(lower<upper);
				if(limit.isSoft())
					pad = 0;

				if(halfAngle < lower+pad)
					angularLimit(-axis, -(lower - halfAngle)*2,limit);
				if(halfAngle > upper-pad)
					angularLimit(axis, (upper - halfAngle)*2, limit);
			}

			PX_FORCE_INLINE void quarterAnglePair(PxReal quarterAngle, PxReal lower, PxReal upper, PxReal pad, const PxVec3& axis, const PxJointLimitParameters& limit)
			{
				if(limit.isSoft())
					pad = 0;

				PX_ASSERT(lower<upper);
				if(quarterAngle < lower+pad)
					angularLimit(-axis, -(lower - quarterAngle)*4,limit);
				if(quarterAngle > upper-pad)
					angularLimit(axis, (upper - quarterAngle)*4, limit);
			}

			// driven linear & angular

			PX_FORCE_INLINE void linear(const PxVec3& axis, PxReal velTarget, PxReal error, const PxD6JointDrive& drive)
			{
				addDrive(linear(axis,error,PxConstraintSolveHint::eNONE),velTarget,drive);
			}

			PX_FORCE_INLINE void angular(const PxVec3& axis, PxReal velTarget, PxReal error, const PxD6JointDrive& drive, PxConstraintSolveHint::Enum hint = PxConstraintSolveHint::eNONE)
			{
				addDrive(angular(axis,error,hint),velTarget,drive);
			}


			PX_FORCE_INLINE PxU32 getCount() { return PxU32(mCurrent - mConstraints); }

			void prepareLockedAxes(const PxQuat& qA, const PxQuat& qB, const PxVec3& cB2cAp, PxU32 lin, PxU32 ang)
			{
				Px1DConstraint* current = mCurrent;
				if(ang)
				{
					PxQuat qB2qA = qA.getConjugate() * qB;

					PxVec3 row[3];
					computeJacobianAxes(row, qA, qB);
					PxVec3 imp = qB2qA.getImaginaryPart();
					if(ang&1) angular(row[0], -imp.x, PxConstraintSolveHint::eEQUALITY, current++);
					if(ang&2) angular(row[1], -imp.y, PxConstraintSolveHint::eEQUALITY, current++);
					if(ang&4) angular(row[2], -imp.z, PxConstraintSolveHint::eEQUALITY, current++);
				}

				if(lin)
				{
					PxMat33 axes(qA);
					if(lin&1) linear(axes[0], -cB2cAp[0], PxConstraintSolveHint::eEQUALITY, current++);
					if(lin&2) linear(axes[1], -cB2cAp[1], PxConstraintSolveHint::eEQUALITY, current++);
					if(lin&4) linear(axes[2], -cB2cAp[2], PxConstraintSolveHint::eEQUALITY, current++);
				}

				for(Px1DConstraint* front = mCurrent; front < current; front++)
					front->flags = Px1DConstraintFlag::eOUTPUT_FORCE;

				mCurrent = current;
			}

			Px1DConstraint *getConstraintRow()
			{
				return mCurrent++;
			}

		private:
			PX_FORCE_INLINE Px1DConstraint* linear(const PxVec3& axis, PxReal posErr, PxConstraintSolveHint::Enum hint)
			{
				Px1DConstraint* c = mCurrent++;

				c->solveHint		= PxU16(hint);
				c->linear0 = axis;					c->angular0	= mRa.cross(axis);
				c->linear1 = axis;					c->angular1 = mRb.cross(axis);
				PX_ASSERT(c->linear0.isFinite());
				PX_ASSERT(c->linear1.isFinite());
				PX_ASSERT(c->angular0.isFinite());
				PX_ASSERT(c->angular1.isFinite());

				c->geometricError	= posErr;		

				return c;
			}

			PX_FORCE_INLINE Px1DConstraint* angular(const PxVec3& axis, PxReal posErr, PxConstraintSolveHint::Enum hint)
			{
				Px1DConstraint* c = mCurrent++;

				c->solveHint		= PxU16(hint);
				c->linear0 = PxVec3(0);		c->angular0			= axis;
				c->linear1 = PxVec3(0);		c->angular1			= axis;

				c->geometricError	= posErr;
				return c;
			}

			PX_FORCE_INLINE Px1DConstraint* linear(const PxVec3& axis, PxReal posErr, PxConstraintSolveHint::Enum hint, Px1DConstraint* c)
			{
				c->solveHint		= PxU16(hint);
				c->linear0 = axis;					c->angular0	= mRa.cross(axis);
				c->linear1 = axis;					c->angular1 = mRb.cross(axis);
				PX_ASSERT(c->linear0.isFinite());
				PX_ASSERT(c->linear1.isFinite());
				PX_ASSERT(c->angular0.isFinite());
				PX_ASSERT(c->angular1.isFinite());

				c->geometricError	= posErr;		
				

				return c;
			}

			PX_FORCE_INLINE Px1DConstraint* angular(const PxVec3& axis, PxReal posErr, PxConstraintSolveHint::Enum hint, Px1DConstraint* c)
			{
				c->solveHint		= PxU16(hint);
				c->linear0 = PxVec3(0.f);		c->angular0			= axis;
				c->linear1 = PxVec3(0.f);		c->angular1			= axis;

				c->geometricError	= posErr;

				return c;
			}

			void addLimit(Px1DConstraint* c, const PxJointLimitParameters& limit)
			{
				PxU16 flags = PxU16(c->flags | Px1DConstraintFlag::eOUTPUT_FORCE);

				if(limit.isSoft())
				{
					flags |= Px1DConstraintFlag::eSPRING;
					c->mods.spring.stiffness = limit.stiffness;
					c->mods.spring.damping = limit.damping;
				}
				else
				{
					c->solveHint = PxConstraintSolveHint::eINEQUALITY;
					c->mods.bounce.restitution = limit.restitution;
					c->mods.bounce.velocityThreshold = limit.bounceThreshold;
					if(c->geometricError>0)
						flags |= Px1DConstraintFlag::eKEEPBIAS;
					if(limit.restitution>0)
						flags |= Px1DConstraintFlag::eRESTITUTION;
				}

				c->flags = flags;
				c->minImpulse = 0;
			}

			void addDrive(Px1DConstraint* c, PxReal velTarget, const PxD6JointDrive& drive)
			{
				c->velocityTarget = velTarget;

				PxU16 flags = PxU16(c->flags | Px1DConstraintFlag::eSPRING | Px1DConstraintFlag::eHAS_DRIVE_LIMIT);
				if(drive.flags & PxD6JointDriveFlag::eACCELERATION)
					flags |= Px1DConstraintFlag::eACCELERATION_SPRING;
				c->flags = flags;
				c->mods.spring.stiffness = drive.stiffness;
				c->mods.spring.damping = drive.damping;
				
				c->minImpulse = -drive.forceLimit;
				c->maxImpulse = drive.forceLimit;

				PX_ASSERT(c->linear0.isFinite());
				PX_ASSERT(c->angular0.isFinite());
			}
		};
	}
} // namespace

}

#endif
