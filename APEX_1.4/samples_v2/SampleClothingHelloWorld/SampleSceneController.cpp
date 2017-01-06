/*
* Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#include "SampleSceneController.h"
#include "Apex.h"
#include "ApexRenderer.h" // for matrix conversion
#include <DirectXMath.h>
#include "XInput.h"
#include "DXUTMisc.h"
#pragma warning(push)
#pragma warning(disable : 4481) // Suppress "nonstandard extension used" warning
#include "DXUTCamera.h"
#pragma warning(pop)

#include "ApexResourceCallback.h"
#include "ClothingCollision.h"

#include "PxMath.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SampleSceneController::SampleSceneController(CFirstPersonCamera* camera, ApexController& apex)
: mApex(apex), mCamera(camera), mActor(NULL), mTime(0)
{
}

SampleSceneController::~SampleSceneController()
{
}

void SampleSceneController::onSampleStart()
{
	// setup camera
	DirectX::XMVECTORF32 lookAtPt = { 0, 45, 0, 0 };
	DirectX::XMVECTORF32 eyePt = { 0, 50, 100, 0 };
	mCamera->SetViewParams(eyePt, lookAtPt);
	mCamera->SetRotateButtons(false, false, true, false);
	mCamera->SetEnablePositionMovement(true);

	// spawn actor
	mActor = mApex.spawnClothingActor("plane32");

	nvidia::PxMat44 globalPose = nvidia::PxMat44(nvidia::PxIdentity);
	globalPose(1, 1) = 0;
	globalPose(2, 2) = 0;
	globalPose(1, 2) = 1;
	globalPose(2, 1) = -1;

	globalPose.setPosition(PxVec3(0, -40, 0));

	mActor->updateState(globalPose, NULL, 0, 0, ClothingTeleportMode::TeleportAndReset);
}

void SampleSceneController::Animate(double dt)
{
	for (std::map<PxRigidDynamic*, ClothingSphere*>::iterator it = mSpheres.begin(); it != mSpheres.end(); it++)
	{
		(*it).second->setPosition((*it).first->getGlobalPose().p);
	}

	mTime += dt;

	mActor->setWind(0.9f, PxVec3(30 + 7 * PxSin(mTime), 0, 40 + 10 * PxCos(mTime)));
}

void SampleSceneController::throwSphere()
{
	PxVec3 eyePos = XMVECTORToPxVec4(mCamera->GetEyePt()).getXYZ();
	PxVec3 lookAtPos = XMVECTORToPxVec4(mCamera->GetLookAtPt()).getXYZ();
	PhysXPrimitive* box = mApex.spawnPhysXPrimitiveBox(PxTransform(eyePos));
	PxRigidDynamic* rigidDynamic = box->getActor()->is<PxRigidDynamic>();
	box->setColor(DirectX::XMFLOAT3(1, 1, 1));

	PxVec3 dir = (lookAtPos - eyePos).getNormalized();
	rigidDynamic->setLinearVelocity(dir * 50);

	ClothingSphere* sphere = mActor->createCollisionSphere(rigidDynamic->getGlobalPose().p, 5);
	mSpheres[rigidDynamic] = sphere;
}
