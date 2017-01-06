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
#ifndef FRACTURE_PATTERN
#define FRACTURE_PATTERN

#include "Delaunay3d.h"
#include "Delaunay2d.h"
#include "CompoundGeometry.h"
#include "PxTransform.h"
#include "PxBounds3.h"

#include "FracturePatternBase.h"

namespace nvidia
{
namespace fracture
{

class Convex;
class Compound;
class CompoundGeometry;

class FracturePattern : public base::FracturePattern
{
	friend class SimScene;
protected:
	FracturePattern(base::SimScene* scene): base::FracturePattern((base::SimScene*)scene) {}
};

}
}

#endif
#endif