/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "RenderDebugInterface.h"
#include "GroundEmitterPreview.h"
#include "GroundEmitterAssetPreview.h"
#include "ApexPreview.h"
#include "ApexUsingNamespace.h"
#include "WriteCheck.h"
#include "ReadCheck.h"

namespace nvidia
{
namespace emitter
{

void GroundEmitterAssetPreview::drawEmitterPreview(void)
{
	WRITE_ZONE();
#ifndef WITHOUT_DEBUG_VISUALIZE
	if (!mApexRenderDebug)
	{
		return;
	}
	PxVec3 tmpUpDirection(0.0f, 1.0f, 0.0f);

	using RENDER_DEBUG::DebugColors;

	//asset preview init
	if (mGroupID == 0)
	{
		mGroupID = RENDER_DEBUG_IFACE(mApexRenderDebug)->beginDrawGroup(PxMat44(PxIdentity));
		// Cylinder that describes the refresh radius, upDirection, and spawnHeight
		RENDER_DEBUG_IFACE(mApexRenderDebug)->setCurrentColor(RENDER_DEBUG_IFACE(mApexRenderDebug)->getDebugColor(DebugColors::Green));
		RENDER_DEBUG_IFACE(mApexRenderDebug)->debugCylinder(
		    PxVec3(0.0f),
		    tmpUpDirection * (mAsset->getSpawnHeight() + mAsset->getRaycastHeight() + 0.01f),
		    mAsset->getRadius());

		// Ray that describes the raycast spawn height
		RENDER_DEBUG_IFACE(mApexRenderDebug)->setCurrentColor(RENDER_DEBUG_IFACE(mApexRenderDebug)->getDebugColor(DebugColors::Yellow), 
															RENDER_DEBUG_IFACE(mApexRenderDebug)->getDebugColor(DebugColors::Yellow));
		RENDER_DEBUG_IFACE(mApexRenderDebug)->setCurrentArrowSize(mScale);
		RENDER_DEBUG_IFACE(mApexRenderDebug)->debugRay(tmpUpDirection * mAsset->getRaycastHeight(), PxVec3(0.0f));
		RENDER_DEBUG_IFACE(mApexRenderDebug)->endDrawGroup();
	}

	//asset preview set pose
	PxMat44 groupPose = mPose;
	RENDER_DEBUG_IFACE(mApexRenderDebug)->setDrawGroupPose(mGroupID, groupPose);

	//asset preview set visibility
	RENDER_DEBUG_IFACE(mApexRenderDebug)->setDrawGroupVisible(mGroupID, true);
#endif
}

void GroundEmitterAssetPreview::destroy(void)
{
	mApexRenderDebug = NULL;
	ApexPreview::destroy();
	delete this;
}

GroundEmitterAssetPreview::~GroundEmitterAssetPreview(void)
{
}

void GroundEmitterAssetPreview::setPose(const PxMat44& pose)
{
	WRITE_ZONE();
	mPose = PxTransform(pose);
	drawEmitterPreview();
}

void GroundEmitterAssetPreview::setScale(float scale)
{
	WRITE_ZONE();
	mScale = scale;
	drawEmitterPreview();
}

const PxMat44 GroundEmitterAssetPreview::getPose() const
{
	READ_ZONE();
	return(mPose);
}

// from RenderDataProvider
void GroundEmitterAssetPreview::lockRenderResources(void)
{
	ApexRenderable::renderDataLock();
}

void GroundEmitterAssetPreview::unlockRenderResources(void)
{
	ApexRenderable::renderDataUnLock();
}

void GroundEmitterAssetPreview::updateRenderResources(bool /*rewriteBuffers*/, void* /*userRenderData*/)
{
	mApexRenderDebug->updateRenderResources();
}

// from Renderable.h
void GroundEmitterAssetPreview::dispatchRenderResources(UserRenderer& renderer)
{
	mApexRenderDebug->dispatchRenderResources(renderer);
}

PxBounds3 GroundEmitterAssetPreview::getBounds(void) const
{
	return mApexRenderDebug->getBounds();
}

void GroundEmitterAssetPreview::release(void)
{
	if (mInRelease)
	{
		return;
	}
	mInRelease = true;
	const_cast<GroundEmitterAssetImpl*>(mAsset)->releaseEmitterPreview(*this);
}

}
} // namespace nvidia::apex
