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

#ifndef GU_PCM_CONTACT_GEN_H
#define GU_PCM_CONTACT_GEN_H


#include "GuConvexSupportTable.h"
#include "GuPersistentContactManifold.h"
#include "GuShapeConvex.h"
#include "GuSeparatingAxes.h"

namespace physx
{

namespace Gu
{


	//full contact gen code for box/convexhull vs convexhull
	bool generateFullContactManifold(Gu::PolygonalData& polyData0, Gu::PolygonalData& polyData1, Gu::SupportLocal* map0, Gu::SupportLocal* map1, Gu::PersistentContact* manifoldContacts, 
		PxU32& numContacts, const Ps::aos::FloatVArg contactDist, const Ps::aos::Vec3VArg normal, const Ps::aos::Vec3VArg closestA, const Ps::aos::Vec3VArg closestB, 
		const PxReal toleranceA, const PxReal toleranceB, bool doOverlapTest, Cm::RenderOutput* renderOutput, const PxReal toleranceScale);

	//full contact gen code for capsule vs convexhulll
	bool generateFullContactManifold(const Gu::CapsuleV& capsule, Gu::PolygonalData& polyData, Gu::SupportLocal* map, const Ps::aos::PsMatTransformV& aToB,  Gu::PersistentContact* manifoldContacts, 
		PxU32& numContacts, const Ps::aos::FloatVArg contactDist, Ps::aos::Vec3V& normal, const Ps::aos::Vec3VArg closest, const PxReal tolerance, bool doOverlapTest, const PxReal toleranceScale);

	//full contact gen code for capsule vs box
	bool generateCapsuleBoxFullContactManifold(const Gu::CapsuleV& capsule, Gu::PolygonalData& polyData, Gu::SupportLocal* map, const Ps::aos::PsMatTransformV& aToB, Gu::PersistentContact* manifoldContacts, PxU32& numContacts,
		const Ps::aos::FloatVArg contactDist, Ps::aos::Vec3V& normal, const Ps::aos::Vec3VArg closest, const PxReal boxMargin, const bool doOverlapTest, const PxReal toeranceScale);

	//MTD code for box/convexhull vs box/convexhull
	bool computeMTD(Gu::PolygonalData& polyData0, Gu::PolygonalData& polyData1,  SupportLocal* map0, SupportLocal* map1, Ps::aos::FloatV& penDepth, Ps::aos::Vec3V& normal);
	
	//MTD code for capsule vs box/convexhull
	bool computeMTD(const Gu::CapsuleV& capsule, Gu::PolygonalData& polyData, Gu::SupportLocal* map, Ps::aos::FloatV& penDepth, Ps::aos::Vec3V& normal);

	void buildPartialHull(const Gu::PolygonalData& polyData, SupportLocal* map, Gu::SeparatingAxes& validAxes, const Ps::aos::Vec3VArg v, const Ps::aos::Vec3VArg _dir);

	//full contact gen code for sphere vs convexhull
	bool generateSphereFullContactManifold(const Gu::CapsuleV& capsule, Gu::PolygonalData& polyData, Gu::SupportLocal* map, Gu::PersistentContact* manifoldContacts, PxU32& numContacts,
		const Ps::aos::FloatVArg contactDist, Ps::aos::Vec3V& normal, const bool doOverlapTest);

}  
}

#endif
