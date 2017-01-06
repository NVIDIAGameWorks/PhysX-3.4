/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __EMITTER_ASSET_PREVIEW_H__
#define __EMITTER_ASSET_PREVIEW_H__

#include "ApexPreview.h"

#include "ApexSDKIntl.h"
#include "EmitterPreview.h"
#include "RenderDebugInterface.h"
#include "EmitterAssetImpl.h"
#include "ApexRWLockable.h"

namespace nvidia
{
namespace emitter
{

class EmitterAssetPreview : public EmitterPreview, public ApexResource, public ApexPreview, public ApexRWLockable
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	EmitterAssetPreview(const EmitterPreviewDesc& pdesc, const EmitterAssetImpl& asset, AssetPreviewScene* previewScene, ApexSDK* myApexSDK);

	bool					isValid() const;

	void					drawEmitterPreview(void);
	void					drawPreviewAssetInfo(void);
	void					drawInfoLine(uint32_t lineNum, const char* str);
	void					destroy();

	void					setPose(const PxMat44& pose);	// Sets the preview instance's pose.  This may include scaling.
	const PxMat44	getPose() const;

	// from RenderDataProvider
	void					lockRenderResources(void);
	void					unlockRenderResources(void);
	void					updateRenderResources(bool rewriteBuffers = false, void* userRenderData = 0);

	// from Renderable.h
	void					dispatchRenderResources(UserRenderer& renderer);
	PxBounds3		getBounds(void) const;

	// from ApexResource.h
	void					release(void);

private:
	~EmitterAssetPreview();

	void					toggleDrawPreview();
	void					setDrawGroupsPose();


	AuthObjTypeID					mModuleID;					// the module ID of Emitter.
	ApexSDK*						mApexSDK;					// pointer to the APEX SDK
	RenderDebugInterface*				mApexRenderDebug;			// Pointer to the RenderLines class to draw the
	PxMat44					mPose;						// the pose for the preview rendering
	float					mScale;
	const EmitterAssetImpl*         mAsset;
	int32_t                    mGroupID;
	AssetPreviewScene*		mPreviewScene;

	void							setScale(float scale);
};

}
} // end namespace nvidia

#endif // __EMITTER_ASSET_PREVIEW_H__
