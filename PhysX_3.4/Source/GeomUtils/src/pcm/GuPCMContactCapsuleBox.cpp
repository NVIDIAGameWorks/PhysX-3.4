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

#include "GuVecBox.h"
#include "GuVecCapsule.h"
#include "GuGeometryUnion.h"

#include "GuContactMethodImpl.h"
#include "GuContactBuffer.h"
#include "GuPCMContactGen.h"
#include "GuPCMShapeConvex.h"
#include "GuGJKPenetration.h"
#include "GuEPA.h"


namespace physx
{
	using namespace Ps::aos;

namespace Gu
{

static bool fullContactsGenerationCapsuleBox(const CapsuleV& capsule, const BoxV& box, const PxVec3 halfExtents,  const PsMatTransformV& aToB, const PsTransformV& transf0, const PsTransformV& transf1,
								PersistentContact* manifoldContacts, PxU32& numContacts, ContactBuffer& contactBuffer, PersistentContactManifold& manifold, Vec3V& normal, const Vec3VArg closest,
								const PxReal boxMargin, const FloatVArg contactDist, const bool doOverlapTest, const PxReal toleranceScale)
{

	PolygonalData polyData;
	PCMPolygonalBox polyBox(halfExtents);
	polyBox.getPolygonalData(&polyData);

	Mat33V identity = M33Identity();
	SupportLocalImpl<BoxV> map(box, transf1, identity, identity);

	PxU32 origContacts = numContacts;
	if (generateCapsuleBoxFullContactManifold(capsule, polyData, &map, aToB, manifoldContacts, numContacts, contactDist, normal, closest, boxMargin, doOverlapTest, toleranceScale))
	{
		//EPA has contacts and we have new contacts, we discard the EPA contacts
		if(origContacts != 0 && numContacts != origContacts)
		{
			numContacts--;
			manifoldContacts++;
		}

		manifold.addBatchManifoldContacts2(manifoldContacts, numContacts);
		
		normal = transf1.rotate(normal);
		
		manifold.addManifoldContactsToContactBuffer(contactBuffer, normal, normal, transf0, capsule.radius, contactDist);
	
		return true;
		
	}

	return false;

}


bool pcmContactCapsuleBox(GU_CONTACT_METHOD_ARGS)
{
	
	PX_UNUSED(renderOutput);

	PersistentContactManifold& manifold = cache.getManifold();
	Ps::prefetchLine(&manifold, 256);
	const PxCapsuleGeometry& shapeCapsule = shape0.get<const PxCapsuleGeometry>();
	const PxBoxGeometry& shapeBox = shape1.get<const PxBoxGeometry>();

	PX_ASSERT(transform1.q.isSane());
	PX_ASSERT(transform0.q.isSane());  

	const Vec3V zeroV = V3Zero();
	const Vec3V boxExtents = V3LoadU(shapeBox.halfExtents);

	const FloatV contactDist = FLoad(params.mContactDistance);

	const PsTransformV transf0 = loadTransformA(transform0);
	const PsTransformV transf1 = loadTransformA(transform1);

	const PsTransformV curRTrans = transf1.transformInv(transf0);
	const PsMatTransformV aToB_(curRTrans);

	const FloatV capsuleRadius = FLoad(shapeCapsule.radius);
	const FloatV capsuleHalfHeight = FLoad(shapeCapsule.halfHeight);

	const PxU32 initialContacts = manifold.mNumContacts;

	const PxReal toleranceLength = params.mToleranceLength;
	const FloatV boxMargin = Gu::CalculatePCMBoxMargin(boxExtents, toleranceLength);
	
	const FloatV minMargin = FMin(boxMargin, capsuleRadius);

	const FloatV projectBreakingThreshold = FMul(minMargin, FLoad(0.8f));
	
	const FloatV refreshDist = FAdd(contactDist, capsuleRadius);
	//refreshContactPoints remove invalid contacts from the manifold and update the number correspondingly
	manifold.refreshContactPoints(aToB_, projectBreakingThreshold, refreshDist);

	const bool bLostContacts = (manifold.mNumContacts != initialContacts);

	if(bLostContacts || manifold.invalidate_SphereCapsule(curRTrans, minMargin))	
	{

		GjkStatus status = manifold.mNumContacts > 0 ? GJK_UNDEFINED : GJK_NON_INTERSECT;

		Vec3V closestA(zeroV), closestB(zeroV);
		Vec3V normal(zeroV); // from a to b
		const FloatV zero = FZero();
		FloatV penDep = zero;

		manifold.setRelativeTransform(curRTrans);
		const PsMatTransformV aToB(curRTrans);
		
		BoxV box(transf1.p, boxExtents);
		//transform capsule into the local space of box
		CapsuleV capsule(aToB.p, aToB.rotate(V3Scale(V3UnitX(), capsuleHalfHeight)), capsuleRadius);
		LocalConvex<CapsuleV> convexA(capsule);
		LocalConvex<BoxV> convexB(box);
		const Vec3V initialSearchDir = V3Sub(capsule.getCenter(), box.getCenter());
		status =  gjkPenetration<LocalConvex<CapsuleV>, LocalConvex<BoxV> >(convexA, convexB, initialSearchDir, contactDist, closestA, closestB, normal, penDep, 
			manifold.mAIndice, manifold.mBIndice, manifold.mNumWarmStartPoints, true);

		PersistentContact* manifoldContacts = PX_CP_TO_PCP(contactBuffer.contacts);
		PxU32 numContacts = 0;
		bool doOverlapTest = false;
		if(status == GJK_NON_INTERSECT)
		{
			return false;
		}
		else if(status == GJK_DEGENERATE)
		{
			return fullContactsGenerationCapsuleBox(capsule, box, shapeBox.halfExtents,  aToB, transf0, transf1, manifoldContacts, numContacts, contactBuffer,
				manifold, normal, closestB, box.getMarginF(), contactDist, true, params.mToleranceLength);
		}
		else 
		{
			if(status == GJK_CONTACT)
			{
				const Vec3V localPointA = aToB.transformInv(closestA);//curRTrans.transformInv(closestA);
				const Vec4V localNormalPen = V4SetW(Vec4V_From_Vec3V(normal), penDep);
				//Add contact to contact stream
				manifoldContacts[numContacts].mLocalPointA = localPointA;
				manifoldContacts[numContacts].mLocalPointB = closestB;
				manifoldContacts[numContacts++].mLocalNormalPen = localNormalPen;
			}
			else
			{
				PX_ASSERT(status == EPA_CONTACT);
	
				status= epaPenetration(convexA, convexB,  manifold.mAIndice, manifold.mBIndice, manifold.mNumWarmStartPoints, 
					closestA, closestB, normal, penDep, true);
				if(status == EPA_CONTACT)
				{
					const Vec3V localPointA = aToB.transformInv(closestA);//curRTrans.transformInv(closestA);
					const Vec4V localNormalPen = V4SetW(Vec4V_From_Vec3V(normal), penDep);
					//Add contact to contact stream
					manifoldContacts[numContacts].mLocalPointA = localPointA;
					manifoldContacts[numContacts].mLocalPointB = closestB;
					manifoldContacts[numContacts++].mLocalNormalPen = localNormalPen;

				}
				else
				{
					doOverlapTest = true;
				}

			}
			

			if(initialContacts == 0 || bLostContacts || doOverlapTest)
			{
				return fullContactsGenerationCapsuleBox(capsule, box, shapeBox.halfExtents,  aToB, transf0, transf1, manifoldContacts, numContacts, contactBuffer, manifold, normal, 
					closestB, box.getMarginF(), contactDist, doOverlapTest, params.mToleranceLength);
			}
			else
			{
				
				//The contacts is either come from GJK or EPA
				const FloatV replaceBreakingThreshold = FMul(minMargin, FLoad(0.1f));
				const Vec4V localNormalPen = V4SetW(Vec4V_From_Vec3V(normal), penDep);
				manifold.addManifoldPoint2(curRTrans.transformInv(closestA), closestB, localNormalPen, replaceBreakingThreshold);
				
				normal = transf1.rotate(normal);
				manifold.addManifoldContactsToContactBuffer(contactBuffer, normal, normal, transf0, capsuleRadius, contactDist);
			
				return true;
			}
		}	
	}
	else if(manifold.getNumContacts() > 0)
	{
		const Vec3V worldNormal = manifold.getWorldNormal(transf1);
		manifold.addManifoldContactsToContactBuffer(contactBuffer, worldNormal, worldNormal, transf0, capsuleRadius, contactDist);
		return true;
	}

	return false;

}
}
}
