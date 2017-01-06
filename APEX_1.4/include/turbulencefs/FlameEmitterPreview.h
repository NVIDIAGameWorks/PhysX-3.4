/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef FLAME_EMITTER_PREVIEW_H
#define FLAME_EMITTER_PREVIEW_H

#include "Apex.h"
#include "AssetPreview.h"

namespace nvidia
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

/**
\brief This class provides the asset preview for APEX Flame Emitter Assets.
*/
class FlameEmitterPreview : public AssetPreview
{
public:
	public:
	///Draws the emitter 
	virtual void	drawFlameEmitterPreview() = 0;
	///Sets the scaling factor of the renderable
	virtual void	setScale(float scale) = 0;

protected:
	FlameEmitterPreview() {}
};


PX_POP_PACK

}
} // namespace nvidia

#endif // FLAME_EMITTER_PREVIEW_H