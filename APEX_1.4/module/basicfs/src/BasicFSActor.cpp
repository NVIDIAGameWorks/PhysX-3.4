/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "ApexDefs.h"
#include "RenderMeshActorDesc.h"
#include "RenderMeshActor.h"
#include "RenderMeshAsset.h"

#include "Apex.h"
#include "BasicFSActor.h"
#include "BasicFSAssetImpl.h"
#include "BasicFSScene.h"
#include "ApexSDKIntl.h"
#include "SceneIntl.h"
#include "RenderDebugInterface.h"

#include <PxScene.h>

#include <FieldSamplerManagerIntl.h>
#include "ApexResourceHelper.h"

namespace nvidia
{
namespace basicfs
{

#define NUM_DEBUG_POINTS 2048

BasicFSActor::BasicFSActor(BasicFSScene& scene)
	: mScene(&scene)
	, mPose(PxIdentity)
	, mScale(1.0f)
	, mFieldSamplerChanged(true)
	, mFieldSamplerEnabled(true)
	, mFieldWeight(1.0f)
{
}

BasicFSActor::~BasicFSActor()
{
}

void BasicFSActor::setPhysXScene(PxScene*) { }
PxScene* BasicFSActor::getPhysXScene() const
{
	return NULL;
}

}
} // end namespace nvidia::apex

