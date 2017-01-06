/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __CUDA_MODULE_SCENE__
#define __CUDA_MODULE_SCENE__

// Make this header is safe for inclusion in headers that are shared with device code.
#if !defined(__CUDACC__)

#include "ApexDefs.h"
#if APEX_CUDA_SUPPORT
#include "SceneIntl.h"
#include "ApexUsingNamespace.h"
#include "PsArray.h"
#include <cuda.h>
#include "ApexCudaWrapper.h"

namespace physx
{
	class PxGpuDispatcher;
}

namespace nvidia
{
class PhysXGpuIndicator;
namespace apex
{
class CudaTestManager;

/* Every CUDA capable ModuleSceneIntl will derive this class.  It
 * provides the access methods to your CUDA kernels that were compiled
 * into object files by nvcc.
 */
class CudaModuleScene : public ApexCudaObjManager
{
public:
	CudaModuleScene(SceneIntl& scene, Module& module, const char* modulePrefix = "");
	virtual ~CudaModuleScene() {}

	void destroy(SceneIntl& scene);

	void*	getHeadCudaObj()
	{
		return ApexCudaObjManager::getObjListHead();
	}

	nvidia::PhysXGpuIndicator*		mPhysXGpuIndicator;

	//ApexCudaObjManager
	virtual void onBeforeLaunchApexCudaFunc(const ApexCudaFunc& func, CUstream stream);
	virtual void onAfterLaunchApexCudaFunc(const ApexCudaFunc& func, CUstream stream);

protected:
	ApexCudaModule* getCudaModule(int modIndex);

	void initCudaObj(ApexCudaTexRef& obj);
	void initCudaObj(ApexCudaVar& obj);
	void initCudaObj(ApexCudaFunc& obj);
	void initCudaObj(ApexCudaSurfRef& obj);

	physx::Array<ApexCudaModule> mCudaModules;

	SceneIntl& mSceneIntl;

private:
	CudaModuleScene& operator=(const CudaModuleScene&);
};

}
} // namespace nvidia

#endif
#endif

#endif // __CUDA_MODULE_SCENE__
