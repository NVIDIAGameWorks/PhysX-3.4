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
#include "NoiseFSAsset.h"
#include "NoiseFSAssetParams.h"
#include "NoiseFSPreview.h"
#include "NoiseFSAssetPreview.h"
#include "ModulePerfScope.h"
#include "ApexUsingNamespace.h"

#include "ReadCheck.h"
#include "WriteCheck.h"

namespace nvidia
{
namespace basicfs
{

using namespace APEX_NOISE;

void NoiseFSAssetPreview::drawNoiseFSPreview(void)
{
	PX_PROFILE_ZONE("NoiseFSDrawPreview", GetInternalApexSDK()->getContextId());
	if (mPreviewDetail & NOISE_DRAW_SHAPE)
	{
		drawShape();
	}

	toggleDrawPreview();
	setDrawGroupsPose();
}

#define ASSET_INFO_XPOS					(-0.9f)	// left position of the asset info
#define ASSET_INFO_YPOS					( 0.9f)	// top position of the asset info
#define DEBUG_TEXT_HEIGHT				(0.35f)	//in screen space -- would be nice to know this!


void NoiseFSAssetPreview::drawInfoLine(uint32_t lineNum, const char* str)
{
#ifdef WITHOUT_DEBUG_VISUALIZE
	PX_UNUSED(lineNum);
	PX_UNUSED(str);
#else
	PxMat44 cameraMatrix = mPreviewScene->getCameraMatrix();
	RENDER_DEBUG_IFACE(mApexRenderDebug)->setCurrentColor(RENDER_DEBUG_IFACE(mApexRenderDebug)->getDebugColor(RENDER_DEBUG::DebugColors::Blue));
	PxVec3 textLocation = mPose.getPosition(); 
	textLocation += cameraMatrix.column1.getXYZ() * (ASSET_INFO_YPOS - (lineNum * DEBUG_TEXT_HEIGHT));
	cameraMatrix.setPosition(textLocation);
	RENDER_DEBUG_IFACE(mApexRenderDebug)->debugText(textLocation, str);
#endif
}

void NoiseFSAssetPreview::drawPreviewAssetInfo()
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
			NoiseFSAssetParams& assetParams = *static_cast<NoiseFSAssetParams*>(mAsset->getAssetNvParameterized());

			APEX_SPRINTF_S(buf, sizeof(buf) - 1, "noiseStrength = %f",
				assetParams.noiseStrength
				);
			drawInfoLine(lineNum++, buf);

			// fieldBoundary filter data info
			if (assetParams.fieldBoundaryFilterDataName.buf)
			{
				myString = "FieldBoundary Filter Data = ";
				myString += ApexSimpleString(assetParams.fieldBoundaryFilterDataName.buf);
				drawInfoLine(lineNum++, myString.c_str());
			}

			// implicit info
			myString = "Fade Percentage = ";
			ApexSimpleString::ftoa(assetParams.boundaryFadePercentage, floatStr);
			myString += floatStr;
			drawInfoLine(lineNum++, myString.c_str());
		}
		RENDER_DEBUG_IFACE(mApexRenderDebug)->popRenderState();
#endif
}

void NoiseFSAssetPreview::drawShape()
{
#ifndef WITHOUT_DEBUG_VISUALIZE
	if (!mApexRenderDebug)
	{
		return;
	}

	NoiseFSAssetParams& assetParams = *static_cast<NoiseFSAssetParams*>(mAsset->getAssetNvParameterized());
	//asset preview init
	if (mDrawGroupShape == 0)
	{
		mDrawGroupShape = RENDER_DEBUG_IFACE(mApexRenderDebug)->beginDrawGroup(PxMat44(PxIdentity));

		//debug visualization
		RENDER_DEBUG_IFACE(mApexRenderDebug)->setCurrentColor(RENDER_DEBUG_IFACE(mApexRenderDebug)->getDebugColor(RENDER_DEBUG::DebugColors::DarkBlue));


		PxVec3 shapeSides = assetParams.defaultScale * assetParams.boundarySize;
		RENDER_DEBUG_IFACE(mApexRenderDebug)->setPose(mPose);
		RENDER_DEBUG_IFACE(mApexRenderDebug)->debugBound( PxBounds3(PxVec3(0.0f), shapeSides) );

		RENDER_DEBUG_IFACE(mApexRenderDebug)->endDrawGroup();
	}
#endif		
}

NoiseFSAssetPreview::~NoiseFSAssetPreview(void)
{
}

void NoiseFSAssetPreview::setPose(const PxMat44& pose)
{
	mPose = pose;
	setDrawGroupsPose();
}

const PxMat44 NoiseFSAssetPreview::getPose() const
{
	return	mPose;
}

void	NoiseFSAssetPreview::toggleDrawPreview()
{
	if (mPreviewDetail & NOISE_DRAW_SHAPE)
	{
		RENDER_DEBUG_IFACE(mApexRenderDebug)->setDrawGroupVisible(mDrawGroupShape, true);
	}
	else
	{
		RENDER_DEBUG_IFACE(mApexRenderDebug)->setDrawGroupVisible(mDrawGroupShape, false);
	}
}

void	NoiseFSAssetPreview::setDrawGroupsPose()
{
	RENDER_DEBUG_IFACE(mApexRenderDebug)->setDrawGroupPose(mDrawGroupShape, mPose);
}


// from RenderDataProvider
void NoiseFSAssetPreview::lockRenderResources(void)
{
	ApexRenderable::renderDataLock();
}

void NoiseFSAssetPreview::unlockRenderResources(void)
{
	ApexRenderable::renderDataUnLock();
}

void NoiseFSAssetPreview::updateRenderResources(bool /*rewriteBuffers*/, void* /*userRenderData*/)
{
	if (mApexRenderDebug)
	{
		mApexRenderDebug->updateRenderResources();
	}
}

// from Renderable.h
void NoiseFSAssetPreview::dispatchRenderResources(UserRenderer& renderer)
{
	if (mApexRenderDebug)
	{
		if (mPreviewDetail & NOISE_DRAW_ASSET_INFO)
		{
			drawPreviewAssetInfo();
		}
		mApexRenderDebug->dispatchRenderResources(renderer);
	}
}

PxBounds3 NoiseFSAssetPreview::getBounds(void) const
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

void NoiseFSAssetPreview::destroy(void)
{
	delete this;
}

void NoiseFSAssetPreview::release(void)
{
	if (mInRelease)
	{
		return;
	}
	mInRelease = true;
	mAsset->releaseNoiseFSPreview(*this);
}

NoiseFSAssetPreview::NoiseFSAssetPreview(const NoiseFSPreviewDesc& PreviewDesc, ApexSDK* myApexSDK, NoiseFSAsset* myNoiseFSAsset, AssetPreviewScene* previewScene) :
	mPose(PreviewDesc.mPose),
	mApexSDK(myApexSDK),
	mAsset(myNoiseFSAsset),
	mPreviewScene(previewScene),
	mPreviewDetail(PreviewDesc.mPreviewDetail),
	mDrawGroupShape(0),
	mApexRenderDebug(0)
{
	drawNoiseFSPreview();
};


void NoiseFSAssetPreview::setDetailLevel(uint32_t detail)
{
	WRITE_ZONE();
	mPreviewDetail = detail;
	setDrawGroupsPose();
}

}
} // namespace nvidia

