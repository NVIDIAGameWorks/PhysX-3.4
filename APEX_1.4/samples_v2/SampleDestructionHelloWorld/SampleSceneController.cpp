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

#include "PxMath.h"

using namespace nvidia;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//												Scenes Setup
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SampleSceneController::AssetDescription SampleSceneController::ASSETS[] = 
{
	{ "Wall_PC", "Wall" },
	{ "arch", "arch" },
	{ "Bunny", "Bunny" },
	{ "block", "block" },
	{ "Fence", "Fence" },
	{ "Post", "Post" },
	{ "Sheet", "Sheet" },
	{ "Moai_PC", "Moai_PC" }
};

int SampleSceneController::getAssetsCount()
{
	return sizeof(ASSETS) / sizeof(ASSETS[0]);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SampleSceneController::SampleSceneController(CFirstPersonCamera* camera, ApexController& apex)
: mApex(apex), mCamera(camera), mActor(NULL)
{
}

SampleSceneController::~SampleSceneController()
{
}

void SampleSceneController::onSampleStart()
{
	// setup camera
	DirectX::XMVECTORF32 lookAtPt = { 0, 10, 0, 0 };
	DirectX::XMVECTORF32 eyePt = { 0, 20, 60, 0 };
	mCamera->SetViewParams(eyePt, lookAtPt);
	mCamera->SetRotateButtons(false, false, true, false);
	mCamera->SetEnablePositionMovement(true);

	// spawn actor
	setCurrentAsset(0);
}

void SampleSceneController::setCurrentAsset(int num)
{
	int assetsCount = getAssetsCount();
	num = nvidia::PxClamp(num, 0, assetsCount - 1);

	mCurrentAsset = num;
	if(mActor != NULL)
	{
		mApex.removeActor(mActor);
		mActor = NULL;
	}
	mActor = mApex.spawnDestructibleActor(ASSETS[num].model);
}

void SampleSceneController::fire(float mouseX, float mouseY)
{
	PxVec3 eyePos, pickDir;
	mApex.getEyePoseAndPickDir(mouseX, mouseY, eyePos, pickDir);

	if (pickDir.normalize() > 0)
	{
		float time;
		PxVec3 normal;
		const int chunkIndex = mActor->rayCast(time, normal, eyePos, pickDir, DestructibleActorRaycastFlags::AllChunks);
		if (chunkIndex != ModuleDestructibleConst::INVALID_CHUNK_INDEX)
		{
			mActor->applyDamage(PX_MAX_F32, 0.0f, eyePos + time * pickDir, pickDir, chunkIndex);
		}
	}
}
