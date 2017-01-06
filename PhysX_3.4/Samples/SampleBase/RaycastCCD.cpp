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

#include "RaycastCCD.h"

#include "geometry/PxBoxGeometry.h"
#include "geometry/PxSphereGeometry.h"
#include "geometry/PxCapsuleGeometry.h"
#include "geometry/PxConvexMeshGeometry.h"
#include "geometry/PxConvexMesh.h"
#include "PxQueryReport.h"
#include "PxScene.h"
#include "PxRigidDynamic.h"
#include "extensions/PxShapeExt.h"
#include <stdio.h>

//#define RAYCAST_CCD_PRINT_DEBUG

PxVec3 getShapeCenter(PxRigidActor* actor, PxShape* shape, const PxVec3& localOffset)
{
	PxVec3 offset = localOffset;

	switch(shape->getGeometryType())
	{
		case PxGeometryType::eCONVEXMESH:
		{
			PxConvexMeshGeometry geometry;
			bool status = shape->getConvexMeshGeometry(geometry);
			PX_UNUSED(status);
			PX_ASSERT(status);

			PxReal mass;
			PxMat33 localInertia;
			PxVec3 localCenterOfMass;
			geometry.convexMesh->getMassInformation(mass, localInertia, localCenterOfMass);

			offset += localCenterOfMass;
		}
		break;
		default:
		break;
	}
	const PxTransform pose = PxShapeExt::getGlobalPose(*shape, *actor);
	return pose.transform(offset);
}

static PxReal computeInternalRadius(PxRigidActor* actor, PxShape* shape, const PxVec3& dir, /*PxVec3& offset,*/ const PxVec3& centerOffset)
{
	const PxBounds3 bounds = PxShapeExt::getWorldBounds(*shape, *actor);
	const PxReal diagonal = (bounds.maximum - bounds.minimum).magnitude();
	const PxReal offsetFromOrigin = diagonal * 2.0f;

	PxTransform pose = PxShapeExt::getGlobalPose(*shape, *actor);

	PxReal internalRadius = 0.0f;
	const PxReal length = offsetFromOrigin*2.0f;

	// PT: we do a switch here anyway since the code is not *exactly* the same all the time
	{
		switch(shape->getGeometryType())
		{
			case PxGeometryType::eBOX:
			case PxGeometryType::eCAPSULE:
			{
				pose.p = PxVec3(0);
				const PxVec3 virtualOrigin = pose.p + dir * offsetFromOrigin;

				PxRaycastHit hit;

				const PxHitFlags sceneQueryFlags = PxHitFlag::ePOSITION|PxHitFlag::eNORMAL|PxHitFlag::eDISTANCE;
				PxU32 nbHits = PxGeometryQuery::raycast(virtualOrigin, -dir, shape->getGeometry().any(), pose, length, sceneQueryFlags, 1, &hit);
				PX_UNUSED(nbHits);
				PX_ASSERT(nbHits);

				internalRadius = offsetFromOrigin - hit.distance;
	//			offset = PxVec3(0.0f);
			}
			break;

			case PxGeometryType::eSPHERE:
			{
				PxSphereGeometry geometry;
				bool status = shape->getSphereGeometry(geometry);
				PX_UNUSED(status);
				PX_ASSERT(status);

				internalRadius = geometry.radius;
	//			offset = PxVec3(0.0f);
			}
			break;

			case PxGeometryType::eCONVEXMESH:
			{
	/*PxVec3 saved = pose.p;
				pose.p = PxVec3(0);

	//			pose.p = geometry.convexMesh->getCenterOfMass();
	//			const PxVec3 virtualOrigin = pose.p + dir * offsetFromOrigin;

	//			const PxVec3 localCenter = computeCenter(geometry.convexMesh);
	//			const PxVec3 localCenter = geometry.convexMesh->getCenterOfMass();
	//			const PxVec3 virtualOrigin = pose.rotate(localCenter) + dir * offsetFromOrigin;
				const PxVec3 localCenter = pose.rotate(geometry.convexMesh->getCenterOfMass());
				const PxVec3 virtualOrigin = localCenter + dir * offsetFromOrigin;

				PxRaycastHit hit;
				PxU32 nbHits = Gu::raycast_convexMesh(geometry, pose, virtualOrigin, -dir, length, 0xffffffff, 1, &hit);
				PX_ASSERT(nbHits);
				internalRadius = offsetFromOrigin - hit.distance;

				pose.p = localCenter;

	PxVec3 shapeCenter = getShapeCenter(shape);
	offset = shapeCenter - saved;*/


				PxVec3 shapeCenter = getShapeCenter(actor, shape, centerOffset);
				shapeCenter -= pose.p;
				pose.p = PxVec3(0);

				const PxVec3 virtualOrigin = shapeCenter + dir * offsetFromOrigin;
				PxRaycastHit hit;
				const PxHitFlags sceneQueryFlags = PxHitFlag::ePOSITION|PxHitFlag::eNORMAL|PxHitFlag::eDISTANCE;
				PxU32 nbHits = PxGeometryQuery::raycast(virtualOrigin, -dir, shape->getGeometry().any(), pose, length, sceneQueryFlags, 1, &hit);
				PX_UNUSED(nbHits);
				PX_ASSERT(nbHits);

				internalRadius = offsetFromOrigin - hit.distance;
	//			offset = shapeCenter;
			}
			break;
			default:
			break;
		}
	}
	return internalRadius;
}

static bool CCDRaycast(PxScene* scene, const PxVec3& origin, const PxVec3& unitDir, const PxReal distance, PxRaycastHit& hit)
{
	const PxHitFlags outputFlags = PxHitFlag::ePOSITION|PxHitFlag::eNORMAL|PxHitFlag::eDISTANCE;

	PxQueryFilterData filterData;
	filterData.flags = PxQueryFlag::eSTATIC;

	PxQueryFilterCallback* filterCall = NULL;

	PxRaycastBuffer buf1;
	scene->raycast(origin, unitDir, distance, buf1, outputFlags, filterData, filterCall, NULL);
	hit = buf1.block;
	return buf1.hasBlock;
}

static PxRigidDynamic* canDoCCD(PxRigidActor& actor, PxShape* shape)
{
	PxRigidDynamic* dyna = actor.is<PxRigidDynamic>();
	if(!dyna)
		return NULL;	// PT: no need to do it for statics

	const PxU32 nbShapes = dyna->getNbShapes();
	if(nbShapes!=1)
		return NULL;	// PT: only works with simple actors for now

	if(dyna->getRigidBodyFlags() & PxRigidBodyFlag::eKINEMATIC)
		return NULL;	// PT: no need to do it for kinematics

	return dyna;
}

bool doRaycastCCD(PxRigidActor* actor, PxShape* shape, PxTransform& newPose, PxVec3& newShapeCenter, const PxVec3& ccdWitness, const PxVec3& ccdWitnessOffset)
{
	PxRigidDynamic* dyna = canDoCCD(*actor, shape);
	if(!dyna)
		return true;

	bool updateCCDWitness = true;

	const PxVec3 offset = newPose.p - newShapeCenter;
	const PxVec3& origin = ccdWitness;
	const PxVec3& dest = newShapeCenter;

	PxVec3 dir = dest - origin;
	const PxReal length = dir.magnitude();
	if(length!=0.0f)
	{
		dir /= length;

		// Compute internal radius
		const PxReal internalRadius = computeInternalRadius(actor, shape, dir, ccdWitnessOffset);

		// Compute distance to impact
		PxRaycastHit hit;
		if(internalRadius!=0.0f && CCDRaycast(actor->getScene(), origin, dir, length, hit))
		{
#ifdef RAYCAST_CCD_PRINT_DEBUG
			static int count=0;
			shdfnd::printFormatted("CCD hit %d\n", count++);
#endif
			updateCCDWitness = false;
			const PxReal radiusLimit = internalRadius * 0.75f;
			if(hit.distance>radiusLimit)
			{
				newShapeCenter = origin + dir * (hit.distance - radiusLimit);
#ifdef RAYCAST_CCD_PRINT_DEBUG
				shdfnd::printFormatted("  Path0: %f | %f\n", hit.distance, radiusLimit);
#endif
			}
			else
			{
				newShapeCenter = origin;
#ifdef RAYCAST_CCD_PRINT_DEBUG
				shdfnd::printFormatted("  Path1: %f\n", hit.distance);
#endif
			}

			{
				newPose.p = offset + newShapeCenter;
				const PxTransform shapeLocalPose = shape->getLocalPose();
				const PxTransform inverseShapeLocalPose = shapeLocalPose.getInverse();
				PxTransform newGlobalPose = newPose * inverseShapeLocalPose;
				dyna->setGlobalPose(newGlobalPose);
			}
		}
	}
	return updateCCDWitness;
}
