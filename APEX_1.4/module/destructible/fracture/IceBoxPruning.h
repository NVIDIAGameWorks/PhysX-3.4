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
#ifndef ICEBOXPRUNING_H
#define ICEBOXPRUNING_H

#include "IceRevisitedRadix.h"
#include "PxVec3.h"
#include "PxBounds3.h"

#include "IceBoxPruningBase.h"

namespace nvidia
{
namespace fracture
{

struct Axes : public base::Axes
{
};

class BoxPruning : public base::BoxPruning
{

};

}
}

#endif
#endif