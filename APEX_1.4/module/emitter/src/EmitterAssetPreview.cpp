/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "EmitterPreview.h"
#include "EmitterAssetPreview.h"

#define ASSET_INFO_XPOS					(-0.9f)	// left position of the asset info
#define ASSET_INFO_YPOS					( 0.9f)	// top position of the asset info
#define DEBUG_TEXT_HEIGHT				(0.4f)	// in screen space

namespace nvidia
{
namespace emitter
{

bool EmitterAssetPreview::isValid() const
{
	return mApexRenderDebug != NULL;
}

void EmitterAssetPreview::drawEmitterPreview(void)
{
	WRITE_ZONE();
#ifndef WITHOUT_DEBUG_VISUALIZE
	if (!mApexRenderDebug)
	{
		return;
	}

	//asset preview init
	if (mGroupID == 0)
	{
		mGroupID = RENDER_DEBUG_IFACE(mApexRenderDebug)->beginDrawGroup(PxMat44(PxIdentity));
		mAsset->mGeom->drawPreview(mScale, mApexRenderDebug);
		RENDER_DEBUG_IFACE(mApexRenderDebug)->endDrawGroup();
	}

	toggleDrawPreview();
	setDrawGroupsPose();
#endif
}

void	EmitterAssetPreview::drawPreviewAssetInfo(void)
{
	if (!mApexRenderDebug)
	{
		return;
	}
		
	char buf[128];
	buf[sizeof(buf) - 1] = 0;

	ApexSimpleString myString;
	ApexSimpleString floatStr;
	uint32_t lineNum = 0;

	using RENDER_DEBUG::DebugColors;
	RENDER_DEBUG_IFACE(mApexRenderDebug)->pushRenderState();
	//	RENDER_DEBUG_IFACE(&mApexRenderDebug)->addToCurrentState(RENDER_DEBUG::DebugRenderState::ScreenSpace);
	RENDER_DEBUG_IFACE(mApexRenderDebug)->addToCurrentState(RENDER_DEBUG::DebugRenderState::NoZbuffer);
	RENDER_DEBUG_IFACE(mApexRenderDebug)->setCurrentTextScale(2.0f);
	RENDER_DEBUG_IFACE(mApexRenderDebug)->setCurrentColor(RENDER_DEBUG_IFACE(mApexRenderDebug)->getDebugColor(DebugColors::Yellow));

	// asset name
	APEX_SPRINTF_S(buf, sizeof(buf) - 1, "%s %s", mAsset->getObjTypeName(), mAsset->getName());
	drawInfoLine(lineNum++, buf);
	lineNum++;
			
	RENDER_DEBUG_IFACE(mApexRenderDebug)->popRenderState();
}


void		EmitterAssetPreview::drawInfoLine(uint32_t lineNum, const char* str)
{
#ifdef WITHOUT_DEBUG_VISUALIZE
	PX_UNUSED(lineNum);
	PX_UNUSED(str);
#else
	using RENDER_DEBUG::DebugColors;
	RENDER_DEBUG_IFACE(mApexRenderDebug)->setCurrentColor(RENDER_DEBUG_IFACE(mApexRenderDebug)->getDebugColor(DebugColors::Green));
	PxVec3 textLocation = mPose.getPosition();
	PxMat44 cameraMatrix = mPreviewScene->getCameraMatrix();
	textLocation += cameraMatrix.column1.getXYZ() * (ASSET_INFO_YPOS - (lineNum * DEBUG_TEXT_HEIGHT));
	RENDER_DEBUG_IFACE(mApexRenderDebug)->debugText(textLocation, str);	
#endif
}


void EmitterAssetPreview::destroy(void)
{
	ApexPreview::destroy();
	delete this;
}

EmitterAssetPreview::~EmitterAssetPreview(void)
{
}

void EmitterAssetPreview::setScale(float scale)
{
	WRITE_ZONE();
	mScale = scale;
	drawEmitterPreview();
}

void EmitterAssetPreview::setPose(const PxMat44& pose)
{
	WRITE_ZONE();
	mPose = pose;
	setDrawGroupsPose();
}

const PxMat44 EmitterAssetPreview::getPose() const
{
	READ_ZONE();
	return mPose;
}

void EmitterAssetPreview::setDrawGroupsPose()
{
	if (mApexRenderDebug)
	{
		RENDER_DEBUG_IFACE(mApexRenderDebug)->setDrawGroupPose(mGroupID, mPose);
	}
}

void EmitterAssetPreview::toggleDrawPreview()
{
	if (mApexRenderDebug)
	{
		//asset preview set visibility
		RENDER_DEBUG_IFACE(mApexRenderDebug)->setDrawGroupVisible(mGroupID, true);
	}
}


// from RenderDataProvider
void EmitterAssetPreview::lockRenderResources(void)
{
	ApexRenderable::renderDataLock();
}

void EmitterAssetPreview::unlockRenderResources(void)
{
	ApexRenderable::renderDataUnLock();
}

void EmitterAssetPreview::updateRenderResources(bool /*rewriteBuffers*/, void* /*userRenderData*/)
{
	if (mApexRenderDebug)
	{
		mApexRenderDebug->updateRenderResources();
	}
}

// from Renderable.h
void EmitterAssetPreview::dispatchRenderResources(UserRenderer& renderer)
{
	if (mApexRenderDebug)
	{
		drawPreviewAssetInfo();
		mApexRenderDebug->dispatchRenderResources(renderer);
	}
}

PxBounds3 EmitterAssetPreview::getBounds(void) const
{
	READ_ZONE();
	return mApexRenderDebug->getBounds();
}

void EmitterAssetPreview::release(void)
{
	if (mInRelease)
	{
		return;
	}
	mInRelease = true;
	const_cast<EmitterAssetImpl*>(mAsset)->releaseEmitterPreview(*this);
}

	EmitterAssetPreview::EmitterAssetPreview(const EmitterPreviewDesc& pdesc, const EmitterAssetImpl& asset, AssetPreviewScene* previewScene, ApexSDK* myApexSDK) :
		mApexSDK(myApexSDK),
		mScale(pdesc.mScale),
		mAsset(&asset),
		mPreviewScene(previewScene),
		mGroupID(0),
		mApexRenderDebug(0)
	{
#ifndef WITHOUT_DEBUG_VISUALIZE
		setPose(pdesc.mPose);
		drawPreviewAssetInfo();
		drawEmitterPreview();
#endif
	};

}
} // namespace nvidia::apex
