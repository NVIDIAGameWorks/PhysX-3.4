/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef PARTICLES_PREVIEW_H
#define PARTICLES_PREVIEW_H

#include "Apex.h"
#include "AssetPreview.h"

namespace nvidia
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

class RenderDebugInterface;

namespace APEX_PARTICLES
{
/**
	\def PARTICLES_DRAW_NOTHING
	Draw no items.
*/
static const uint32_t PARTICLES_DRAW_NOTHING = (0x00);
/**
	\def PARTICLES_DRAW_ICON
	Draw the icon.
*/
static const uint32_t PARTICLES_DRAW_ICON = (0x01);
/**
	\def PARTICLES_DRAW_BOUNDARIES
	Draw the Particles include shapes.
*/
static const uint32_t PARTICLES_DRAW_BOUNDARIES = (0x2);
/**
	\def PARTICLES_DRAW_WITH_CYLINDERS
	Draw the dynamicsystem field boundaries.
*/
static const uint32_t PARTICLES_DRAW_WITH_CYLINDERS = (0x4);
/**
	\def PARTICLES_DRAW_FULL_DETAIL
	Draw all of the preview rendering items.
*/
static const uint32_t PARTICLES_DRAW_FULL_DETAIL = (PARTICLES_DRAW_ICON + PARTICLES_DRAW_BOUNDARIES);
/**
	\def PARTICLES_DRAW_FULL_DETAIL_BOLD
	Draw all of the preview rendering items using cylinders instead of lines to make the text and icon look BOLD.
*/
static const uint32_t PARTICLES_DRAW_FULL_DETAIL_BOLD = (PARTICLES_DRAW_FULL_DETAIL + PARTICLES_DRAW_WITH_CYLINDERS);
}

/**
\brief APEX asset preview wind asset.
*/
class ParticlesPreview : public AssetPreview
{
public:
	/**
	Set the scale of the icon.
	The unscaled icon has is 1.0x1.0 game units.
	By default the scale of the icon is 1.0. (unscaled)
	*/
	virtual void	setIconScale(float scale) = 0;
	/**
	Set the detail level of the preview rendering by selecting which features to enable.
	Any, all, or none of the following masks may be added together to select what should be drawn.
	The defines for the individual items are PARTICLES_DRAW_NOTHING, PARTICLES_DRAW_ICON, PARTICLES_DRAW_BOUNDARIES,
	PARTICLES_DRAW_WITH_CYLINDERS.
	All items may be drawn using the macro PARTICLES_DRAW_FULL_DETAIL and PARTICLES_DRAW_FULL_DETAIL_BOLD.
	*/
	virtual void	setDetailLevel(uint32_t detail) = 0;

protected:
	ParticlesPreview() {};
};


PX_POP_PACK

}
} // end namespace nvidia

#endif // PARTICLES_PREVIEW_H
