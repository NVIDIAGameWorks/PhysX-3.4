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
#ifndef POLYGON_TRIANGULATOR_H
#define POLYGON_TRIANGULATOR_H

#include <PxVec3.h>
#include <PsArray.h>

#include "PolygonTriangulatorBase.h"

namespace nvidia
{
namespace fracture
{

class PolygonTriangulator : public base::PolygonTriangulator
{
	friend class SimScene;
protected:
	PolygonTriangulator(base::SimScene* scene): base::PolygonTriangulator((base::SimScene*)scene) {}
};

}
}

#endif
#endif