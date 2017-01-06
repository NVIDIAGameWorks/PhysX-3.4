/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.

#include "PhaseConfig.h"
#include "ApexUsingNamespace.h"
#include "PsMathUtils.h"

namespace nvidia
{
namespace cloth
{
PhaseConfig transform(const PhaseConfig&);
}
}

using namespace nvidia;

namespace
{
float safeLog2(float x)
{
	float saturated = PxMax(0.0f, PxMin(x, 1.0f));
	return saturated ? physx::shdfnd::log2(saturated) : -FLT_MAX_EXP;
}
}

cloth::PhaseConfig::PhaseConfig(uint16_t index)
: mPhaseIndex(index)
, mPadding(0xffff)
, mStiffness(1.0f)
, mStiffnessMultiplier(1.0f)
, mCompressionLimit(1.0f)
, mStretchLimit(1.0f)
{
}

// convert from user input to solver format
cloth::PhaseConfig cloth::transform(const PhaseConfig& config)
{
	PhaseConfig result(config.mPhaseIndex);

	result.mStiffness = safeLog2(1.0f - config.mStiffness);
	result.mStiffnessMultiplier = safeLog2(config.mStiffnessMultiplier);

	// negative for compression, positive for stretch
	result.mCompressionLimit = 1.f - 1.f / config.mCompressionLimit;
	result.mStretchLimit = 1.f - 1.f / config.mStretchLimit;

	return result;
}
