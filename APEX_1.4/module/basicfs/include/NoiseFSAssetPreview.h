/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __NOISE_ASSET_PREVIEW_H__
#define __NOISE_ASSET_PREVIEW_H__

#include "ApexPreview.h"
#include "ApexRWLockable.h"
#include "ApexSDKIntl.h"
#include "NoiseFSPreview.h"
#include "RenderDebugInterface.h"

namespace nvidia
{
namespace basicfs
{

class NoiseFSAsset;

/**
\brief Descriptor for a NoiseFS Preview Asset
*/
class NoiseFSPreviewDesc
{
public:
	NoiseFSPreviewDesc() :
		mPose(PxMat44()),
		mPreviewDetail(APEX_NOISE::NOISE_DRAW_FULL_DETAIL)
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
class NoiseFSAssetPreview : public NoiseFSPreview, public ApexResource, public ApexPreview, public ApexRWLockable
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	NoiseFSAssetPreview(const NoiseFSPreviewDesc& PreviewDesc, ApexSDK* myApexSDK, NoiseFSAsset* myAsset, AssetPreviewScene* previewScene);
	void					drawNoiseFSPreview(void);
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

	~NoiseFSAssetPreview();
	PxMat44					mPose;						// the pose for the preview rendering
	ApexSDK*						mApexSDK;					// pointer to the APEX SDK
	NoiseFSAsset*					mAsset;						// our parent NoiseFS Asset
	RenderDebugInterface*				mApexRenderDebug;			// Pointer to the RenderLines class to draw the
	AssetPreviewScene*		mPreviewScene;

	// preview stuff
	uint32_t					mPreviewDetail;				// the detail options of the preview drawing

	int32_t					mDrawGroupShape;

//	void							setHalfLengthDimensions(PxVec3 halfLenDim);
	void							setDetailLevel(uint32_t detail);

	void							drawPreviewAssetInfo();
	void							drawShape(/*uint32_t color*/);
	void							toggleDrawPreview();
	void							setDrawGroupsPose();
	void							drawInfoLine(uint32_t lineNum, const char* str);
};

}
} // namespace nvidia

#endif // __NOISE_ASSET_PREVIEW_H__
