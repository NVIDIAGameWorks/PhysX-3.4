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


#ifndef PX_FOUNDATION_NXVOLUMEINTEGRATION
#define PX_FOUNDATION_NXVOLUMEINTEGRATION
/** \addtogroup foundation
  @{
*/


#include "foundation/Px.h"
#include "foundation/PxVec3.h"
#include "foundation/PxMat33.h"
#include "CmPhysXCommon.h"

namespace physx
{

class PxSimpleTriangleMesh;
class PxConvexMeshDesc;

/**
\brief Data structure used to store mass properties.
*/
struct PxIntegrals
	{
	PxVec3 COM;					//!< Center of mass
	PxF64 mass;						//!< Total mass
	PxF64 inertiaTensor[3][3];		//!< Inertia tensor (mass matrix) relative to the origin
	PxF64 COMInertiaTensor[3][3];	//!< Inertia tensor (mass matrix) relative to the COM

	/**
	\brief Retrieve the inertia tensor relative to the center of mass.

	\param inertia Inertia tensor.
	*/
	void getInertia(PxMat33& inertia)
	{
		for(PxU32 j=0;j<3;j++)
		{
			for(PxU32 i=0;i<3;i++)
			{
				inertia(i,j) = PxF32(COMInertiaTensor[i][j]);
			}
		}
	}

	/**
	\brief Retrieve the inertia tensor relative to the origin.

	\param inertia Inertia tensor.
	*/
	void getOriginInertia(PxMat33& inertia)
	{
		for(PxU32 j=0;j<3;j++)
		{
			for(PxU32 i=0;i<3;i++)
			{
				inertia(i,j) = PxF32(inertiaTensor[i][j]);
			}
		}
	}
	};

	bool computeVolumeIntegrals(const PxSimpleTriangleMesh& mesh, PxReal density, PxIntegrals& integrals);

	// specialized method taking polygons directly, so we don't need to compute and store triangles for each polygon
	bool computeVolumeIntegralsEberly(const PxConvexMeshDesc& mesh, PxReal density, PxIntegrals& integrals, const PxVec3& origin);   // Eberly simplified method

	// specialized method taking polygons directly, so we don't need to compute and store triangles for each polygon, SIMD version
	bool computeVolumeIntegralsEberlySIMD(const PxConvexMeshDesc& mesh, PxReal density, PxIntegrals& integrals, const PxVec3& origin);   // Eberly simplified method
}

 /** @} */
#endif
