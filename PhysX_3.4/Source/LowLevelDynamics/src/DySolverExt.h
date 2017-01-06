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


#ifndef DY_SOLVEREXTBODY_H
#define DY_SOLVEREXTBODY_H

#include "foundation/PxVec3.h"
#include "foundation/PxTransform.h"
#include "CmPhysXCommon.h"
#include "CmSpatialVector.h"

namespace physx
{

class PxsRigidBody;
struct PxsBodyCore;
struct PxSolverBody;
struct PxSolverBodyData;


namespace Dy
{


struct FsData;
struct SolverConstraint1D;

class SolverExtBody
{
public:
	union
	{
		const FsData* mFsData;
		const PxSolverBody* mBody;
	};
	const PxSolverBodyData* mBodyData;

	PxU16 mLinkIndex;

	SolverExtBody(const void* bodyOrArticulation, const void* bodyData, PxU16 linkIndex): 
	  mBody(reinterpret_cast<const PxSolverBody*>(bodyOrArticulation)),
	  mBodyData(reinterpret_cast<const PxSolverBodyData*>(bodyData)),
		  mLinkIndex(linkIndex)
	  {}

	  void getResponse(const PxVec3& linImpulse, const PxVec3& angImpulse,
					   PxVec3& linDeltaV, PxVec3& angDeltaV, PxReal dominance) const;

	  PxReal projectVelocity(const PxVec3& linear, const PxVec3& angular) const;
	  PxVec3 getLinVel() const;
	  PxVec3 getAngVel() const;
};

}

}

#endif //DY_SOLVEREXTBODY_H
