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

#include "SamplePreprocessor.h"
#include "SampleCameraController.h"
#include "SampleCamera.h"
#include "SampleApplication.h"
#include "SamplePlatform.h"
#include "PsUtilities.h"
#include "SampleBaseInputEventIds.h"
#include <SampleFrameworkInputEventIds.h>
#include "SampleUserInputDefines.h"

#include <SampleUserInputIds.h>

using namespace physx;
using namespace SampleRenderer;
using namespace SampleFramework;

// PT: this replicates the default SampleApplication behaviour, but with a much better design. We
// want to be able to isolate & replace the camera controlling code easily.

static const float g_smoothCamBaseVel  = 6.0f;
static const float g_smoothCamPosLerp  = 0.4f;
static const float g_smoothCamFastMul = 4.0f;

static const float g_smoothCamRotSpeed = 0.005f;
static const float g_smoothCamRotLerp  = 0.4f;

static PxReal	gGamepadYawInc			= 0.0f;
static PxReal	gGamepadPitchInc		= 0.0f;
static PxReal	gGamepadForwardInc		= 0.0f;
static PxReal	gGamepadLateralInc		= 0.0f;

static PxVec3 QuatToEuler(const PxQuat& q)
{
	PxVec3 dir = -PxMat33(q)[2];
	PxReal r = PxSqrt(dir.x * dir.x + dir.z * dir.z);
	PxVec3 rot(0.0f, PxHalfPi, 0.0f);
	if (r != 0.0f)
	{
		rot.x = -PxAtan(dir.y / r);
		rot.y =  PxAsin(dir.x / r);
		if (dir.z > 0.0f)
			rot.y = PxPi - rot.y;
	}

	return rot;
}

///////////////////////////////////////////////////////////////////////////////

DefaultCameraController::DefaultCameraController() :
	mMouseButtonDown		(false),
	mKeyFWDown				(false),
	mKeyBKDown				(false),
	mKeyRTDown				(false),
	mKeyLTDown				(false),
	mKeyUpDown				(false),
	mKeyDownDown			(false),
	mKeyShiftDown			(false),
	mCameraSpeed			(0.f),
	mCameraSpeedMultiplier	(1.f),
	mMouseLookOnMB			(true), 
	mMouseSensitivity		(1.f)
{
	mTargetEyePos	= PxVec3(0);
	mTargetEyeRot	= PxVec3(0);
	mEyePos			= PxVec3(0);
	mEyeRot			= PxVec3(0);
}

DefaultCameraController::~DefaultCameraController()
{
}

///////////////////////////////////////////////////////////////////////////////

void DefaultCameraController::init(const PxVec3& pos, const PxVec3& rot)
{
	mTargetEyePos = mEyePos = pos;
	mTargetEyeRot = mEyeRot = rot;
}

void DefaultCameraController::init(const PxTransform& pose)
{
	init(pose.p, QuatToEuler(pose.q));
}

void DefaultCameraController::onPointerInputEvent(const InputEvent& ie, physx::PxU32 x, physx::PxU32 y, physx::PxReal dx, physx::PxReal dy, bool val)
{
	switch (ie.m_Id)
	{
	case CAMERA_MOUSE_LOOK:
		{
			if (!mMouseLookOnMB || mMouseButtonDown)
				onMouseDelta(static_cast<physx::PxI32>(dx), static_cast<physx::PxI32>(dy));
		}
		break;
	case MOUSE_LOOK_BUTTON:
		{
			mMouseButtonDown = val;
		}
		break;
	}

}

void DefaultCameraController::onMouseDelta(PxI32 dx, PxI32 dy)
{
	mTargetEyeRot.x -= dy * mMouseSensitivity * g_smoothCamRotSpeed;
	mTargetEyeRot.y += dx * mMouseSensitivity * g_smoothCamRotSpeed;
}

void DefaultCameraController::collectInputEvents(std::vector<const InputEvent*>& inputEvents)
{
	//digital keyboard events
	DIGITAL_INPUT_EVENT_DEF(CAMERA_MOVE_FORWARD,											SCAN_CODE_FORWARD,		XKEY_W,					X1KEY_W,				PS3KEY_W,				PS4KEY_W,				AKEY_UNKNOWN,			SCAN_CODE_FORWARD,		IKEY_UNKNOWN,			SCAN_CODE_FORWARD,		WIIUKEY_UNKNOWN);
	DIGITAL_INPUT_EVENT_DEF(CAMERA_MOVE_BACKWARD,											SCAN_CODE_BACKWARD,		XKEY_S,					X1KEY_S,				PS3KEY_S,				PS4KEY_S,				AKEY_UNKNOWN,			SCAN_CODE_BACKWARD,		IKEY_UNKNOWN,			SCAN_CODE_BACKWARD,		WIIUKEY_UNKNOWN);
	DIGITAL_INPUT_EVENT_DEF(CAMERA_MOVE_UP,													SCAN_CODE_UP,			XKEY_E,					X1KEY_E,				PS3KEY_E,				PS4KEY_E,				AKEY_UNKNOWN,			SCAN_CODE_UP,			IKEY_UNKNOWN,			SCAN_CODE_UP,			WIIUKEY_UNKNOWN);
	DIGITAL_INPUT_EVENT_DEF(CAMERA_MOVE_DOWN,												SCAN_CODE_DOWN,			XKEY_C,					X1KEY_C,				PS3KEY_C,				PS4KEY_C,				AKEY_UNKNOWN,			SCAN_CODE_DOWN,			IKEY_UNKNOWN,			SCAN_CODE_DOWN,			WIIUKEY_UNKNOWN);
	DIGITAL_INPUT_EVENT_DEF(CAMERA_MOVE_LEFT,												SCAN_CODE_LEFT,			XKEY_A,					X1KEY_A,				PS3KEY_A,				PS4KEY_A,				AKEY_UNKNOWN,			SCAN_CODE_LEFT,			IKEY_UNKNOWN,			SCAN_CODE_LEFT,			WIIUKEY_UNKNOWN);
	DIGITAL_INPUT_EVENT_DEF(CAMERA_MOVE_RIGHT,												SCAN_CODE_RIGHT,		XKEY_D,					X1KEY_D,				PS3KEY_D,				PS4KEY_D,				AKEY_UNKNOWN,			SCAN_CODE_RIGHT,		IKEY_UNKNOWN,			SCAN_CODE_RIGHT,		WIIUKEY_UNKNOWN);
	DIGITAL_INPUT_EVENT_DEF(CAMERA_SHIFT_SPEED,												SCAN_CODE_LEFT_SHIFT,	XKEY_SHIFT,				X1KEY_SHIFT,			PS3KEY_SHIFT,			PS4KEY_SHIFT,			AKEY_UNKNOWN,			OSXKEY_SHIFT,			IKEY_UNKNOWN,			LINUXKEY_SHIFT,			WIIUKEY_UNKNOWN);
	DIGITAL_INPUT_EVENT_DEF(CAMERA_SPEED_INCREASE,											WKEY_ADD,				XKEY_ADD,				X1KEY_ADD,				PS3KEY_ADD,				PS4KEY_ADD,				AKEY_UNKNOWN,			OSXKEY_ADD,				IKEY_UNKNOWN,			LINUXKEY_ADD,			WIIUKEY_UNKNOWN);
	DIGITAL_INPUT_EVENT_DEF(CAMERA_SPEED_DECREASE,											WKEY_SUBTRACT,			XKEY_SUBTRACT,			X1KEY_SUBTRACT,			PS3KEY_SUBTRACT,		PS4KEY_SUBTRACT,		AKEY_UNKNOWN,			OSXKEY_SUBTRACT,		IKEY_UNKNOWN,			LINUXKEY_SUBTRACT,		WIIUKEY_UNKNOWN);
														
	//digital mouse events																					
	DIGITAL_INPUT_EVENT_DEF(MOUSE_LOOK_BUTTON,												MOUSE_BUTTON_LEFT,		XKEY_UNKNOWN,			X1KEY_UNKNOWN,			PS3KEY_UNKNOWN,			PS4KEY_UNKNOWN,			AKEY_UNKNOWN,			MOUSE_BUTTON_LEFT,		IKEY_UNKNOWN,			LINUXKEY_UNKNOWN,		WIIUKEY_UNKNOWN);
									
	//digital gamepad events																
	DIGITAL_INPUT_EVENT_DEF(CAMERA_SPEED_INCREASE,											GAMEPAD_LEFT_STICK,		GAMEPAD_LEFT_STICK,		GAMEPAD_LEFT_STICK,		GAMEPAD_LEFT_STICK,		GAMEPAD_LEFT_STICK,		AKEY_UNKNOWN,			OSXKEY_UNKNOWN,			IKEY_UNKNOWN,			LINUXKEY_UNKNOWN,		GAMEPAD_LEFT_STICK);
	DIGITAL_INPUT_EVENT_DEF(CAMERA_SPEED_DECREASE,											GAMEPAD_RIGHT_STICK,	GAMEPAD_RIGHT_STICK,	GAMEPAD_RIGHT_STICK,	GAMEPAD_LEFT_STICK,		GAMEPAD_LEFT_STICK,		AKEY_UNKNOWN,			OSXKEY_UNKNOWN,			IKEY_UNKNOWN,			LINUXKEY_UNKNOWN,		GAMEPAD_RIGHT_STICK);

	//analog gamepad events
	ANALOG_INPUT_EVENT_DEF(CAMERA_GAMEPAD_ROTATE_LEFT_RIGHT, GAMEPAD_ROTATE_SENSITIVITY,	GAMEPAD_RIGHT_STICK_X,	GAMEPAD_RIGHT_STICK_X,	GAMEPAD_RIGHT_STICK_X,	GAMEPAD_RIGHT_STICK_X,	GAMEPAD_RIGHT_STICK_X,	GAMEPAD_RIGHT_STICK_X,	GAMEPAD_RIGHT_STICK_X,	GAMEPAD_RIGHT_STICK_X,	LINUXKEY_UNKNOWN,		GAMEPAD_RIGHT_STICK_X);
	ANALOG_INPUT_EVENT_DEF(CAMERA_GAMEPAD_ROTATE_UP_DOWN, GAMEPAD_ROTATE_SENSITIVITY,		GAMEPAD_RIGHT_STICK_Y,	GAMEPAD_RIGHT_STICK_Y,	GAMEPAD_RIGHT_STICK_Y,	GAMEPAD_RIGHT_STICK_Y,	GAMEPAD_RIGHT_STICK_Y,	GAMEPAD_RIGHT_STICK_Y,	GAMEPAD_RIGHT_STICK_Y,	GAMEPAD_RIGHT_STICK_Y,	LINUXKEY_UNKNOWN,		GAMEPAD_RIGHT_STICK_Y);
	ANALOG_INPUT_EVENT_DEF(CAMERA_GAMEPAD_MOVE_LEFT_RIGHT, GAMEPAD_DEFAULT_SENSITIVITY,		GAMEPAD_LEFT_STICK_X,	GAMEPAD_LEFT_STICK_X,	GAMEPAD_LEFT_STICK_X,	GAMEPAD_LEFT_STICK_X,	GAMEPAD_LEFT_STICK_X,	GAMEPAD_LEFT_STICK_X,	GAMEPAD_LEFT_STICK_X,	GAMEPAD_LEFT_STICK_X,	LINUXKEY_UNKNOWN,		GAMEPAD_LEFT_STICK_X);
	ANALOG_INPUT_EVENT_DEF(CAMERA_GAMEPAD_MOVE_FORWARD_BACK, GAMEPAD_DEFAULT_SENSITIVITY,	GAMEPAD_LEFT_STICK_Y,	GAMEPAD_LEFT_STICK_Y,	GAMEPAD_LEFT_STICK_Y,	GAMEPAD_LEFT_STICK_Y,	GAMEPAD_LEFT_STICK_Y,	GAMEPAD_LEFT_STICK_Y,	GAMEPAD_LEFT_STICK_Y,	GAMEPAD_LEFT_STICK_Y,	LINUXKEY_UNKNOWN,		GAMEPAD_LEFT_STICK_Y);
}

///////////////////////////////////////////////////////////////////////////////

void DefaultCameraController::onDigitalInputEvent(const SampleFramework::InputEvent& ie, bool val)
{
	if(val)
	{
				if(ie.m_Id == CAMERA_MOVE_FORWARD)	mKeyFWDown = true;
		else	if(ie.m_Id == CAMERA_MOVE_UP)	mKeyUpDown = true;
		else	if(ie.m_Id == CAMERA_MOVE_BACKWARD)	mKeyBKDown = true;
		else	if(ie.m_Id == CAMERA_MOVE_LEFT)	mKeyLTDown = true;
		else	if(ie.m_Id == CAMERA_MOVE_RIGHT)	mKeyRTDown = true;
		else	if(ie.m_Id == CAMERA_SHIFT_SPEED)	mKeyShiftDown = true;
		else    if(ie.m_Id == CAMERA_MOVE_DOWN)	mKeyDownDown = true;
		else	if(ie.m_Id == CAMERA_SPEED_DECREASE) mCameraSpeedMultiplier *= 0.5f;
		else	if(ie.m_Id == CAMERA_SPEED_INCREASE) mCameraSpeedMultiplier *= 2.0f;
	}
	else
	{
				if(ie.m_Id == CAMERA_MOVE_FORWARD)	mKeyFWDown = false;
		else	if(ie.m_Id == CAMERA_MOVE_UP)	mKeyUpDown = false;
		else	if(ie.m_Id == CAMERA_MOVE_BACKWARD)	mKeyBKDown = false;
		else	if(ie.m_Id == CAMERA_MOVE_LEFT)	mKeyLTDown = false;
		else	if(ie.m_Id == CAMERA_MOVE_RIGHT)	mKeyRTDown = false;
		else	if(ie.m_Id == CAMERA_SHIFT_SPEED)	mKeyShiftDown = false;
		else    if(ie.m_Id == CAMERA_MOVE_DOWN)	mKeyDownDown = false;
	}
}

static PX_FORCE_INLINE PxReal remapAxisValue(PxReal absolutePosition)
{
	return absolutePosition * absolutePosition * absolutePosition * 5.0f;
}

void DefaultCameraController::onAnalogInputEvent(const SampleFramework::InputEvent& ie, float val)
{
	if(ie.m_Id == CAMERA_GAMEPAD_ROTATE_LEFT_RIGHT)
	{
		gGamepadYawInc = remapAxisValue(val);
	}
	else if(ie.m_Id == CAMERA_GAMEPAD_ROTATE_UP_DOWN)
	{
		gGamepadPitchInc = - remapAxisValue(val);
	}
	else if(ie.m_Id == CAMERA_GAMEPAD_MOVE_LEFT_RIGHT)
	{
		gGamepadLateralInc = - val;
	}
	else if(ie.m_Id == CAMERA_GAMEPAD_MOVE_FORWARD_BACK)
	{
		gGamepadForwardInc = val;
	}
}

///////////////////////////////////////////////////////////////////////////////

// PT: TODO: refactor this
static PxMat33 EulerToMat33(const PxVec3 &e)
{
	float c1 = cosf(e.z);
	float s1 = sinf(e.z);
	float c2 = cosf(e.y);
	float s2 = sinf(e.y);
	float c3 = cosf(e.x);
	float s3 = sinf(e.x);
	PxMat33 m(PxVec3(c1*c2,              -s1*c2,             s2),
			  PxVec3((s1*c3)+(c1*s2*s3), (c1*c3)-(s1*s2*s3),-c2*s3),
			  PxVec3((s1*s3)-(c1*s2*c3), (c1*s3)+(s1*s2*c3), c2*c3));

	return m;
}

void DefaultCameraController::update(Camera& camera, PxReal dtime)
{
	// PT: copied from SampleApplication::onMouseMove
	
	const float eyeCap = 1.5f;

	mTargetEyeRot.x += gGamepadPitchInc * dtime;
	mTargetEyeRot.y += gGamepadYawInc * dtime;

	if(mTargetEyeRot.x >  eyeCap) mTargetEyeRot.x =  eyeCap;
	if(mTargetEyeRot.x < -eyeCap) mTargetEyeRot.x = -eyeCap;
	mEyeRot += (mTargetEyeRot - mEyeRot) * g_smoothCamRotLerp;

	const PxMat33 tmp = EulerToMat33(mEyeRot);
	const PxVec3 forward = -tmp[2];
	const PxVec3 right = -tmp[0];

	const PxReal padEyeSpeed = mCameraSpeed == 0.f ? g_smoothCamBaseVel * mCameraSpeedMultiplier * dtime * g_smoothCamFastMul : mCameraSpeed;
	mTargetEyePos += forward * padEyeSpeed * gGamepadForwardInc;
	mTargetEyePos += right * padEyeSpeed * gGamepadLateralInc;

	const PxReal keyEyeSpeed = mCameraSpeed == 0.f ? g_smoothCamBaseVel * mCameraSpeedMultiplier * dtime * (mKeyShiftDown ? g_smoothCamFastMul : 1.0f) : mCameraSpeed;
	if(mKeyFWDown) mTargetEyePos -= tmp[2] * keyEyeSpeed;
	if(mKeyBKDown) mTargetEyePos += tmp[2] * keyEyeSpeed;
	if(mKeyLTDown) mTargetEyePos -= tmp[0] * keyEyeSpeed;
	if(mKeyRTDown) mTargetEyePos += tmp[0] * keyEyeSpeed;
	if(mKeyUpDown) mTargetEyePos += tmp[1] * keyEyeSpeed;
	if(mKeyDownDown) mTargetEyePos -= tmp[1] * keyEyeSpeed;

	mEyePos += (mTargetEyePos - mEyePos) * g_smoothCamPosLerp;

	camera.setPos(mEyePos);
	camera.setRot(mEyeRot);
}

///////////////////////////////////////////////////////////////////////////////
