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
#ifndef DELAUNAY_2D_H
#define DELAUNAY_2D_H

#include <PxVec3.h>
#include <PsArray.h>

#include "Delaunay2dBase.h"

namespace nvidia
{
namespace fracture
{

class Delaunay2d : public base::Delaunay2d
{
	friend class SimScene;
protected:
	Delaunay2d(base::SimScene* scene): base::Delaunay2d((base::SimScene*)scene) {}
};

}
}

#endif
#endif