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
#include "GuVecConvexHull.h"
#include "GuVecShrunkConvexHull.h"
#include "GuVecShrunkConvexHullNoScale.h"
#include "GuVecConvexHullNoScale.h"
#include "GuGeometryUnion.h"   

#include "GuContactMethodImpl.h"
#include "GuPCMShapeConvex.h"
#include "GuPCMContactGen.h"
#include "GuContactBuffer.h"


namespace physx
{


	using namespace Ps::aos;

namespace Gu
{

static bool fullContactsGenerationConvexConvex(const ConvexHullV& convexHull0, Gu::ConvexHullV& convexHull1, const PsTransformV& transf0, const PsTransformV& transf1, 
											   const bool idtScale0, const bool idtScale1, PersistentContact* manifoldContacts,  ContactBuffer& contactBuffer, 
											   PersistentContactManifold& manifold, Vec3VArg normal, const Vec3VArg closestA, const Vec3VArg closestB,
											   const FloatVArg contactDist, const bool doOverlapTest, Cm::RenderOutput* renderOutput, const PxReal toleranceScale)
{
	Gu::PolygonalData polyData0, polyData1;
	getPCMConvexData(convexHull0, idtScale0, polyData0);
	getPCMConvexData(convexHull1, idtScale1, polyData1);

	PxU8 buff0[sizeof(SupportLocalImpl<ConvexHullV>)];
	PxU8 buff1[sizeof(SupportLocalImpl<ConvexHullV>)];

	SupportLocal* map0 = (idtScale0 ? static_cast<SupportLocal*>(PX_PLACEMENT_NEW(buff0, SupportLocalImpl<ConvexHullNoScaleV>)(static_cast<const ConvexHullNoScaleV&>(convexHull0), transf0, convexHull0.vertex2Shape, convexHull0.shape2Vertex, idtScale0)) : 
		static_cast<SupportLocal*>(PX_PLACEMENT_NEW(buff0, SupportLocalImpl<ConvexHullV>)(convexHull0, transf0, convexHull0.vertex2Shape, convexHull0.shape2Vertex, idtScale0)));

	SupportLocal* map1 = (idtScale1 ? static_cast<SupportLocal*>(PX_PLACEMENT_NEW(buff1, SupportLocalImpl<ConvexHullNoScaleV>)(static_cast<const ConvexHullNoScaleV&>(convexHull1), transf1, convexHull1.vertex2Shape, convexHull1.shape2Vertex, idtScale1)) : 
		static_cast<SupportLocal*>(PX_PLACEMENT_NEW(buff1, SupportLocalImpl<ConvexHullV>)(convexHull1, transf1, convexHull1.vertex2Shape, convexHull1.shape2Vertex, idtScale1)));


	PxU32 numContacts = 0;

	if(generateFullContactManifold(polyData0, polyData1, map0, map1, manifoldContacts, numContacts, contactDist, normal, closestA, closestB, convexHull0.getMarginF(), 
		convexHull1.getMarginF(), doOverlapTest, renderOutput, toleranceScale))
	{
		
		if (numContacts > 0)
		{
			//reduce contacts
			manifold.addBatchManifoldContacts(manifoldContacts, numContacts, toleranceScale);

			const Vec3V worldNormal = manifold.getWorldNormal(transf1);

			//add the manifold contacts;
			manifold.addManifoldContactsToContactBuffer(contactBuffer, worldNormal, transf1, contactDist);


#if	PCM_LOW_LEVEL_DEBUG
			manifold.drawManifold(*renderOutput, transf0, transf1);
#endif
		}
		else
		{
			//if doOverlapTest is true, which means GJK/EPA degenerate so we won't have any contact in the manifoldContacts array
			if (!doOverlapTest)
			{
				const Vec3V worldNormal = manifold.getWorldNormal(transf1);

				manifold.addManifoldContactsToContactBuffer(contactBuffer, worldNormal, transf1, contactDist);
			}

		}
		return true;
		
	}

	return false;

}

static GjkStatus convexHullNoScale0(const ShrunkConvexHullV& convexHull0, const ShrunkConvexHullV& convexHull1, const bool idtScale1, const PsMatTransformV& aToB, const FloatVArg contactDist,
									 Vec3V& closestA, Vec3V& closestB, Vec3V& normal, FloatV& penDep, PersistentContactManifold& manifold)
{
	const RelativeConvex<ShrunkConvexHullNoScaleV> convexA(static_cast<const ShrunkConvexHullNoScaleV&>(convexHull0), aToB);
	if(idtScale1)
	{
		const LocalConvex<ShrunkConvexHullNoScaleV> convexB(static_cast<const ShrunkConvexHullNoScaleV&>(convexHull1));
		return gjkPenetration<RelativeConvex<ShrunkConvexHullNoScaleV>, LocalConvex<ShrunkConvexHullNoScaleV> >(convexA, convexB, aToB.p, contactDist, closestA, closestB, normal, penDep,
						manifold.mAIndice, manifold.mBIndice, manifold.mNumWarmStartPoints, false);
	}
	else
	{
		const LocalConvex<ShrunkConvexHullV> convexB(convexHull1);
		return gjkPenetration<RelativeConvex<ShrunkConvexHullNoScaleV>, LocalConvex<ShrunkConvexHullV> >(convexA, convexB, aToB.p, contactDist, closestA, closestB, normal, penDep,
					manifold.mAIndice, manifold.mBIndice, manifold.mNumWarmStartPoints,false);
	}
}

static GjkStatus convexHullHasScale0(ShrunkConvexHullV& convexHull0, ShrunkConvexHullV& convexHull1, const bool idtScale1, const PsMatTransformV& aToB, 
									   const FloatVArg contactDist, Vec3V& closestA, Vec3V& closestB, Vec3V& normal, FloatV& penDep, PersistentContactManifold& manifold)
{
	RelativeConvex<ShrunkConvexHullV> convexA(convexHull0, aToB);
	if(idtScale1)
	{
		LocalConvex<ShrunkConvexHullNoScaleV> convexB(static_cast<ShrunkConvexHullNoScaleV&>(convexHull1));
		return gjkPenetration< RelativeConvex<ShrunkConvexHullV>, LocalConvex<ShrunkConvexHullNoScaleV> >(convexA, convexB, aToB.p, contactDist, closestA, closestB, normal, penDep,
						manifold.mAIndice, manifold.mBIndice, manifold.mNumWarmStartPoints,false);
	}
	else
	{
		LocalConvex<ShrunkConvexHullV> convexB(convexHull1);
		return gjkPenetration<RelativeConvex<ShrunkConvexHullV>, LocalConvex<ShrunkConvexHullV> >(convexA, convexB, aToB.p, contactDist, closestA, closestB, normal, penDep,
					manifold.mAIndice, manifold.mBIndice, manifold.mNumWarmStartPoints, false);
	}
}


static bool addGJKEPAContacts(Gu::ShrunkConvexHullV& convexHull0, Gu::ShrunkConvexHullV& convexHull1, const PsMatTransformV& aToB, 
	GjkStatus& status, Gu::PersistentContact* manifoldContacts, const FloatV replaceBreakingThreshold, Vec3V& closestA, Vec3V& closestB, Vec3V& normal, FloatV& penDep, 
	Gu::PersistentContactManifold& manifold)
{
	bool doOverlapTest = false;
	if (status == GJK_CONTACT)
	{
		const Vec3V localPointA = aToB.transformInv(closestA);//curRTrans.transformInv(closestA);
		const Vec4V localNormalPen = V4SetW(Vec4V_From_Vec3V(normal), penDep);

		//Add contact to contact stream
		manifoldContacts[0].mLocalPointA = localPointA;
		manifoldContacts[0].mLocalPointB = closestB;
		manifoldContacts[0].mLocalNormalPen = localNormalPen;

		//Add contact to manifold
		manifold.addManifoldPoint(localPointA, closestB, localNormalPen, replaceBreakingThreshold);
	}
	else
	{
		PX_ASSERT(status == EPA_CONTACT);

		RelativeConvex<ConvexHullV> convexA1(convexHull0, aToB);
		LocalConvex<ConvexHullV> convexB1(convexHull1);

		status = epaPenetration(convexA1, convexB1, manifold.mAIndice, manifold.mBIndice, manifold.mNumWarmStartPoints,
			closestA, closestB, normal, penDep);

		if (status == EPA_CONTACT)
		{
			const Vec3V localPointA = aToB.transformInv(closestA);//curRTrans.transformInv(closestA);
			const Vec4V localNormalPen = V4SetW(Vec4V_From_Vec3V(normal), penDep);

			//Add contact to contact stream
			manifoldContacts[0].mLocalPointA = localPointA;
			manifoldContacts[0].mLocalPointB = closestB;
			manifoldContacts[0].mLocalNormalPen = localNormalPen;

			//Add contact to manifold
			manifold.addManifoldPoint(localPointA, closestB, localNormalPen, replaceBreakingThreshold);

		}
		else
		{
			doOverlapTest = true;
		}
	}

	return doOverlapTest;
}

bool pcmContactConvexConvex(GU_CONTACT_METHOD_ARGS)
{
	const PxConvexMeshGeometryLL& shapeConvex0 = shape0.get<const PxConvexMeshGeometryLL>();
	const PxConvexMeshGeometryLL& shapeConvex1 = shape1.get<const PxConvexMeshGeometryLL>();
	PersistentContactManifold& manifold = cache.getManifold();

	Ps::prefetchLine(shapeConvex0.hullData);
	Ps::prefetchLine(shapeConvex1.hullData);

	PX_ASSERT(transform1.q.isSane());
	PX_ASSERT(transform0.q.isSane());

	const Vec3V vScale0 = V3LoadU_SafeReadW(shapeConvex0.scale.scale);	// PT: safe because 'rotation' follows 'scale' in PxMeshScale
	const Vec3V vScale1 = V3LoadU_SafeReadW(shapeConvex1.scale.scale);	// PT: safe because 'rotation' follows 'scale' in PxMeshScale
	const FloatV contactDist = FLoad(params.mContactDistance);

	//Transfer A into the local space of B
	const PsTransformV transf0 = loadTransformA(transform0);
	const PsTransformV transf1 = loadTransformA(transform1);
	const PsTransformV curRTrans(transf1.transformInv(transf0));
	const PsMatTransformV aToB(curRTrans);
	
	const Gu::ConvexHullData* hullData0 = shapeConvex0.hullData;
	const Gu::ConvexHullData* hullData1 = shapeConvex1.hullData;

	const PxReal toleranceLength = params.mToleranceLength;
	const FloatV convexMargin0 = Gu::CalculatePCMConvexMargin(hullData0, vScale0, toleranceLength);
	const FloatV convexMargin1 = Gu::CalculatePCMConvexMargin(hullData1, vScale1, toleranceLength);
	
	const PxU32 initialContacts = manifold.mNumContacts;

	const FloatV minMargin = FMin(convexMargin0, convexMargin1);
	const FloatV projectBreakingThreshold = FMul(minMargin, FLoad(0.8f));
	
	manifold.refreshContactPoints(aToB, projectBreakingThreshold, contactDist);

	//ML: after refreshContactPoints, we might lose some contacts
	const bool bLostContacts = (manifold.mNumContacts != initialContacts);

	if(bLostContacts || manifold.invalidate_BoxConvex(curRTrans, minMargin) )
	{

		GjkStatus status = manifold.mNumContacts > 0 ? GJK_UNDEFINED : GJK_NON_INTERSECT;

		const bool idtScale0 = shapeConvex0.scale.isIdentity();
		const bool idtScale1 = shapeConvex1.scale.isIdentity();
		const float maxMargin0 = shapeConvex0.maxMargin;
		const float maxMargin1 = shapeConvex1.maxMargin;
		const QuatV vQuat0 = QuatVLoadU(&shapeConvex0.scale.rotation.x);
		const QuatV vQuat1 = QuatVLoadU(&shapeConvex1.scale.rotation.x);
		const Vec3V zeroV = V3Zero();


		Gu::ShrunkConvexHullV convexHull0(hullData0, V3LoadU(hullData0->mCenterOfMass), vScale0, vQuat0, idtScale0);
		Gu::ShrunkConvexHullV convexHull1(hullData1, V3LoadU(hullData1->mCenterOfMass), vScale1, vQuat1, idtScale1);

		convexHull0.setMaxMargin(maxMargin0);
		convexHull1.setMaxMargin(maxMargin1);

		Vec3V closestA(zeroV), closestB(zeroV), normal(zeroV); // from a to b
		FloatV penDep =  FZero();

		if(idtScale0)
		{
			status = convexHullNoScale0(convexHull0, convexHull1, idtScale1, aToB, contactDist, closestA, closestB, normal, penDep, manifold);
		}
		else
		{
			status = convexHullHasScale0(convexHull0, convexHull1, idtScale1, aToB, contactDist, closestA, closestB, normal, penDep, manifold);
		}

		manifold.setRelativeTransform(curRTrans);

		Gu::PersistentContact* manifoldContacts = PX_CP_TO_PCP(contactBuffer.contacts);
		
		if(status == GJK_DEGENERATE)
		{
			return fullContactsGenerationConvexConvex(convexHull0, convexHull1, transf0, transf1, idtScale0, idtScale1, manifoldContacts, contactBuffer, 
				manifold, normal, closestA, closestB, contactDist, true, renderOutput, params.mToleranceLength);
		}
		else if(status == GJK_NON_INTERSECT)
		{
			return false;
		}
		else
		{
			const FloatV replaceBreakingThreshold = FMul(minMargin, FLoad(0.05f));

			const Vec3V localNor = manifold.mNumContacts ? manifold.getLocalNormal() : V3Zero();
			
			const bool doOverlapTest = addGJKEPAContacts(convexHull0, convexHull1, aToB,
				status, manifoldContacts, replaceBreakingThreshold, closestA, closestB, normal, penDep, manifold);
			
			//manifold.drawManifold(*renderOutput, transf0, transf1);
			//ML: after we refresh the contacts(newContacts) and generate a GJK/EPA contacts(we will store that in the manifold), if the number of contacts is still less than the original contacts,
			//which means we lose too mang contacts and we should regenerate all the contacts in the current configuration
			//Also, we need to look at the existing contacts, if the existing contacts has very different normal than the GJK/EPA contacts,
			//which means we should throw away the existing contacts and do full contact gen
			const bool fullContactGen = FAllGrtr(FLoad(0.707106781f), V3Dot(localNor, normal)) || (manifold.mNumContacts < initialContacts);
			if (fullContactGen  || doOverlapTest)
			{
				return fullContactsGenerationConvexConvex(convexHull0, convexHull1, transf0, transf1, idtScale0, idtScale1, manifoldContacts, contactBuffer, 
					manifold, normal, closestA, closestB, contactDist, doOverlapTest, renderOutput, params.mToleranceLength);
			}
			else
			{
				const Vec3V newLocalNor = V3Add(localNor, normal);
				const Vec3V worldNormal = V3Normalize(transf1.rotate(newLocalNor));
				
				//const Vec3V worldNormal = manifold.getWorldNormal(transf1);
				manifold.addManifoldContactsToContactBuffer(contactBuffer, worldNormal, transf1, contactDist);
				return true;
			}
			
		}
	}
	else if(manifold.getNumContacts()> 0)
	{
		const Vec3V worldNormal =  manifold.getWorldNormal(transf1);
		manifold.addManifoldContactsToContactBuffer(contactBuffer, worldNormal, transf1, contactDist);
#if	PCM_LOW_LEVEL_DEBUG
		manifold.drawManifold(*renderOutput, transf0, transf1);
#endif
		return true;
	}

	return false;
	
}
}
}
