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


#include "foundation/PxPreprocessor.h"
#include "PsVecMath.h"
#include "DyArticulationContactPrep.h"
#include "DySolverConstraintDesc.h"
#include "DySolverConstraint1D.h"
#include "DySolverContact.h"
#include "DySolverContactPF.h"
#include "DyArticulationHelper.h"
#include "PxcNpWorkUnit.h"
#include "PxsMaterialManager.h"
#include "PxsMaterialCombiner.h"
#include "DyCorrelationBuffer.h"
#include "DySolverConstraintExtShared.h"

using namespace physx;
using namespace Gu;

// constraint-gen only, since these use getVelocityFast methods
// which aren't valid during the solver phase

namespace physx
{

namespace Dy
{


bool setupFinalizeExtSolverContactsCoulomb(
						    const ContactBuffer& buffer,
							const CorrelationBuffer& c,
							const PxTransform& bodyFrame0,
							const PxTransform& bodyFrame1,
							PxU8* workspace,
							PxReal invDt,
							PxReal bounceThresholdF32,
							const SolverExtBody& b0,
							const SolverExtBody& b1,
							PxU32 frictionCountPerPoint,
							PxReal invMassScale0, PxReal invInertiaScale0, 
							PxReal invMassScale1, PxReal invInertiaScale1,
							PxReal restDist,
							PxReal ccdMaxDistance)	
{
	// NOTE II: the friction patches are sparse (some of them have no contact patches, and
	// therefore did not get written back to the cache) but the patch addresses are dense,
	// corresponding to valid patches

	const FloatV ccdMaxSeparation = FLoad(ccdMaxDistance);

	PxU8* PX_RESTRICT ptr = workspace;

	//KS - TODO - this should all be done in SIMD to avoid LHS
	const PxF32 maxPenBias0 = b0.mLinkIndex == PxSolverConstraintDesc::NO_LINK ? b0.mBodyData->penBiasClamp : getMaxPenBias(*b0.mFsData)[b0.mLinkIndex];
	const PxF32 maxPenBias1 = b1.mLinkIndex == PxSolverConstraintDesc::NO_LINK ? b1.mBodyData->penBiasClamp : getMaxPenBias(*b1.mFsData)[b1.mLinkIndex];

	const FloatV maxPenBias = FLoad(PxMax(maxPenBias0, maxPenBias1)/invDt);

	const FloatV restDistance = FLoad(restDist); 
	const FloatV bounceThreshold = FLoad(bounceThresholdF32);

	const FloatV invDtV = FLoad(invDt);
	const FloatV pt8 = FLoad(0.8f);

	const FloatV invDtp8 = FMul(invDtV, pt8);

	Ps::prefetchLine(c.contactID);
	Ps::prefetchLine(c.contactID, 128);

	const PxU32 frictionPatchCount = c.frictionPatchCount;

	const PxU32 pointStride = sizeof(SolverContactPointExt);
	const PxU32 frictionStride = sizeof(SolverContactFrictionExt);
	const PxU8 pointHeaderType = DY_SC_TYPE_EXT_CONTACT;
	const PxU8 frictionHeaderType = DY_SC_TYPE_EXT_FRICTION;

	PxReal d0 = invMassScale0;
	PxReal d1 = invMassScale1;
	PxReal angD0 = invInertiaScale0;
	PxReal angD1 = invInertiaScale1;

	PxU8 flags = 0;

	for(PxU32 i=0;i< frictionPatchCount;i++)
	{
		const PxU32 contactCount = c.frictionPatchContactCounts[i];
		if(contactCount == 0)
			continue;

		const Gu::ContactPoint* contactBase0 = buffer.contacts + c.contactPatches[c.correlationListHeads[i]].start;

		const Vec3V normalV = Ps::aos::V3LoadA(contactBase0->normal);
		const Vec3V normal = V3LoadA(contactBase0->normal);

		const PxReal combinedRestitution = contactBase0->restitution;
	
		
		SolverContactCoulombHeader* PX_RESTRICT header = reinterpret_cast<SolverContactCoulombHeader*>(ptr);
		ptr += sizeof(SolverContactCoulombHeader);

		Ps::prefetchLine(ptr, 128);
		Ps::prefetchLine(ptr, 256);
		Ps::prefetchLine(ptr, 384);

		const FloatV restitution = FLoad(combinedRestitution);


		header->numNormalConstr		= PxU8(contactCount);
		header->type				= pointHeaderType;
		//header->setRestitution(combinedRestitution);

		header->setDominance0(d0);
		header->setDominance1(d1);
		header->angDom0 = angD0;
		header->angDom1 = angD1;
		header->flags = flags;
		
		header->setNormal(normalV);
		
		for(PxU32 patch=c.correlationListHeads[i]; 
			patch!=CorrelationBuffer::LIST_END; 
			patch = c.contactPatches[patch].next)
		{
			const PxU32 count = c.contactPatches[patch].count;
			const Gu::ContactPoint* contactBase = buffer.contacts + c.contactPatches[patch].start;
				
			PxU8* p = ptr;
			for(PxU32 j=0;j<count;j++)
			{
				const Gu::ContactPoint& contact = contactBase[j];

				SolverContactPointExt* PX_RESTRICT solverContact = reinterpret_cast<SolverContactPointExt*>(p);
				p += pointStride;

				setupExtSolverContact(b0, b1, d0, d1, angD0, angD1, bodyFrame0, bodyFrame1, normal, invDtV, invDtp8, restDistance, maxPenBias, restitution,
					bounceThreshold, contact, *solverContact, ccdMaxSeparation);
			}			
			ptr = p;
		}
	}

	//construct all the frictions

	PxU8* PX_RESTRICT ptr2 = workspace;

	const PxF32 orthoThreshold = 0.70710678f;
	const PxF32 eps = 0.00001f;
	bool hasFriction = false;

	for(PxU32 i=0;i< frictionPatchCount;i++)
	{
		const PxU32 contactCount = c.frictionPatchContactCounts[i];
		if(contactCount == 0)
			continue;

		SolverContactCoulombHeader* header = reinterpret_cast<SolverContactCoulombHeader*>(ptr2); 
		header->frictionOffset = PxU16(ptr - ptr2);
		ptr2 += sizeof(SolverContactCoulombHeader) + header->numNormalConstr * pointStride;

		const Gu::ContactPoint* contactBase0 = buffer.contacts + c.contactPatches[c.correlationListHeads[i]].start;

		PxVec3 normal = contactBase0->normal;

		const PxReal staticFriction = contactBase0->staticFriction;
		const bool disableStrongFriction = !!(contactBase0->materialFlags & PxMaterialFlag::eDISABLE_FRICTION);
		const bool haveFriction = (disableStrongFriction == 0);
	
		SolverFrictionHeader* frictionHeader = reinterpret_cast<SolverFrictionHeader*>(ptr);
		frictionHeader->numNormalConstr = Ps::to8(c.frictionPatchContactCounts[i]);
		frictionHeader->numFrictionConstr = Ps::to8(haveFriction ? c.frictionPatchContactCounts[i] * frictionCountPerPoint : 0);
		frictionHeader->flags = flags;
		ptr += sizeof(SolverFrictionHeader);
		PxF32* forceBuffer = reinterpret_cast<PxF32*>(ptr);
		ptr += frictionHeader->getAppliedForcePaddingSize(c.frictionPatchContactCounts[i]);
		PxMemZero(forceBuffer, sizeof(PxF32) * c.frictionPatchContactCounts[i]);
		Ps::prefetchLine(ptr, 128);
		Ps::prefetchLine(ptr, 256);
		Ps::prefetchLine(ptr, 384);


		const PxVec3 t0Fallback1(0.f, -normal.z, normal.y);
		const PxVec3 t0Fallback2(-normal.y, normal.x, 0.f) ;
		const PxVec3 tFallback1 = orthoThreshold > PxAbs(normal.x) ? t0Fallback1 : t0Fallback2;
		const PxVec3 vrel = b0.getLinVel() - b1.getLinVel();
		const PxVec3 t0_ = vrel - normal * (normal.dot(vrel));
		const PxReal sqDist = t0_.dot(t0_);
		const PxVec3 tDir0 = (sqDist > eps ? t0_: tFallback1).getNormalized();
		const PxVec3 tDir1 = tDir0.cross(normal);
		PxVec3 tFallback[2] = {tDir0, tDir1};

		PxU32 ind = 0;

		if(haveFriction)
		{
			hasFriction = true;
			frictionHeader->setStaticFriction(staticFriction);
			frictionHeader->invMass0D0 = d0;
			frictionHeader->invMass1D1 = d1;
			frictionHeader->angDom0 = angD0;
			frictionHeader->angDom1 = angD1;
			frictionHeader->type			= frictionHeaderType;
			
			PxU32 totalPatchContactCount = 0;
		
			for(PxU32 patch=c.correlationListHeads[i]; 
				patch!=CorrelationBuffer::LIST_END; 
				patch = c.contactPatches[patch].next)
			{
				const PxU32 count = c.contactPatches[patch].count;
				const PxU32 start = c.contactPatches[patch].start;
				const Gu::ContactPoint* contactBase = buffer.contacts + start;
					
				PxU8* p = ptr;

				for(PxU32 j =0; j < count; j++)
				{
					const Gu::ContactPoint& contact = contactBase[j];
					const PxVec3 ra = contact.point - bodyFrame0.p;
					const PxVec3 rb = contact.point - bodyFrame1.p;
						
					const PxVec3 targetVel = contact.targetVel;
					const PxVec3 pVRa = b0.getLinVel() + b0.getAngVel().cross(ra);
					const PxVec3 pVRb = b1.getLinVel() + b1.getAngVel().cross(rb);
					//const PxVec3 vrel = pVRa - pVRb;

					for(PxU32 k = 0; k < frictionCountPerPoint; ++k)
					{
						SolverContactFrictionExt* PX_RESTRICT f0 = reinterpret_cast<SolverContactFrictionExt*>(p);
						p += frictionStride;

						PxVec3 t0 = tFallback[ind];
						ind = 1 - ind;
						PxVec3 raXn = ra.cross(t0); 
						PxVec3 rbXn = rb.cross(t0); 
						Cm::SpatialVector deltaV0, deltaV1;

						const Cm::SpatialVector resp0 = createImpulseResponseVector(t0, raXn, b0);
						const Cm::SpatialVector resp1 = createImpulseResponseVector(-t0, -rbXn, b1);

						PxReal unitResponse = getImpulseResponse(b0, resp0, deltaV0, d0, angD0,
																 b1, resp1, deltaV1, d1, angD1);

						PxReal tv = targetVel.dot(t0);
						if(b0.mLinkIndex == PxSolverConstraintDesc::NO_LINK)
							tv += pVRa.dot(t0);
						else if(b1.mLinkIndex == PxSolverConstraintDesc::NO_LINK)
							tv -= pVRb.dot(t0);


						f0->setVelMultiplier(FLoad(unitResponse>0.0f ? 1.f/unitResponse : 0.0f));
						f0->setRaXn(resp0.angular);
						f0->setRbXn(-resp1.angular);
						f0->targetVel = tv;
						f0->setNormal(t0);
						f0->setAppliedForce(0.0f);
						f0->linDeltaVA = V3LoadA(deltaV0.linear);
						f0->angDeltaVA = V3LoadA(deltaV0.angular);
						f0->linDeltaVB = V3LoadA(deltaV1.linear);
						f0->angDeltaVB = V3LoadA(deltaV1.angular);
					}					
				}

				totalPatchContactCount += c.contactPatches[patch].count;
				
				ptr = p;	
			}
		}
	}
	//PX_ASSERT(ptr - workspace == n.solverConstraintSize);
	return hasFriction;
}


}

}
