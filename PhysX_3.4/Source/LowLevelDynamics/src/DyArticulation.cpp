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
// Copyright (c) 2008-2018 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  


#include "PsMathUtils.h"
#include "CmConeLimitHelper.h"
#include "DySolverConstraint1D.h"
#include "DyArticulation.h"
#include "DyArticulationHelper.h"
#include "PxsRigidBody.h"
#include "PxcConstraintBlockStream.h"
#include "DyArticulationContactPrep.h"
#include "DyDynamics.h"
#include "DyArticulationReference.h"
#include "DyArticulationPImpl.h"
#include <stdio.h>

using namespace physx;

// we encode articulation link handles in the lower bits of the pointer, so the
// articulation has to be aligned, which in an aligned pool means we need to size it
// appropriately

namespace physx
{
	namespace Dy
	{
		void SolverCoreRegisterArticulationFns();

		void SolverCoreRegisterArticulationFnsCoulomb();


PX_COMPILE_TIME_ASSERT((sizeof(Articulation)&(DY_ARTICULATION_MAX_SIZE-1))==0);

Articulation::Articulation(Sc::ArticulationSim* sim)
:	mSolverDesc(NULL), mArticulationSim(sim)
{
	PX_ASSERT((reinterpret_cast<size_t>(this) & (DY_ARTICULATION_MAX_SIZE-1))==0);
}

Articulation::~Articulation()
{
}


/* computes the implicit impulse and the drive scale at the joint, in joint coords */

PxU32 Articulation::getLinkIndex(ArticulationLinkHandle handle)	const	
{ 
	return PxU32(handle&DY_ARTICULATION_IDMASK); 
}

#if DY_DEBUG_ARTICULATION

void Articulation::computeResiduals(const Cm::SpatialVector *v, 
									   const ArticulationJointTransforms* jointTransforms,
									   bool /*dump*/) const
{
	typedef ArticulationFnsScalar Fns;

	PxReal error = 0, energy = 0;
	for(PxU32 i=1;i<mSolverDesc->linkCount;i++)
	{
		const ArticulationJointTransforms &b = jointTransforms[i];
		PxU32 parent = mSolverDesc->links[i].parent;
		const ArticulationJointCore &j = *mSolverDesc->links[i].inboundJoint;
		PX_UNUSED(j);

		Cm::SpatialVector residual = Fns::translateMotion(mSolverDesc->poses[i].p - b.cB2w.p, v[i])
								   - Fns::translateMotion(mSolverDesc->poses[parent].p - b.cB2w.p, v[parent]);

		error += residual.linear.magnitudeSquared();
		energy += residual.angular.magnitudeSquared();

	}
//	if(dump)
		printf("Energy %f, Error %f\n", energy, error);
}


Cm::SpatialVector Articulation::computeMomentum(const FsInertia *inertia) const
{
	typedef ArticulationFnsScalar Fns;

	Cm::SpatialVector *velocity = reinterpret_cast<Cm::SpatialVector*>(getVelocity(*mSolverDesc->fsData));
	Cm::SpatialVector m = Cm::SpatialVector::zero();
	for(PxU32 i=0;i<mSolverDesc->linkCount;i++)
		m += Fns::translateForce(mSolverDesc->poses[i].p - mSolverDesc->poses[0].p, ArticulationFnsScalar::multiply(inertia[i], velocity[i]));
	return m;
}



void Articulation::checkLimits() const
{
	for(PxU32 i=1;i<mSolverDesc->linkCount;i++)
	{
		PxTransform cA2w = mSolverDesc->poses[mSolverDesc->links[i].parent].transform(mSolverDesc->links[i].inboundJoint->parentPose);
		PxTransform cB2w = mSolverDesc->poses[i].transform(mSolverDesc->links[i].inboundJoint->childPose);
		
		PxTransform cB2cA = cA2w.transformInv(cB2w);

		// the relative quat must be the short way round for limits to work...

		if(cB2cA.q.w<0)
			cB2cA.q	= -cB2cA.q;

		const ArticulationJointCore& j = *mSolverDesc->links[i].inboundJoint;
		
		PxQuat swing, twist;
		if(j.twistLimited || j.swingLimited)
			Ps::separateSwingTwist(cB2cA.q, swing, twist);
		
		if(j.swingLimited)
		{
			PxReal swingLimitContactDistance = PxMin(j.swingYLimit, j.swingZLimit)/4;

			Cm::ConeLimitHelper eh(PxTan(j.swingYLimit/4), 
								   PxTan(j.swingZLimit/4),
								   PxTan(swingLimitContactDistance/4));

			PxVec3 axis;
			PxReal error = 0.0f;
			if(eh.getLimit(swing, axis, error))
				printf("%u, (%f, %f), %f, (%f, %f, %f), %f\n", i, j.swingYLimit, j.swingZLimit, swingLimitContactDistance, axis.x, axis.y, axis.z, error);
		}

//		if(j.twistLimited)
//		{
//			PxReal tqTwistHigh = PxTan(j.twistLimitHigh/4),
//				   tqTwistLow  = PxTan(j.twistLimitLow/4),
//				   twistPad = (tqTwistHigh - tqTwistLow)*0.25f;
//				   //twistPad = j.twistLimitContactDistance;
//
//			PxVec3 axis = jointTransforms[i].cB2w.rotate(PxVec3(1,0,0));
//			PxReal tqPhi = Ps::tanHalf(twist.x, twist.w);
//
//			if(tqPhi < tqTwistLow + twistPad)
//				constraintData.pushBack(ConstraintData(-axis, -(tqTwistLow - tqPhi)*4));
//
//			if(tqPhi > tqTwistHigh - twistPad)
//				constraintData.pushBack(ConstraintData(axis, (tqTwistHigh - tqPhi)*4));
//		}
	}
	puts("");
}

#endif

void PxvRegisterArticulations()
{
	ArticulationPImpl::sComputeUnconstrainedVelocities = &ArticulationHelper::computeUnconstrainedVelocities;
	ArticulationPImpl::sUpdateBodies = &ArticulationHelper::updateBodies;
	ArticulationPImpl::sSaveVelocity = &ArticulationHelper::saveVelocity;

	SolverCoreRegisterArticulationFns();
	SolverCoreRegisterArticulationFnsCoulomb();
}

void Articulation::getDataSizes(PxU32 linkCount, PxU32 &solverDataSize, PxU32& totalSize, PxU32& scratchSize)
{
	solverDataSize = sizeof(FsData)													// header
				   + sizeof(Cm::SpatialVectorV)	* linkCount								// velocity
				   + sizeof(Cm::SpatialVectorV)	* linkCount								// deferredVelocity
				   + sizeof(Vec3V)				* linkCount								// deferredSZ
				   + sizeof(PxReal)				* ((linkCount + 15) & 0xFFFFFFF0)		// The maxPenBias values
				   + sizeof(FsJointVectors)	* linkCount								// joint offsets
			   	   + sizeof(FsInertia)												// featherstone root inverse inertia
				   + sizeof(FsRow)			* linkCount;							// featherstone matrix rows

	totalSize = solverDataSize
			  + sizeof(LtbRow)		 * linkCount			// lagrange matrix rows
			  + sizeof(Cm::SpatialVectorV) * linkCount			// ref velocity
			  + sizeof(FsRowAux)	 * linkCount;

	scratchSize = PxU32(sizeof(FsInertia)*linkCount*3
		        + ((sizeof(ArticulationJointTransforms)+15)&~15) * linkCount
				+ sizeof(Mat33V) * linkCount
				+ ((sizeof(ArticulationJointTransforms)+15)&~15) * linkCount);
}


void PxvArticulationDriveCache::initialize(FsData &cache,
										   PxU16 linkCount,
										   const ArticulationLink* links,
										   PxReal compliance,
										   PxU32 iterations,
										   char* scratchMemory,
										   PxU32 scratchMemorySize)
{
	ArticulationHelper::initializeDriveCache(cache, linkCount, links, compliance, iterations, scratchMemory, scratchMemorySize);
}

PxU32	PxvArticulationDriveCache::getLinkCount(const FsData& cache)
{
	return cache.linkCount;
}

void PxvArticulationDriveCache::applyImpulses(const FsData& cache,
										 	  Cm::SpatialVectorV* Z,
											  Cm::SpatialVectorV* V)
{
	ArticulationHelper::applyImpulses(cache, Z, V);
}

void	PxvArticulationDriveCache::getImpulseResponse(const FsData& cache, 
													  PxU32 linkID, 
													  const Cm::SpatialVectorV& impulse,
													  Cm::SpatialVectorV& deltaV)
{
	ArticulationHelper::getImpulseResponse(cache, linkID, impulse, deltaV);
}

}
}
