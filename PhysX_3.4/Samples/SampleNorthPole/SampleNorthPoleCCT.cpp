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

#include "PxPhysicsAPI.h"
#include "extensions/PxExtensionsAPI.h"

#include "characterkinematic/PxControllerManager.h"
#include "characterkinematic/PxCapsuleController.h"

#include "SampleNorthPole.h"
#include "SampleNorthPoleCameraController.h"
#include "SampleNorthPoleInputEventIds.h"
#include <SamplePlatform.h>
#include <SampleUserInput.h>
#include <SampleUserInputIds.h>
#include <SampleUserInputDefines.h>

using namespace SampleRenderer;
using namespace SampleFramework;

void SampleNorthPole::tryStandup()
{
	PxSceneWriteLock scopedLock(*mScene);

	// overlap with upper part
	PxReal r = mController->getRadius();
	PxReal dh = mStandingSize-mCrouchingSize-2*r;
	PxCapsuleGeometry geom(r, dh*.5f);

	PxExtendedVec3 position = mController->getPosition();
	PxVec3 pos((float)position.x,(float)position.y+mStandingSize*.5f+r,(float)position.z);
	PxQuat orientation(PxHalfPi, PxVec3(0.0f, 0.0f, 1.0f));

	PxOverlapBuffer hit;
	if(getActiveScene().overlap(geom, PxTransform(pos,orientation),hit,
		PxQueryFilterData(PxQueryFlag::eANY_HIT|PxQueryFlag::eSTATIC|PxQueryFlag::eDYNAMIC)))
		return;

	// if no hit, we can stand up
	resizeController(mStandingSize);
	mDoStandup = false;
}

void SampleNorthPole::resizeController(PxReal height)
{
	PxSceneWriteLock scopedLock(*mScene);
	mController->resize(height);
}

PxCapsuleController* SampleNorthPole::createCharacter(const PxExtendedVec3& position)
{
	PxCapsuleControllerDesc cDesc;
	cDesc.material			= &getDefaultMaterial();
	cDesc.position			= position;
	cDesc.height			= mStandingSize;
	cDesc.radius			= mControllerRadius;
	cDesc.slopeLimit		= 0.0f;
	cDesc.contactOffset		= 0.1f;
	cDesc.stepOffset		= 0.02f;
	cDesc.reportCallback	= this;
	cDesc.behaviorCallback	= this;

	mControllerInitialPosition = cDesc.position;

	PxCapsuleController* ctrl = static_cast<PxCapsuleController*>(mControllerManager->createController(cDesc));

	// remove controller shape from scene query for standup overlap test
	PxRigidDynamic* actor = ctrl->getActor();
	if(actor)
	{
		if(actor->getNbShapes())
		{
			PxShape* ctrlShape;
			actor->getShapes(&ctrlShape,1);
			ctrlShape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE,false);
		}
		else
			fatalError("character actor has no shape");
	}
	else
		fatalError("character could not create actor");

	// uncomment the next line to render the character
	//createRenderObjectsFromActor(ctrl->getActor());

	return ctrl;
}


void SampleNorthPole::onShapeHit(const PxControllerShapeHit& hit)
{
	PxRigidDynamic* actor = hit.shape->getActor()->is<PxRigidDynamic>();
	if(actor)
	{
		// We only allow horizontal pushes. Vertical pushes when we stand on dynamic objects creates
		// useless stress on the solver. It would be possible to enable/disable vertical pushes on
		// particular objects, if the gameplay requires it.
		if(hit.dir.y==0.0f)
		{
			PxReal coeff = actor->getMass() * hit.length;
			PxRigidBodyExt::addForceAtLocalPos(*actor,hit.dir*coeff, PxVec3(0,0,0), PxForceMode::eIMPULSE);
		}
	}
}

void SampleNorthPole::resetScene()
{	
	PxSceneWriteLock scopedLock(*mScene);

	mController->setPosition(mControllerInitialPosition);
	mNorthPoleCamera->setView(0,0);

	while(mPhysicsActors.size())
	{
		PxRigidActor* actor = mPhysicsActors.back();
		removeActor(actor);
	}

	createSnowMen();
	buildHeightField();

	for(unsigned int b = 0; b < NUM_BALLS; b++)
	{
		PxRigidDynamic* ball = mSnowBalls[b];
		if(ball)
		{
			removeActor(ball);
			ball->release();
			mSnowBalls[b] = 0;
			
			// render actor is released inside of removeActor()
			mSnowBallsRenderActors[b] = 0;
		}
	}
}

void SampleNorthPole::collectInputEvents(std::vector<const SampleFramework::InputEvent*>& inputEvents)
{
	PhysXSample::collectInputEvents(inputEvents);

	getApplication().getPlatform()->getSampleUserInput()->unregisterInputEvent(SPAWN_DEBUG_OBJECT);
	getApplication().getPlatform()->getSampleUserInput()->unregisterInputEvent(CAMERA_MOVE_BUTTON);

	//digital keyboard events
	DIGITAL_INPUT_EVENT_DEF(CROUCH,			SCAN_CODE_DOWN,				XKEY_C,						X1KEY_C,					PS3KEY_C,					PS4KEY_C,					AKEY_UNKNOWN,	SCAN_CODE_DOWN,				IKEY_UNKNOWN,	SCAN_CODE_DOWN,		WIIUKEY_UNKNOWN);
	DIGITAL_INPUT_EVENT_DEF(RESET_SCENE,	WKEY_R,						XKEY_R,						X1KEY_R,					PS3KEY_R,					PS4KEY_R,					AKEY_UNKNOWN,	OSXKEY_R,					IKEY_UNKNOWN,	LINUXKEY_R,			WIIUKEY_UNKNOWN);

	//digital gamepad events
	DIGITAL_INPUT_EVENT_DEF(CROUCH,			GAMEPAD_WEST,				GAMEPAD_WEST,				GAMEPAD_WEST,				GAMEPAD_WEST,				GAMEPAD_WEST,				AKEY_UNKNOWN,	GAMEPAD_WEST,				IKEY_UNKNOWN,	LINUXKEY_UNKNOWN,	GAMEPAD_WEST);
	DIGITAL_INPUT_EVENT_DEF(RESET_SCENE,	GAMEPAD_NORTH,				GAMEPAD_NORTH,				GAMEPAD_NORTH,				GAMEPAD_NORTH,				GAMEPAD_NORTH,				KEY_UNKNOWN,	GAMEPAD_NORTH,				IKEY_UNKNOWN,	LINUXKEY_UNKNOWN,	GAMEPAD_NORTH);
	DIGITAL_INPUT_EVENT_DEF(THROW_BALL,		GAMEPAD_RIGHT_SHOULDER_BOT, GAMEPAD_RIGHT_SHOULDER_BOT, GAMEPAD_RIGHT_SHOULDER_BOT, GAMEPAD_RIGHT_SHOULDER_BOT, GAMEPAD_RIGHT_SHOULDER_BOT,	AKEY_UNKNOWN,	GAMEPAD_RIGHT_SHOULDER_BOT, IKEY_UNKNOWN,	LINUXKEY_UNKNOWN,	GAMEPAD_RIGHT_SHOULDER_BOT);

	//digital mouse events
	if (!isPaused())
	{
		DIGITAL_INPUT_EVENT_DEF(THROW_BALL,	MOUSE_BUTTON_LEFT, 			XKEY_UNKNOWN, 				X1KEY_UNKNOWN, 				PS3KEY_UNKNOWN, 			PS4KEY_UNKNOWN,				AKEY_UNKNOWN,	MOUSE_BUTTON_LEFT, 			IKEY_UNKNOWN, 	MOUSE_BUTTON_LEFT,	WIIUKEY_UNKNOWN);
	}
    
    //touch events
    TOUCH_INPUT_EVENT_DEF(RESET_SCENE,			"Reset",		ABUTTON_5,			IBUTTON_5);
	TOUCH_INPUT_EVENT_DEF(CROUCH,               "Crouch",		ABUTTON_6,			IBUTTON_6);
	TOUCH_INPUT_EVENT_DEF(THROW_BALL,           "Throw Ball",	AQUICK_BUTTON_1,	IQUICK_BUTTON_1);
}

void SampleNorthPole::onDigitalInputEvent(const SampleFramework::InputEvent& ie, bool val)
{
	switch (ie.m_Id)
	{
	case CROUCH:
		{
			if(val)
			{
				resizeController(mCrouchingSize);
			}
			else
			{
				mDoStandup = true;
			}
		}
		break;
 	case RESET_SCENE:
		{
			if(val)
			{
				resetScene();
			}
		}
		break;
	case THROW_BALL:
		{
			if(val)
			{
				throwBall();
			}
		}
		break;
	}

	PhysXSample::onDigitalInputEvent(ie,val);
}

