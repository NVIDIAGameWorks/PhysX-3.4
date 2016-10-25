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
#include "ForceFieldPreview.h"
#include "ForceFieldAssetPreviewImpl.h"
#include "ModulePerfScope.h"
#include "ApexUsingNamespace.h"
#include "PxProfiler.h"

namespace nvidia
{
namespace forcefield
{

using namespace APEX_FORCEFIELD;

#ifndef WITHOUT_DEBUG_VISUALIZE

static const ForceFieldAssetPreviewImpl::point2 iconFrame[] =
{
	{ -0.50f, -0.50f}, { +0.50f, -0.50f}, { +0.50f, +0.50f}, { -0.50f, 0.50f}, { -0.50f, -0.50f}
};

static const ForceFieldAssetPreviewImpl::point2 iconBombCircle[] =
{
	{ +0.35000f, +0.00000f}, { +0.33807f, +0.09059f}, { +0.30311f, +0.17500f}, { +0.24749f, +0.24749f}, { +0.17500f, +0.30311f},
	{ +0.09059f, +0.33807f}, { +0.00000f, +0.35000f}, { -0.09059f, +0.33807f}, { -0.17500f, +0.30311f}, { -0.24749f, +0.24749f},
	{ -0.30311f, +0.17500f}, { -0.33807f, +0.09059f}, { -0.35000f, +0.00000f}, { -0.33807f, -0.09059f}, { -0.30311f, -0.17500f},
	{ -0.24749f, -0.24749f}, { -0.17500f, -0.30311f}, { -0.09059f, -0.33807f}, { +0.00000f, -0.35000f}, { +0.09059f, -0.33807f},
	{ +0.17500f, -0.30311f}, { +0.24749f, -0.24749f}, { +0.30311f, -0.17500f}, { +0.33807f, -0.09059f}, { +0.35000f, +0.00000f}
};

static const ForceFieldAssetPreviewImpl::point2 iconFuse[] =
{
	{ +0.00000f, +0.35000f}, { +0.00000f, +0.36500f}, { +0.00500f, +0.38000f}, { +0.02500f, +0.41000f}, { +0.05000f, +0.41500f}
};
static const ForceFieldAssetPreviewImpl::point2 iconSpark1[] =
{
	{ +0.0500f, +0.4300f}, { +0.0500f, +0.4550f}
};
static const ForceFieldAssetPreviewImpl::point2 iconSpark2[] =
{
	{ +0.0650f, +0.4150f}, { +0.0900f, +0.4150f}
};
static const ForceFieldAssetPreviewImpl::point2 iconSpark3[] =
{
	{ +0.0500f, +0.4000f}, { +0.0500f, +0.3700f}
};
static const ForceFieldAssetPreviewImpl::point2 iconSpark4[] =
{
	{ +0.0350f, +0.4300f}, { +0.0200f, +0.4400f}
};
static const ForceFieldAssetPreviewImpl::point2 iconSpark5[] =
{
	{ +0.0600f, +0.4300f}, { +0.0750f, +0.4450f}
};
static const ForceFieldAssetPreviewImpl::point2 iconSpark6[] =
{
	{ +0.0600f, +0.4000f}, { +0.0750f, +0.3850f}
};
static const ForceFieldAssetPreviewImpl::point2 iconSpark7[] =
{
	{ +0.0350f, +0.4000f}, { +0.0200f, +0.3850f}
};

#endif

void ForceFieldAssetPreviewImpl::drawMultilinePoint2(const point2* pts, uint32_t numPoints, uint32_t color)
{
	uint32_t i;
	PxVec3 p1, p2;

	PX_ASSERT(numPoints > 1);
	RENDER_DEBUG_IFACE(mApexRenderDebug)->pushRenderState();

	for (i = 0; i < numPoints - 1; i++)
	{
		p1.x = -pts->x;
		p1.y = 0.0f;
		p1.z = pts->y;
		pts++;
		p2.x = -pts->x;
		p2.y = 0.0f;
		p2.z = pts->y;

		RENDER_DEBUG_IFACE(mApexRenderDebug)->setCurrentColor(color);

		if (mDrawWithCylinder)
		{
			//draw with cylinders - makes it look BOLD.
			RENDER_DEBUG_IFACE(mApexRenderDebug)->addToCurrentState(RENDER_DEBUG::DebugRenderState::SolidShaded);
			RENDER_DEBUG_IFACE(mApexRenderDebug)->debugCylinder(p1, p2, .01f);
			RENDER_DEBUG_IFACE(mApexRenderDebug)->removeFromCurrentState(RENDER_DEBUG::DebugRenderState::SolidShaded);
		}
		else
		{
			RENDER_DEBUG_IFACE(mApexRenderDebug)->debugLine(p1, p2);
		}
	}
	RENDER_DEBUG_IFACE(mApexRenderDebug)->popRenderState();
}

void ForceFieldAssetPreviewImpl::drawIcon(void)
{
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
	PX_PROFILE_ZONE("ForceFieldDrawIcon", GetInternalApexSDK()->getContextId());
#ifndef WITHOUT_DEBUG_VISUALIZE
	using RENDER_DEBUG::DebugColors;
	drawMultilinePoint2(iconFrame,		ARRAY_SIZE(iconFrame),		RENDER_DEBUG_IFACE(mApexRenderDebug)->getDebugColor(DebugColors::White));
	drawMultilinePoint2(iconBombCircle,	ARRAY_SIZE(iconBombCircle),	RENDER_DEBUG_IFACE(mApexRenderDebug)->getDebugColor(DebugColors::Black));
	drawMultilinePoint2(iconFuse,		ARRAY_SIZE(iconFuse),		RENDER_DEBUG_IFACE(mApexRenderDebug)->getDebugColor(DebugColors::Black));
	drawMultilinePoint2(iconSpark1,		ARRAY_SIZE(iconSpark1),		RENDER_DEBUG_IFACE(mApexRenderDebug)->getDebugColor(DebugColors::Red));
	drawMultilinePoint2(iconSpark2,		ARRAY_SIZE(iconSpark2),		RENDER_DEBUG_IFACE(mApexRenderDebug)->getDebugColor(DebugColors::Red));
	drawMultilinePoint2(iconSpark3,		ARRAY_SIZE(iconSpark3),		RENDER_DEBUG_IFACE(mApexRenderDebug)->getDebugColor(DebugColors::Red));
	drawMultilinePoint2(iconSpark4,		ARRAY_SIZE(iconSpark4),		RENDER_DEBUG_IFACE(mApexRenderDebug)->getDebugColor(DebugColors::Red));
	drawMultilinePoint2(iconSpark5,		ARRAY_SIZE(iconSpark5),		RENDER_DEBUG_IFACE(mApexRenderDebug)->getDebugColor(DebugColors::Red));
	drawMultilinePoint2(iconSpark6,		ARRAY_SIZE(iconSpark6),		RENDER_DEBUG_IFACE(mApexRenderDebug)->getDebugColor(DebugColors::Red));
	drawMultilinePoint2(iconSpark7,		ARRAY_SIZE(iconSpark7),		RENDER_DEBUG_IFACE(mApexRenderDebug)->getDebugColor(DebugColors::Red));
#endif
}

void ForceFieldAssetPreviewImpl::drawForceFieldPreviewIcon(void)
{
#ifndef WITHOUT_DEBUG_VISUALIZE
	if (!mApexRenderDebug)
	{
		return;
	}

	//asset preview init
	if (mDrawGroupIconScaled == 0)
	{
		mDrawGroupIconScaled =  (uint32_t)RENDER_DEBUG_IFACE(mApexRenderDebug)->beginDrawGroup(PxMat44(PxIdentity));
		drawIcon();
		RENDER_DEBUG_IFACE(mApexRenderDebug)->endDrawGroup();
	}

	toggleDrawPreview();
	setDrawGroupsPoseScaled();
#endif
}

void	ForceFieldAssetPreviewImpl::setDrawGroupsPoseScaled()
{
			PxMat44 scaledPose;	// scaling pose for the preview rendering items that need to be scaled
			PxMat44 scaleMatrix;

			//set scale
			{
				scaledPose = mPose;
				scaleMatrix = PxMat44(PxVec4(mIconScale,		0.0f,			0.0f,		0.0f),
											 PxVec4(0.0f,			mIconScale,		0.0f,		0.0f),
											 PxVec4(0.0f,			0.0f,			mIconScale,	0.0f),
											 PxVec4(0.0f,			0.0f,			0.0f,		1.0f));
				//scaledPose.t.y = 2.0f; - what it was supposed to do?
				scaledPose = scaledPose * scaleMatrix;
			}

			RENDER_DEBUG_IFACE(mApexRenderDebug)->setDrawGroupPose((int32_t)mDrawGroupIconScaled, scaledPose);
}


void	ForceFieldAssetPreviewImpl::toggleDrawPreview()
{
		if (mPreviewDetail & FORCEFIELD_DRAW_ICON)
		{
			RENDER_DEBUG_IFACE(mApexRenderDebug)->setDrawGroupVisible((int32_t)mDrawGroupIconScaled, true);
		}
		else
		{
			RENDER_DEBUG_IFACE(mApexRenderDebug)->setDrawGroupVisible((int32_t)mDrawGroupIconScaled, false);
		}
}


void	ForceFieldAssetPreviewImpl::setDetailLevel(uint32_t detail)
{
	if(detail != mPreviewDetail)
	{
		mPreviewDetail = detail;
		toggleDrawPreview();
	}
};


void	ForceFieldAssetPreviewImpl::setIconScale(float scale)
{
	mIconScale = scale;
	drawForceFieldPreview();
};


void	ForceFieldAssetPreviewImpl::setPose(PxMat44 pose)
{
	WRITE_ZONE();
	mPose = pose;
	drawForceFieldPreview();
};
	

void ForceFieldAssetPreviewImpl::drawForceFieldBoundaries(void)
{
}

void ForceFieldAssetPreviewImpl::drawForceFieldWithCylinder()
{
}

void ForceFieldAssetPreviewImpl::drawForceFieldPreview(void)
{
	PX_PROFILE_ZONE("ForceFieldDrawForceFieldPreview", GetInternalApexSDK()->getContextId());
	mDrawWithCylinder = true;

	if(mPreviewDetail & FORCEFIELD_DRAW_ICON)
	{
		drawForceFieldPreviewIcon();
	}

	if(mPreviewDetail & FORCEFIELD_DRAW_BOUNDARIES)
	{
		drawForceFieldBoundaries();
	}

	if(mPreviewDetail & FORCEFIELD_DRAW_WITH_CYLINDERS)
	{
		drawForceFieldWithCylinder();
	}
}

ForceFieldAssetPreviewImpl::~ForceFieldAssetPreviewImpl(void)
{
}

void ForceFieldAssetPreviewImpl::setPose(const PxMat44& pose)
{
	WRITE_ZONE();
	mPose = pose;
	setDrawGroupsPoseScaled();
}

const PxMat44 ForceFieldAssetPreviewImpl::getPose() const
{
	READ_ZONE();
	return  mPose;
}

// from RenderDataProvider
void ForceFieldAssetPreviewImpl::lockRenderResources(void)
{
	ApexRenderable::renderDataLock();
}

void ForceFieldAssetPreviewImpl::unlockRenderResources(void)
{
	ApexRenderable::renderDataUnLock();
}

void ForceFieldAssetPreviewImpl::updateRenderResources(bool /*rewriteBuffers*/, void* /*userRenderData*/)
{
	if (mApexRenderDebug)
	{
		mApexRenderDebug->updateRenderResources();
	}
}

// from Renderable.h
void ForceFieldAssetPreviewImpl::dispatchRenderResources(UserRenderer& renderer)
{
	if (mApexRenderDebug)
	{
		mApexRenderDebug->dispatchRenderResources(renderer);
	}
}

PxBounds3 ForceFieldAssetPreviewImpl::getBounds(void) const
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

void ForceFieldAssetPreviewImpl::destroy(void)
{
	delete this;
}

void ForceFieldAssetPreviewImpl::release(void)
{
	if (mInRelease)
	{
		return;
	}
	mAsset->releaseForceFieldPreview(*this);
}

ForceFieldAssetPreviewImpl::ForceFieldAssetPreviewImpl(const ForceFieldPreviewDesc& PreviewDesc, ApexSDK* myApexSDK, ForceFieldAssetImpl* myForceFieldAsset, AssetPreviewScene* previewScene) :
	mPose(PreviewDesc.mPose),
	mApexSDK(myApexSDK),
	mAsset(myForceFieldAsset),
	mPreviewScene(previewScene),
	mDrawGroupIconScaled(0),
	mDrawGroupCylinder(0),
	mPreviewDetail(PreviewDesc.mPreviewDetail),
	mIconScale(1.0f),
	mApexRenderDebug(0)
{
	drawForceFieldPreview();
};

}
} // end namespace nvidia

