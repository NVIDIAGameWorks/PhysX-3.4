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

#include "SampleVehicle_CameraController.h"
#include "PhysXSampleApplication.h"
#include "PxRigidDynamic.h"
#include "PxQueryFiltering.h"
#include "PxScene.h"
#include "PxSceneLock.h"
#include "vehicle/PxVehicleWheels.h"

using namespace SampleRenderer;
using namespace SampleFramework;

///////////////////////////////////////////////////////////////////////////////



SampleVehicle_CameraController::SampleVehicle_CameraController()
:	mRotateInputY				(0.0f),
	mRotateInputZ				(0.0f),
	mMaxCameraRotateSpeed		(5.0f),
	mCameraRotateAngleY			(0.0f),
	mCameraRotateAngleZ			(0.33f),
	mCameraPos					(PxVec3(0,0,0)),
	mCameraTargetPos			(PxVec3(0,0,0)),
	mLastCarPos					(PxVec3(0,0,0)),
	mLastCarVelocity			(PxVec3(0,0,0)),
	mCameraInit					(false),
	mLockOnFocusVehTransform	(true),
	mLastFocusVehTransform		(PxTransform(PxIdentity))
{
}

SampleVehicle_CameraController::~SampleVehicle_CameraController()
{
}

static void dampVec3(const PxVec3& oldPosition, PxVec3& newPosition, PxF32 timestep)
{
	PxF32 t = 0.7f * timestep * 8.0f;
	t = PxMin(t, 1.0f);
	newPosition = oldPosition * (1 - t) + newPosition * t;
}

void SampleVehicle_CameraController::update(const PxReal dtime, const PxRigidDynamic* actor, PxScene& scene)
{
	PxSceneReadLock scopedLock(scene);
	PxTransform carChassisTransfm;
	if(mLockOnFocusVehTransform)
	{
		carChassisTransfm = actor->getGlobalPose();
		mLastFocusVehTransform = carChassisTransfm;
	}
	else
	{
		carChassisTransfm = mLastFocusVehTransform;
	}

	PxF32 camDist = 15.0f;
	PxF32 cameraYRotExtra = 0.0f;

	PxVec3 velocity = mLastCarPos-carChassisTransfm.p;

	if (mCameraInit)
	{
		//Work out the forward and sideways directions.
		PxVec3 unitZ(0,0,1);
		PxVec3 carDirection = carChassisTransfm.q.rotate(unitZ);
		PxVec3 unitX(1,0,0);
		PxVec3 carSideDirection = carChassisTransfm.q.rotate(unitX);

		//Acceleration (note that is not scaled by time).
		PxVec3 acclVec = mLastCarVelocity-velocity;

		//Higher forward accelerations allow the car to speed away from the camera.
		PxF32 acclZ = carDirection.dot(acclVec);
		camDist = PxMax(camDist+acclZ*400.0f, 5.0f);

		//Higher sideways accelerations allow the car's rotation to speed away from the camera's rotation.
		PxF32 acclX = carSideDirection.dot(acclVec);
		cameraYRotExtra = -acclX*10.0f;
		//At very small sideways speeds the camera greatly amplifies any numeric error in the body and leads to a slight jitter.
		//Scale cameraYRotExtra by a value in range (0,1) for side speeds in range (0.1,1.0) and by zero for side speeds less than 0.1.
		PxFixedSizeLookupTable<4> table;
		table.addPair(0.0f, 0.0f);
		table.addPair(0.1f*dtime, 0);
		table.addPair(1.0f*dtime, 1);
		PxF32 velX = carSideDirection.dot(velocity);
		cameraYRotExtra *= table.getYVal(PxAbs(velX));
	}

	mCameraRotateAngleY+=mRotateInputY*mMaxCameraRotateSpeed*dtime;
	mCameraRotateAngleY=physx::intrinsics::fsel(mCameraRotateAngleY-10*PxPi, mCameraRotateAngleY-10*PxPi, physx::intrinsics::fsel(-mCameraRotateAngleY-10*PxPi, mCameraRotateAngleY + 10*PxPi, mCameraRotateAngleY));
	mCameraRotateAngleZ+=mRotateInputZ*mMaxCameraRotateSpeed*dtime;
	mCameraRotateAngleZ=PxClamp(mCameraRotateAngleZ,-PxPi*0.05f,PxPi*0.45f);

	PxVec3 cameraDir=PxVec3(0,0,1)*PxCos(mCameraRotateAngleY+cameraYRotExtra) + PxVec3(1,0,0)*PxSin(mCameraRotateAngleY+cameraYRotExtra);

	cameraDir=cameraDir*PxCos(mCameraRotateAngleZ)-PxVec3(0,1,0)*PxSin(mCameraRotateAngleZ);

	const PxVec3 direction = carChassisTransfm.q.rotate(cameraDir);
	PxVec3 target = carChassisTransfm.p;
	target.y+=0.5f;

	PxRaycastBuffer hit;
	PxQueryFilterData filterData(PxQueryFlag::eSTATIC);
	scene.raycast(target, -direction, camDist, hit, PxHitFlag::eDISTANCE, filterData);
	if (hit.hasBlock && hit.block.shape != NULL)
	{
		camDist = hit.block.distance-0.25f;
	}

	camDist = PxMax(5.0f, PxMin(camDist, 50.0f));

	PxVec3 position = target-direction*camDist;

	if (mCameraInit)
	{
		dampVec3(mCameraPos, position, dtime);
		dampVec3(mCameraTargetPos, target, dtime);
	}

	mCameraPos = position;
	mCameraTargetPos = target;
	mCameraInit = true;

	mLastCarVelocity = velocity;
	mLastCarPos = carChassisTransfm.p;
}

void SampleVehicle_CameraController::update(const PxReal dtime, const PxVehicleWheels& focusVehicle, PxScene& scene)
{
	const PxRigidDynamic* actor=focusVehicle.getRigidDynamicActor();
	update(dtime,actor,scene);
}


