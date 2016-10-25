/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __FORCEFIELD_ASSET_PREVIEW_IMPL_H__
#define __FORCEFIELD_ASSET_PREVIEW_IMPL_H__

#include "ApexPreview.h"

#include "ApexSDKIntl.h"
#include "ForceFieldPreview.h"
#include "RenderDebugInterface.h"
#include "ForceFieldAssetImpl.h"

#include "ApexRWLockable.h"
#include "ReadCheck.h"
#include "WriteCheck.h"

namespace nvidia
{
namespace forcefield
{

/*
	APEX asset preview explosion asset.
	Preview.
*/
class ForceFieldAssetPreviewImpl : public ForceFieldPreview, public ApexResource, public ApexPreview, public ApexRWLockable
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	ForceFieldAssetPreviewImpl(const ForceFieldPreviewDesc& PreviewDesc, ApexSDK* myApexSDK, ForceFieldAssetImpl* myForceFieldAsset, AssetPreviewScene* previewScene);
	void					drawForceFieldPreview(void);
	void					drawForceFieldPreviewUnscaled(void);
	void					drawForceFieldPreviewScaled(void);
	void					drawForceFieldPreviewIcon(void);
	void					drawForceFieldBoundaries(void);
	void					drawForceFieldWithCylinder();
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

	void					setDetailLevel(uint32_t detail);

	typedef struct
	{
		float x, y;
	} point2;

private:

	~ForceFieldAssetPreviewImpl();
	PxMat44					mPose;						// the pose for the preview rendering
	ApexSDK*						mApexSDK;					// pointer to the APEX SDK
	ForceFieldAssetImpl*				mAsset;						// our parent ForceField Asset
	RenderDebugInterface*				mApexRenderDebug;			// Pointer to the RenderLines class to draw the
	// preview stuff
	uint32_t					mDrawGroupIconScaled;		// the ApexDebugRenderer draw group for the Icon
	uint32_t					mDrawGroupCylinder;
	uint32_t					mPreviewDetail;				// the detail options of the preview drawing
	float					mIconScale;					// the scale for the icon
	AssetPreviewScene*		mPreviewScene;

	bool							mDrawWithCylinder;

	void							drawIcon(void);
	void							drawMultilinePoint2(const point2* pts, uint32_t numPoints, uint32_t color);
	void							setIconScale(float scale);

	void							setPose(PxMat44 pose);

	void							toggleDrawPreview();
	void							setDrawGroupsPoseScaled();
};

}
} // end namespace nvidia

#endif // __FORCEFIELD_ASSET_PREVIEW_IMPL_H__
