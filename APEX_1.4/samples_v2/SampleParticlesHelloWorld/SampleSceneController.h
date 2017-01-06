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

using namespace physx;
using namespace nvidia;

class SampleSceneController : public ISampleController
{
public:
	SampleSceneController(CFirstPersonCamera* camera, ApexController& apex);
	virtual ~SampleSceneController();

	struct AssetDescription
	{
		const char* model;
		const char* uiName;
	};
	static AssetDescription ASSETS[];
	static int getAssetsCount();

	void setCurrentAsset(int);

	int getCurrentAsset()
	{
		return mCurrentAsset;
	}

	virtual void onSampleStart();
	virtual void Animate(double dt);

	void throwCube();

	enum TouchEvent
	{
		ePRESS,
		eDRAG,
		eRELEASE
	};

	void onTouchEvent(TouchEvent touchEvent, float mouseX, float mouseY);

private:
	SampleSceneController& operator= (SampleSceneController&);

	void loadEffectPackageDatabase();

	int mCurrentAsset;

	PxRigidDynamic* mDraggingActor;
	nvidia::PxVec3 mDraggingActorHookLocalPoint;
	nvidia::PxVec3 mDragAttractionPoint;
	float mDragDistance;

	ApexController& mApex;
	CFirstPersonCamera* mCamera;

	EffectPackageActor* mActor;

};

#endif