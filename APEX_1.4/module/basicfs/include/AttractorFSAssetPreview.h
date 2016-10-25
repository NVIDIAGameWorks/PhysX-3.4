/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __ATTRACTOR_FSPREVIEW_H__
#define __ATTRACTOR_FSPREVIEW_H__

#include "ApexPreview.h"
#include "ApexRWLockable.h"
#include "ApexSDKIntl.h"
#include "AttractorFSPreview.h"
#include "RenderDebugInterface.h"

namespace nvidia
{
namespace basicfs
{

class AttractorFSAsset;

/**
\brief Descriptor for a AttractorFS Preview Asset
*/
class AttractorFSPreviewDesc
{
public:
	AttractorFSPreviewDesc() :
		mPose(PxMat44()),
		mPreviewDetail()
	{
		mPose = PxMat44(PxIdentity);
	};

	/**
	\brief The pose that translates from preview coordinates to world coordinates.
	*/
	PxMat44			mPose;
	/**
	\brief Radius of the attractor.
	*/
	float			mRadius;
	/**
	\brief The detail options of the preview drawing
	*/
	uint32_t			mPreviewDetail;
};

/*
	APEX asset preview asset.
	Preview.
*/
class AttractorFSAssetPreview : public AttractorFSPreview, public ApexResource, public ApexPreview, public ApexRWLockable
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	AttractorFSAssetPreview(const AttractorFSPreviewDesc& PreviewDesc, ApexSDK* myApexSDK, AttractorFSAsset* myAsset, AssetPreviewScene* previewScene);
	void					drawAttractorFSPreview(void);
	void					destroy();

	float					getAttractorRadius(NvParameterized::Interface* assetParams);

	void					setPose(const PxMat44& pose);	// Sets the preview instance's pose.  This may include scaling.
	const PxMat44	getPose() const;

	void					setRadius(float radius);
	const float				getRadius() const;

	void					setDetailLevel(uint32_t detail);

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
	~AttractorFSAssetPreview();

	PxMat44					mPose;						// the pose for the preview rendering
	ApexSDK*						mApexSDK;					// pointer to the APEX SDK
	AttractorFSAsset*				mAsset;						// our parent AttractorFS Asset
	RenderDebugInterface*				mApexRenderDebug;			// Pointer to the RenderLines class to draw the
	AssetPreviewScene*		mPreviewScene;
																// preview stuff
	float					mRadius;					// the radius of the attractor
	uint32_t					mPreviewDetail;				// the detail options of the preview drawing

	int32_t					mDrawGroupBox;

	void drawPreviewShape();
	void drawPreviewAssetInfo();
	void toggleDrawPreview();
	void setDrawGroupsPose();

	void drawInfoLine(uint32_t lineNum, const char* str);
};

}
} // namespace nvidia

#endif // __TURBULENCE_ASSET_PREVIEW_H__
