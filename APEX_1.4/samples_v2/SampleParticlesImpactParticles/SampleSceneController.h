/*
* Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#ifndef SCENE_CONTROLLER_H
#define SCENE_CONTROLLER_H

#include "ApexController.h"
#include "SampleManager.h"
#include "PxPhysicsAPI.h"

class CFirstPersonCamera;

namespace nvidia
{
	namespace apex
	{
		class ImpactEmitterActor;
		class ImpactEmitterAsset;
	}
}

using namespace physx;
using namespace nvidia;


class SampleSceneController : public ISampleController
{
public:
	SampleSceneController(CFirstPersonCamera* camera, ApexController& apex);
	virtual ~SampleSceneController();

	virtual void onSampleStart();

	// commands
	void fire(float mouseX, float mouseY);

private:
	SampleSceneController& operator= (SampleSceneController&);

	ApexController& mApex;
	CFirstPersonCamera* mCamera;

	nvidia::apex::ImpactEmitterActor* mActor;
	nvidia::apex::ImpactEmitterAsset* mAsset;
};

#endif