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


#ifndef DY_BODYCORE_INTEGRATOR_H
#define DY_BODYCORE_INTEGRATOR_H

#include "CmPhysXCommon.h"
#include "PxvDynamics.h"
#include "PsMathUtils.h"
#include "PxsRigidBody.h"
#include "DySolverBody.h"
#include "DySleepingConfigulation.h"
#include "PxsIslandSim.h"

namespace physx
{

namespace Dy
{

PX_FORCE_INLINE void bodyCoreComputeUnconstrainedVelocity
(const PxVec3& gravity, const PxReal dt, const PxReal linearDamping, const PxReal angularDamping, const PxReal accelScale, 
const PxReal maxLinearVelocitySq, const PxReal maxAngularVelocitySq, PxVec3& inOutLinearVelocity, PxVec3& inOutAngularVelocity,
bool disableGravity)
{

	//Multiply everything that needs multiplied by dt to improve code generation.

	PxVec3 linearVelocity = inOutLinearVelocity;
	PxVec3 angularVelocity = inOutAngularVelocity;
	
	const PxReal linearDampingTimesDT=linearDamping*dt;
	const PxReal angularDampingTimesDT=angularDamping*dt;
	const PxReal oneMinusLinearDampingTimesDT=1.0f-linearDampingTimesDT;
	const PxReal oneMinusAngularDampingTimesDT=1.0f-angularDampingTimesDT;

	//TODO context-global gravity
	if (!disableGravity)
	{
		const PxVec3 linearAccelTimesDT = gravity*dt *accelScale;
		linearVelocity += linearAccelTimesDT;
	}

	//Apply damping.
	const PxReal linVelMultiplier = physx::intrinsics::fsel(oneMinusLinearDampingTimesDT, oneMinusLinearDampingTimesDT, 0.0f);
	const PxReal angVelMultiplier = physx::intrinsics::fsel(oneMinusAngularDampingTimesDT, oneMinusAngularDampingTimesDT, 0.0f);
	linearVelocity*=linVelMultiplier;
	angularVelocity*=angVelMultiplier;

	// Clamp velocity
	const PxReal linVelSq = linearVelocity.magnitudeSquared();
	if(linVelSq > maxLinearVelocitySq)
	{
		linearVelocity *= PxSqrt(maxLinearVelocitySq / linVelSq);
	}
	const PxReal angVelSq = angularVelocity.magnitudeSquared();
	if(angVelSq > maxAngularVelocitySq)
	{
		angularVelocity *= PxSqrt(maxAngularVelocitySq / angVelSq);
	}

	inOutLinearVelocity = linearVelocity;
	inOutAngularVelocity = angularVelocity;
}


PX_FORCE_INLINE void integrateCore(PxVec3& motionLinearVelocity, PxVec3& motionAngularVelocity, PxSolverBody& solverBody, PxSolverBodyData& solverBodyData, const PxF32 dt)
{
	PxU32 lockFlags = solverBodyData.lockFlags;
	if (lockFlags)
	{
		if (lockFlags & PxRigidDynamicLockFlag::eLOCK_LINEAR_X)
		{
			motionLinearVelocity.x = 0.f;
			solverBody.linearVelocity.x = 0.f;
		}
		if (lockFlags & PxRigidDynamicLockFlag::eLOCK_LINEAR_Y)
		{
			motionLinearVelocity.y = 0.f;
			solverBody.linearVelocity.y = 0.f;
		}
		if (lockFlags & PxRigidDynamicLockFlag::eLOCK_LINEAR_Z)
		{
			motionLinearVelocity.z = 0.f;
			solverBody.linearVelocity.z = 0.f;
		}
		
		//The angular velocity should be 0 because it is now impossible to make it rotate around that axis!
		if (lockFlags & PxRigidDynamicLockFlag::eLOCK_ANGULAR_X)
		{
			motionAngularVelocity.x = 0.f;
			solverBody.angularState.x = 0.f;
		}
		if (lockFlags & PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y)
		{
			motionAngularVelocity.y = 0.f;
			solverBody.angularState.y = 0.f;
		}
		if (lockFlags & PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z)
		{
			motionAngularVelocity.z = 0.f;
			solverBody.angularState.z = 0.f;
		}
	}

	// Integrate linear part
	PxVec3 linearMotionVel = solverBodyData.linearVelocity + motionLinearVelocity;
	PxVec3 delta = linearMotionVel * dt;
	PxVec3 angularMotionVel = solverBodyData.angularVelocity + solverBodyData.sqrtInvInertia * motionAngularVelocity;
	PxReal w = angularMotionVel.magnitudeSquared();
	solverBodyData.body2World.p += delta;
	PX_ASSERT(solverBodyData.body2World.p.isFinite());

	//Store back the linear and angular velocities
	//core.linearVelocity += solverBody.linearVelocity * solverBodyData.sqrtInvMass;
	solverBodyData.linearVelocity += solverBody.linearVelocity;
	solverBodyData.angularVelocity += solverBodyData.sqrtInvInertia * solverBody.angularState;
	
	// Integrate the rotation using closed form quaternion integrator
	if (w != 0.0f)
	{
		w = PxSqrt(w);
		// Perform a post-solver clamping
		// TODO(dsequeira): ignore this for the moment
		//just clamp motionVel to half float-range
		const PxReal maxW = 1e+7f;		//Should be about sqrt(PX_MAX_REAL/2) or smaller
		if (w > maxW)
		{
			angularMotionVel = angularMotionVel.getNormalized() * maxW;
			w = maxW;
		}
		const PxReal v = dt * w * 0.5f;
		PxReal s, q;
		Ps::sincos(v, s, q);
		s /= w;

		const PxVec3 pqr = angularMotionVel * s;
		const PxQuat quatVel(pqr.x, pqr.y, pqr.z, 0);
		PxQuat result = quatVel * solverBodyData.body2World.q;

		result += solverBodyData.body2World.q * q;

		solverBodyData.body2World.q = result.getNormalized();
		PX_ASSERT(solverBodyData.body2World.q.isSane());
		PX_ASSERT(solverBodyData.body2World.q.isFinite());
	}

	motionLinearVelocity = linearMotionVel;
	motionAngularVelocity = angularMotionVel;
}


PX_FORCE_INLINE PxReal updateWakeCounter(PxsRigidBody* originalBody, PxReal dt, PxReal /*invDt*/, const bool enableStabilization, const bool useAdaptiveForce, Cm::SpatialVector& motionVelocity,
	bool hasStaticTouch)
{
	//KS - at most one of these features can be enabled at any time
	PX_ASSERT(!useAdaptiveForce || !enableStabilization);
	PxsBodyCore& bodyCore = originalBody->getCore();

	// update the body's sleep state and 
	PxReal wakeCounterResetTime = 20.0f*0.02f;

	PxReal wc = bodyCore.wakeCounter;

	{
		if (enableStabilization)
		{
			bool freeze = false;
			const PxTransform& body2World = bodyCore.body2World;

			// calculate normalized energy: kinetic energy divided by mass

			const PxVec3 t = bodyCore.inverseInertia;
			const PxVec3 inertia(t.x > 0.f ? 1.0f / t.x : 1.f, t.y > 0.f ? 1.0f / t.y : 1.f, t.z > 0.f ? 1.0f / t.z : 1.f);


			PxVec3 sleepLinVelAcc = motionVelocity.linear;
			PxVec3 sleepAngVelAcc = body2World.q.rotateInv(motionVelocity.angular);

			// scale threshold by cluster factor (more contacts => higher sleep threshold)
			//const PxReal clusterFactor = PxReal(1u + getNumUniqueInteractions());

			PxReal invMass = bodyCore.inverseMass;
			if (invMass == 0.f)
				invMass = 1.f;

			const PxReal angular = sleepAngVelAcc.multiply(sleepAngVelAcc).dot(inertia) * invMass;
			const PxReal linear = sleepLinVelAcc.magnitudeSquared();
			PxReal frameNormalizedEnergy = 0.5f * (angular + linear);

			const PxReal cf = hasStaticTouch ? PxReal(PxMin(10u, bodyCore.numBodyInteractions)) : 0.f;
			const PxReal freezeThresh = cf*bodyCore.freezeThreshold;

			originalBody->freezeCount = PxMax(originalBody->freezeCount - dt, 0.0f);
			bool settled = true;

			PxReal accelScale = PxMin(1.f, originalBody->accelScale + dt);

			if (!hasStaticTouch)
				accelScale = 1.f;

			if (frameNormalizedEnergy >= freezeThresh)
			{
				settled = false;
				originalBody->freezeCount = PXD_FREEZE_INTERVAL;
			}

			if (settled)
			{
				//Dampen bodies that are just about to go to sleep
				if (cf > 1.f)
				{
					const PxReal sleepDamping = PXD_SLEEP_DAMPING;
					const PxReal sleepDampingTimesDT = sleepDamping*dt;
					const PxReal d = 1.0f - sleepDampingTimesDT;
					bodyCore.linearVelocity = bodyCore.linearVelocity * d;
					bodyCore.angularVelocity = bodyCore.angularVelocity * d;
					accelScale = PXD_FREEZE_SCALE;
				}
				freeze = originalBody->freezeCount == 0.f && frameNormalizedEnergy < (bodyCore.freezeThreshold * PXD_FREEZE_TOLERANCE);
			}

			originalBody->accelScale = accelScale;

			if (freeze)
			{
				//current flag isn't frozen but freeze flag raise so we need to raise the frozen flag in this frame
				bool wasNotFrozen = (originalBody->mInternalFlags & PxsRigidBody::eFROZEN) == 0;
				PxU16 flags = PxU16((originalBody->mInternalFlags & PxsRigidBody::eDISABLE_GRAVITY) | PxsRigidBody::eFROZEN);
				if (wasNotFrozen)
				{
					flags |= PxsRigidBody::eFREEZE_THIS_FRAME;
				}
				originalBody->mInternalFlags = flags;
				bodyCore.body2World = originalBody->getLastCCDTransform();
			}
			else
			{
				PxU16 flags = PxU16(originalBody->mInternalFlags & PxsRigidBody::eDISABLE_GRAVITY);
				bool wasFrozen = (originalBody->mInternalFlags & PxsRigidBody::eFROZEN) != 0;
				if (wasFrozen)
				{
					flags |= PxsRigidBody::eUNFREEZE_THIS_FRAME;
				}
				originalBody->mInternalFlags = flags;
			}

			/*KS: New algorithm for sleeping when using stabilization:
			* Energy *this frame* must be higher than sleep threshold and accumulated energy over previous frames
			* must be higher than clusterFactor*energyThreshold.
			*/
			if (wc < wakeCounterResetTime * 0.5f || wc < dt)
			{
				//Accumulate energy
				originalBody->sleepLinVelAcc += sleepLinVelAcc;
				originalBody->sleepAngVelAcc += sleepAngVelAcc;

				//If energy this frame is high
				if (frameNormalizedEnergy >= bodyCore.sleepThreshold)
				{
					//Compute energy over sleep preparation time
					const PxReal sleepAngular = originalBody->sleepAngVelAcc.multiply(originalBody->sleepAngVelAcc).dot(inertia) * invMass;
					const PxReal sleepLinear = originalBody->sleepLinVelAcc.magnitudeSquared();
					PxReal normalizedEnergy = 0.5f * (sleepAngular + sleepLinear);
					const PxReal sleepClusterFactor = PxReal(1u + bodyCore.numCountedInteractions);
					// scale threshold by cluster factor (more contacts => higher sleep threshold)
					const PxReal threshold = sleepClusterFactor*bodyCore.sleepThreshold;

					//If energy over sleep preparation time is high
					if (normalizedEnergy >= threshold)
					{
						//Wake up
						//PX_ASSERT(isActive());
						originalBody->sleepAngVelAcc = PxVec3(0);
						originalBody->sleepLinVelAcc = PxVec3(0);

						const float factor = bodyCore.sleepThreshold == 0.f ? 2.0f : PxMin(normalizedEnergy / threshold, 2.0f);
						PxReal oldWc = wc;
						wc = factor * 0.5f * wakeCounterResetTime + dt * (sleepClusterFactor - 1.0f);
						bodyCore.solverWakeCounter = wc;
						//if (oldWc == 0.0f)  // for the case where a sleeping body got activated by the system (not the user) AND got processed by the solver as well
						//	notifyNotReadyForSleeping(bodyCore.nodeIndex);

						if (oldWc == 0.0f)
							originalBody->mInternalFlags |= PxsRigidBody::eACTIVATE_THIS_FRAME;

						return wc;
					}
				}
			}

		}
		else 
		{
			if (useAdaptiveForce)
			{
				if (hasStaticTouch && bodyCore.numBodyInteractions > 1)
					originalBody->accelScale = 1.f / PxReal(bodyCore.numBodyInteractions);
				else
					originalBody->accelScale = 1.f;
			}
			if (wc < wakeCounterResetTime * 0.5f || wc < dt)
			{
				const PxTransform& body2World = bodyCore.body2World;

				// calculate normalized energy: kinetic energy divided by mass
				const PxVec3 t = bodyCore.inverseInertia;
				const PxVec3 inertia(t.x > 0.f ? 1.0f / t.x : 1.f, t.y > 0.f ? 1.0f / t.y : 1.f, t.z > 0.f ? 1.0f / t.z : 1.f);

				PxVec3 sleepLinVelAcc = motionVelocity.linear;
				PxVec3 sleepAngVelAcc = body2World.q.rotateInv(motionVelocity.angular);

				originalBody->sleepLinVelAcc += sleepLinVelAcc;
				originalBody->sleepAngVelAcc += sleepAngVelAcc;

				PxReal invMass = bodyCore.inverseMass;
				if (invMass == 0.f)
					invMass = 1.f;

				const PxReal angular = originalBody->sleepAngVelAcc.multiply(originalBody->sleepAngVelAcc).dot(inertia) * invMass;
				const PxReal linear = originalBody->sleepLinVelAcc.magnitudeSquared();
				PxReal normalizedEnergy = 0.5f * (angular + linear);

				// scale threshold by cluster factor (more contacts => higher sleep threshold)
				const PxReal clusterFactor = PxReal(1 + bodyCore.numCountedInteractions);
				const PxReal threshold = clusterFactor*bodyCore.sleepThreshold;

				if (normalizedEnergy >= threshold)
				{
					//PX_ASSERT(isActive());
					originalBody->sleepLinVelAcc = PxVec3(0);
					originalBody->sleepAngVelAcc = PxVec3(0);
					const float factor = threshold == 0.f ? 2.0f : PxMin(normalizedEnergy / threshold, 2.0f);
					PxReal oldWc = wc;
					wc = factor * 0.5f * wakeCounterResetTime + dt * (clusterFactor - 1.0f);
					bodyCore.solverWakeCounter = wc;
					PxU16 flags = PxU16(originalBody->mInternalFlags & PxsRigidBody::eDISABLE_GRAVITY);
					if (oldWc == 0.0f)  // for the case where a sleeping body got activated by the system (not the user) AND got processed by the solver as well
					{
						flags |= PxsRigidBody::eACTIVATE_THIS_FRAME;
						//notifyNotReadyForSleeping(bodyCore.nodeIndex);
					}

					originalBody->mInternalFlags = flags;

					return wc;
				}
			}
		}
	}

	wc = PxMax(wc - dt, 0.0f);
	bodyCore.solverWakeCounter = wc;
	return wc;
}

PX_FORCE_INLINE void sleepCheck(PxsRigidBody* originalBody, const PxReal dt, const PxReal intDt, const bool enableStabilization, bool useAdaptiveForce, Cm::SpatialVector& motionVelocity,
	bool hasStaticTouch)
{

	PxReal wc = updateWakeCounter(originalBody, dt, intDt, enableStabilization, useAdaptiveForce, motionVelocity, hasStaticTouch);
	bool wakeCounterZero = (wc == 0.0f);

	if (wakeCounterZero)
	{
		//PxsBodyCore& bodyCore = originalBody->getCore();
		originalBody->mInternalFlags |= PxsRigidBody::eDEACTIVATE_THIS_FRAME;
		//	notifyReadyForSleeping(bodyCore.nodeIndex);
		originalBody->sleepLinVelAcc = PxVec3(0);
		originalBody->sleepAngVelAcc = PxVec3(0);
	}
}

}

}

#endif //DY_BODYCORE_INTEGRATOR_H
