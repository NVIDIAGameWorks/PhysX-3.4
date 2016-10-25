/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __WIND_ASSET_PREVIEW_H__
#define __WIND_ASSET_PREVIEW_H__

#include "ApexPreview.h"
#include "ApexRWLockable.h"
#include "ApexSDKIntl.h"
#include "WindFSPreview.h"
#include "RenderDebugInterface.h"

namespace nvidia
{
namespace basicfs
{

class WindFSAsset;

/**
\brief Descriptor for a WindFS Preview Asset
*/
class WindFSPreviewDesc
{
public:
	WindFSPreviewDesc() :
		mPose(PxMat44()),
		mPreviewDetail(APEX_WIND::WIND_DRAW_FULL_DETAIL)
	{
		mPose = PxMat44(PxIdentity);
	};

	/**
	\brief The pose that translates from preview coordinates to world coordinates.
	*/
	PxMat44			mPose;
	/**
	\brief The detail options of the preview drawing
	*/
	uint32_t			mPreviewDetail;
};

/*
	APEX asset preview asset.
	Preview.
*/
class WindFSAssetPreview : public WindFSPreview, public ApexResource, public ApexPreview, public ApexRWLockable
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	WindFSAssetPreview(const WindFSPreviewDesc& PreviewDesc, ApexSDK* myApexSDK, WindFSAsset* myAsset, AssetPreviewScene* previewScene);
	void					destroy();

	void					setPose(const PxMat44& pose);	// Sets the preview instance's pose.  This may include scaling.
	const PxMat44	getPose() const;

	// from RenderDataProvider
	void					lockRenderResources(void);
	void					unlockRenderResources(void);
	void					updateRenderResources(bool rewriteBuffers = false, void* userRenderData = 0);

	// from Renderable.h
	void					dispatchRenderResources(UserRenderer& renderer);
	PxBounds3				getBounds(void) const;

	// from ApexResource.h
	void					release(void);

private:

	~WindFSAssetPreview();
	PxMat44					mPose;						// the pose for the preview rendering
	ApexSDK*						mApexSDK;					// pointer to the APEX SDK
	WindFSAsset*					mAsset;						// our parent WindFS Asset
	RenderDebugInterface*				mApexRenderDebug;			// Pointer to the RenderLines class to draw the
	AssetPreviewScene*		mPreviewScene;

	// preview stuff
	uint32_t					mPreviewDetail;				// the detail options of the preview drawing


//	void							setHalfLengthDimensions(PxVec3 halfLenDim);
	void							setDetailLevel(uint32_t detail);

	void							drawPreviewAssetInfo();
	void							drawInfoLine(uint32_t lineNum, const char* str);
};

}
} // namespace nvidia

#endif // __NOISE_ASSET_PREVIEW_H__
