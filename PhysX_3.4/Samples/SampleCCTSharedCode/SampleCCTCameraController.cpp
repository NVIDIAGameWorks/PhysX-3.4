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

#include "PhysXSample.h"
#include "SampleCCTCameraController.h"
#include "SampleCCTActor.h"
#include "SampleCCTJump.h"
#include "SampleConsole.h"
#include <SampleFrameworkInputEventIds.h>
#include <SamplePlatform.h>
#include <SampleUserInput.h>
#include <SampleUserInputIds.h>
#include <SampleUserInputDefines.h>

using namespace SampleRenderer;
using namespace SampleFramework;

// PT: TODO: move this inside the CCT?
static const bool gTransferPlatformMomentum = true;

static bool gDumpCCTState = false;
static bool gDumpCCTStats = false;

static bool canJump(ControlledActor* actor)
{
	PxControllerState cctState;
	actor->getController()->getState(cctState);
	return (cctState.collisionFlags & PxControllerCollisionFlag::eCOLLISION_DOWN)!=0;
}

static void gConsole_DumpCCTState(Console* console, const char* text, void* userData)
{
	gDumpCCTState = !gDumpCCTState;
}

static void gConsole_DumpCCTStats(Console* console, const char* text, void* userData)
{
	gDumpCCTStats = !gDumpCCTStats;
}

#if defined(WIN32) && !PX_XBOXONE
static bool copyToClipboard(const char* text)
{
	if(!text)
		return false;

	if(OpenClipboard(NULL))
	{
		HGLOBAL hMem = GlobalAlloc(GHND|GMEM_DDESHARE, strlen(text)+1);
		if(hMem)
		{
			char* pMem = (char*)GlobalLock(hMem);
			strcpy(pMem, text);
			GlobalUnlock(hMem);
			EmptyClipboard();
			SetClipboardData(CF_TEXT, hMem);
		}
		CloseClipboard();
	}
	return true;
}
#endif

static void gConsole_DumpCCTPosition(Console* console, const char* text, void* userData)
{
#if defined(WIN32) && !PX_XBOXONE
	SampleCCTCameraController* sample = (SampleCCTCameraController*)userData;
	const PxExtendedVec3 pos = sample->getControlledActor()->getController()->getPosition();

	char buffer[4096];
	sprintf(buffer, "PxExtendedVec3(%f, %f, %f);", float(pos.x), float(pos.y), float(pos.z));
	copyToClipboard(buffer);

	console->out("Position has been copied to clipboard");
#endif
}

SampleCCTCameraController::SampleCCTCameraController(PhysXSample& base) :
	mBase				(base),
	mObstacleContext	(NULL),
	mFilterData			(NULL),
	mFilterCallback		(NULL),
	mCCTFilterCallback	(NULL),
	mControlledIndex	(0),
	mNbCCTs				(0),
	mCCTs				(NULL),
	mTargetYaw			(0.0f-PxPi/2),
	mTargetPitch		(0.0f),
	mPitchMin			(-PxHalfPi),
	mPitchMax			(PxHalfPi),
	mGamepadPitchInc	(0.0f),
	mGamepadYawInc		(0.0f),
	mGamepadForwardInc	(0.0f),
	mGamepadLateralInc	(0.0f),
	mSensibility		(0.001f),
	mFwd				(false),
	mBwd				(false),
	mLeft				(false),
	mRight				(false),
	mKeyShiftDown		(false),
	mCCTEnabled			(true),
	mRunningSpeed		(10.0f),
	mWalkingSpeed		(2.5f),
//	mWalkingSpeed		(0.5f),
	mGamepadWalkingSpeed(10.0f),
	mCameraMaxSpeed		(FLT_MAX),
	mJumpForce			(30.0f),
	mGravity			(-9.81f),
	mLinkCameraToPhysics(false)
{
	Console* console = base.getConsole();
	if(console)
	{
		console->addCmd("DumpCCTState", gConsole_DumpCCTState);
		console->addCmd("DumpCCTStats", gConsole_DumpCCTStats);
		console->addCmd("DumpCCTPosition", gConsole_DumpCCTPosition);

		console->setUserData(this);
	}
}

void SampleCCTCameraController::setControlled(ControlledActor** controlled, PxU32 controlledIndex, PxU32 nbCCTs)
{
	mControlledIndex	= controlledIndex;
	mNbCCTs				= nbCCTs;
	mCCTs				= controlled;
}

void SampleCCTCameraController::startJump()
{
	ControlledActor* actor = getControlledActor();
	if(canJump(actor))
		actor->jump(mJumpForce);
}

void SampleCCTCameraController::collectInputEvents(std::vector<const SampleFramework::InputEvent*>& inputEvents)
{
	//digital keyboard events
	DIGITAL_INPUT_EVENT_DEF(CAMERA_MOVE_FORWARD,											SCAN_CODE_FORWARD,			XKEY_W,						X1KEY_W,					PS3KEY_W,					PS4KEY_W,					AKEY_UNKNOWN,			SCAN_CODE_FORWARD,			IKEY_UNKNOWN,			SCAN_CODE_FORWARD,	WIIUKEY_UNKNOWN);
	DIGITAL_INPUT_EVENT_DEF(CAMERA_MOVE_BACKWARD,											SCAN_CODE_BACKWARD,			XKEY_S,						X1KEY_S,					PS3KEY_S,					PS4KEY_S,					AKEY_UNKNOWN,			SCAN_CODE_BACKWARD,			IKEY_UNKNOWN,			SCAN_CODE_BACKWARD,	WIIUKEY_UNKNOWN);
	DIGITAL_INPUT_EVENT_DEF(CAMERA_MOVE_LEFT,												SCAN_CODE_LEFT,				XKEY_A,						X1KEY_A,					PS3KEY_A,					PS4KEY_A,					AKEY_UNKNOWN,			SCAN_CODE_LEFT,				IKEY_UNKNOWN,			SCAN_CODE_LEFT,		WIIUKEY_UNKNOWN);
	DIGITAL_INPUT_EVENT_DEF(CAMERA_MOVE_RIGHT,												SCAN_CODE_RIGHT,			XKEY_D,						X1KEY_D,					PS3KEY_D,					PS4KEY_D,					AKEY_UNKNOWN,			SCAN_CODE_RIGHT,			IKEY_UNKNOWN,			SCAN_CODE_RIGHT,	WIIUKEY_UNKNOWN);
	DIGITAL_INPUT_EVENT_DEF(CAMERA_SHIFT_SPEED,												SCAN_CODE_LEFT_SHIFT,		XKEY_SHIFT,					X1KEY_SHIFT,				PS3KEY_SHIFT,				PS4KEY_SHIFT,				AKEY_UNKNOWN,			OSXKEY_SHIFT,				IKEY_UNKNOWN,			LINUXKEY_SHIFT,		WIIUKEY_UNKNOWN);
	DIGITAL_INPUT_EVENT_DEF(CAMERA_JUMP,													SCAN_CODE_SPACE,			XKEY_SPACE,					X1KEY_SPACE,				PS3KEY_SPACE,				PS4KEY_SPACE,				AKEY_UNKNOWN,			OSXKEY_SPACE,				IKEY_UNKNOWN,			LINUXKEY_SPACE,		WIIUKEY_UNKNOWN);
	DIGITAL_INPUT_EVENT_DEF(CAMERA_CROUCH,													SCAN_CODE_DOWN,				XKEY_C,						X1KEY_C,					PS3KEY_C,					PS4KEY_C,					AKEY_UNKNOWN,			SCAN_CODE_DOWN,				IKEY_UNKNOWN,			SCAN_CODE_DOWN,		WIIUKEY_UNKNOWN);
	DIGITAL_INPUT_EVENT_DEF(CAMERA_CONTROLLER_INCREASE,										WKEY_ADD,					XKEY_ADD,					X1KEY_ADD,					PS3KEY_ADD,					PS4KEY_ADD,					AKEY_UNKNOWN,			OSXKEY_ADD,					IKEY_UNKNOWN,			LINUXKEY_ADD,		WIIUKEY_UNKNOWN);
	DIGITAL_INPUT_EVENT_DEF(CAMERA_CONTROLLER_DECREASE,										WKEY_SUBTRACT,				XKEY_SUBTRACT,				X1KEY_SUBTRACT,				PS3KEY_SUBTRACT,			PS4KEY_SUBTRACT,			AKEY_UNKNOWN,			OSXKEY_SUBTRACT,			IKEY_UNKNOWN,			LINUXKEY_SUBTRACT,	WIIUKEY_UNKNOWN);
											
	//digital gamepad events											
	DIGITAL_INPUT_EVENT_DEF(CAMERA_JUMP,													GAMEPAD_SOUTH,				GAMEPAD_SOUTH,				GAMEPAD_SOUTH,				GAMEPAD_SOUTH,				GAMEPAD_SOUTH,				AKEY_UNKNOWN,			GAMEPAD_SOUTH,				IKEY_UNKNOWN, 			LINUXKEY_UNKNOWN,	GAMEPAD_SOUTH);
	DIGITAL_INPUT_EVENT_DEF(CAMERA_CROUCH,													GAMEPAD_LEFT_STICK,			GAMEPAD_LEFT_STICK,			GAMEPAD_LEFT_STICK,			GAMEPAD_LEFT_STICK,			GAMEPAD_LEFT_STICK,			AKEY_UNKNOWN,			GAMEPAD_LEFT_STICK,			IKEY_UNKNOWN, 			LINUXKEY_UNKNOWN,	GAMEPAD_LEFT_STICK);
	DIGITAL_INPUT_EVENT_DEF(CAMERA_CONTROLLER_INCREASE,										GAMEPAD_RIGHT_SHOULDER_TOP,	GAMEPAD_RIGHT_SHOULDER_TOP,	GAMEPAD_RIGHT_SHOULDER_TOP,	GAMEPAD_RIGHT_SHOULDER_TOP,	GAMEPAD_RIGHT_SHOULDER_TOP,	AKEY_UNKNOWN,			GAMEPAD_RIGHT_SHOULDER_TOP,	IKEY_UNKNOWN, 			LINUXKEY_UNKNOWN,	GAMEPAD_RIGHT_SHOULDER_TOP);
	DIGITAL_INPUT_EVENT_DEF(CAMERA_CONTROLLER_DECREASE,										GAMEPAD_LEFT_SHOULDER_TOP,	GAMEPAD_LEFT_SHOULDER_TOP,	GAMEPAD_LEFT_SHOULDER_TOP,	GAMEPAD_LEFT_SHOULDER_TOP,	GAMEPAD_LEFT_SHOULDER_TOP,	AKEY_UNKNOWN,			GAMEPAD_LEFT_SHOULDER_TOP,	IKEY_UNKNOWN, 			LINUXKEY_UNKNOWN,	GAMEPAD_LEFT_SHOULDER_TOP);

	//analog gamepad events
	ANALOG_INPUT_EVENT_DEF(CAMERA_GAMEPAD_ROTATE_LEFT_RIGHT, GAMEPAD_ROTATE_SENSITIVITY,	GAMEPAD_RIGHT_STICK_X,		GAMEPAD_RIGHT_STICK_X,		GAMEPAD_RIGHT_STICK_X,		GAMEPAD_RIGHT_STICK_X,		GAMEPAD_RIGHT_STICK_X,		GAMEPAD_RIGHT_STICK_X,	GAMEPAD_RIGHT_STICK_X,		GAMEPAD_RIGHT_STICK_X, 	LINUXKEY_UNKNOWN,	GAMEPAD_RIGHT_STICK_X);
	ANALOG_INPUT_EVENT_DEF(CAMERA_GAMEPAD_ROTATE_UP_DOWN, GAMEPAD_ROTATE_SENSITIVITY,		GAMEPAD_RIGHT_STICK_Y,		GAMEPAD_RIGHT_STICK_Y,		GAMEPAD_RIGHT_STICK_Y,		GAMEPAD_RIGHT_STICK_Y,		GAMEPAD_RIGHT_STICK_Y,		GAMEPAD_RIGHT_STICK_Y,	GAMEPAD_RIGHT_STICK_Y,		GAMEPAD_RIGHT_STICK_Y, 	LINUXKEY_UNKNOWN,	GAMEPAD_RIGHT_STICK_Y);
	ANALOG_INPUT_EVENT_DEF(CAMERA_GAMEPAD_MOVE_LEFT_RIGHT, GAMEPAD_DEFAULT_SENSITIVITY,		GAMEPAD_LEFT_STICK_X,		GAMEPAD_LEFT_STICK_X,		GAMEPAD_LEFT_STICK_X,		GAMEPAD_LEFT_STICK_X,		GAMEPAD_LEFT_STICK_X,		GAMEPAD_LEFT_STICK_X,	GAMEPAD_LEFT_STICK_X,		GAMEPAD_LEFT_STICK_X, 	LINUXKEY_UNKNOWN,	GAMEPAD_LEFT_STICK_X);
	ANALOG_INPUT_EVENT_DEF(CAMERA_GAMEPAD_MOVE_FORWARD_BACK, GAMEPAD_DEFAULT_SENSITIVITY,	GAMEPAD_LEFT_STICK_Y,		GAMEPAD_LEFT_STICK_Y,		GAMEPAD_LEFT_STICK_Y,		GAMEPAD_LEFT_STICK_Y,		GAMEPAD_LEFT_STICK_Y,		GAMEPAD_LEFT_STICK_Y,	GAMEPAD_LEFT_STICK_Y,		GAMEPAD_LEFT_STICK_Y, 	LINUXKEY_UNKNOWN,	GAMEPAD_LEFT_STICK_Y);

    //touch events (these are defined in the samples, since overwriting is currently not well possible)
}

void SampleCCTCameraController::onDigitalInputEvent(const SampleFramework::InputEvent& ie, bool val)
{
	Console* console = mBase.getConsole();
	if(console && console->isActive())
		return;

	switch (ie.m_Id)
	{
	case CAMERA_MOVE_FORWARD:
		{
			mFwd = val;
		}
		break;
	case CAMERA_MOVE_BACKWARD:
		{
			mBwd = val;
		}
		break;
	case CAMERA_MOVE_LEFT:
		{
			mLeft = val;
		}
		break;
	case CAMERA_MOVE_RIGHT:
		{
			mRight = val;
		}
		break;
	case CAMERA_SHIFT_SPEED:
		{
			mKeyShiftDown = val;
		}
		break;
	case CAMERA_JUMP:
		{
			if(val)
				startJump();
		}
		break;
	case CAMERA_CROUCH:
		{
			if(val)
			{
				getControlledActor()->resizeCrouching();
			}
			else
			{
				getControlledActor()->mDoStandup = true;
			}
		}
		break;
	case CAMERA_CONTROLLER_INCREASE:
		{
			if(val)
			{
				if(mControlledIndex<mNbCCTs-1)
					mControlledIndex++;
			}
		}
		break;
	case CAMERA_CONTROLLER_DECREASE:
		{
			if(val)
			{
				if(mControlledIndex)
					mControlledIndex--;
			}
		}
		break;
	case CAMERA_SPEED_INCREASE:
		{
			if(val)
			{
				if(mKeyShiftDown)
				{
					if(mRunningSpeed * 2.0f <= mCameraMaxSpeed)
						mRunningSpeed *= 2.f;
				}
				else
				{
					if(mWalkingSpeed * 2.0f <= mCameraMaxSpeed)
						mWalkingSpeed *= 2.f;

					if(mGamepadWalkingSpeed * 2.0f <= mCameraMaxSpeed)
						mGamepadWalkingSpeed *= 2.f;
				}
			}
		}
		break;
	case CAMERA_SPEED_DECREASE:
		{
			if(val)
			{
				if(mKeyShiftDown)
				{
					mRunningSpeed *= 0.5f;
				}
				else
				{
					mWalkingSpeed *= 0.5f;
					mGamepadWalkingSpeed *= 0.5f;
				}
			}
		}
		break;
	}

}

static PX_FORCE_INLINE PxReal remapAxisValue(PxReal absolutePosition)
{
	return absolutePosition * absolutePosition * absolutePosition * 5.0f;
}

void SampleCCTCameraController::onAnalogInputEvent(const SampleFramework::InputEvent& ie, float val)
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

void SampleCCTCameraController::onPointerInputEvent(const SampleFramework::InputEvent &ie, physx::PxU32, physx::PxU32, physx::PxReal dx, physx::PxReal dy, bool val)
{
	if (ie.m_Id == CAMERA_MOUSE_LOOK)
	{
		mTargetYaw		-= dx * mSensibility;
		mTargetPitch	+= dy * mSensibility;
	}
}

void SampleCCTCameraController::setView(PxReal pitch, PxReal yaw)
{
	mTargetPitch = pitch;
	mTargetYaw   = yaw;
}

void SampleCCTCameraController::update(Camera& camera, PxReal dtime)
{
	PxSceneReadLock scopedLock(mBase.getActiveScene());

//	shdfnd::printFormatted("SampleCCTCameraController::update\n");

	if(!mCCTs)
		return;

	// Update CCT
//	bool doUpdate = false;
	if(!mBase.isPaused() && mCCTEnabled)
	{
/*		PxScene& scene = mBase.getActiveScene();
		static PxU32 timestamp = 0;
		PxU32 t = scene.getTimestamp();
		const PxU32 dt = t - timestamp;
		doUpdate = t!=timestamp;
		shdfnd::printFormatted("doUpdate: %d\n", doUpdate);
		timestamp = t;
		if(doUpdate)
			dtime = float(dt) * 1.0f/60.0f;
		if(doUpdate)*/

		{

		const PxControllerFilters filters(mFilterData, mFilterCallback, mCCTFilterCallback);

		for(PxU32 i=0;i<mNbCCTs;i++)
		{
			PxVec3 disp;

			const PxF32 heightDelta = mCCTs[i]->mJump.getHeight(dtime);
			float dy;
			if(heightDelta!=0.0f)
				dy = heightDelta;
			else
				dy = mGravity * dtime;
//			shdfnd::printFormatted("%f\n", dy);

			if(i==mControlledIndex)
			{
				PxVec3 targetKeyDisplacement(0);
				PxVec3 targetPadDisplacement(0);

				PxVec3 forward = camera.getViewDir();
				forward.y = 0;
				forward.normalize();
				PxVec3 up = PxVec3(0,1,0);
				PxVec3 right = forward.cross(up);

//				if(canJump(mCCTs[i]))	// PT: prevent displacement in mid-air
				{
					if(mFwd)	targetKeyDisplacement += forward;
					if(mBwd)	targetKeyDisplacement -= forward;

					if(mRight)	targetKeyDisplacement += right;
					if(mLeft)	targetKeyDisplacement -= right;

					targetKeyDisplacement *= mKeyShiftDown ? mRunningSpeed : mWalkingSpeed;
					targetKeyDisplacement *= dtime;

					targetPadDisplacement += forward * mGamepadForwardInc * mGamepadWalkingSpeed;
					targetPadDisplacement += right * mGamepadLateralInc * mGamepadWalkingSpeed;
					targetPadDisplacement *= dtime;
				}

				disp = targetKeyDisplacement + targetPadDisplacement;
				disp.y = dy;
			}
			else
			{
				disp = PxVec3(0, dy, 0);
			}

			if(gTransferPlatformMomentum)
			{
//				shdfnd::printFormatted("%d\n", mCCTs[i]->mTransferMomentum);
				if(mCCTs[i]->mTransferMomentum)
				{
					PxVec3 dd = mCCTs[i]->mDelta * dtime;
//					shdfnd::printFormatted("dd: %f %f %f (%f)\n", dd.x, dd.y, dd.z, dtime);
					disp.x += dd.x;
					disp.z += dd.z;
				}
			}

//			const PxU32 flags = mCCTs[i]->mController->move(disp, 0.001f, dtime, filters, mObstacleContext);
			const PxU32 flags = mCCTs[i]->mController->move(disp, 0.0f, dtime, filters, mObstacleContext);
			if(flags & PxControllerCollisionFlag::eCOLLISION_DOWN)
			{
//				shdfnd::printFormatted("Stop jump\n");
				mCCTs[i]->mJump.stopJump();
			}
//			shdfnd::printFormatted("%d\n", flags & PxControllerCollisionFlag::eCOLLISION_DOWN);

			if(gTransferPlatformMomentum)
			{
				if(!flags)
				{
					mCCTs[i]->mTransferMomentum = true;
				}
				else
				{
					// ### optimize this
					mCCTs[i]->mTransferMomentum = false;
					PxControllerState cctState;
					mCCTs[i]->mController->getState(cctState);
					mCCTs[i]->mDelta = cctState.deltaXP;
//					shdfnd::printFormatted("delta out: %f %f %f\n", cctState.deltaXP.x, cctState.deltaXP.y, cctState.deltaXP.z);
				}
			}

			if(gDumpCCTState && i==mControlledIndex)
			{
				PxControllerState cctState;
				mCCTs[i]->mController->getState(cctState);
				shdfnd::printFormatted("\nCCT state:\n");
				shdfnd::printFormatted("delta:             %.02f | %.02f | %.02f\n", cctState.deltaXP.x, cctState.deltaXP.y, cctState.deltaXP.z);
				shdfnd::printFormatted("touchedShape:      %p\n", cctState.touchedShape);
				shdfnd::printFormatted("touchedObstacleHandle:   %d\n", cctState.touchedObstacleHandle);
				shdfnd::printFormatted("standOnAnotherCCT: %d\n", cctState.standOnAnotherCCT);
				shdfnd::printFormatted("standOnObstacle:   %d\n", cctState.standOnObstacle);
				shdfnd::printFormatted("isMovingUp:        %d\n", cctState.isMovingUp);
				shdfnd::printFormatted("collisionFlags:    %d\n", cctState.collisionFlags);
			}

			if(gDumpCCTStats && i==mControlledIndex)
			{
				PxControllerStats cctStats;
				mCCTs[i]->mController->getStats(cctStats);
				shdfnd::printFormatted("nbIterations:     %d\n", cctStats.nbIterations);
				shdfnd::printFormatted("nbFullUpdates:    %d\n", cctStats.nbFullUpdates);
				shdfnd::printFormatted("nbPartialUpdates: %d\n", cctStats.nbPartialUpdates);
				shdfnd::printFormatted("nbTessellation:   %d\n", cctStats.nbTessellation);
			}
		}

		}
	}

	// Update camera
	PxController* cct = mCCTs[mControlledIndex]->mController;
	if(cct/* && doUpdate*/)
	{
		mTargetYaw		+= mGamepadYawInc * dtime;
		mTargetPitch	+= mGamepadPitchInc * dtime;

		// Clamp pitch
		if(mTargetPitch<mPitchMin)	mTargetPitch = mPitchMin;
		if(mTargetPitch>mPitchMax)	mTargetPitch = mPitchMax;

		camera.setRot(PxVec3(-mTargetPitch,-mTargetYaw,0));
//shdfnd::printFormatted("Pitch: %f\n", mTargetPitch);

		PxExtendedVec3 camTarget;
		if(!mLinkCameraToPhysics)
		{
			camTarget = cct->getFootPosition();
		}
		else
		{
			const PxVec3 delta = cct->getPosition() - cct->getFootPosition();

			const PxVec3 physicsPos = cct->getActor()->getGlobalPose().p - delta;
			camTarget = PxExtendedVec3(physicsPos.x, physicsPos.y, physicsPos.z);
		}

		const float height = 1.0f;
		camTarget += PxVec3(0, height, 0);
		const PxVec3 target = toVec3(camTarget) - camera.getViewDir()*5.0f;
//const PxVec3 target2 = target;
//shdfnd::printFormatted("target: %f | %f | %f\n", target.x, target.y, target.z);
		camera.setPos(target);
	}
}

