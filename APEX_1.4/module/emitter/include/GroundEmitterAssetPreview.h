/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __GROUND_EMITTER_ASSET_PREVIEW_H__
#define __GROUND_EMITTER_ASSET_PREVIEW_H__

#include "ApexPreview.h"

#include "ApexSDKIntl.h"
#include "GroundEmitterPreview.h"
#include "RenderDebugInterface.h"
#include "GroundEmitterAssetImpl.h"
#include "ApexRWLockable.h"

namespace nvidia
{
namespace emitter
{

class GroundEmitterAssetPreview : public GroundEmitterPreview, public ApexResource, public ApexPreview, public ApexRWLockable
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	GroundEmitterAssetPreview(const GroundEmitterPreviewDesc& pdesc, const GroundEmitterAssetImpl& asset, ApexSDK* myApexSDK, AssetPreviewScene* previewScene) :
		mApexSDK(myApexSDK),
		mApexRenderDebug(0),
		mScale(pdesc.mScale),
		mAsset(&asset),
		mPreviewScene(previewScene),
		mGroupID(0)
	{
#ifndef WITHOUT_DEBUG_VISUALIZE
		setPose(pdesc.mPose);
		drawEmitterPreview();
#endif
	};

	bool                isValid() const
	{
		return mApexRenderDebug != NULL;
	}
	void				drawEmitterPreview(void);
	void				destroy();

	void				setPose(const PxMat44& pose);	// Sets the preview instance's pose.  This may include scaling.
	const PxMat44	getPose() const;

	// from RenderDataProvider
	void lockRenderResources(void);
	void unlockRenderResources(void);
	void updateRenderResources(bool rewriteBuffers = false, void* userRenderData = 0);

	// from Renderable.h
	void dispatchRenderResources(UserRenderer& renderer);
	PxBounds3 getBounds(void) const;

	// from ApexResource.h
	void	release(void);

private:
	~GroundEmitterAssetPreview();

	AuthObjTypeID					mModuleID;					// the module ID of Emitter.
	UserRenderResourceManager*	mRrm;						// pointer to the users render resource manager
	ApexSDK*						mApexSDK;					// pointer to the APEX SDK
	RenderDebugInterface*				mApexRenderDebug;			// Pointer to the RenderLines class to draw the
	PxTransform				mPose;						// the pose for the preview rendering
	float					mScale;
	const GroundEmitterAssetImpl*       mAsset;
	int32_t                    mGroupID;
	AssetPreviewScene*		mPreviewScene;

	void							setScale(float scale);
};

}
} // end namespace nvidia

#endif // __APEX_EMITTER_ASSET_PREVIEW_H__
