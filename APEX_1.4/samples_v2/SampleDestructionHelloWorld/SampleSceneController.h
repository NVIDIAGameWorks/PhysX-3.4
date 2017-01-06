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

class CFirstPersonCamera;

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

	// commands
	void fire(float mouseX, float mouseY);

private:
	SampleSceneController& operator= (SampleSceneController&);

	int mCurrentAsset;

	ApexController& mApex;
	CFirstPersonCamera* mCamera;

	DestructibleActor* mActor;
};

#endif