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

#include "SampleCustomGravity.h"
#include "SampleCustomGravityCameraController.h"
#include "SampleCCTJump.h"
#include "PxRigidDynamic.h"
#include "geometry/PxCapsuleGeometry.h"
#include "PxShape.h"
#include <SampleBaseInputEventIds.h>
#include <SampleUserInputIds.h>
#include <SampleUserInputDefines.h>
#include <SamplePlatform.h>

using namespace SampleRenderer;
using namespace SampleFramework;

/*static*/ PxF32 gJumpForce = 30.0f;

static Jump gJump;

SampleCustomGravityCameraController::SampleCustomGravityCameraController(PxController& controlled, SampleCustomGravity& base) :
	mCCT						(controlled),
	mBase						(base),
	mTargetYaw					(0.0f-PxPi/2),
	mTargetPitch				(0.0f),
	mPitchMin					(-PxHalfPi),
	mPitchMax					(PxHalfPi),
	mGamepadPitchInc			(0.0f),
	mGamepadYawInc				(0.0f),
	mGamepadForwardInc			(0.0f),
	mGamepadLateralInc			(0.0f),
	mSensibility				(0.001f),
	mFwd						(false),
	mBwd						(false),
	mLeft						(false),
	mRight						(false),
	mKeyShiftDown				(false),
	mRunningSpeed				(25.0f),
	mWalkingSpeed				(2.5f)
{
	mForward = PxVec3(0);
	mRightV = PxVec3(0);
	mTarget = PxVec3(0);

	mNbRecords = 0;

	mTest = PxMat33(PxIdentity);
}

void SampleCustomGravityCameraController::collectInputEvents(std::vector<const SampleFramework::InputEvent*>& inputEvents)
{
	//digital keyboard events
	DIGITAL_INPUT_EVENT_DEF(CAMERA_MOVE_FORWARD,											SCAN_CODE_FORWARD,		XKEY_W,					X1KEY_W,				PS3KEY_W,				PS4KEY_W,				AKEY_UNKNOWN,			SCAN_CODE_FORWARD,		IKEY_UNKNOWN,			SCAN_CODE_FORWARD,		WIIUKEY_UNKNOWN);
	DIGITAL_INPUT_EVENT_DEF(CAMERA_MOVE_BACKWARD,											SCAN_CODE_BACKWARD,		XKEY_S,					X1KEY_S,				PS3KEY_S,				PS4KEY_S,				AKEY_UNKNOWN,			SCAN_CODE_BACKWARD,		IKEY_UNKNOWN,			SCAN_CODE_BACKWARD,		WIIUKEY_UNKNOWN);
	DIGITAL_INPUT_EVENT_DEF(CAMERA_MOVE_LEFT,												SCAN_CODE_LEFT,			XKEY_A,					X1KEY_A,				PS3KEY_A,				PS4KEY_A,				AKEY_UNKNOWN,			SCAN_CODE_LEFT,			IKEY_UNKNOWN,			SCAN_CODE_LEFT,			WIIUKEY_UNKNOWN);
	DIGITAL_INPUT_EVENT_DEF(CAMERA_MOVE_RIGHT,												SCAN_CODE_RIGHT,		XKEY_D,					X1KEY_D,				PS3KEY_D,				PS4KEY_D,				AKEY_UNKNOWN,			SCAN_CODE_RIGHT,		IKEY_UNKNOWN,			SCAN_CODE_RIGHT,		WIIUKEY_UNKNOWN);
	DIGITAL_INPUT_EVENT_DEF(CAMERA_SHIFT_SPEED,												SCAN_CODE_LEFT_SHIFT,	XKEY_SHIFT,				X1KEY_SHIFT,			PS3KEY_SHIFT,			PS4KEY_SHIFT,			AKEY_UNKNOWN,			OSXKEY_SHIFT,			IKEY_UNKNOWN,			LINUXKEY_SHIFT,			WIIUKEY_UNKNOWN);
	DIGITAL_INPUT_EVENT_DEF(CAMERA_JUMP,													SCAN_CODE_SPACE,		XKEY_SPACE,				X1KEY_SPACE,			PS3KEY_SPACE,			PS4KEY_SPACE,			AKEY_UNKNOWN,			OSXKEY_SPACE,			IKEY_UNKNOWN,			LINUXKEY_SPACE,			WIIUKEY_UNKNOWN);

	//analog mouse events
	ANALOG_INPUT_EVENT_DEF(CAMERA_GAMEPAD_ROTATE_LEFT_RIGHT, GAMEPAD_ROTATE_SENSITIVITY,	GAMEPAD_RIGHT_STICK_X,	GAMEPAD_RIGHT_STICK_X,	GAMEPAD_RIGHT_STICK_X,	GAMEPAD_RIGHT_STICK_X,	GAMEPAD_RIGHT_STICK_X,	GAMEPAD_RIGHT_STICK_X,	GAMEPAD_RIGHT_STICK_X,	GAMEPAD_RIGHT_STICK_X,	LINUXKEY_UNKNOWN,		GAMEPAD_RIGHT_STICK_X);
	ANALOG_INPUT_EVENT_DEF(CAMERA_GAMEPAD_ROTATE_UP_DOWN, GAMEPAD_ROTATE_SENSITIVITY,		GAMEPAD_RIGHT_STICK_Y,	GAMEPAD_RIGHT_STICK_Y,	GAMEPAD_RIGHT_STICK_Y,	GAMEPAD_RIGHT_STICK_Y,	GAMEPAD_RIGHT_STICK_Y,	GAMEPAD_RIGHT_STICK_Y,	GAMEPAD_RIGHT_STICK_Y,	GAMEPAD_RIGHT_STICK_Y,	LINUXKEY_UNKNOWN,		GAMEPAD_RIGHT_STICK_Y);
	ANALOG_INPUT_EVENT_DEF(CAMERA_GAMEPAD_MOVE_LEFT_RIGHT, GAMEPAD_DEFAULT_SENSITIVITY,		GAMEPAD_LEFT_STICK_X,	GAMEPAD_LEFT_STICK_X,	GAMEPAD_LEFT_STICK_X,	GAMEPAD_LEFT_STICK_X,	GAMEPAD_LEFT_STICK_X,	GAMEPAD_LEFT_STICK_X,	GAMEPAD_LEFT_STICK_X,	GAMEPAD_LEFT_STICK_X,	LINUXKEY_UNKNOWN,		GAMEPAD_LEFT_STICK_X);
	ANALOG_INPUT_EVENT_DEF(CAMERA_GAMEPAD_MOVE_FORWARD_BACK, GAMEPAD_DEFAULT_SENSITIVITY,	GAMEPAD_LEFT_STICK_Y,	GAMEPAD_LEFT_STICK_Y,	GAMEPAD_LEFT_STICK_Y,	GAMEPAD_LEFT_STICK_Y,	GAMEPAD_LEFT_STICK_Y,	GAMEPAD_LEFT_STICK_Y,	GAMEPAD_LEFT_STICK_Y,	GAMEPAD_LEFT_STICK_Y,	LINUXKEY_UNKNOWN,		GAMEPAD_LEFT_STICK_Y);

	//digital gamepad events
	DIGITAL_INPUT_EVENT_DEF(CAMERA_JUMP,    												GAMEPAD_SOUTH,			GAMEPAD_SOUTH,			GAMEPAD_SOUTH, 			GAMEPAD_SOUTH,			GAMEPAD_SOUTH,			AKEY_UNKNOWN,			GAMEPAD_SOUTH,			IKEY_UNKNOWN,			LINUXKEY_UNKNOWN,	GAMEPAD_SOUTH);

    //touch events
    TOUCH_INPUT_EVENT_DEF(CAMERA_JUMP,	"Jump",	AQUICK_BUTTON_1,	IQUICK_BUTTON_1);

}

void SampleCustomGravityCameraController::onDigitalInputEvent(const SampleFramework::InputEvent& ie, bool val)
{
	if(val)
	{
		if(ie.m_Id == CAMERA_MOVE_FORWARD)	mFwd = true;
		else	if(ie.m_Id == CAMERA_MOVE_BACKWARD)	mBwd = true;
		else	if(ie.m_Id == CAMERA_MOVE_LEFT)	mLeft = true;
		else	if(ie.m_Id == CAMERA_MOVE_RIGHT)	mRight = true;
		else	if(ie.m_Id == CAMERA_SHIFT_SPEED)	mKeyShiftDown = true;
		else	if(ie.m_Id == CAMERA_JUMP)	gJump.startJump(gJumpForce);
	}
	else
	{
		if(ie.m_Id == CAMERA_MOVE_FORWARD)	mFwd = false;
		else	if(ie.m_Id == CAMERA_MOVE_BACKWARD)	mBwd = false;
		else	if(ie.m_Id == CAMERA_MOVE_LEFT)	mLeft = false;
		else	if(ie.m_Id == CAMERA_MOVE_RIGHT)	mRight = false;
		else	if(ie.m_Id == CAMERA_SHIFT_SPEED)	mKeyShiftDown = false;
	}
}

static PX_FORCE_INLINE PxReal remapAxisValue(PxReal absolutePosition)
{
	return absolutePosition * absolutePosition * absolutePosition * 5.0f;
}

void SampleCustomGravityCameraController::onAnalogInputEvent(const SampleFramework::InputEvent& ie, float val)
{
	if(ie.m_Id == CAMERA_GAMEPAD_ROTATE_LEFT_RIGHT)
	{
		mGamepadYawInc = - remapAxisValue(val);
	}
	else if(ie.m_Id == CAMERA_GAMEPAD_ROTATE_UP_DOWN)
	{
		// PT: ideally we'd need an option to "invert Y axis" here
		//		mGamepadPitchInc = - remapAxisValue(val);
		mGamepadPitchInc = remapAxisValue(val);
	}
	else if(ie.m_Id == CAMERA_GAMEPAD_MOVE_LEFT_RIGHT)
	{
		mGamepadLateralInc = val;
	}
	else if(ie.m_Id == CAMERA_GAMEPAD_MOVE_FORWARD_BACK)
	{
		mGamepadForwardInc = val;
	}
}

void SampleCustomGravityCameraController::onPointerInputEvent(const SampleFramework::InputEvent &ie, physx::PxU32, physx::PxU32, physx::PxReal dx, physx::PxReal dy, bool val)
{
	if (ie.m_Id == CAMERA_MOUSE_LOOK)
	{
		mTargetYaw		-= dx * mSensibility;
		mTargetPitch	+= dy * mSensibility;
	}
}

void SampleCustomGravityCameraController::setView(PxReal pitch, PxReal yaw)
{
	mTargetPitch = pitch;
	mTargetYaw   = yaw;
}

static PxQuat rotationArc(const PxVec3& v0, const PxVec3& v1, bool& res)
{
	PxVec3 _v0 = v0;
	PxVec3 _v1 = v1;
	_v0.normalize();
	_v1.normalize();

	float s = sqrtf((1.0f + (v0.dot(v1))) * 2.0f);
	if(s<0.001f)
	{
		res = false;
		return PxQuat(PxIdentity);
	}

	PxVec3 p = (_v0.cross(_v1)) / s;
	float w = s * 0.5f;
	PxQuat q(p.x, p.y, p.z, w);
	q.normalize();

	res = true;
	return q;
}

void SampleCustomGravityCameraController::update(Camera& camera, PxReal dtime)
{
	PxSceneReadLock scopedLock(mBase.getActiveScene());

	const PxExtendedVec3& currentPos = mCCT.getPosition();
	const PxVec3 curPos = toVec3(currentPos);

	// Compute up vector for current CCT position
	PxVec3 upVector;
	mBase.mPlanet.getUpVector(upVector, curPos);
	PX_ASSERT(upVector.isFinite());

	// Update CCT
	if(!mBase.isPaused())
	{
		if(1)
		{
			bool recordPos = true;
			const PxU32 currentSize = mNbRecords;
			if(currentSize)
			{
				const PxVec3 lastPos = mHistory[currentSize-1];
//				const float limit = 0.1f;
				const float limit = 0.5f;
				if((curPos - lastPos).magnitude()<limit)
					recordPos = false;
			}
			if(recordPos)
			{
				if(mNbRecords==POS_HISTORY_LIMIT)
				{
					for(PxU32 i=1;i<mNbRecords;i++)
						mHistory[i-1] = mHistory[i];
					mNbRecords--;
				}
				mHistory[mNbRecords++] = curPos;
			}
		}

		// Subtract off the 'up' component of the view direction to get our forward motion vector.
		PxVec3 viewDir = camera.getViewDir();
		PxVec3 forward = (viewDir - upVector * upVector.dot(viewDir)).getNormalized();
	
//		PxVec3 forward = mForward;

		// Compute "right" vector
		PxVec3 right = forward.cross(upVector);
		right.normalize();
//		PxVec3 right = mRightV;

		PxVec3 targetKeyDisplacement(0);
		if(mFwd)	targetKeyDisplacement += forward;
		if(mBwd)	targetKeyDisplacement -= forward;

		if(mRight)	targetKeyDisplacement += right;
		if(mLeft)	targetKeyDisplacement -= right;

		targetKeyDisplacement *= mKeyShiftDown ? mRunningSpeed : mWalkingSpeed;
//		targetKeyDisplacement += PxVec3(0,-9.81,0);
		targetKeyDisplacement *= dtime;

		PxVec3 targetPadDisplacement(0);
		targetPadDisplacement += forward * mGamepadForwardInc * mRunningSpeed;
		targetPadDisplacement += right * mGamepadLateralInc * mRunningSpeed;
//		targetPadDisplacement += PxVec3(0,-9.81,0);
		targetPadDisplacement *= dtime;


		const PxF32 heightDelta = gJump.getHeight(dtime);
//		shdfnd::printFormatted("%f\n", heightDelta);
		PxVec3 upDisp = upVector;
		if(heightDelta!=0.0f)
			upDisp *= heightDelta;
		else
			upDisp *= -9.81f * dtime;
		const PxVec3 disp = targetKeyDisplacement + targetPadDisplacement + upDisp;

//upDisp.normalize();
//shdfnd::printFormatted("%f | %f | %f\n", upDisp.x, upDisp.y, upDisp.z);
//		shdfnd::printFormatted("%f | %f | %f\n", targetKeyDisplacement.x, targetKeyDisplacement.y, targetKeyDisplacement.z);
//		shdfnd::printFormatted("%f | %f | %f\n\n", targetPadDisplacement.x, targetPadDisplacement.y, targetPadDisplacement.z);

		mCCT.setUpDirection(upVector);
//		const PxU32 flags = mCCT.move(disp, 0.001f, dtime, PxControllerFilters());
		const PxU32 flags = mCCT.move(disp, 0.0f, dtime, PxControllerFilters());
		if(flags & PxControllerCollisionFlag::eCOLLISION_DOWN)
		{
			gJump.stopJump();
//			shdfnd::printFormatted("Stop jump\n");
		}
	}

	// Update camera
	if(1)
	{
		mTargetYaw		+= mGamepadYawInc * dtime;
		mTargetPitch	+= mGamepadPitchInc * dtime;

		// Clamp pitch
//		if(mTargetPitch<mPitchMin)	mTargetPitch = mPitchMin;
//		if(mTargetPitch>mPitchMax)	mTargetPitch = mPitchMax;
	}

	if(1)
	{
		PxVec3 up = upVector;

		PxQuat localPitchQ(mTargetPitch, PxVec3(1.0f, 0.0f, 0.0f));
		PX_ASSERT(localPitchQ.isSane());
		PxMat33 localPitchM(localPitchQ);

		const PxVec3 upRef(0.0f, 1.0f, 0.0f);

		PxQuat localYawQ(mTargetYaw, upRef);
		PX_ASSERT(localYawQ.isSane());
		PxMat33 localYawM(localYawQ);

			bool res;
			PxQuat localToWorldQ = rotationArc(upRef, up, res);
			static PxQuat memory(0,0,0,1);
			if(!res)
			{
				localToWorldQ = memory;
			}
			else
			{
				memory = localToWorldQ;
			}
			PX_ASSERT(localToWorldQ.isSane());
			PxMat33 localToWorld(localToWorldQ);
		

			static PxVec3 previousUp(0.0f, 1.0f, 0.0f);
			static PxQuat incLocalToWorldQ(0.0f, 0.0f, 0.0f, 1.0f);
			PxQuat incQ = rotationArc(previousUp, up, res);
			PX_ASSERT(incQ.isSane());
//			incLocalToWorldQ = incLocalToWorldQ * incQ;
			incLocalToWorldQ = incQ * incLocalToWorldQ;
			PX_ASSERT(incLocalToWorldQ.isSane());
			incLocalToWorldQ.normalize();
			PxMat33 incLocalToWorldM(incLocalToWorldQ);
			localToWorld = incLocalToWorldM;
			previousUp = up;

mTest = localToWorld;
//mTest = localToWorld * localYawM;

//		PxMat33 rot = localYawM * localToWorld;
		PxMat33 rot = localToWorld * localYawM * localPitchM;
//		PxMat33 rot = localToWorld * localYawM;
		PX_ASSERT(rot.column0.isFinite());
		PX_ASSERT(rot.column1.isFinite());
		PX_ASSERT(rot.column2.isFinite());

		////

		PxMat44 view(rot.column0, rot.column1, rot.column2, PxVec3(0));

		mForward = -rot.column2;
		mRightV = rot.column0;

		camera.setView(PxTransform(view));
		PX_ASSERT(camera.getViewDir().isFinite());

		////

		PxRigidActor* characterActor = mCCT.getActor();
		
		PxShape* shape;
		characterActor->getShapes(&shape,1);

		PxCapsuleGeometry geom;
		shape->getCapsuleGeometry(geom);

		up *= geom.halfHeight+geom.radius;

		const PxVec3 headPos = curPos + up;
		const float distanceToTarget = 10.0f;
//		const float distanceToTarget = 20.0f;
//		const float distanceToTarget = 5.0f;
//		const PxVec3 camPos = headPos - viewDir*distanceToTarget;
		const PxVec3 camPos = headPos - mForward*distanceToTarget;// + up * 20.0f;
//		view.t = camPos;
		view.column3 = PxVec4(camPos,0);
//		camera.setView(view);
		camera.setView(PxTransform(view));
		mTarget = headPos;
	}

	if(0)
	{
		PxControllerState cctState;
		mCCT.getState(cctState);
		shdfnd::printFormatted("\nCCT state:\n");
		shdfnd::printFormatted("delta:             %.02f | %.02f | %.02f\n", cctState.deltaXP.x, cctState.deltaXP.y, cctState.deltaXP.z);
		shdfnd::printFormatted("touchedShape:      %p\n", cctState.touchedShape);
		shdfnd::printFormatted("touchedObstacleHandle:   %d\n", cctState.touchedObstacleHandle);
		shdfnd::printFormatted("standOnAnotherCCT: %d\n", cctState.standOnAnotherCCT);
		shdfnd::printFormatted("standOnObstacle:   %d\n", cctState.standOnObstacle);
		shdfnd::printFormatted("isMovingUp:        %d\n", cctState.isMovingUp);
		shdfnd::printFormatted("collisionFlags:    %d\n", cctState.collisionFlags);
	}

}

