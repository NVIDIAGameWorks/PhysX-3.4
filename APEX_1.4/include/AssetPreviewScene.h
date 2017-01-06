/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef ASSET_PREVIEW_SCENE_H
#define ASSET_PREVIEW_SCENE_H

/*!
\file
\brief classes Scene, SceneStats, SceneDesc
*/

#include "ApexDesc.h"
#include "Renderable.h"
#include "Context.h"
#include "foundation/PxVec3.h"
#include <ApexDefs.h>

#if PX_PHYSICS_VERSION_MAJOR == 3
#include "PxFiltering.h"
#endif
namespace physx
{
	class PxActor;
	class PxScene;
	class PxRenderBuffer;
	
	class PxCpuDispatcher;
	class PxGpuDispatcher;
	class PxTaskManager;
	class PxBaseTask;
}

namespace NvParameterized
{
class Interface;
}

namespace nvidia
{
namespace apex
{

PX_PUSH_PACK_DEFAULT


/**
\brief An APEX class for 
*/
class AssetPreviewScene : public ApexInterface
{
public:
	/**
	\brief Sets the view matrix. Should be called whenever the view matrix needs to be updated.
	*/
	virtual void					setCameraMatrix(const PxMat44& viewTransform) = 0;

	/**
	\brief Returns the view matrix set by the user for the given viewID.
	*/
	virtual PxMat44					getCameraMatrix() const = 0;

	/**
	\brief Sets whether the asset preview should simply show asset names or many other parameter values
	*/
	virtual void					setShowFullInfo(bool showFullInfo) = 0;

	/**
	\brief Get the bool which determines whether the asset preview shows just asset names or parameter values
	*/
	virtual bool					getShowFullInfo() const = 0;
};


PX_POP_PACK
}
} // end namespace nvidia::apex

#endif // ASSET_PREVIEW_SCENE_H
