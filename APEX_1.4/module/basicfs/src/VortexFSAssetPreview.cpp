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
#include "VortexFSAsset.h"
#include "VortexFSAssetParams.h"
#include "VortexFSPreview.h"
#include "VortexFSAssetPreview.h"
#include "ModulePerfScope.h"
#include "ApexUsingNamespace.h"

#include "ReadCheck.h"
#include "WriteCheck.h"

namespace nvidia
{
namespace basicfs
{

using namespace APEX_VORTEX;

void VortexFSAssetPreview::drawVortexFSPreview(void)
{
	PX_PROFILE_ZONE("VortexFSDrawPreview", GetInternalApexSDK()->getContextId());

	if (mPreviewDetail & VORTEX_DRAW_SHAPE)
	{
		drawPreviewShape();
	}
}

#define ASSET_INFO_XPOS					(-0.9f)	// left position of the asset info
#define ASSET_INFO_YPOS					( 0.9f)	// top position of the asset info
#define DEBUG_TEXT_HEIGHT				(0.35f)	//in screen space -- would be nice to know this!


void VortexFSAssetPreview::drawPreviewShape()
{
#ifndef WITHOUT_DEBUG_VISUALIZE
	if (!mApexRenderDebug)
	{
		return;
	}

	if (mDrawGroupBox == 0)
	{
		mDrawGroupBox = RENDER_DEBUG_IFACE(mApexRenderDebug)->beginDrawGroup(PxMat44(PxIdentity));
		RENDER_DEBUG_IFACE(mApexRenderDebug)->setCurrentColor(RENDER_DEBUG_IFACE(mApexRenderDebug)->getDebugColor(RENDER_DEBUG::DebugColors::DarkGreen));
		RENDER_DEBUG_IFACE(mApexRenderDebug)->debugSphere(PxVec3(0.0f), mRadius);	// * scale);
		RENDER_DEBUG_IFACE(mApexRenderDebug)->endDrawGroup();
	}

	setDrawGroupsPose();
#endif
}

void	VortexFSAssetPreview::toggleDrawPreview()
{
	if (mPreviewDetail & VORTEX_DRAW_SHAPE)
	{
		RENDER_DEBUG_IFACE(mApexRenderDebug)->setDrawGroupVisible(mDrawGroupBox, true);
	}
	else
	{
		RENDER_DEBUG_IFACE(mApexRenderDebug)->setDrawGroupVisible(mDrawGroupBox, false);
	}
}

void	VortexFSAssetPreview::setDrawGroupsPose()
{
	RENDER_DEBUG_IFACE(mApexRenderDebug)->setDrawGroupPose(mDrawGroupBox, mPose);
}


void VortexFSAssetPreview::drawInfoLine(uint32_t lineNum, const char* str)
{
#ifdef WITHOUT_DEBUG_VISUALIZE
	PX_UNUSED(lineNum);
	PX_UNUSED(str);
#else
	RENDER_DEBUG_IFACE(mApexRenderDebug)->setCurrentColor(RENDER_DEBUG_IFACE(mApexRenderDebug)->getDebugColor(RENDER_DEBUG::DebugColors::Green));
	PxMat44 cameraMatrix = mPreviewScene->getCameraMatrix();
	PxVec3 textLocation = mPose.getPosition();
	textLocation += cameraMatrix.column1.getXYZ() * (ASSET_INFO_YPOS - (lineNum * DEBUG_TEXT_HEIGHT));
	RENDER_DEBUG_IFACE(mApexRenderDebug)->debugText(textLocation, str);
#endif
}

void VortexFSAssetPreview::drawPreviewAssetInfo()
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
	//	RENDER_DEBUG_IFACE(&mApexRenderDebug)->addToCurrentState(RENDER_DEBUG::DebugRenderState::ScreenSpace);
	RENDER_DEBUG_IFACE(mApexRenderDebug)->addToCurrentState(RENDER_DEBUG::DebugRenderState::NoZbuffer);
	RENDER_DEBUG_IFACE(mApexRenderDebug)->setCurrentTextScale(1.0f);
	RENDER_DEBUG_IFACE(mApexRenderDebug)->setCurrentColor(RENDER_DEBUG_IFACE(mApexRenderDebug)->getDebugColor(RENDER_DEBUG::DebugColors::Yellow));

	// asset name
	APEX_SPRINTF_S(buf, sizeof(buf) - 1, "%s %s", mAsset->getObjTypeName(), mAsset->getName());
	drawInfoLine(lineNum++, buf);
	lineNum++;

	if(mPreviewScene->getShowFullInfo())
	{
		// TODO: cache strings
		VortexFSAssetParams* assetParams = static_cast<VortexFSAssetParams*>(mAsset->getAssetNvParameterized());
		PX_ASSERT(assetParams);

		float rotationalStrength		= assetParams->rotationalStrength;
		float radialStrength	= assetParams->radialStrength;
		float liftStrength	= assetParams->liftStrength;

		myString = "Rotational field strength coefficient = ";
		ApexSimpleString::ftoa(rotationalStrength, floatStr);
		myString += floatStr;
		drawInfoLine(lineNum++, myString.c_str());

		myString = "Radial field strength coefficient = ";
		ApexSimpleString::ftoa(radialStrength, floatStr);
		myString += floatStr;
		drawInfoLine(lineNum++, myString.c_str());

		myString = "Lifting field strength coefficient = ";
		ApexSimpleString::ftoa(liftStrength, floatStr);
		myString += floatStr;
		drawInfoLine(lineNum++, myString.c_str());

		// fieldSampler filter data info
		if (assetParams->fieldSamplerFilterDataName.buf)
		{
			APEX_SPRINTF_S(buf, sizeof(buf) - 1, "FieldSampler Filter Data = %s",
				assetParams->fieldSamplerFilterDataName.buf
				);
			drawInfoLine(lineNum++, buf);
		}

		// fieldBoundary filter data info
		if (assetParams->fieldBoundaryFilterDataName.buf)
		{
			myString = "FieldBoundary Filter Data = ";
			myString += ApexSimpleString(assetParams->fieldBoundaryFilterDataName.buf);
			drawInfoLine(lineNum++, myString.c_str());
		}

		// implicit info
		myString = "Fade Percentage = ";
		ApexSimpleString::ftoa(assetParams->boundaryFadePercentage, floatStr);
		myString += floatStr;
		drawInfoLine(lineNum++, myString.c_str());
	}

	RENDER_DEBUG_IFACE(mApexRenderDebug)->popRenderState();
#endif
}

VortexFSAssetPreview::~VortexFSAssetPreview(void)
{
}

void VortexFSAssetPreview::setPose(const PxMat44& pose)
{
	mPose = pose;
	setDrawGroupsPose();
}

const PxMat44 VortexFSAssetPreview::getPose() const
{
	return mPose;
}

void VortexFSAssetPreview::setRadius(float radius)
{
	WRITE_ZONE();
	mRadius = radius;
}

const float VortexFSAssetPreview::getRadius() const
{
	READ_ZONE();
	return mRadius;
}

// from RenderDataProvider
void VortexFSAssetPreview::lockRenderResources(void)
{
	ApexRenderable::renderDataLock();
}

void VortexFSAssetPreview::unlockRenderResources(void)
{
	ApexRenderable::renderDataUnLock();
}

void VortexFSAssetPreview::updateRenderResources(bool /*rewriteBuffers*/, void* /*userRenderData*/)
{
	if (mApexRenderDebug)
	{
		mApexRenderDebug->updateRenderResources();
	}
}

void VortexFSAssetPreview::dispatchRenderResources(UserRenderer& renderer)
{
	if (mApexRenderDebug)
	{
		if (mPreviewDetail & VORTEX_DRAW_ASSET_INFO)
		{
			drawPreviewAssetInfo();
		}
		mApexRenderDebug->dispatchRenderResources(renderer);
	}
}

PxBounds3 VortexFSAssetPreview::getBounds(void) const
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

void VortexFSAssetPreview::destroy(void)
{
	delete this;
}

float VortexFSAssetPreview::getVortexRadius(NvParameterized::Interface* assetParams)
{
	float radius = 0.0f;
	const char* name = "radius";

	NvParameterized::Handle handle(*assetParams, name);
	bool handleIsValid = handle.isValid();
	PX_ASSERT(handleIsValid);
	PX_UNUSED(handleIsValid);
	//APEX_DEBUG_WARNING("Test.");
	NvParameterized::ErrorType errorGetRadius = handle.getParamF32(radius);
	PX_ASSERT(errorGetRadius == NvParameterized::ERROR_NONE);
	PX_UNUSED(errorGetRadius);

	return radius;

	// the other way to do it ...
	//VortexFSAssetParams* attractorAssetParams = static_cast<VortexFSAssetParams*>(assetParams);
	//PX_ASSERT(assetParams);

	//return attractorAssetParams->radius;
}

void VortexFSAssetPreview::release(void)
{
	if (mInRelease)
	{
		return;
	}
	mInRelease = true;
	mAsset->releaseVortexFSPreview(*this);
}

VortexFSAssetPreview::VortexFSAssetPreview(const VortexFSPreviewDesc& PreviewDesc, ApexSDK* myApexSDK, VortexFSAsset* mVortexFSAsset, AssetPreviewScene* previewScene) :
	mPose(PreviewDesc.mPose),
	mApexSDK(myApexSDK),
	mAsset(mVortexFSAsset),
	mPreviewScene(previewScene),
	mPreviewDetail(PreviewDesc.mPreviewDetail),
	mDrawGroupBox(0),
	mApexRenderDebug(0)
{
	NvParameterized::Interface* assetParams = mVortexFSAsset->getAssetNvParameterized(); //FIXME: const
	PX_ASSERT(assetParams);

	if (assetParams)
	{
		mRadius = getVortexRadius(assetParams);
	}

	drawVortexFSPreview();
};


void VortexFSAssetPreview::setDetailLevel(uint32_t detail)
{
	WRITE_ZONE();
	if(detail != mPreviewDetail)
	{
		mPreviewDetail = detail;
		toggleDrawPreview();
	}
}

}
} // namespace nvidia
