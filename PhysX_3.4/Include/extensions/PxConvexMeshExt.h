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


#ifndef PX_PHYSICS_EXTENSIONS_CONVEX_MESH_H
#define PX_PHYSICS_EXTENSIONS_CONVEX_MESH_H
/** \addtogroup extensions
  @{
*/

#include "PxPhysXConfig.h"
#include "common/PxPhysXCommonConfig.h"

#if !PX_DOXYGEN
namespace physx
{
#endif

	class PxConvexMeshGeometry;

	/**
	\brief Computes closest polygon of the convex hull geometry for a given impact point
	and impact direction. When doing sweeps against a scene, one might want to delay
	the rather expensive computation of the hit face index for convexes until it is clear
	the information is really needed and then use this method to get the corresponding
	face index.
	
	\param[in] convexGeom The convex mesh geometry.
	\param[in] geomPose Pose for the geometry object.
	\param[in] impactPos Impact position.
	\param[in] unitDir Normalized impact direction.

	\return Closest face index of the convex geometry.

	@see PxTransform PxConvexMeshGeometry
	*/
	PxU32 PxFindFaceIndex(const PxConvexMeshGeometry& convexGeom, 
								   const PxTransform& geomPose,
								   const PxVec3& impactPos, 
								   const PxVec3& unitDir);

#if !PX_DOXYGEN
} // namespace physx
#endif

/** @} */
#endif
