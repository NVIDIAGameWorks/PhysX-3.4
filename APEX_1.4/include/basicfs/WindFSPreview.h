/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef WIND_FSPREVIEW_H
#define WIND_FSPREVIEW_H

#include "Apex.h"
#include "AssetPreview.h"

namespace nvidia
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

class RenderDebugInterface;

/**
	\brief The APEX_WIND namespace contains the defines for setting the preview detail levels.

	WIND_DRAW_NOTHING - draw nothing<BR>
	WIND_DRAW_ASSET_INFO - draw the asset info in the screen space<BR>
	WIND_DRAW_FULL_DETAIL - draw all components of the preview<BR>
*/
namespace APEX_WIND
{
/**
	\def WIND_DRAW_NOTHING
	Draw no preview items.
*/
static const uint32_t WIND_DRAW_NOTHING = 0x00;
/**
	\def WIND_DRAW_ASSET_INFO
	Draw the Asset Info in Screen Space.  This displays the various asset members that are relevant to the current asset.
	parameters that are not relevant because they are disabled are not displayed.
*/
static const uint32_t WIND_DRAW_ASSET_INFO = 0x02;
/**
	\def WIND_DRAW_FULL_DETAIL
	Draw all of the preview rendering items.
*/
static const uint32_t WIND_DRAW_FULL_DETAIL = (WIND_DRAW_ASSET_INFO);
}

/**
\brief This class provides the asset preview for APEX WindFS Assets.  The class provides multiple levels of prevew
detail that can be selected individually.
*/
class WindFSPreview : public AssetPreview
{
public:
	/**
	Set the detail level of the preview rendering by selecting which features to enable.<BR>
	Any, all, or none of the following masks may be added together to select what should be drawn.<BR>
	The defines for the individual items are:<br>
		WIND_DRAW_NOTHING - draw nothing<BR>
		WIND_DRAW_ASSET_INFO - draw the asset info in the screen space<BR>
		WIND_DRAW_FULL_DETAIL - draw all components of the asset preview<BR>
	All items may be drawn using the macro DRAW_FULL_DETAIL.
	*/
	virtual void	setDetailLevel(uint32_t detail) = 0;
};


PX_POP_PACK

}
} // namespace nvidia

#endif // WIND_FSPREVIEW_H
