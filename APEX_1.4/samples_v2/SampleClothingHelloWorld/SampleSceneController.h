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
#include "PhysXPrimitive.h"

#include <map>

class CFirstPersonCamera;

class SampleSceneController : public ISampleController
{
public:
	SampleSceneController(CFirstPersonCamera* camera, ApexController& apex);
	virtual ~SampleSceneController();

	virtual void onSampleStart();
	virtual void Animate(double dt);

	// commands
	void throwSphere();

private:

	SampleSceneController& operator= (SampleSceneController&);

	std::map<PxRigidDynamic*, ClothingSphere*> mSpheres;

	ApexController& mApex;
	CFirstPersonCamera* mCamera;

	ClothingActor* mActor;
	double mTime;
};

#endif