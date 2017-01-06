/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "Apex.h"
#include "ApexDefs.h"
#include "ApexAssetPreviewScene.h"
#include "ApexSceneTasks.h"
#include "ApexSDKImpl.h"
#include "ApexActor.h"
#include "FrameworkPerfScope.h"
#include "ApexRenderDebug.h"
#include "ModuleIntl.h"
#include "ApexPvdClient.h"
#include "PsTime.h"
#include "ApexUsingNamespace.h"
#include "PsSync.h"
#include "PxTask.h"
#include "PxTaskManager.h"
#include "PxGpuDispatcher.h"
#include "PxCudaContextManager.h"


namespace nvidia
{
namespace apex
{

	ApexAssetPreviewScene::ApexAssetPreviewScene(ApexSDKImpl* sdk) : mApexSDK(sdk)
	, mShowFullInfo(false)
	{
		mCameraMatrix = PxMat44(PxIdentity);
	}

	void ApexAssetPreviewScene::setCameraMatrix(const PxMat44& cameraMatrix)
	{
		mCameraMatrix = cameraMatrix;
	}

	PxMat44 ApexAssetPreviewScene::getCameraMatrix() const
	{
		return mCameraMatrix;
	}

	void ApexAssetPreviewScene::setShowFullInfo(bool showFullInfo)
	{
		mShowFullInfo = showFullInfo;
	}

	bool ApexAssetPreviewScene::getShowFullInfo() const
	{
		return mShowFullInfo;
	}

	void ApexAssetPreviewScene::release()
	{
		mApexSDK->releaseAssetPreviewScene(this);
	}

	void ApexAssetPreviewScene::destroy()
	{
		PX_DELETE(this);
	}
}
} // end namespace nvidia::apex
