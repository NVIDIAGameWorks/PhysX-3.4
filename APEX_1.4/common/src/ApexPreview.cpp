/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "Apex.h"
#include "ApexPreview.h"

namespace nvidia
{
namespace apex
{

ApexPreview::ApexPreview() :
	mInRelease(false),
	mPose(physx::PxIdentity)
{
}

ApexPreview::~ApexPreview()
{
	destroy();
}

void ApexPreview::setPose(const PxMat44& pose)
{
	mPose = pose;
}

const PxMat44 ApexPreview::getPose() const
{
	return mPose;
}

void ApexPreview::destroy()
{
	mInRelease = true;

	renderDataLock();
}

}
} // end namespace nvidia::apex
