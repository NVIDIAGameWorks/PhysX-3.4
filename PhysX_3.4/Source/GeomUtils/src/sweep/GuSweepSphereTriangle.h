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

#ifndef GU_SWEEP_SPHERE_TRIANGLE_H
#define GU_SWEEP_SPHERE_TRIANGLE_H

#include "GuSweepTriangleUtils.h"

namespace physx
{

namespace Gu
{
	/**
	Sweeps a sphere against a triangle.

	All input parameters (sphere, triangle, sweep direction) must be in the same space. Sweep length is assumed to be infinite.

	By default, 'testInitialOverlap' must be set to true to properly handle the case where the sphere already overlaps the triangle
	at the start of the sweep. In such a case, returned impact distance is exactly 0.0f. If it is known ahead of time that the sphere
	cannot overlap the triangle at t=0.0, then 'testInitialOverlap' can be set to false to skip the initial overlap test and make the
	function run faster.

	If the ray defined by the sphere's center and the unit direction directly intersects the triangle-related part of the TSS (*) (i.e.
	the prism from the Minkowski sum of the inflated triangle) then 'directHit' is set to true. Otherwise it is set to false.

	(*) For Triangle Swept Sphere, see http://gamma.cs.unc.edu/SSV/ssv.pdf for the origin of these names.

	\param triVerts				[in] triangle vertices
	\param triUnitNormal		[in] triangle's normalized normal
	\param sphereCenter			[in] sphere's center
	\param sphereRadius			[in] sphere's radius
	\param unitDir				[in] normalized sweep direction.
	\param impactDistance		[out] impact distance, if a hit has been found. Does not need to be initialized before calling the function.
	\param directHit			[out] true if a direct hit has been found, see comments above.
	\param testInitialOverlap	[in] true if an initial sphere-vs-triangle overlap test must be performed, see comments above.

	\return	true if an impact has been found (in which case returned result values are valid)
	*/
	bool sweepSphereVSTri(	const PxVec3* PX_RESTRICT triVerts, const PxVec3& triUnitNormal,// Triangle data	
							const PxVec3& sphereCenter, PxReal sphereRadius,				// Sphere data	
							const PxVec3& unitDir,											// Ray data	
							PxReal& impactDistance, bool& directHit,						// Results
							bool testInitialOverlap);										// Query modifier

	/**
	Sweeps a sphere against a quad.

	All input parameters (sphere, quad, sweep direction) must be in the same space. Sweep length is assumed to be infinite.

	Quad must be formed by 2 tris like this:

	p0___p2
	|   /|
	|  / |
	| /  |
	|/   |
	p1---p3

	\param quadVerts			[in] quad vertices
	\param quadUnitNormal		[in] quad's normalized normal
	\param sphereCenter			[in] sphere's center
	\param sphereRadius			[in] sphere's radius
	\param unitDir				[in] normalized sweep direction.
	\param impactDistance		[out] impact distance, if a hit has been found. Does not need to be initialized before calling the function.

	\return	true if an impact has been found (in which case returned result values are valid)
	*/
	bool sweepSphereVSQuad(	const PxVec3* PX_RESTRICT quadVerts, const PxVec3& quadUnitNormal,	// Quad data
							const PxVec3& sphereCenter, float sphereRadius,						// Sphere data
							const PxVec3& unitDir,												// Ray data
							float& impactDistance);												// Results


	// PT: computes proper impact data for sphere-sweep-vs-tri, after the closest tri has been found
	PX_FORCE_INLINE bool computeSphereTriangleImpactData(PxSweepHit& h, PxVec3& triNormalOut, PxU32 index, PxReal curT, 
														 const PxVec3& center, const PxVec3& unitDir, const PxVec3& bestTriNormal,
														 const PxTriangle* PX_RESTRICT triangles,
														 bool isDoubleSided, bool meshBothSides)
	{
		if(index==PX_INVALID_U32)
			return false;	// We didn't touch any triangle

		// Compute impact data only once, using best triangle
		PxVec3 hitPos, normal;
		computeSphereTriImpactData(hitPos, normal, center, unitDir, curT, triangles[index]);

		// PT: by design, returned normal is opposed to the sweep direction.
		if(shouldFlipNormal(normal, meshBothSides, isDoubleSided, bestTriNormal, unitDir))
			normal = -normal;

		h.position	= hitPos;
		h.normal	= normal;
		h.distance	= curT;
		h.faceIndex	= index;
		h.flags		= PxHitFlag::eDISTANCE|PxHitFlag::eNORMAL|PxHitFlag::ePOSITION;
		triNormalOut = bestTriNormal;
		return true;
	}

	/**
	Sweeps a sphere against a set of triangles.

	\param nbTris				[in] number of triangles in input array
	\param triangles			[in] array of input triangles
	\param center				[in] sphere's center
	\param radius				[in] sphere's radius
	\param unitDir				[in] sweep's unit direcion
	\param distance				[in] sweep's length
	\param cachedIndex			[in] cached triangle index, or NULL. Cached triangle will be tested first.
	\param hit					[out] results
	\param triNormalOut			[out] triangle normal
	\param isDoubleSided		[in] true if input triangles are double-sided
	\param meshBothSides		[in] true if PxHitFlag::eMESH_BOTH_SIDES is used
	\param anyHit				[in] true if PxHitFlag::eMESH_ANY is used
	\param testInitialOverlap	[in] true if PxHitFlag::eASSUME_NO_INITIAL_OVERLAP is not used
	\return	true if an impact has been found
	*/
	bool sweepSphereTriangles(	PxU32 nbTris, const PxTriangle* PX_RESTRICT triangles,							// Triangle data
								const PxVec3& center, const PxReal radius,										// Sphere data
								const PxVec3& unitDir, PxReal distance,											// Ray data
								const PxU32* PX_RESTRICT cachedIndex,											// Cache data
								PxSweepHit& hit, PxVec3& triNormalOut,											// Results
								bool isDoubleSided, bool meshBothSides, bool anyHit, bool testInitialOverlap);	// Query modifiers

} // namespace Gu

}

#endif
