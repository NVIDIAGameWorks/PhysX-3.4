/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef APEX_ASSET_PREVIEW_SCENE_H
#define APEX_ASSET_PREVIEW_SCENE_H

#include "Apex.h"
#include "ApexResource.h"
#include "PsUserAllocated.h"
#include "ApexSDKImpl.h"
#include "AssetPreviewScene.h"
#include "ModuleIntl.h"
#include "ApexContext.h"
#include "PsMutex.h"

#if PX_PHYSICS_VERSION_MAJOR == 3
#include "PxScene.h"
#include "PxRenderBuffer.h"
#endif

#include "ApexSceneUserNotify.h"

#include "PsSync.h"
#include "PxTask.h"
#include "PxTaskManager.h"

#include "ApexGroupsFiltering.h"
#include "ApexRWLockable.h"

namespace nvidia
{
namespace apex
{

class ApexAssetPreviewScene : public AssetPreviewScene, public ApexRWLockable, public UserAllocated
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	ApexAssetPreviewScene(ApexSDKImpl* sdk);
	virtual ~ApexAssetPreviewScene() {}

	//Sets the view matrix. Should be called whenever the view matrix needs to be updated.
	virtual void					setCameraMatrix(const PxMat44& viewTransform);
		
	//Returns the view matrix set by the user for the given viewID.
	virtual PxMat44			getCameraMatrix() const;

	virtual void					setShowFullInfo(bool showFullInfo);

	virtual bool					getShowFullInfo() const;

	virtual void					release();

	void							destroy();

private:
	ApexSDKImpl*						mApexSDK;

	PxMat44					mCameraMatrix;				// the pose for the preview rendering
	bool							mShowFullInfo;
};

}
} // end namespace nvidia::apex

#endif // APEX_ASSET_PREVIEW_SCENE_H
