/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef JET_FSPREVIEW_H
#define JET_FSPREVIEW_H

#include "Apex.h"
#include "AssetPreview.h"

namespace nvidia
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

class RenderDebugInterface;

/**
	\brief The APEX_JET namespace contains the defines for setting the preview detail levels.

	JET_DRAW_NOTHING - draw nothing<BR>
	JET_DRAW_SHAPE - draw the shape<BR>
	JET_DRAW_ASSET_INFO - draw the asset info in the screen space<BR>
	JET_DRAW_FULL_DETAIL - draw all components of the preview<BR>
*/
namespace APEX_JET
{
/**
	\def JET_DRAW_NOTHING
	Draw no preview items.
*/
static const uint32_t JET_DRAW_NOTHING = 0x00;
/**
	\def JET_DRAW_SHAPE
	Draw the shape.  The top of the shape should be above ground and the bottom should be below ground.
*/
static const uint32_t JET_DRAW_SHAPE = 0x01;
/**
	\def JET_DRAW_ASSET_INFO
	Draw the Asset Info in Screen Space.  This displays the various asset members that are relevant to the current asset.
	parameters that are not relevant because they are disabled are not displayed.
*/
static const uint32_t JET_DRAW_ASSET_INFO = 0x02;
/**
	\def JET_DRAW_FULL_DETAIL
	Draw all of the preview rendering items.
*/
static const uint32_t JET_DRAW_FULL_DETAIL = (JET_DRAW_SHAPE + JET_DRAW_ASSET_INFO);
}

/**
\brief This class provides the asset preview for APEX TurbulenceFS Assets.  The class provides multiple levels of prevew
detail that can be selected individually.
*/
class JetFSPreview : public AssetPreview
{
public:
	/**
	Set the detail level of the preview rendering by selecting which features to enable.<BR>
	Any, all, or none of the following masks may be added together to select what should be drawn.<BR>
	The defines for the individual items are:<br>
		JET_DRAW_NOTHING - draw nothing<BR>
		JET_DRAW_SHAPE - draw the shape<BR>
		JET_DRAW_ASSET_INFO - draw the turbulencefs asset info in the screen space<BR>
		JET_DRAW_FULL_DETAIL - draw all components of the turbulencefs preview<BR>
	All items may be drawn using the macro DRAW_FULL_DETAIL.
	*/
	virtual void	setDetailLevel(uint32_t detail) = 0;
};


PX_POP_PACK

}
} // namespace nvidia

#endif // JET_FSPREVIEW_H
