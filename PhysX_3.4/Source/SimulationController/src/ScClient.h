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


#ifndef PX_PHYSICS_CLIENT
#define PX_PHYSICS_CLIENT

#include "PxScene.h"
#include "CmPhysXCommon.h"
#include "PsUserAllocated.h"

namespace physx
{
namespace Sc
{

	class Client : public Ps::UserAllocated
	{
	public:
		Client() :
			activeTransforms		(PX_DEBUG_EXP("clientActiveTransforms")),
			activeActors			(PX_DEBUG_EXP("clientActiveActors")),
			posePreviewBodies		(PX_DEBUG_EXP("clientPosePreviewBodies")),
			posePreviewBuffer		(PX_DEBUG_EXP("clientPosePreviewBuffer")),
			behaviorFlags			(0),
			simulationEventCallback	(NULL),
			broadPhaseCallback		(NULL)
		{}

		Ps::Array<PxActiveTransform>	activeTransforms;
		Ps::Array<PxActor*>				activeActors;
		Ps::Array<const PxRigidBody*>	posePreviewBodies;	// buffer for bodies that requested early report of the integrated pose (eENABLE_POSE_INTEGRATION_PREVIEW).
															// This buffer gets exposed to users. Is officially accessible from PxSimulationEventCallback::onAdvance()
															// until the next simulate()/advance().
		Ps::Array<PxTransform>			posePreviewBuffer;	// buffer of newly integrated poses for the bodies that requested a preview. This buffer gets exposed
															// to users.
		PxClientBehaviorFlags			behaviorFlags;// Tracks behavior bits for clients.
		// User callbacks
		PxSimulationEventCallback*		simulationEventCallback;
		PxBroadPhaseCallback*			broadPhaseCallback;
	};

} // namespace Sc

}

#endif
