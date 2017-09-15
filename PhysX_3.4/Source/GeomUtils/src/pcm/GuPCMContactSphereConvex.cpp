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


#include "GuGJKPenetration.h"
#include "GuEPA.h"
#include "GuVecCapsule.h"
#include "GuVecConvexHull.h"
#include "GuVecConvexHullNoScale.h"
#include "GuVecShrunkConvexHull.h"
#include "GuVecShrunkConvexHullNoScale.h"
#include "GuGeometryUnion.h"

#include "GuContactMethodImpl.h"
#include "GuContactBuffer.h"
#include "GuPCMContactGen.h"
#include "GuPCMShapeConvex.h"

namespace physx
{
namespace Gu
{

static void addToContactBuffer(Gu::ContactBuffer& contactBuffer, const Ps::aos::Vec3VArg worldNormal, const Ps::aos::Vec3VArg worldPoint, const Ps::aos::FloatVArg penDep)
{
	using namespace Ps::aos;
	Gu::ContactPoint& contact = contactBuffer.contacts[contactBuffer.count++];
	V4StoreA(Vec4V_From_Vec3V(worldNormal), reinterpret_cast<PxF32*>(&contact.normal.x));
	V4StoreA(Vec4V_From_Vec3V(worldPoint), reinterpret_cast<PxF32*>(&contact.point.x));
	FStore(penDep, &contact.separation);

	PX_ASSERT(contact.point.isFinite());
	PX_ASSERT(contact.normal.isFinite());
	PX_ASSERT(PxIsFinite(contact.separation));

	contact.internalFaceIndex1 = PXC_CONTACT_NO_FACE_INDEX;

}

static bool fullContactsGenerationSphereConvex(const Gu::CapsuleV& capsule, const Gu::ConvexHullV& convexHull, const Ps::aos::PsTransformV& transf0,const Ps::aos::PsTransformV& transf1,
								Gu::PersistentContact* manifoldContacts, Gu::ContactBuffer& contactBuffer, const bool idtScale, Gu::PersistentContactManifold& manifold, 
								Ps::aos::Vec3VArg normal, const Ps::aos::FloatVArg contactDist, bool doOverlapTest, Cm::RenderOutput* renderOutput)
{
	PX_UNUSED(renderOutput);
	using namespace Ps::aos;
	Gu::PolygonalData polyData;
	getPCMConvexData(convexHull,idtScale, polyData);

	PxU8 buff[sizeof(SupportLocalImpl<ConvexHullV>)];
	SupportLocal* map = (idtScale ? static_cast<SupportLocal*>(PX_PLACEMENT_NEW(buff, SupportLocalImpl<ConvexHullNoScaleV>)(static_cast<const ConvexHullNoScaleV&>(convexHull), transf1, convexHull.vertex2Shape, convexHull.shape2Vertex, idtScale)) : 
	static_cast<SupportLocal*>(PX_PLACEMENT_NEW(buff, SupportLocalImpl<ConvexHullV>)(convexHull, transf1, convexHull.vertex2Shape, convexHull.shape2Vertex, idtScale)));

	PxU32 numContacts = 0;
	if(generateSphereFullContactManifold(capsule, polyData, map, manifoldContacts, numContacts, contactDist, normal, doOverlapTest))
	{

		if(numContacts > 0)
		{
			
			Gu::PersistentContact& p = manifold.getContactPoint(0);

			p.mLocalPointA = manifoldContacts[0].mLocalPointA;
			p.mLocalPointB = manifoldContacts[0].mLocalPointB;
			p.mLocalNormalPen = manifoldContacts[0].mLocalNormalPen;
			manifold.mNumContacts =1;

			//transform normal to world space
			const Vec3V worldNormal = transf1.rotate(normal);
			const Vec3V worldP = V3NegScaleSub(worldNormal, capsule.radius, transf0.p);
			const FloatV penDep = FSub(V4GetW(manifoldContacts[0].mLocalNormalPen), capsule.radius);

#if	PCM_LOW_LEVEL_DEBUG
			manifold.drawManifold(*renderOutput, transf0, transf1, capsule.radius);
#endif

			addToContactBuffer(contactBuffer, worldNormal, worldP, penDep);

			return true;
		}
		
	}

	return false;
}

bool pcmContactSphereConvex(GU_CONTACT_METHOD_ARGS)
{
	using namespace Ps::aos;

	PX_ASSERT(transform1.q.isSane());
	PX_ASSERT(transform0.q.isSane());
	
	
	const PxConvexMeshGeometryLL& shapeConvex = shape1.get<const PxConvexMeshGeometryLL>();
	const PxSphereGeometry& shapeSphere = shape0.get<const PxSphereGeometry>();

	Gu::PersistentContactManifold& manifold = cache.getManifold();

	const Vec3V zeroV = V3Zero();

	Ps::prefetchLine(shapeConvex.hullData);
	const Vec3V vScale = V3LoadU_SafeReadW(shapeConvex.scale.scale);	// PT: safe because 'rotation' follows 'scale' in PxMeshScale
	const FloatV sphereRadius = FLoad(shapeSphere.radius);
	const FloatV contactDist = FLoad(params.mContactDistance);
	const Gu::ConvexHullData* hullData = shapeConvex.hullData;
	
	//Transfer A into the local space of B
	const PsTransformV transf0 = loadTransformA(transform0);
	const PsTransformV transf1 = loadTransformA(transform1);
	const PsTransformV curRTrans(transf1.transformInv(transf0));
	const PsMatTransformV aToB(curRTrans);
	
	const PxReal toleranceLength = params.mToleranceLength;
	const FloatV convexMargin = Gu::CalculatePCMConvexMargin(hullData, vScale, toleranceLength);

	const PxU32 initialContacts = manifold.mNumContacts;
	const FloatV minMargin = FMin(convexMargin, sphereRadius);
	const FloatV projectBreakingThreshold = FMul(minMargin, FLoad(0.05f));
	
	const FloatV refreshDistance = FAdd(sphereRadius, contactDist);
	manifold.refreshContactPoints(aToB, projectBreakingThreshold, refreshDistance);
	//ML: after refreshContactPoints, we might lose some contacts
	const bool bLostContacts = (manifold.mNumContacts != initialContacts);

	GjkStatus status = manifold.mNumContacts > 0 ? GJK_UNDEFINED : GJK_NON_INTERSECT;

		
	Vec3V closestA(zeroV), closestB(zeroV);
	Vec3V normal(zeroV); // from a to b
	const FloatV zero = FZero();
	FloatV penDep = zero;

	PX_UNUSED(bLostContacts);


	if(bLostContacts || manifold.invalidate_SphereCapsule(curRTrans, minMargin))
	{

		manifold.setRelativeTransform(curRTrans);
		
		const QuatV vQuat = QuatVLoadU(&shapeConvex.scale.rotation.x);  
	
		const bool idtScale = shapeConvex.scale.isIdentity();
		//use the original shape
		ConvexHullV convexHull(hullData, zeroV, vScale, vQuat, idtScale);
		convexHull.setMargin(0.f);
		//transform capsule into the local space of convexHull
		CapsuleV capsule(aToB.p, sphereRadius);

		LocalConvex<CapsuleV> convexA(capsule);
		const Vec3V initialSearchDir = V3Sub(capsule.getCenter(), convexHull.getCenter());
		if(idtScale)
		{
			LocalConvex<ConvexHullNoScaleV> convexB(*PX_CONVEX_TO_NOSCALECONVEX(&convexHull));
			status = gjkPenetration<LocalConvex<CapsuleV>,  LocalConvex<ConvexHullNoScaleV> >(convexA, convexB, initialSearchDir, contactDist, closestA, closestB, normal, penDep, 
				manifold.mAIndice, manifold.mBIndice, manifold.mNumWarmStartPoints, true);
		}
		else
		{
			LocalConvex<ConvexHullV> convexB(convexHull);
			status = gjkPenetration<LocalConvex<CapsuleV>, LocalConvex<ConvexHullV> >(convexA, convexB, initialSearchDir, contactDist, closestA, closestB, normal, penDep,
				manifold.mAIndice, manifold.mBIndice, manifold.mNumWarmStartPoints, true);

		}

		if(status == GJK_NON_INTERSECT)
		{
			return false;
		}
		else if(status == GJK_CONTACT)
		{
			Gu::PersistentContact& p = manifold.getContactPoint(0);
			p.mLocalPointA = zeroV;//sphere center
			p.mLocalPointB = closestB;
			p.mLocalNormalPen = V4SetW(Vec4V_From_Vec3V(normal), penDep);
			manifold.mNumContacts =1;

#if	PCM_LOW_LEVEL_DEBUG
			manifold.drawManifold(*renderOutput, transf0, transf1, capsule.radius);
#endif
			
			//transform normal to world space
			const Vec3V worldNormal = transf1.rotate(normal);
			const Vec3V worldP = V3NegScaleSub(worldNormal, sphereRadius, transf0.p);
			penDep = FSub(penDep, sphereRadius);
			addToContactBuffer(contactBuffer, worldNormal, worldP, penDep);
			return true;

		}
		else if(status == GJK_DEGENERATE)
		{
			Gu::PersistentContact* manifoldContacts = PX_CP_TO_PCP(contactBuffer.contacts);
			
			return fullContactsGenerationSphereConvex(capsule, convexHull, transf0, transf1, manifoldContacts, contactBuffer, idtScale, 
				manifold, normal, contactDist, true, renderOutput);
		}
		else if(status == EPA_CONTACT)
		{
			
			if(idtScale)
			{
				LocalConvex<ConvexHullNoScaleV> convexB(*PX_CONVEX_TO_NOSCALECONVEX(&convexHull));
				
				status= Gu::epaPenetration(convexA, convexB, manifold.mAIndice, manifold.mBIndice, manifold.mNumWarmStartPoints,
						closestA, closestB, normal, penDep, true);
				
			}
			else
			{
				LocalConvex<ConvexHullV> convexB(convexHull);
			
				status= Gu::epaPenetration(convexA, convexB, manifold.mAIndice, manifold.mBIndice, manifold.mNumWarmStartPoints,
						closestA, closestB, normal, penDep, true);

			}

			if(status == EPA_CONTACT)
			{
				Gu::PersistentContact& p = manifold.getContactPoint(0);
				p.mLocalPointA = zeroV;//sphere center
				p.mLocalPointB = closestB;
				p.mLocalNormalPen = V4SetW(Vec4V_From_Vec3V(normal), penDep);
				manifold.mNumContacts =1;

#if	PCM_LOW_LEVEL_DEBUG
				manifold.drawManifold(*renderOutput, transf0, transf1, capsule.radius);
#endif
				
				//transform normal to world space
				const Vec3V worldNormal = transf1.rotate(normal);
				const Vec3V worldP = V3NegScaleSub(worldNormal, sphereRadius, transf0.p);
				penDep = FSub(penDep, sphereRadius);

				addToContactBuffer(contactBuffer, worldNormal, worldP, penDep);
				return true;
			}
			else
			{
				Gu::PersistentContact* manifoldContacts = PX_CP_TO_PCP(contactBuffer.contacts);
				return fullContactsGenerationSphereConvex(capsule, convexHull, transf0, transf1, manifoldContacts,  contactBuffer, idtScale, 
					manifold, normal, contactDist, true, renderOutput);

			}
			
		}
	}
	else if(manifold.mNumContacts > 0)
	{
		//ML:: the manifold originally has contacts
		Gu::PersistentContact& p = manifold.getContactPoint(0);
		const Vec3V worldNormal = transf1.rotate(Vec3V_From_Vec4V(p.mLocalNormalPen));
		const Vec3V worldP = V3NegScaleSub(worldNormal, sphereRadius, transf0.p);
		penDep = FSub(V4GetW(p.mLocalNormalPen), sphereRadius);

#if	PCM_LOW_LEVEL_DEBUG
		manifold.drawManifold(*renderOutput, transf0, transf1, sphereRadius);
#endif
	
		addToContactBuffer(contactBuffer, worldNormal, worldP, penDep);
		return true;
	}

	return false;

}  
}//Gu
}//phyxs

