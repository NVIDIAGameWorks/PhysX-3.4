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
#ifndef COMPOUND_CREATOR_H
#define COMPOUND_CREATOR_H

#include <PxVec3.h>
#include <PxTransform.h>
#include <PsArray.h>

#include "CompoundCreatorBase.h"

namespace nvidia
{
namespace fracture
{

class CompoundCreator : public base::CompoundCreator
{
	friend class SimScene;
protected:
	CompoundCreator(base::SimScene* scene): base::CompoundCreator((base::SimScene*)scene) {}
};

}
}

#endif
#endif