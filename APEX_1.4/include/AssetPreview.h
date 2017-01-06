/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef ASSET_PREVIEW_H
#define ASSET_PREVIEW_H

/*!
\file
\brief class AssetPreview
*/

#include "ApexInterface.h"
#include "Renderable.h"

namespace nvidia
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

/**
\brief Base class of all APEX asset previews

    The preview is a lightweight Renderable instance which can be
	rendered without having to create an ApexScene.  It is intended to be
	used by level editors to place assets within a level.
*/
class AssetPreview : public ApexInterface, public Renderable
{
public:

	/**
	\brief Set the preview instance's world pose.  This may include scaling.
	*/
	virtual void setPose(const PxMat44& pose) = 0;

	/**
	\brief Get the preview instance's world pose.

	\note can't return by reference as long as the internal representation is not identical.
	*/
	virtual const PxMat44 getPose() const = 0;
protected:
	AssetPreview() {}
};

PX_POP_PACK

}
} // end namespace nvidia::apex

#endif // ASSET_PREVIEW_H
