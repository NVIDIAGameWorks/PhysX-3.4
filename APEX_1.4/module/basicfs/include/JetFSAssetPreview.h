/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __JET_ASSET_PREVIEW_H__
#define __JET_ASSET_PREVIEW_H__

#include "ApexPreview.h"
#include "ApexRWLockable.h"
#include "ApexSDKIntl.h"
#include "JetFSPreview.h"
#include "RenderDebugInterface.h"

namespace nvidia
{
namespace basicfs
{

class JetFSAsset;

/**
\brief Descriptor for a JetFS Preview Asset
*/
class JetFSPreviewDesc
{
public:
	JetFSPreviewDesc() :
		mPose(PxMat44()),
		mPreviewDetail(APEX_JET::JET_DRAW_FULL_DETAIL)
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
class JetFSAssetPreview : public JetFSPreview, public ApexResource, public ApexPreview, public ApexRWLockable
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	JetFSAssetPreview(const JetFSPreviewDesc& PreviewDesc, ApexSDK* myApexSDK, JetFSAsset* myAsset, AssetPreviewScene* previewScene);
	void					drawJetFSPreview(void);
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

	~JetFSAssetPreview();
	PxMat44					mPose;						// the pose for the preview rendering
	ApexSDK*						mApexSDK;					// pointer to the APEX SDK
	JetFSAsset*						mAsset;						// our parent JetFS Asset
	RenderDebugInterface*				mApexRenderDebug;			// Pointer to the RenderLines class to draw the
	PxMat33					mDirToWorld;
	AssetPreviewScene*		mPreviewScene;

	// preview stuff
	uint32_t					mPreviewDetail;				// the detail options of the preview drawing

	int32_t					mDrawGroupShape;
	int32_t					mDrawGroupTorus;

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

#endif // __TURBULENCE_ASSET_PREVIEW_H__
