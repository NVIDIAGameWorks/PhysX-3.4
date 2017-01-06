/*
* Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#ifndef SAMPLE_MANAGER_H
#define SAMPLE_MANAGER_H

#include "Utils.h"
#pragma warning(push)
#pragma warning(disable : 4350)
#include <list>
#include <string>


class ISampleController : public IVisualController
{
  public:
	virtual void onInitialize() {}
	virtual void onSampleStart() {}
	virtual void onSampleStop() {}
	virtual void onTerminate() {}
};


class SampleManager
{
  public:
	SampleManager(LPWSTR sampleName);
	void addControllerToFront(ISampleController* controller);
	int run();

  private:
	DeviceManager* mDeviceManager;
	std::list<ISampleController*> mControllers;
	LPWSTR mSampleName;
};


#endif