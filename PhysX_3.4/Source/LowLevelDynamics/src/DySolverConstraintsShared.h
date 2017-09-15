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

#ifndef DY_SOLVER_CORE_SHARED_H
#define DY_SOLVER_CORE_SHARED_H

#include "foundation/PxPreprocessor.h"
#include "PsVecMath.h"

#include "CmPhysXCommon.h"
#include "DySolverBody.h"
#include "DySolverContact.h"
#include "DySolverConstraint1D.h"
#include "DySolverConstraintDesc.h"
#include "PsUtilities.h"
#include "DyConstraint.h"
#include "PsAtomic.h"


namespace physx
{

namespace Dy
{
	PX_FORCE_INLINE static FloatV solveDynamicContacts(SolverContactPoint* contacts, const PxU32 nbContactPoints, const Vec3VArg contactNormal,
	const FloatVArg invMassA, const FloatVArg invMassB, const FloatVArg angDom0, const FloatVArg angDom1, Vec3V& linVel0_, Vec3V& angState0_, 
	Vec3V& linVel1_, Vec3V& angState1_, PxF32* PX_RESTRICT forceBuffer)
{
	Vec3V linVel0 = linVel0_;
	Vec3V angState0 = angState0_;
	Vec3V linVel1 = linVel1_;
	Vec3V angState1 = angState1_;
	FloatV accumulatedNormalImpulse = FZero();

	const Vec3V delLinVel0 = V3Scale(contactNormal, invMassA);
	const Vec3V delLinVel1 = V3Scale(contactNormal, invMassB);

	for(PxU32 i=0;i<nbContactPoints;i++)
	{
		SolverContactPoint& c = contacts[i];
		Ps::prefetchLine(&contacts[i], 128);

		const Vec3V raXn = c.raXn;

		const Vec3V rbXn = c.rbXn;

		const FloatV appliedForce = FLoad(forceBuffer[i]);
		const FloatV velMultiplier = c.getVelMultiplier();
		
		/*const FloatV targetVel = c.getTargetVelocity();
		const FloatV nScaledBias = c.getScaledBias();*/
		const FloatV maxImpulse = c.getMaxImpulse();

		//Compute the normal velocity of the constraint.
		const Vec3V v0 = V3MulAdd(linVel0, contactNormal, V3Mul(angState0, raXn));
		const Vec3V v1 = V3MulAdd(linVel1, contactNormal, V3Mul(angState1, rbXn));
		const FloatV normalVel = V3SumElems(V3Sub(v0, v1));

		const FloatV biasedErr = c.getBiasedErr();//FScaleAdd(targetVel, velMultiplier, nScaledBias);

		//KS - clamp the maximum force
		const FloatV _deltaF = FMax(FNegScaleSub(normalVel, velMultiplier, biasedErr), FNeg(appliedForce));
		const FloatV _newForce = FAdd(appliedForce, _deltaF);
		const FloatV newForce = FMin(_newForce, maxImpulse);
		const FloatV deltaF = FSub(newForce, appliedForce);

		linVel0 = V3ScaleAdd(delLinVel0, deltaF, linVel0);
		linVel1 = V3NegScaleSub(delLinVel1, deltaF, linVel1);
		angState0 = V3ScaleAdd(raXn, FMul(deltaF, angDom0), angState0);
		angState1 = V3NegScaleSub(rbXn, FMul(deltaF, angDom1), angState1);
		
		FStore(newForce, &forceBuffer[i]);

		accumulatedNormalImpulse = FAdd(accumulatedNormalImpulse, newForce);
	}

	linVel0_ = linVel0;
	angState0_ = angState0;
	linVel1_ = linVel1;
	angState1_ = angState1;
	return accumulatedNormalImpulse;
}

PX_FORCE_INLINE static FloatV solveStaticContacts(SolverContactPoint* contacts, const PxU32 nbContactPoints, const Vec3VArg contactNormal,
	const FloatVArg invMassA, const FloatVArg angDom0, Vec3V& linVel0_, Vec3V& angState0_, PxF32* PX_RESTRICT forceBuffer)
{
	Vec3V linVel0 = linVel0_;
	Vec3V angState0 = angState0_;
	FloatV accumulatedNormalImpulse = FZero();

	const Vec3V delLinVel0 = V3Scale(contactNormal, invMassA);

	for(PxU32 i=0;i<nbContactPoints;i++)
	{
		SolverContactPoint& c = contacts[i];
		Ps::prefetchLine(&contacts[i],128);

		const Vec3V raXn = c.raXn;
		
		const FloatV appliedForce = FLoad(forceBuffer[i]);
		const FloatV velMultiplier = c.getVelMultiplier();

		/*const FloatV targetVel = c.getTargetVelocity();
		const FloatV nScaledBias = c.getScaledBias();*/
		const FloatV maxImpulse = c.getMaxImpulse();
		
		const Vec3V v0 = V3MulAdd(linVel0, contactNormal, V3Mul(angState0, raXn));
		const FloatV normalVel = V3SumElems(v0);


		const FloatV biasedErr = c.getBiasedErr();//FScaleAdd(targetVel, velMultiplier, nScaledBias);

		// still lots to do here: using loop pipelining we can interweave this code with the
		// above - the code here has a lot of stalls that we would thereby eliminate
		const FloatV _deltaF = FMax(FNegScaleSub(normalVel, velMultiplier, biasedErr), FNeg(appliedForce));
		const FloatV _newForce = FAdd(appliedForce, _deltaF);
		const FloatV newForce = FMin(_newForce, maxImpulse);
		const FloatV deltaF = FSub(newForce, appliedForce);

		linVel0 = V3ScaleAdd(delLinVel0, deltaF, linVel0);
		angState0 = V3ScaleAdd(raXn, FMul(deltaF, angDom0), angState0);

		FStore(newForce, &forceBuffer[i]);

		accumulatedNormalImpulse = FAdd(accumulatedNormalImpulse, newForce);
	}

	linVel0_ = linVel0;
	angState0_ = angState0;
	return accumulatedNormalImpulse;
}

PX_FORCE_INLINE static FloatV solveExtContacts(SolverContactPointExt* contacts, const PxU32 nbContactPoints, const Vec3VArg contactNormal,
		Vec3V& linVel0, Vec3V& angVel0, 
		Vec3V& linVel1, Vec3V& angVel1, 
		Vec3V& li0, Vec3V& ai0,
		Vec3V& li1, Vec3V& ai1, 
		PxF32* PX_RESTRICT appliedForceBuffer)
	{

		FloatV accumulatedNormalImpulse = FZero();
		for(PxU32 i=0;i<nbContactPoints;i++)
		{
			SolverContactPointExt& c = contacts[i];
			Ps::prefetchLine(&contacts[i+1]);

			const Vec3V raXn = c.raXn;
			const Vec3V rbXn = c.rbXn;

			const FloatV appliedForce = FLoad(appliedForceBuffer[i]);
			const FloatV velMultiplier = c.getVelMultiplier();

			/*const FloatV targetVel = c.getTargetVelocity();
			const FloatV scaledBias = c.getScaledBias();*/

			//Compute the normal velocity of the constraint.

			Vec3V v = V3MulAdd(linVel0, contactNormal, V3Mul(angVel0, raXn));
			v = V3Sub(v, V3MulAdd(linVel1, contactNormal, V3Mul(angVel1, rbXn)));
			const FloatV normalVel = V3SumElems(v);
			
			const FloatV biasedErr = c.getBiasedErr();//FNeg(scaledBias);

			// still lots to do here: using loop pipelining we can interweave this code with the
			// above - the code here has a lot of stalls that we would thereby eliminate

			const FloatV deltaF = FMax(FNegScaleSub(normalVel, velMultiplier, biasedErr), FNeg(appliedForce));

			linVel0 = V3ScaleAdd(c.linDeltaVA, deltaF, linVel0);	
			angVel0 = V3ScaleAdd(c.angDeltaVA, deltaF, angVel0);
			linVel1 = V3ScaleAdd(c.linDeltaVB, deltaF, linVel1);	
			angVel1 = V3ScaleAdd(c.angDeltaVB, deltaF, angVel1);

			li0 = V3ScaleAdd(contactNormal, deltaF, li0);	ai0 = V3ScaleAdd(raXn, deltaF, ai0);
			li1 = V3ScaleAdd(contactNormal, deltaF, li1);	ai1 = V3ScaleAdd(rbXn, deltaF, ai1);

			const FloatV newAppliedForce = FAdd(appliedForce, deltaF);

			FStore(newAppliedForce, &appliedForceBuffer[i]);

			accumulatedNormalImpulse = FAdd(accumulatedNormalImpulse, newAppliedForce);
		}
		return accumulatedNormalImpulse;
	}

}

}

#endif //DY_SOLVER_CORE_SHARED_H

