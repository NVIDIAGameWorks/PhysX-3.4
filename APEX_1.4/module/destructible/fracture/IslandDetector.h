/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "RTdef.h"
#if RT_COMPILE
#ifndef ISLAND_DETECTOR_H
#define ISLAND_DETECTOR_H

#include <PxVec3.h>
#include <PsArray.h>
#include "IceBoxPruning.h"

#include "IslandDetectorBase.h"

namespace nvidia
{
namespace fracture
{

class IslandDetector : public base::IslandDetector
{
	friend class SimScene;
protected:
	IslandDetector(base::SimScene* scene): base::IslandDetector((base::SimScene*)scene) {}
};

}
}

#endif
#endif
