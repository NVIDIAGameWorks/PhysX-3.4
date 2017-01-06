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
#ifndef DELAUNAY_3D_H
#define DELAUNAY_3D_H

#include <PxVec3.h>
#include <PsArray.h>

#include "CompoundGeometry.h"

#include "Delaunay3dBase.h"

namespace nvidia
{
namespace fracture
{

class Delaunay3d : public base::Delaunay3d
{
	friend class SimScene;
protected:
	Delaunay3d(base::SimScene* scene): base::Delaunay3d((base::SimScene*)scene) {}
};

}
}

#endif
#endif
