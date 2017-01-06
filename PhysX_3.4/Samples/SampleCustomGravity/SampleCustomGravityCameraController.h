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

#include "PhysXSampleApplication.h"
#include "SampleCameraController.h"
#include "characterkinematic/PxController.h"

#define POS_HISTORY_LIMIT	1024

class SampleCustomGravityCameraController : public CameraController
{
	public:
										SampleCustomGravityCameraController(PxController& controlled, SampleCustomGravity& base);

		virtual void					onDigitalInputEvent(const SampleFramework::InputEvent& , bool val);
		virtual void					onAnalogInputEvent(const SampleFramework::InputEvent& , float val);
		virtual void					onPointerInputEvent(const SampleFramework::InputEvent&, physx::PxU32, physx::PxU32, physx::PxReal dx, physx::PxReal dy, bool val);
		virtual void					collectInputEvents(std::vector<const SampleFramework::InputEvent*>& inputEvents);

		virtual	void					update(Camera& camera, PxReal dtime);

				void					setView(PxReal pitch, PxReal yaw);

//	private:
				PxController&			mCCT;
				SampleCustomGravity&	mBase;

				PxVec3					mTarget;

PxMat33	mTest;
				PxVec3					mForward, mRightV;
				PxU32					mNbRecords;
				PxVec3					mHistory[POS_HISTORY_LIMIT];

				PxReal					mTargetYaw, mTargetPitch;
				PxReal					mPitchMin,	mPitchMax;

				PxReal					mGamepadPitchInc, mGamepadYawInc;
				PxReal					mGamepadForwardInc, mGamepadLateralInc;
				PxReal					mSensibility;

				bool					mFwd,mBwd,mLeft,mRight,mKeyShiftDown;

				PxReal					mRunningSpeed;
				PxReal					mWalkingSpeed;
	private:
				SampleCustomGravityCameraController& operator=(const SampleCustomGravityCameraController&);

};
