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

#include "PxPhysicsAPI.h"

#include "characterkinematic/PxControllerManager.h"
#include "characterkinematic/PxBoxController.h"
#include "characterkinematic/PxCapsuleController.h"

#include "geometry/PxMeshQuery.h"
#include "geometry/PxTriangle.h"

#include "SampleCustomGravity.h"
#include "SampleCustomGravityCameraController.h"
#include "SampleCCTActor.h"
#include "SampleCustomGravityInputEventIds.h"
#include <SampleUserInputDefines.h>
#include <SamplePlatform.h>
#include <SampleUserInputIds.h>

#include "RenderBaseActor.h"
#include "RenderBoxActor.h"
#include "RenderCapsuleActor.h"



using namespace SampleRenderer;
using namespace SampleFramework;

#ifdef USE_BOX_CONTROLLER
PxBoxController* SampleCustomGravity::createCharacter(const PxExtendedVec3& position)
#else
PxCapsuleController* SampleCustomGravity::createCharacter(const PxExtendedVec3& position)
#endif
{
	const float height = 2.0f;		
//	const float height = 1e-6f;	// PT: TODO: make it work with 0?

#ifdef USE_BOX_CONTROLLER
	PxBoxControllerDesc cDesc;
	cDesc.halfHeight			= height;
	cDesc.halfSideExtent		= mControllerRadius;
	cDesc.halfForwardExtent		= mControllerRadius;
#else
	PxCapsuleControllerDesc cDesc;
	cDesc.height				= height;
	cDesc.radius				= mControllerRadius;
#endif
	cDesc.material				= &getDefaultMaterial();
	cDesc.position				= position;
	cDesc.slopeLimit			= SLOPE_LIMIT;
	cDesc.contactOffset			= CONTACT_OFFSET;
	cDesc.stepOffset			= STEP_OFFSET;
	cDesc.invisibleWallHeight	= INVISIBLE_WALLS_HEIGHT;
	cDesc.maxJumpHeight			= MAX_JUMP_HEIGHT;
	cDesc.reportCallback		= this;

	mControllerInitialPosition = cDesc.position;

#ifdef USE_BOX_CONTROLLER
	PxBoxController* ctrl = static_cast<PxBoxController*>(mControllerManager->createController(cDesc));
#else
	PxCapsuleController* ctrl = static_cast<PxCapsuleController*>(mControllerManager->createController(cDesc));
#endif
	// remove controller shape from scene query for standup overlap test
	PxRigidDynamic* actor = ctrl->getActor();
	if(actor)
	{
		if(actor->getNbShapes())
		{
			PxShape* ctrlShape;
			actor->getShapes(&ctrlShape,1);
			ctrlShape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, false);

#ifdef USE_BOX_CONTROLLER
			const PxVec3 standingExtents(mControllerRadius, height, mControllerRadius);
 			mRenderActor = SAMPLE_NEW(RenderBoxActor)(*getRenderer(), standingExtents);
#else
 			mRenderActor = SAMPLE_NEW(RenderCapsuleActor)(*getRenderer(), mControllerRadius, height*0.5f);
#endif
			if(mRenderActor)
				mRenderActors.push_back(mRenderActor);
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

void SampleCustomGravity::onShapeHit(const PxControllerShapeHit& hit)
{
	PX_ASSERT(hit.shape);

	// PT: TODO: rewrite this
	if(hit.triangleIndex!=PxU32(-1))
	{
		PxTriangleMeshGeometry meshGeom;
		PxHeightFieldGeometry hfGeom;
		if(hit.shape->getTriangleMeshGeometry(meshGeom))
		{
			PxTriangle touchedTriangle;
			PxMeshQuery::getTriangle(meshGeom, PxShapeExt::getGlobalPose(*hit.shape, *hit.shape->getActor()), hit.triangleIndex, touchedTriangle, NULL/*, NULL, NULL*/);
			mValidTouchedTriangle = true;
			mTouchedTriangle[0] = touchedTriangle.verts[0];
			mTouchedTriangle[1] = touchedTriangle.verts[1];
			mTouchedTriangle[2] = touchedTriangle.verts[2];
		}
		else if(hit.shape->getHeightFieldGeometry(hfGeom))
		{
			PxTriangle touchedTriangle;
			PxMeshQuery::getTriangle(hfGeom, PxShapeExt::getGlobalPose(*hit.shape, *hit.shape->getActor()), hit.triangleIndex, touchedTriangle, NULL/*, NULL, NULL*/);
			mValidTouchedTriangle = true;
			mTouchedTriangle[0] = touchedTriangle.verts[0];
			mTouchedTriangle[1] = touchedTriangle.verts[1];
			mTouchedTriangle[2] = touchedTriangle.verts[2];
		}
	}

	defaultCCTInteraction(hit);
}

void SampleCustomGravity::resetScene()
{
#ifdef USE_BOX_CONTROLLER
	mBoxController->setPosition(mControllerInitialPosition);
#else
	mCapsuleController->setPosition(mControllerInitialPosition);
#endif
	mCCTCamera->setView(0,0);

/*	while(mPhysicsActors.size())
	{
		PxRigidActor* actor = mPhysicsActors.back();
		removeActor(actor);
	}*/
}

void SampleCustomGravity::collectInputEvents(std::vector<const SampleFramework::InputEvent*>& inputEvents)
{
	PhysXSample::collectInputEvents(inputEvents);

	//digital keyboard events
	DIGITAL_INPUT_EVENT_DEF(DRAW_WALLS,				WKEY_B,	XKEY_B,	X1KEY_B,	PS3KEY_B,	PS4KEY_B,	AKEY_UNKNOWN,	OSXKEY_B,	IKEY_UNKNOWN,	LINUXKEY_B,	WIIUKEY_UNKNOWN);
	DIGITAL_INPUT_EVENT_DEF(DEBUG_RENDER,			WKEY_J,	XKEY_J,	X1KEY_J,	PS3KEY_J,	PS4KEY_J,	AKEY_UNKNOWN,	OSXKEY_J,	IKEY_UNKNOWN,	LINUXKEY_J,	WIIUKEY_UNKNOWN);
	DIGITAL_INPUT_EVENT_DEF(RELEASE_TOUCH_SHAPE,	WKEY_T,	XKEY_T,	X1KEY_T,	PS3KEY_T,	PS4KEY_T,	AKEY_UNKNOWN,	OSXKEY_T,	IKEY_UNKNOWN,	LINUXKEY_T,	WIIUKEY_UNKNOWN);

	//digital mouse events
	DIGITAL_INPUT_EVENT_DEF(RAYCAST_HIT,			MOUSE_BUTTON_RIGHT,	XKEY_UNKNOWN,	X1KEY_UNKNOWN, PS3KEY_UNKNOWN,	PS4KEY_UNKNOWN,	AKEY_UNKNOWN,	MOUSE_BUTTON_RIGHT,	IKEY_UNKNOWN, MOUSE_BUTTON_RIGHT,	WIIUKEY_UNKNOWN);

    //touch events
    TOUCH_INPUT_EVENT_DEF(SPAWN_DEBUG_OBJECT,	"Throw Object",		ABUTTON_5,	IBUTTON_5);
}

void SampleCustomGravity::onDigitalInputEvent(const SampleFramework::InputEvent& ie, bool val)
{
	if(val)
	{
		switch(ie.m_Id)
		{
		case RESET_SCENE:
			{
				resetScene();
			}
			break;
		case DRAW_WALLS:
			{
				mDrawInvisibleWalls = !mDrawInvisibleWalls;
			}
			break;
		case DEBUG_RENDER:
			{
				mEnableCCTDebugRender = !mEnableCCTDebugRender; 
			}
			break;
		case RELEASE_TOUCH_SHAPE:
			{
#ifndef USE_BOX_CONTROLLER
				PxControllerState state;
				mCapsuleController->getState(state);
				if (state.touchedShape && (!state.touchedShape->getActor()->is<PxRigidStatic>()))
				{
					PxRigidActor* actor = state.touchedShape->getActor();

					std::vector<PxRigidDynamic*>::iterator i;
					for(i=mDebugActors.begin(); i != mDebugActors.end(); i++)
					{
						if ((*i) == actor)
						{
							mDebugActors.erase(i);
							break;
						}
					}
					removeActor(actor);
				}
#endif
			}
			break;
		default:
			break;
		};
	}

	PhysXSample::onDigitalInputEvent(ie,val);
}
