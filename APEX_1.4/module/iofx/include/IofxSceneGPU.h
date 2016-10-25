/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __IOFX_SCENE_GPU_H__
#define __IOFX_SCENE_GPU_H__

#if ENABLE_TEST
#include "IofxTestScene.h"
#define IOFX_SCENE IofxTestScene
#else
#include "IofxScene.h"
#define IOFX_SCENE IofxScene
#endif

#if APEX_CUDA_SUPPORT

#include "ApexCudaWrapper.h"
#include "ApexCuda.h"
#include "CudaModuleScene.h"

#include "../cuda/include/common.h"
#include "ModifierData.h"

#define SCENE_CUDA_OBJ(scene, name) static_cast<IofxSceneGPU&>(scene).APEX_CUDA_OBJ_NAME(name)

namespace nvidia
{
namespace iofx
{

class IofxSceneGPU : public IOFX_SCENE, public CudaModuleScene
{
public:
	IofxSceneGPU(ModuleIofxImpl& module, SceneIntl& scene, RenderDebugInterface* debugRender, ResourceList& list);
	~IofxSceneGPU();

	void				submitTasks(float elapsedTime, float substepSize, uint32_t numSubSteps);

	virtual void		prepareRenderables();

	void*				getHeadCudaObj()
	{
		return CudaModuleScene::getHeadCudaObj();
	}

//CUDA module objects
#include "../cuda/include/moduleList.h"

protected:
	/* device and host pinned buffers, etc */
	PxCudaContextManager*	mContextManager;

	nvidia::Array<CUgraphicsResource>	mToMapArray, mToUnmapArray;
};

}
} // namespace nvidia

#endif
#endif
