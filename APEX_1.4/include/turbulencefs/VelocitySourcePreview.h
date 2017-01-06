/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef VELOCITY_SOURCE_PREVIEW_H
#define VELOCITY_SOURCE_PREVIEW_H

#include "Apex.h"
#include "AssetPreview.h"

namespace nvidia
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

/**
\brief This class provides the asset preview for APEX VelocitySource Assets.
*/
class VelocitySourcePreview : public AssetPreview
{
public:
	public:
	///Draws the emitter 
	virtual void	drawVelocitySourcePreview() = 0;
	///Sets the scaling factor of the renderable
	virtual void	setScale(float scale) = 0;

protected:
	VelocitySourcePreview() {}
};


PX_POP_PACK

}
} // namespace nvidia

#endif // VELOCITY_SOURCE_PREVIEW_H