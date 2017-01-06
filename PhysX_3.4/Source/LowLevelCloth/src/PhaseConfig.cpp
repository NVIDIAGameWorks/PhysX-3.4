// This code contains NVIDIA Confidential Information and is disclosed to you
// under a form of NVIDIA software license agreement provided separately to you.
//
// Notice
// NVIDIA Corporation and its licensors retain all intellectual property and
// proprietary rights in and to this software and related documentation and
// any modifications thereto. Any use, reproduction, disclosure, or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA Corporation is strictly prohibited.
//
// ALL NVIDIA DESIGN SPECIFICATIONS, CODE ARE PROVIDED "AS IS.". NVIDIA MAKES
// NO WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES OF NONINFRINGEMENT,
// MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Information and code furnished is believed to be accurate and reliable.
// However, NVIDIA Corporation assumes no responsibility for the consequences of use of such
// information or for any infringement of patents or other rights of third parties that may
// result from its use. No license is granted by implication or otherwise under any patent
// or patent rights of NVIDIA Corporation. Details are subject to change without notice.
// This code supersedes and replaces all information previously supplied.
// NVIDIA Corporation products are not authorized for use as critical
// components in life support devices or systems without express written approval of
// NVIDIA Corporation.
//
// Copyright (c) 2008-2017 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.

#include "PhaseConfig.h"
#include "PsMathUtils.h"

namespace physx
{
namespace cloth
{
PhaseConfig transform(const PhaseConfig&);
}
}

using namespace physx;

namespace
{
float safeLog2(float x)
{
	float saturated = PxMax(0.0f, PxMin(x, 1.0f));
	return saturated ? shdfnd::log2(saturated) : -FLT_MAX_EXP;
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
	result.mCompressionLimit = 1 - 1 / config.mCompressionLimit;
	result.mStretchLimit = 1 - 1 / config.mStretchLimit;

	return result;
}
