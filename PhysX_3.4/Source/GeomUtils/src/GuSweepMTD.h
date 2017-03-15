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

#ifndef GU_SWEEP_MTD_H
#define GU_SWEEP_MTD_H

namespace physx
{
	class PxConvexMeshGeometry;
	class PxTriangleMeshGeometry;
	class PxGeometry;
	class PxHeightFieldGeometry;

namespace Gu
{
	class Sphere;
	class Capsule;

	bool computeCapsule_TriangleMeshMTD(const PxTriangleMeshGeometry& triMeshGeom, const PxTransform& pose, Gu::CapsuleV& capsuleV, PxReal inflatedRadius, bool isDoubleSided, PxSweepHit& hit);

	bool computeCapsule_HeightFieldMTD(const PxHeightFieldGeometry& heightFieldGeom, const PxTransform& pose, Gu::CapsuleV& capsuleV, PxReal inflatedRadius, bool isDoubleSided, const PxU32 flags, PxSweepHit& hit);

	bool computeBox_TriangleMeshMTD(const PxTriangleMeshGeometry& triMeshGeom, const PxTransform& pose, const Gu::Box& box, const PxTransform& boxTransform, PxReal inflation,
									bool isDoubleSided,  PxSweepHit& hit);    

	bool computeBox_HeightFieldMTD(	const PxHeightFieldGeometry& heightFieldGeom, const PxTransform& pose, const Gu::Box& box, const PxTransform& boxTransform, PxReal inflation, 
									bool isDoubleSided, const PxU32 flags, PxSweepHit& hit);

	bool computeConvex_TriangleMeshMTD(	const PxTriangleMeshGeometry& triMeshGeom, const PxTransform& pose, const PxConvexMeshGeometry& convexGeom, const PxTransform& convexTransform, PxReal inflation,
										bool isDoubleSided, PxSweepHit& hit);

	bool computeConvex_HeightFieldMTD(	const PxHeightFieldGeometry& heightFieldGeom, const PxTransform& pose, const PxConvexMeshGeometry& convexGeom, const PxTransform& convexTransform, PxReal inflation,
										bool isDoubleSided, const PxU32 flags, PxSweepHit& hit);

	bool computeSphere_SphereMTD(const Sphere& sphere0, const Sphere& sphere1, PxSweepHit& hit);
	bool computeSphere_CapsuleMTD(const Sphere& sphere, const Capsule& capsule, PxSweepHit& hit);

	bool computeCapsule_CapsuleMTD(const Capsule& capsule0, const Capsule& capsule1, PxSweepHit& hit);

	bool computePlane_CapsuleMTD(const PxPlane& plane, const Capsule& capsule, PxSweepHit& hit);
	bool computePlane_BoxMTD(const PxPlane& plane, const Box& box, PxSweepHit& hit);
	bool computePlane_ConvexMTD(const PxPlane& plane, const PxConvexMeshGeometry& convexGeom, const PxTransform& convexPose, PxSweepHit& hit);

	// PT: wrapper just to avoid duplicating these lines.
	PX_FORCE_INLINE void setupSweepHitForMTD(PxSweepHit& sweepHit, bool hasContacts, const PxVec3& unitDir)
	{
		sweepHit.flags = PxHitFlag::eDISTANCE | PxHitFlag::eNORMAL | PxHitFlag::eFACE_INDEX;
		if(!hasContacts)
		{
			sweepHit.distance	= 0.0f;
			sweepHit.normal		= -unitDir;
		}
		else
		{
			//ML: touching contact. We need to overwrite the normal to the negative of sweep direction
			if (sweepHit.distance == 0.0f)
			{
				sweepHit.normal = -unitDir;
			}
			sweepHit.flags |= PxHitFlag::ePOSITION;
			
		}
	}
}

}


#endif
