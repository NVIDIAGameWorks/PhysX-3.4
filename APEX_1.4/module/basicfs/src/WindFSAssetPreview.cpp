/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "ApexDefs.h"
#include "nvparameterized/NvParamUtils.h"
#include "WindFSAsset.h"
#include "WindFSAssetParams.h"
#include "WindFSPreview.h"
#include "WindFSAssetPreview.h"
#include "ModulePerfScope.h"
#include "ApexUsingNamespace.h"

#include "WriteCheck.h"

namespace nvidia
{
namespace basicfs
{

using namespace APEX_WIND;


#define ASSET_INFO_XPOS					(-0.9f)	// left position of the asset info
#define ASSET_INFO_YPOS					( 0.9f)	// top position of the asset info
#define DEBUG_TEXT_HEIGHT				(0.35f)	//in screen space -- would be nice to know this!


void WindFSAssetPreview::drawInfoLine(uint32_t lineNum, const char* str)
{
#ifdef WITHOUT_DEBUG_VISUALIZE
	PX_UNUSED(lineNum);
	PX_UNUSED(str);
#else
	PxMat44 cameraMatrix = mPreviewScene->getCameraMatrix();
	RENDER_DEBUG_IFACE(mApexRenderDebug)->setCurrentColor(RENDER_DEBUG_IFACE(mApexRenderDebug)->getDebugColor(RENDER_DEBUG::DebugColors::Blue));
	PxVec3 textLocation = mPose.getPosition(); 
	textLocation += cameraMatrix.column1.getXYZ() * (ASSET_INFO_YPOS - (lineNum * DEBUG_TEXT_HEIGHT));
	RENDER_DEBUG_IFACE(mApexRenderDebug)->debugText(textLocation, str);
#endif
}

void WindFSAssetPreview::drawPreviewAssetInfo()
{
#ifndef WITHOUT_DEBUG_VISUALIZE
	if (!mApexRenderDebug)
	{
		return;
	}

		char buf[128];
		buf[sizeof(buf) - 1] = 0;

		ApexSimpleString myString;
		ApexSimpleString floatStr;
		uint32_t lineNum = 0;

		RENDER_DEBUG_IFACE(mApexRenderDebug)->pushRenderState();
		RENDER_DEBUG_IFACE(mApexRenderDebug)->addToCurrentState(RENDER_DEBUG::DebugRenderState::NoZbuffer);
		RENDER_DEBUG_IFACE(mApexRenderDebug)->setCurrentTextScale(1.0f);

		// asset name
		APEX_SPRINTF_S(buf, sizeof(buf) - 1, "%s %s", mAsset->getObjTypeName(), mAsset->getName());
		drawInfoLine(lineNum++, buf);
		lineNum++;

		if(mPreviewScene->getShowFullInfo())
		{
			// TODO: cache strings
			WindFSAssetParams& assetParams = *static_cast<WindFSAssetParams*>(mAsset->getAssetNvParameterized());

			APEX_SPRINTF_S(buf, sizeof(buf) - 1, "fieldStrength = %f",
				assetParams.fieldStrength
				);
			drawInfoLine(lineNum++, buf);
		}
		RENDER_DEBUG_IFACE(mApexRenderDebug)->popRenderState();
#endif
}

WindFSAssetPreview::~WindFSAssetPreview(void)
{
}

void WindFSAssetPreview::setPose(const PxMat44& pose)
{
	mPose = pose;
}

const PxMat44 WindFSAssetPreview::getPose() const
{
	return	mPose;
}


// from RenderDataProvider
void WindFSAssetPreview::lockRenderResources(void)
{
	ApexRenderable::renderDataLock();
}

void WindFSAssetPreview::unlockRenderResources(void)
{
	ApexRenderable::renderDataUnLock();
}

void WindFSAssetPreview::updateRenderResources(bool /*rewriteBuffers*/, void* /*userRenderData*/)
{
	if (mApexRenderDebug)
	{
		mApexRenderDebug->updateRenderResources();
	}
}

// from Renderable.h
void WindFSAssetPreview::dispatchRenderResources(UserRenderer& renderer)
{
	if (mApexRenderDebug)
	{
		if (mPreviewDetail & WIND_DRAW_ASSET_INFO)
		{
			drawPreviewAssetInfo();
		}
		mApexRenderDebug->dispatchRenderResources(renderer);
	}
}

PxBounds3 WindFSAssetPreview::getBounds(void) const
{
	if (mApexRenderDebug)
	{
		return mApexRenderDebug->getBounds();
	}
	else
	{
		PxBounds3 b;
		b.setEmpty();
		return b;
	}
}

void WindFSAssetPreview::destroy(void)
{
	delete this;
}

void WindFSAssetPreview::release(void)
{
	if (mInRelease)
	{
		return;
	}
	mInRelease = true;
	mAsset->releaseWindFSPreview(*this);
}

WindFSAssetPreview::WindFSAssetPreview(const WindFSPreviewDesc& PreviewDesc, ApexSDK* myApexSDK, WindFSAsset* myWindFSAsset, AssetPreviewScene* previewScene) :
	mPose(PreviewDesc.mPose),
	mApexSDK(myApexSDK),
	mAsset(myWindFSAsset),
	mPreviewScene(previewScene),
	mPreviewDetail(PreviewDesc.mPreviewDetail),
	mApexRenderDebug(0)
{
};


void WindFSAssetPreview::setDetailLevel(uint32_t detail)
{
	WRITE_ZONE();
	mPreviewDetail = detail;
}

}
} // namespace nvidia
