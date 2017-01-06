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

#ifndef GU_CONTACTPOLYGONPOLYGON_H
#define GU_CONTACTPOLYGONPOLYGON_H

#include "foundation/PxVec3.h"
#include "CmPhysXCommon.h"
#include "PxPhysXCommonConfig.h"

namespace physx
{

namespace Cm
{
	class Matrix34;
	class FastVertex2ShapeScaling;
}

namespace Gu
{
class ContactBuffer;

PX_PHYSX_COMMON_API PxMat33 findRotationMatrixFromZ(const PxVec3& to);

PX_PHYSX_COMMON_API bool contactPolygonPolygonExt(	PxU32 numVerts0, const PxVec3* vertices0, const PxU8* indices0,//polygon 0
													const Cm::Matrix34& world0, const PxPlane& localPlane0,			//xform of polygon 0, plane of polygon
													const PxMat33& RotT0,

													PxU32 numVerts1, const PxVec3* vertices1, const PxU8* indices1,//polygon 1
													const Cm::Matrix34& world1, const PxPlane& localPlane1,			//xform of polygon 1, plane of polygon
													const PxMat33& RotT1,

													const PxVec3& worldSepAxis,									//world normal of separating plane - this is the world space normal of polygon0!!
													const Cm::Matrix34& transform0to1, const Cm::Matrix34& transform1to0,//transforms between polygons
													PxU32 polyIndex0, PxU32 polyIndex1,	//face indices for contact callback,
													ContactBuffer& contactBuffer,
													bool flipNormal, const PxVec3& posShift, float sepShift
													);	// shape order, post gen shift.
}
}

#endif
