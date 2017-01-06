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

#include "characterkinematic/PxControllerManager.h"

#include "PxPhysicsAPI.h"

#include "SampleNorthPole.h"
#include "SampleNorthPoleCameraController.h"

#include "SampleUtils.h"
#include "SampleCommandLine.h"
#include "SampleAllocatorSDKClasses.h"
#include "RendererMemoryMacros.h"
#include "RenderMaterial.h"
#include "SampleNorthPoleInputEventIds.h"

#include <SamplePlatform.h>
#include <SampleUserInput.h>

REGISTER_SAMPLE(SampleNorthPole, "SampleNorthPole")

using namespace SampleFramework;
using namespace SampleRenderer;

///////////////////////////////////////////////////////////////////////////////

SampleNorthPole::SampleNorthPole(PhysXSampleApplication& app) :
	PhysXSample					(app),
	mNorthPoleCamera			(NULL),
	mController					(NULL),
	mControllerManager			(NULL),
	mDoStandup					(false)
{
	mCreateGroundPlane	= false;
	//mStepperType = FIXED_STEPPER;
	mStandingSize = 1.0f;
	mCrouchingSize = 0.20f;
	mControllerRadius = 0.3f;
	mControllerInitialPosition = PxExtendedVec3(5,mStandingSize,5);

	memset(mSnowBalls,0,sizeof(mSnowBalls));
	memset(mSnowBallsRenderActors,0,sizeof(mSnowBallsRenderActors));
}

SampleNorthPole::~SampleNorthPole()
{
}

///////////////////////////////////////////////////////////////////////////////

void SampleNorthPole::customizeSample(SampleSetup& setup)
{
	setup.mName	= "SampleNorthPole";
}

///////////////////////////////////////////////////////////////////////////////

void SampleNorthPole::onInit()
{
	PhysXSample::onInit();

	PxSceneWriteLock scopedLock(*mScene);

	mApplication.setMouseCursorHiding(true);
	mApplication.setMouseCursorRecentering(true);

	getRenderer()->setAmbientColor(RendererColor(100, 100, 100));

	// some colors for the rendering
	mSnowMaterial	= SAMPLE_NEW(RenderMaterial)(*getRenderer(), PxVec3(0.85f, 0.85f, 0.95f), 1.0f, false, 0, NULL);
	mCarrotMaterial = SAMPLE_NEW(RenderMaterial)(*getRenderer(), PxVec3(1.00f, 0.50f, 0.00f), 1.0f, false, 1, NULL);
	mButtonMaterial = SAMPLE_NEW(RenderMaterial)(*getRenderer(), PxVec3(0.00f, 0.00f, 0.00f), 1.0f, false, 2, NULL);

	mRenderMaterials.push_back(mSnowMaterial);
	mRenderMaterials.push_back(mCarrotMaterial);
	mRenderMaterials.push_back(mButtonMaterial);

	
	// PhysX
	buildHeightField();
	buildIglooTriMesh();

	// add some stand-up characters
	cookCarrotConvexMesh();

	createSnowMen();

	mControllerManager = PxCreateControllerManager(getActiveScene());

	mController = createCharacter(mControllerInitialPosition);

	mNorthPoleCamera = SAMPLE_NEW(SampleNorthPoleCameraController)(*mController,*this);
	setCameraController(mNorthPoleCamera);

	mNorthPoleCamera->setView(0,0);
}

void SampleNorthPole::onShutdown()
{
	{
		PxSceneWriteLock scopedLock(*mScene);
		DELETESINGLE(mNorthPoleCamera);
		mControllerManager->release();
	}
	
	PhysXSample::onShutdown();
}

void SampleNorthPole::helpRender(PxU32 x, PxU32 y, PxU8 textAlpha)
{
	Renderer* renderer = getRenderer();
	const PxU32 yInc = 18;
	const PxReal scale = 0.5f;
	const PxReal shadowOffset = 6.0f;
	const RendererColor textColor(255, 255, 255, textAlpha);
	const bool isMouseSupported = getApplication().getPlatform()->getSampleUserInput()->mouseSupported();
	const bool isPadSupported = getApplication().getPlatform()->getSampleUserInput()->gamepadSupported();
	const char* msg;

	if (isMouseSupported && isPadSupported)
		renderer->print(x, y += yInc, "Use mouse or right stick to rotate the camera", scale, shadowOffset, textColor);
	else if (isMouseSupported)
		renderer->print(x, y += yInc, "Use mouse to rotate the camera", scale, shadowOffset, textColor);
	else if (isPadSupported)
		renderer->print(x, y += yInc, "Use right stick to rotate the camera", scale, shadowOffset, textColor);
	if (isPadSupported)
		renderer->print(x, y += yInc, "Use left stick to move",scale, shadowOffset, textColor);
	msg = mApplication.inputMoveInfoMsg("Press "," to move", CAMERA_MOVE_FORWARD,CAMERA_MOVE_BACKWARD, CAMERA_MOVE_LEFT, CAMERA_MOVE_RIGHT);
	if(msg)
		renderer->print(x, y += yInc, msg,scale, shadowOffset, textColor);
	msg = mApplication.inputInfoMsg("Press "," to move fast", CAMERA_SHIFT_SPEED, -1);
	if(msg)
		renderer->print(x, y += yInc, msg, scale, shadowOffset, textColor);
	msg = mApplication.inputInfoMsg("Press "," to crouch", CROUCH, -1);
	if(msg)
		renderer->print(x, y += yInc, msg,scale, shadowOffset, textColor);
	msg = mApplication.inputInfoMsg("Press "," to reset scene", RESET_SCENE, -1);
	if(msg)
		renderer->print(x, y += yInc, msg,scale, shadowOffset, textColor);
	msg = mApplication.inputInfoMsg("Press "," to throw a ball", THROW_BALL, -1);
	if(msg)
		renderer->print(x, y += yInc, msg,scale, shadowOffset, textColor);
}

void SampleNorthPole::descriptionRender(PxU32 x, PxU32 y, PxU8 textAlpha)
{
	bool print=(textAlpha!=0.0f);

	if(print)
	{
		Renderer* renderer = getRenderer();
		const PxU32 yInc = 24;
		const PxReal scale = 0.5f;
		const PxReal shadowOffset = 6.0f;
		const RendererColor textColor(255, 255, 255, textAlpha);

		char line0[256]="This sample demonstrates the creation of dynamic objects (snowmen) with";
		char line1[256]="multiple shapes.  The snowmen, though visibly identical, are configured"; 
		char line2[256]="with different masses, inertias and centres of mass in order to show how";
		char line3[256]="to set up these properties using the sdk, and the effect they have on";
		char line4[256]="behavior. A technical description of the snowmen's rigid body properties";
		char line5[256]="can be found in the PhysX Guide documentation in Rigid Body Dynamics:";
		char line6[256]="Mass Properties. The sample also introduces contact notification reports as";
		char line7[256]="a mechanism to detach snowman body parts, and applies ccd flags in order";
		char line8[256]="to prevent small objects such as snowballs and detached snowman body parts ";
		char line9[256]="from tunneling through collision geometry.";
		
		renderer->print(x, y+=yInc, line0, scale, shadowOffset, textColor);
		renderer->print(x, y+=yInc, line1, scale, shadowOffset, textColor);
		renderer->print(x, y+=yInc, line2, scale, shadowOffset, textColor);
		renderer->print(x, y+=yInc, line3, scale, shadowOffset, textColor);
		renderer->print(x, y+=yInc, line4, scale, shadowOffset, textColor);
		renderer->print(x, y+=yInc, line5, scale, shadowOffset, textColor);
		renderer->print(x, y+=yInc, line6, scale, shadowOffset, textColor);
		renderer->print(x, y+=yInc, line7, scale, shadowOffset, textColor);
		renderer->print(x, y+=yInc, line8, scale, shadowOffset, textColor);
		renderer->print(x, y+=yInc, line9, scale, shadowOffset, textColor);
	}
}

void SampleNorthPole::onTickPreRender(PxReal dtime)
{
	if(mDoStandup)
		tryStandup();

	PhysXSample::onTickPreRender(dtime);
}

void SampleNorthPole::onSubstep(float dtime)
{
	detach();
}

void SampleNorthPole::onPointerInputEvent(const SampleFramework::InputEvent& ie, physx::PxU32 x, physx::PxU32 y, physx::PxReal dx, physx::PxReal dy, bool val)
{
	if((ie.m_Id == THROW_BALL) && val)
	{
		throwBall();
	}

	if((ie.m_Id == RAYCAST_HIT) && val)
	{
		PxRaycastBuffer hit;
		getActiveScene().raycast(getCamera().getPos()+getCamera().getViewDir(), getCamera().getViewDir(), 1.0f, hit);
		shdfnd::printFormatted("hits: %p\n",hit.block.shape);
	}

	PhysXSample::onPointerInputEvent(ie,x,y,dx,dy,val);
}

///////////////////////////////////////////////////////////////////////////////
