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
#include "ApexRenderer.h" // for matrix conversion
#include <DirectXMath.h>
#include "XInput.h"
#include "DXUTMisc.h"
#pragma warning(push)
#pragma warning(disable : 4481) // Suppress "nonstandard extension used" warning
#include "DXUTCamera.h"
#pragma warning(pop)

#include "PxPhysicsAPI.h"
#include "PxMath.h"

#include "ApexResourceCallback.h"
#include "PhysXPrimitive.h"

#include "ImpactEmitterAsset.h"
#include "ImpactEmitterActor.h"

using namespace physx;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SampleSceneController::SampleSceneController(CFirstPersonCamera* camera, ApexController& apex)
	: mApex(apex), mCamera(camera), mActor(NULL), mAsset(NULL)
{
}

SampleSceneController::~SampleSceneController()
{
}

void SampleSceneController::onSampleStart()
{
	PX_ASSERT_WITH_MESSAGE(mApex.getModuleParticles(), "Particle dll can't be found or ApexFramework was built withoud particles support");
	if (mApex.getModuleParticles())
	{
		// setup camera
		DirectX::XMVECTORF32 lookAtPt = {0, 2, 0, 0};
		DirectX::XMVECTORF32 eyePt = {0, 5, 10, 0};
		mCamera->SetViewParams(eyePt, lookAtPt);
		mCamera->SetRotateButtons(false, false, true, false);
		mCamera->SetEnablePositionMovement(true);

		// spawn mesh emitter
		mAsset = (nvidia::apex::ImpactEmitterAsset *)mApex.getApexSDK()->getNamedResourceProvider()->getResource(IMPACT_EMITTER_AUTHORING_TYPE_NAME, "testImpactEmitter");
		NvParameterized::Interface *defaultActorDesc = mAsset->getDefaultActorDesc();
		NvParameterized::setParamTransform(*defaultActorDesc, "InitialPose", PxTransform(PxIdentity));
		mActor = (ImpactEmitterActor *)mAsset->createApexActor(*defaultActorDesc, *(mApex.getApexScene()));
	}
}


void SampleSceneController::fire(float mouseX, float mouseY)
{
	PxVec3 eyePos, pickDir;
	mApex.getEyePoseAndPickDir(mouseX, mouseY, eyePos, pickDir);

	if (mApex.getModuleParticles() == NULL || pickDir.normalize() <= 0)
	{
		return;
	}

	PxHitFlags outputFlags;
	outputFlags |= PxHitFlag::ePOSITION;
	outputFlags |= PxHitFlag::eDISTANCE;
	PxRaycastBuffer rcBuffer;
	bool collision = false;

	PxScene* scene = mApex.getApexScene()->getPhysXScene();
	scene->lockRead(__FILE__, __LINE__);
	collision = scene->raycast(eyePos, pickDir, 1e6, rcBuffer, outputFlags);
	scene->unlockRead();
	
	uint32_t surfType = mAsset->querySetID("meshParticleEvent");

	if (mActor)
	{
		mActor->registerImpact(rcBuffer.block.position, pickDir, rcBuffer.block.normal, surfType);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

