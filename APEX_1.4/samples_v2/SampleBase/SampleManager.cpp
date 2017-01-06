/*
* Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#include "SampleManager.h"

#include "Utils.h"
#pragma warning(push)
#pragma warning(disable : 4917)
#pragma warning(disable : 4365)
#include "XInput.h"
#include "DXUTMisc.h"
#pragma warning(pop)


SampleManager::SampleManager(LPWSTR sampleName)
: mSampleName(sampleName)
{
	mDeviceManager = new DeviceManager();
}

void SampleManager::addControllerToFront(ISampleController* controller)
{
	mControllers.push_back(controller);
}

int SampleManager::run()
{
	// FirstPersonCamera uses this timer, without it it will be FPS-dependent
	DXUTGetGlobalTimer()->Start();

	for (auto it = mControllers.begin(); it != mControllers.end(); it++)
		mDeviceManager->AddControllerToFront(*it);

	DeviceCreationParameters deviceParams;
	deviceParams.swapChainFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	deviceParams.swapChainSampleCount = 1;
	deviceParams.startFullscreen = false;
	deviceParams.backBufferWidth = 1280;
	deviceParams.backBufferHeight = 720;
#if defined(DEBUG) | defined(_DEBUG)
	deviceParams.createDeviceFlags = D3D11_CREATE_DEVICE_DEBUG;
#endif
	deviceParams.featureLevel = D3D_FEATURE_LEVEL_11_0;

	if (FAILED(mDeviceManager->CreateWindowDeviceAndSwapChain(deviceParams, mSampleName)))
	{
		MessageBox(nullptr, "Cannot initialize the D3D11 device with the requested parameters", "Error",
			MB_OK | MB_ICONERROR);
		return 1;
	}

	for (auto it = mControllers.begin(); it != mControllers.end(); it++)
		(*it)->onInitialize();

	for (auto it = mControllers.begin(); it != mControllers.end(); it++)
		(*it)->onSampleStart();

	mDeviceManager->SetVsyncEnabled(false);
	mDeviceManager->MessageLoop();

	for (auto it = mControllers.begin(); it != mControllers.end(); it++)
		(*it)->onSampleStop();

	for (auto it = mControllers.begin(); it != mControllers.end(); it++)
		(*it)->onTerminate();

	mDeviceManager->Shutdown();
	delete mDeviceManager;

	return 0;
}
