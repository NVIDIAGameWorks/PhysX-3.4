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


#ifndef PXS_BODYATOM_H
#define PXS_BODYATOM_H

#include "PxcRigidBody.h"
#include "PxvDynamics.h"

namespace physx
{

class PxsRigidBody : public PxcRigidBody
{
	public:
	PX_FORCE_INLINE									PxsRigidBody(PxsBodyCore* core)	: PxcRigidBody(core) {  }
	PX_FORCE_INLINE									~PxsRigidBody() {}

	PX_FORCE_INLINE		const PxTransform&			getPose()								const	{ PX_ASSERT(mCore->body2World.isSane()); return mCore->body2World; }

	//PX_FORCE_INLINE		const Cm::SpatialVector&	getAccelerationV()						const	{ return mAcceleration;		}
	//PX_FORCE_INLINE		void						setAccelerationV(const Cm::SpatialVector& v)	{ mAcceleration = v;		}

	PX_FORCE_INLINE		const PxVec3&				getLinearVelocity()						const	{ PX_ASSERT(mCore->linearVelocity.isFinite()); return mCore->linearVelocity;			}
	PX_FORCE_INLINE		const PxVec3&				getAngularVelocity()					const	{ PX_ASSERT(mCore->angularVelocity.isFinite()); return mCore->angularVelocity;			}
	
	PX_FORCE_INLINE		void	 					setVelocity(const PxVec3& linear,
																const PxVec3& angular)				{ PX_ASSERT(linear.isFinite()); PX_ASSERT(angular.isFinite());
																									  mCore->linearVelocity = linear;
																									  mCore->angularVelocity = angular; }
	PX_FORCE_INLINE		void						setLinearVelocity(const PxVec3& linear)			{ PX_ASSERT(linear.isFinite()); mCore->linearVelocity = linear; }
	PX_FORCE_INLINE		void						setAngularVelocity(const PxVec3& angular)		{ PX_ASSERT(angular.isFinite()); mCore->angularVelocity = angular; }

	PX_FORCE_INLINE		void						constrainLinearVelocity();
	PX_FORCE_INLINE		void						constrainAngularVelocity();

	PX_FORCE_INLINE		PxU32						getIterationCounts()							{ return mCore->solverIterationCounts; }

	PX_FORCE_INLINE		PxReal						getReportThreshold()					const	{ return mCore->contactReportThreshold;	}

	// AP newccd todo: merge into get both velocities, compute inverse transform once, precompute mLastTransform.getInverse()
	PX_FORCE_INLINE		PxVec3						getLinearMotionVelocity(PxReal invDt)		const	{
														// delta(t0(x))=t1(x)
														// delta(t0(t0`(x)))=t1(t0`(x))
														// delta(x)=t1(t0`(x))
														PxVec3 deltaP = mCore->body2World.p - getLastCCDTransform().p;
														return deltaP * invDt;
													}
	PX_FORCE_INLINE		PxVec3						getAngularMotionVelocity(PxReal invDt)		const	{
														PxQuat deltaQ = mCore->body2World.q * getLastCCDTransform().q.getConjugate();
														PxVec3 axis;
														PxReal angle;
														deltaQ.toRadiansAndUnitAxis(angle, axis);
														return axis * angle * invDt;
													}
	PX_FORCE_INLINE		PxVec3						getLinearMotionVelocity(PxReal dt, const PxsBodyCore* PX_RESTRICT bodyCore)		const	{
															// delta(t0(x))=t1(x)
															// delta(t0(t0`(x)))=t1(t0`(x))
															// delta(x)=t1(t0`(x))
															PxVec3 deltaP = bodyCore->body2World.p - getLastCCDTransform().p;
															return deltaP * 1.0f / dt;
													}
	PX_FORCE_INLINE		PxVec3						getAngularMotionVelocity(PxReal dt, const PxsBodyCore* PX_RESTRICT bodyCore)		const	{
															PxQuat deltaQ = bodyCore->body2World.q * getLastCCDTransform().q.getConjugate();
															PxVec3 axis;
															PxReal angle;
															deltaQ.toRadiansAndUnitAxis(angle, axis);
															return axis * angle * 1.0f/dt;
													}

	PX_FORCE_INLINE		const PxTransform&			getLastCCDTransform()					const	{ return mLastTransform;}
	PX_FORCE_INLINE		void						saveLastCCDTransform()							{ mLastTransform = mCore->body2World; }

	PX_FORCE_INLINE		bool						isKinematic()							const	{ return (mCore->inverseMass == 0.0f); }
						
	PX_FORCE_INLINE		void						setPose(const PxTransform& pose)				{ mCore->body2World = pose; }
	PX_FORCE_INLINE		void						setPosition(const PxVec3& position)				{ mCore->body2World.p = position; }
	PX_FORCE_INLINE		PxReal						getInvMass()							const	{ return mCore->inverseMass; }
	PX_FORCE_INLINE		PxVec3						getInvInertia()							const	{ return mCore->inverseInertia; }
	PX_FORCE_INLINE		PxReal						getMass()								const	{ return 1.0f/mCore->inverseMass; }
	PX_FORCE_INLINE		PxVec3						getInertia()							const	{ return PxVec3(1.0f/mCore->inverseInertia.x,
																													1.0f/mCore->inverseInertia.y,
																													1.0f/mCore->inverseInertia.z); }
	PX_FORCE_INLINE		PxsBodyCore&				getCore()										{ return *mCore;					}
	PX_FORCE_INLINE		const PxsBodyCore&			getCore()								const	{ return *mCore;					}

	PX_FORCE_INLINE		PxU32						isActivateThisFrame()					const	{ return PxU32(mInternalFlags & eACTIVATE_THIS_FRAME); }

	PX_FORCE_INLINE		PxU32						isDeactivateThisFrame()					const	{ return PxU32(mInternalFlags & eDEACTIVATE_THIS_FRAME); }

	PX_FORCE_INLINE		PxU32						isFreezeThisFrame()						const	{ return PxU32(mInternalFlags & eFREEZE_THIS_FRAME); }

	PX_FORCE_INLINE		PxU32						isUnfreezeThisFrame()					const	{ return PxU32(mInternalFlags & eUNFREEZE_THIS_FRAME); }

	PX_FORCE_INLINE		void						clearFreezeFlag()								{ mInternalFlags &= ~eFREEZE_THIS_FRAME;	}

	PX_FORCE_INLINE		void						clearUnfreezeFlag()								{ mInternalFlags &= ~eUNFREEZE_THIS_FRAME; }

	PX_FORCE_INLINE		void						clearAllFrameFlags()							{ mInternalFlags &= (eFROZEN | eDISABLE_GRAVITY); }

						void						advanceToToi(PxReal toi, PxReal dt, bool clip);
						void						advancePrevPoseToToi(PxReal toi);
						PxTransform					getAdvancedTransform(PxReal toi) const;
						Cm::SpatialVector			getPreSolverVelocities() const;


};

void PxsRigidBody::constrainLinearVelocity()
{
	const PxU32 lockFlags = mCore->lockFlags;

	if (lockFlags)
	{
		if (lockFlags & PxRigidDynamicLockFlag::eLOCK_LINEAR_X)
			mCore->linearVelocity.x = 0.f;
		if (lockFlags & PxRigidDynamicLockFlag::eLOCK_LINEAR_Y)
			mCore->linearVelocity.y = 0.f;
		if (lockFlags & PxRigidDynamicLockFlag::eLOCK_LINEAR_Z)
			mCore->linearVelocity.z = 0.f;
	}
}

void PxsRigidBody::constrainAngularVelocity()
{
	const PxU32 lockFlags = mCore->lockFlags;

	if (lockFlags)
	{
		if (lockFlags & PxRigidDynamicLockFlag::eLOCK_ANGULAR_X)
			mCore->angularVelocity.x = 0.f;
		if (lockFlags & PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y)
			mCore->angularVelocity.y = 0.f;
		if (lockFlags & PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z)
			mCore->angularVelocity.z = 0.f;
	}
}

}

#endif
