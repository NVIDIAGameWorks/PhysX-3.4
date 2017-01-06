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

#include "foundation/PxTransform.h"
#include "SqBounds.h"
#include "CmTransformUtils.h"
#include "SqPruner.h"
#include "ScbShape.h"
#include "ScbActor.h"
#include "ScbRigidStatic.h"
#include "ScbBody.h"
#include "PsAllocator.h"
#include "GuBounds.h"

using namespace physx;
using namespace Sq;

void Sq::computeStaticWorldAABB(PxBounds3& bounds, const Scb::Shape& scbShape, const Scb::Actor& scbActor)
{
	const PxTransform& shape2Actor = scbShape.getShape2Actor();

	PX_ALIGN(16, PxTransform) globalPose;

	Cm::getStaticGlobalPoseAligned(static_cast<const Scb::RigidStatic&>(scbActor).getActor2World(), shape2Actor, globalPose);
	Gu::computeBounds(bounds, scbShape.getGeometry(), globalPose, 0.0f, NULL, SQ_PRUNER_INFLATION, false);
}

void Sq::computeDynamicWorldAABB(PxBounds3& bounds, const Scb::Shape& scbShape, const Scb::Actor& scbActor)
{
	const PxTransform& shape2Actor = scbShape.getShape2Actor();

	PX_ALIGN(16, PxTransform) globalPose;
	{
		const Scb::Body& body = static_cast<const Scb::Body&>(scbActor);
		PX_ALIGN(16, PxTransform) kinematicTarget;
		const PxU16 sqktFlags = PxRigidBodyFlag::eKINEMATIC | PxRigidBodyFlag::eUSE_KINEMATIC_TARGET_FOR_SCENE_QUERIES;
		const bool useTarget = (PxU16(body.getFlags()) & sqktFlags) == sqktFlags;
		const PxTransform& body2World = (useTarget && body.getKinematicTarget(kinematicTarget)) ? kinematicTarget : body.getBody2World();
		Cm::getDynamicGlobalPoseAligned(body2World, shape2Actor, body.getBody2Actor(), globalPose);
	}

	Gu::computeBounds(bounds, scbShape.getGeometry(), globalPose, 0.0f, NULL, SQ_PRUNER_INFLATION, false);
}

const ComputeBoundsFunc Sq::gComputeBoundsTable[2] = 
{ 
	computeStaticWorldAABB, 
	computeDynamicWorldAABB 
};
