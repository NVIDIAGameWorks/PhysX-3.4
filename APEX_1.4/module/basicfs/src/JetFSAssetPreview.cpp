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
#include "JetFSAsset.h"
#include "JetFSAssetParams.h"
#include "JetFSPreview.h"
#include "JetFSAssetPreview.h"
#include "ModulePerfScope.h"
#include "ApexUsingNamespace.h"

#include "ReadCheck.h"
#include "WriteCheck.h"

namespace nvidia
{
namespace basicfs
{

using namespace APEX_JET;

void JetFSAssetPreview::drawJetFSPreview(void)
{
	PX_PROFILE_ZONE("JetFSDrawPreview", GetInternalApexSDK()->getContextId());
	if (mPreviewDetail & JET_DRAW_SHAPE)
	{
		drawShape();
	}

	toggleDrawPreview();
	setDrawGroupsPose();
}

#define ASSET_INFO_XPOS					(-0.9f)	// left position of the asset info
#define ASSET_INFO_YPOS					( 0.9f)	// top position of the asset info
#define DEBUG_TEXT_HEIGHT				(0.35f)	//in screen space -- would be nice to know this!



void JetFSAssetPreview::drawInfoLine(uint32_t lineNum, const char* str)
{
#ifdef WITHOUT_DEBUG_VISUALIZE
	PX_UNUSED(lineNum);
	PX_UNUSED(str);
#else
	if (!mApexRenderDebug)
	{
		return;
	}
	PxMat44 cameraMatrix = mPreviewScene->getCameraMatrix();
	RENDER_DEBUG_IFACE(mApexRenderDebug)->setCurrentColor(RENDER_DEBUG_IFACE(mApexRenderDebug)->getDebugColor(RENDER_DEBUG::DebugColors::Blue));
	PxVec3 textLocation = mPose.getPosition(); 
	textLocation += cameraMatrix.column1.getXYZ() * (ASSET_INFO_YPOS - (lineNum * DEBUG_TEXT_HEIGHT));
	cameraMatrix.setPosition(textLocation);
	RENDER_DEBUG_IFACE(mApexRenderDebug)->debugText(textLocation, str);
#endif
}

void JetFSAssetPreview::drawPreviewAssetInfo()
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
		JetFSAssetParams& assetParams = *static_cast<JetFSAssetParams*>(mAsset->getAssetNvParameterized());

		APEX_SPRINTF_S(buf, sizeof(buf) - 1, "fieldStrength = %f",
			assetParams.fieldStrength
			);
		drawInfoLine(lineNum++, buf);


		APEX_SPRINTF_S(buf, sizeof(buf) - 1, "fieldStrengthDeviationPercentage = %f",
			assetParams.fieldStrengthDeviationPercentage
			);
		drawInfoLine(lineNum++, buf);


		APEX_SPRINTF_S(buf, sizeof(buf) - 1, "fieldDirectionDeviationAngle = %f",
			assetParams.fieldDirectionDeviationAngle
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

void JetFSAssetPreview::drawShape()
{
#ifndef WITHOUT_DEBUG_VISUALIZE
	if (!mApexRenderDebug)
	{
		return;
	}

	JetFSAssetParams& assetParams = *static_cast<JetFSAssetParams*>(mAsset->getAssetNvParameterized());
	//asset preview init
	if (mDrawGroupShape == 0)
	{
		mDrawGroupShape = RENDER_DEBUG_IFACE(mApexRenderDebug)->beginDrawGroup(PxMat44(PxIdentity));

		//debug visualization
		RENDER_DEBUG_IFACE(mApexRenderDebug)->setCurrentColor(RENDER_DEBUG_IFACE(mApexRenderDebug)->getDebugColor(RENDER_DEBUG::DebugColors::DarkBlue));

		PxMat44 dirToWorld = PxMat44(PxVec4(mDirToWorld.column0, 0), PxVec4(mDirToWorld.column1, 0), PxVec4(mDirToWorld.column2, 0),  PxVec4(0, 0, 0, 1));
		//dirToWorld.setPosition(mPose.getPosition());
		RENDER_DEBUG_IFACE(mApexRenderDebug)->setPose(dirToWorld);
		RENDER_DEBUG_IFACE(mApexRenderDebug)->debugCapsule(assetParams.farRadius, assetParams.farRadius * assetParams.directionalStretch, 2);
		RENDER_DEBUG_IFACE(mApexRenderDebug)->setPose(dirToWorld);
		RENDER_DEBUG_IFACE(mApexRenderDebug)->debugCapsule(assetParams.gridShapeRadius, assetParams.gridShapeHeight, 2);
		RENDER_DEBUG_IFACE(mApexRenderDebug)->endDrawGroup();
	}

	if(mDrawGroupTorus == 0)
	{
		//draw torus
		//RENDER_DEBUG_IFACE(&mApexRenderDebug)->setPose(mPose);???

		mDrawGroupTorus = RENDER_DEBUG_IFACE(mApexRenderDebug)->beginDrawGroup(PxMat44(PxIdentity));
		RENDER_DEBUG_IFACE(mApexRenderDebug)->setCurrentColor(RENDER_DEBUG_IFACE(mApexRenderDebug)->getDebugColor(RENDER_DEBUG::DebugColors::DarkBlue));
		const uint32_t NUM_PHI_SLICES = 16;
		const uint32_t NUM_THETA_SLICES = 16;

		const float torusRadius = assetParams.farRadius / 2;

		float cosPhiLast = 1;
		float sinPhiLast = 0;
		for (uint32_t i = 1; i <= NUM_PHI_SLICES; ++i)
		{
			float phi = (i * PxTwoPi / NUM_PHI_SLICES);
			float cosPhi = PxCos(phi);
			float sinPhi = PxSin(phi);

			RENDER_DEBUG_IFACE(mApexRenderDebug)->debugLine(
				mDirToWorld * PxVec3(cosPhiLast * assetParams.pivotRadius, 0, sinPhiLast * assetParams.pivotRadius),
				mDirToWorld * PxVec3(cosPhi * assetParams.pivotRadius, 0, sinPhi * assetParams.pivotRadius));

			RENDER_DEBUG_IFACE(mApexRenderDebug)->debugLine(
				mDirToWorld * PxVec3(cosPhiLast * assetParams.nearRadius, 0, sinPhiLast * assetParams.nearRadius),
				mDirToWorld * PxVec3(cosPhi * assetParams.nearRadius, 0, sinPhi * assetParams.nearRadius));

			float cosThetaLast = 1;
			float sinThetaLast = 0;
			for (uint32_t j = 1; j <= NUM_THETA_SLICES; ++j)
			{
				float theta = (j * PxTwoPi / NUM_THETA_SLICES);
				float cosTheta = PxCos(theta);
				float sinTheta = PxSin(theta);

				float d = torusRadius * (1 + cosTheta);
				float h = torusRadius * sinTheta * assetParams.directionalStretch;

				float dLast = torusRadius * (1 + cosThetaLast);
				float hLast = torusRadius * sinThetaLast * assetParams.directionalStretch;

				RENDER_DEBUG_IFACE(mApexRenderDebug)->debugLine(
					mDirToWorld * PxVec3(cosPhi * dLast, hLast, sinPhi * dLast),
					mDirToWorld * PxVec3(cosPhi * d, h, sinPhi * d));

				RENDER_DEBUG_IFACE(mApexRenderDebug)->debugLine(
					mDirToWorld * PxVec3(cosPhiLast * d, h, sinPhiLast * d),
					mDirToWorld * PxVec3(cosPhi * d, h, sinPhi * d));

				RENDER_DEBUG_IFACE(mApexRenderDebug)->debugLine(
					mDirToWorld * PxVec3(cosPhiLast * dLast, hLast, sinPhiLast * dLast),
					mDirToWorld * PxVec3(cosPhi * dLast, hLast, sinPhi * dLast));

				cosThetaLast = cosTheta;
				sinThetaLast = sinTheta;
			}

			cosPhiLast = cosPhi;
			sinPhiLast = sinPhi;
		}
		RENDER_DEBUG_IFACE(mApexRenderDebug)->endDrawGroup();
	}
#endif		
}

JetFSAssetPreview::~JetFSAssetPreview(void)
{
}

void JetFSAssetPreview::setPose(const PxMat44& pose)
{
	mPose = pose;
	setDrawGroupsPose();
}

const PxMat44 JetFSAssetPreview::getPose() const
{
	return	mPose;
}

void	JetFSAssetPreview::toggleDrawPreview()
{
	if (!mApexRenderDebug)
	{
		return;
	}
	if (mPreviewDetail & JET_DRAW_SHAPE)
	{
		RENDER_DEBUG_IFACE(mApexRenderDebug)->setDrawGroupVisible(mDrawGroupShape, true);
		RENDER_DEBUG_IFACE(mApexRenderDebug)->setDrawGroupVisible(mDrawGroupTorus, true);
	}
	else
	{
		RENDER_DEBUG_IFACE(mApexRenderDebug)->setDrawGroupVisible(mDrawGroupShape, false);
		RENDER_DEBUG_IFACE(mApexRenderDebug)->setDrawGroupVisible(mDrawGroupTorus, false);
	}
}

void	JetFSAssetPreview::setDrawGroupsPose()
{
	if (!mApexRenderDebug)
	{
		return;
	}
	RENDER_DEBUG_IFACE(mApexRenderDebug)->setDrawGroupPose(mDrawGroupShape, mPose);
	RENDER_DEBUG_IFACE(mApexRenderDebug)->setDrawGroupPose(mDrawGroupTorus, mPose);
}


// from RenderDataProvider
void JetFSAssetPreview::lockRenderResources(void)
{
	ApexRenderable::renderDataLock();
}

void JetFSAssetPreview::unlockRenderResources(void)
{
	ApexRenderable::renderDataUnLock();
}

void JetFSAssetPreview::updateRenderResources(bool /*rewriteBuffers*/, void* /*userRenderData*/)
{
	if (mApexRenderDebug)
	{
		mApexRenderDebug->updateRenderResources();
	}
}

// from Renderable.h
void JetFSAssetPreview::dispatchRenderResources(UserRenderer& renderer)
{
	if (mApexRenderDebug)
	{
		if (mPreviewDetail & JET_DRAW_ASSET_INFO)
		{
			drawPreviewAssetInfo();
		}
		mApexRenderDebug->dispatchRenderResources(renderer);
	}
}

PxBounds3 JetFSAssetPreview::getBounds(void) const
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

void JetFSAssetPreview::destroy(void)
{
	delete this;
}

void JetFSAssetPreview::release(void)
{
	if (mInRelease)
	{
		return;
	}
	mInRelease = true;
	mAsset->releaseJetFSPreview(*this);
}

JetFSAssetPreview::JetFSAssetPreview(const JetFSPreviewDesc& PreviewDesc, ApexSDK* myApexSDK, JetFSAsset* myJetFSAsset, AssetPreviewScene* previewScene) :
	mPose(PreviewDesc.mPose),
	mApexSDK(myApexSDK),
	mAsset(myJetFSAsset),
	mPreviewScene(previewScene),
	mPreviewDetail(PreviewDesc.mPreviewDetail),
	mDrawGroupShape(0),
	mDrawGroupTorus(0),
	mApexRenderDebug(0)
{
	PxMat33 poseRot = PxMat33(mPose.column0.getXYZ(), mPose.column1.getXYZ(), mPose.column2.getXYZ());
	PxVec3 vecN = poseRot.transform(mAsset->mParams->fieldDirection.getNormalized());
	vecN.normalize();
	PxVec3 vecP, vecQ;
	BuildPlaneBasis(vecN, vecP, vecQ);

	mDirToWorld.column0 = vecP;
	mDirToWorld.column1 = vecN;
	mDirToWorld.column2 = vecQ;

	drawJetFSPreview();
};


void JetFSAssetPreview::setDetailLevel(uint32_t detail)
{
	WRITE_ZONE();
	mPreviewDetail = detail;
	setDrawGroupsPose();
}

}
} // namespace nvidia

