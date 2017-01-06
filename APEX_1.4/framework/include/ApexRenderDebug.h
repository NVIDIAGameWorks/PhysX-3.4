/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef APEX_RENDER_DEBUG_H
#define APEX_RENDER_DEBUG_H

#include "RenderDebugInterface.h"

namespace nvidia
{
namespace apex
{

class ApexSDKImpl;

RenderDebugInterface* createApexRenderDebug(ApexSDKImpl* a, RENDER_DEBUG::RenderDebugInterface* interface, bool useRemote);
void				releaseApexRenderDebug(RenderDebugInterface* n);

}
} // end namespace nvidia::apex

#endif
