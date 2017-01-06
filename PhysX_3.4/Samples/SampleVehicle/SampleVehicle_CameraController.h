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


#ifndef SAMPLE_VEHICLE_CAMERA_CONTROLLER_H
#define SAMPLE_VEHICLE_CAMERA_CONTROLLER_H

#include "common/PxPhysXCommonConfig.h"
#include "foundation/PxVec3.h"
#include "foundation/PxTransform.h"

using namespace physx;

namespace physx
{
	class PxScene;
	class PxVehicleWheels;
	class PxRigidDynamic;
}

class Camera;

class SampleVehicle_CameraController 
{
public:

	SampleVehicle_CameraController();
	~SampleVehicle_CameraController();

	void setInputs(const PxF32 rotateInputY, const PxF32 rotateInputZ)
	{
		mRotateInputY=rotateInputY;
		mRotateInputZ=rotateInputZ;
	}

	void update(const PxF32 dtime, const PxVehicleWheels& focusVehicle, PxScene& scene);

	void restart() {}

	bool getIsLockedOnVehicleTransform() const {return mLockOnFocusVehTransform;}
	void toggleLockOnVehTransform() {mLockOnFocusVehTransform = !mLockOnFocusVehTransform;}
	
	const PxVec3& getCameraPos() const {return mCameraPos;}
	const PxVec3& getCameraTar() const {return mCameraTargetPos;}

private:

	PxF32			mRotateInputY;
	PxF32			mRotateInputZ;

	PxF32			mMaxCameraRotateSpeed;
	PxF32			mCameraRotateAngleY;
	PxF32			mCameraRotateAngleZ;
	PxVec3			mCameraPos;
	PxVec3			mCameraTargetPos;
	PxVec3			mLastCarPos;
	PxVec3			mLastCarVelocity;
	bool			mCameraInit;

	bool			mLockOnFocusVehTransform;
	PxTransform		mLastFocusVehTransform;

	void update(const PxReal dtime, const PxRigidDynamic* actor, PxScene& scene);
};

#endif //SAMPLE_VEHICLE_CAMERA_CONTROLLER_H