/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __IOFX_SCENE_CPU_H__
#define __IOFX_SCENE_CPU_H__

#if ENABLE_TEST
#include "IofxTestScene.h"
#define IOFX_SCENE IofxTestScene
#else
#include "IofxScene.h"
#define IOFX_SCENE IofxScene
#endif

namespace nvidia
{
namespace iofx
{

class IofxSceneCPU : public IOFX_SCENE
{
public:
	IofxSceneCPU(ModuleIofxImpl& module, SceneIntl& scene, RenderDebugInterface* debugRender, ResourceList& list);
};

}
} // namespace nvidia

#endif
