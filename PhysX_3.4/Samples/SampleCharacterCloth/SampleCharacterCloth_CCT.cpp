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

#include "PxPhysXConfig.h"

#if PX_USE_CLOTH_API

#include "SampleCharacterCloth.h"

#include "SampleCharacterClothCameraController.h"
#include "SampleAllocatorSDKClasses.h"

#include "PxPhysicsAPI.h"
#include "extensions/PxExtensionsAPI.h"

#include "characterkinematic/PxControllerManager.h"
#include "characterkinematic/PxBoxController.h"
#include "characterkinematic/PxCapsuleController.h"


#include "geometry/PxMeshQuery.h"
#include "geometry/PxTriangle.h"

#include "RenderBaseActor.h"
#include "RenderBoxActor.h"
#include "RenderCapsuleActor.h"
#include <SampleUserInputIds.h>
#include <SamplePlatform.h>
#include <SampleUserInput.h>
#include <SampleUserInputDefines.h>

using namespace SampleRenderer;
using namespace SampleFramework;

///////////////////////////////////////////////////////////////////////////////
void SampleCharacterCloth::bufferCCTMotion(const PxVec3& targetDisp, PxReal dtime)
{
	PxVec3 disp = targetDisp;

	if(!isPaused())
	{
		// advance jump controller
		mJump.tick(dtime);

		// modify y component of displacement due to jump status
		disp.y = mJump.getDisplacement(dtime);

		if (mJump.isInFreefall())
		{
			disp.x = 0;
			disp.z = 0;
			mCCTActive = false;
		}
	}

	// buffer the displacement and timestep to feed in onSubstep function
	mCCTDisplacement = disp;
	mCCTTimeStep = dtime;
}

///////////////////////////////////////////////////////////////////////////////
void SampleCharacterCloth::createCCTController()
{
	mControllerManager = PxCreateControllerManager(getActiveScene());

	// use ray casting to position the CCT on the terrain
	PxScene& scene = getActiveScene();
	PxRaycastBuffer hit;
	bool didHit = scene.raycast(PxVec3(0,100,0), PxVec3(0,-1,0), 500.0f, hit, PxHitFlags(0xffff));
	PX_UNUSED(didHit);
	PX_ASSERT(didHit);
	mControllerInitialPosition = hit.block.position + PxVec3(0.0f, 1.6f, 0.0f);

	// create and fill in the descriptor for the capsule controller
	PxCapsuleControllerDesc cDesc;
	
	cDesc.height				= 2.0f;
	cDesc.radius				= 0.5f;
	cDesc.material				= &getDefaultMaterial();
	cDesc.position				= PxExtendedVec3(mControllerInitialPosition.x, mControllerInitialPosition.y, mControllerInitialPosition.z);
	cDesc.slopeLimit			= 0.1f;
	cDesc.contactOffset			= 0.01f;
	cDesc.stepOffset			= 0.1f;
	cDesc.scaleCoeff			= 0.9f;
	cDesc.climbingMode			= PxCapsuleClimbingMode::eEASY;
	cDesc.invisibleWallHeight	= 0.0f;
	cDesc.maxJumpHeight			= 2.0f;
	cDesc.reportCallback		= this;

	mController = static_cast<PxCapsuleController*>(mControllerManager->createController(cDesc));
	
	// create camera
	mCCTCamera = SAMPLE_NEW(SampleCharacterClothCameraController)(*mController, *this);

	setCameraController(mCCTCamera);
}


///////////////////////////////////////////////////////////////////////////////
void SampleCharacterCloth::onShapeHit(const PxControllerShapeHit& hit)
{
	PxRigidDynamic* hitActor = hit.shape->getActor()->is<PxRigidDynamic>();

	const size_t nbPlatforms = mPlatforms.size();
	for(PxU32 i = 0;i < nbPlatforms;i++)
	{
		if (mPlatforms[i]->getType() == SampleCharacterClothPlatform::ePLATFORM_TYPE_TRIGGERED)
		{
			PxRigidDynamic* actor = mPlatforms[i]->getPhysicsActor();
			if (hitActor == actor)
				mPlatforms[i]->setActive(true);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
void SampleCharacterCloth::onControllerHit(const PxControllersHit& hit)
{
}

///////////////////////////////////////////////////////////////////////////////
void SampleCharacterCloth::onObstacleHit(const PxControllerObstacleHit& hit)
{
}

///////////////////////////////////////////////////////////////////////////////
PxControllerBehaviorFlags SampleCharacterCloth::getBehaviorFlags(const PxShape&, const PxActor&)
{
	return PxControllerBehaviorFlags(0);
}

///////////////////////////////////////////////////////////////////////////////
PxControllerBehaviorFlags SampleCharacterCloth::getBehaviorFlags(const PxController&)
{
	return PxControllerBehaviorFlags(0);
}

///////////////////////////////////////////////////////////////////////////////
PxControllerBehaviorFlags SampleCharacterCloth::getBehaviorFlags(const PxObstacle& obstacle)
{
	return PxControllerBehaviorFlag::eCCT_CAN_RIDE_ON_OBJECT;
}

///////////////////////////////////////////////////////////////////////////////
void SampleCharacterCloth::updateCCT(float dtime)
{
	// compute target cct displacment for this time step
	PxVec3 dispCurStep = (mCCTTimeStep > 0.0f) ? mCCTDisplacement * (dtime / mCCTTimeStep) : PxVec3(0.0f);

	// limit horizontal acceleration so that character motion does not produce abrupt impulse
	PxVec3 accel = dispCurStep - mCCTDisplacementPrevStep;
	const PxReal accelLimit = 0.3f * dtime;
	PxVec3 accelHorizontal(accel.x, 0, accel.z);
	PxReal accelHorizontalMagnitude = accelHorizontal.magnitude();
	if (accelHorizontalMagnitude > accelLimit)
	{
		const PxReal accelScale = accelLimit / accelHorizontalMagnitude;
		accel.x *= accelScale;
		accel.z *= accelScale;
	}
	
	dispCurStep = mCCTDisplacementPrevStep + accel;
	mCCTDisplacementPrevStep = dispCurStep;
	
	// move the CCT and change jump status due to contact
	const PxU32 flags = mController->move(dispCurStep, 0.01f, dtime, PxControllerFilters());
	if(flags & PxControllerCollisionFlag::eCOLLISION_DOWN)
		mJump.reset();

	// update camera controller
	mCCTCamera->updateFromCCT(dtime);

}

#endif // PX_USE_CLOTH_API
