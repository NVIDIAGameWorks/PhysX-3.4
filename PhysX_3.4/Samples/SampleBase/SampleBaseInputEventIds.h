// This code contains NVIDIA Confidential Information and is disclosed to you
// under a form of NVIDIA software license agreement provided separately to you.
//
// Notice
// NVIDIA Corporation and its licensors retain all intellectual property and
// proprietary rights in and to this software and related documentation and
// any modifications thereto. Any use, reproduction, disclosure, or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA Corporation is strictly prohibited.
//
// ALL NVIDIA DESIGN SPECIFICATIONS, CODE ARE PROVIDED "AS IS.". NVIDIA MAKES
// NO WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES OF NONINFRINGEMENT,
// MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Information and code furnished is believed to be accurate and reliable.
// However, NVIDIA Corporation assumes no responsibility for the consequences of use of such
// information or for any infringement of patents or other rights of third parties that may
// result from its use. No license is granted by implication or otherwise under any patent
// or patent rights of NVIDIA Corporation. Details are subject to change without notice.
// This code supersedes and replaces all information previously supplied.
// NVIDIA Corporation products are not authorized for use as critical
// components in life support devices or systems without express written approval of
// NVIDIA Corporation.
//
// Copyright (c) 2008-2017 NVIDIA Corporation. All rights reserved.
#ifndef SAMPLE_BASE_INPUT_EVENT_IDS_H
#define SAMPLE_BASE_INPUT_EVENT_IDS_H

#include <SampleFrameworkInputEventIds.h>

// InputEvents used by SampleBase
enum SampleBaseInputEventIds
{
	SAMPLE_BASE_FIRST = NUM_SAMPLE_FRAMEWORK_INPUT_EVENT_IDS,

	PICKUP ,

	SPAWN_DEBUG_OBJECT ,

	PAUSE_SAMPLE ,
	STEP_ONE_FRAME ,
	TOGGLE_VISUALIZATION ,
	DECREASE_DEBUG_RENDER_SCALE ,
	INCREASE_DEBUG_RENDER_SCALE ,
	HIDE_GRAPHICS ,
	WIREFRAME ,	
	TOGGLE_PVD_CONNECTION ,
	SHOW_HELP ,
	SHOW_DESCRIPTION ,
	SHOW_EXTENDED_HELP ,
	VARIABLE_TIMESTEP,
	DELETE_PICKED,

	QUIT,
	MENU_VISUALIZATIONS,
	MENU_SAMPLES,

	MENU_ESCAPE,
	MENU_UP,
	MENU_DOWN,
	MENU_LEFT,
	MENU_RIGHT,
	MENU_SELECT,

	MENU_QUICK_UP,
	MENU_QUICK_DOWN,
	MENU_QUICK_LEFT,
	MENU_QUICK_RIGHT,

	TOGGLE_CPU_GPU,

	MOUSE_LOOK_BUTTON,

	CONSOLE_OPEN,
	CONSOLE_ESCAPE,
	CONSOLE_BACKSPACE,
	CONSOLE_ENTER,
	CONSOLE_SCROLL_UP,
	CONSOLE_SCROLL_DOWN,
	CONSOLE_LIST_COMMAND_UP,
	CONSOLE_LIST_COMMAND_DOWN,

	NEXT_PAGE,
	PREVIOUS_PAGE,

	RUN_NEXT_SAMPLE,
	RUN_PREVIOUS_SAMPLE,

	PROFILE_ONLY_PVD,

	NUM_SAMPLE_BASE_INPUT_EVENT_IDS, 
};

#endif
