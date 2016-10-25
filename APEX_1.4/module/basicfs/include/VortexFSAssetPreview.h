/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __VORTEX_FSPREVIEW_H__
#define __VORTEX_FSPREVIEW_H__

#include "ApexPreview.h"
#include "ApexRWLockable.h"
#include "ApexSDKIntl.h"
#include "VortexFSPreview.h"
#include "RenderDebugInterface.h"

namespace nvidia
{
namespace basicfs
{

class VortexFSAsset;

/**
\brief Descriptor for a VortexFS Preview Asset
*/
class VortexFSPreviewDesc
{
public:
	VortexFSPreviewDesc() :
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
class VortexFSAssetPreview : public VortexFSPreview, public ApexResource, public ApexPreview, public ApexRWLockable
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	VortexFSAssetPreview(const VortexFSPreviewDesc& PreviewDesc, ApexSDK* myApexSDK, VortexFSAsset* myAsset, AssetPreviewScene* previewScene);
	void					drawVortexFSPreview(void);
	void					destroy();

	float					getVortexRadius(NvParameterized::Interface* assetParams);

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
	~VortexFSAssetPreview();

	PxMat44					mPose;						// the pose for the preview rendering
	ApexSDK*						mApexSDK;					// pointer to the APEX SDK
	VortexFSAsset*				mAsset;						// our parent VortexFS Asset
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
