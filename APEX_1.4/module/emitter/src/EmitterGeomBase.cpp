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
#include "EmitterGeomBase.h"

namespace nvidia
{
namespace emitter
{

/* Return percentage of new volume not covered by old volume */
float EmitterGeomBase::computeNewlyCoveredVolume(
    const PxMat44& oldPose,
    const PxMat44& newPose,
	float scale,
    QDSRand& rand) const
{
	// estimate by sampling
	const uint32_t numSamples = 100;
	uint32_t numOutsideOldVolume = 0;
	for (uint32_t i = 0; i < numSamples; i++)
	{
		if (!isInEmitter(randomPosInFullVolume(PxMat44(newPose) * scale, rand), PxMat44(oldPose) * scale))
		{
			numOutsideOldVolume++;
		}
	}

	return (float) numOutsideOldVolume / numSamples;
}


// TODO make better, this is very slow when emitter moves slowly
// SJB: I'd go one further, this seems mildly retarted
PxVec3 EmitterGeomBase::randomPosInNewlyCoveredVolume(const PxMat44& pose, const PxMat44& oldPose, QDSRand& rand) const
{
	PxVec3 pos;
	do
	{
		pos = randomPosInFullVolume(pose, rand);
	}
	while (isInEmitter(pos, oldPose));
	return pos;
}

}
} // namespace nvidia::apex
