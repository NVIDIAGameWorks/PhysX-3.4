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

	DestructibleActor* getCurrentActor() { return mActor; }

	virtual void onSampleStart();

	// commands
	void throwCube();

	// ui exposed settings:
	float& getCubeVelocity() { return mCubeVelocity; }
	float& getCubeMass() { return mCubeMass; }
	const float* getDamageThreshold() { return &mDamageThreshold; }
	const float getLastImpactDamage() { return mLastImpactDamage; }

private:
	SampleSceneController& operator= (SampleSceneController&);

	class SampleImpactDamageReport : public UserImpactDamageReport
	{
	public:
		SampleImpactDamageReport() : mScene(0) {}
		void setScene(SampleSceneController* scene)
		{
			mScene = scene;
		}
		void onImpactDamageNotify(const ImpactDamageEventData* buffer, uint32_t bufferSize);
	private:
		SampleImpactDamageReport& operator= (SampleImpactDamageReport&);
		SampleSceneController* mScene;
	};
	SampleImpactDamageReport mImpactDamageReport;

	ApexController& mApex;
	CFirstPersonCamera* mCamera;

	DestructibleActor* mActor;

	float mCubeVelocity;
	float mCubeMass;
	float mDamageThreshold;
	float mLastImpactDamage;
	char mLastCubeName[32];
	int mCubesCount;
};

PxFilterFlags DestructionImpactDamageFilterShader(
	PxFilterObjectAttributes attributes0,
	PxFilterData filterData0,
	PxFilterObjectAttributes attributes1,
	PxFilterData filterData1,
	PxPairFlags& pairFlags,
	const void* constantBlock,
	uint32_t constantBlockSize);

#endif