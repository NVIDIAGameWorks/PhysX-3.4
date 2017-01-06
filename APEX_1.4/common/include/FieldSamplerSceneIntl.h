/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef FIELD_SAMPLER_SCENE_INTL_H
#define FIELD_SAMPLER_SCENE_INTL_H

#include "ApexDefs.h"
#include "PxTask.h"

#include "ModuleIntl.h"
#include "ApexSDKIntl.h"

namespace nvidia
{

namespace fieldsampler
{
	struct FieldSamplerKernelLaunchDataIntl;
}

namespace apex
{

class ApexCudaConstStorage;

struct FieldSamplerSceneDescIntl
{
	bool	isPrimary;

	FieldSamplerSceneDescIntl()
	{
		isPrimary = false;
	}
};

struct FieldSamplerQueryDataIntl;

class FieldSamplerSceneIntl : public ModuleSceneIntl
{
public:
	virtual void getFieldSamplerSceneDesc(FieldSamplerSceneDescIntl& desc) const = 0;

	virtual const PxTask* onSubmitFieldSamplerQuery(const FieldSamplerQueryDataIntl& data, const PxTask* )
	{
		PX_UNUSED(data);
		return 0;
	}

#if APEX_CUDA_SUPPORT
	virtual ApexCudaConstStorage*	getFieldSamplerCudaConstStorage()
	{
		APEX_INVALID_OPERATION("not implemented");
		return 0;
	}

	virtual bool					launchFieldSamplerCudaKernel(const nvidia::fieldsampler::FieldSamplerKernelLaunchDataIntl&)
	{
		APEX_INVALID_OPERATION("not implemented");
		return false;
	}
#endif

	virtual SceneStats* getStats()
	{
		return 0;
	}

};

#define FSST_PHYSX_MONITOR_LOAD		"FieldSamplerScene::PhysXMonitorLoad"
#define FSST_PHYSX_MONITOR_FETCH	"FieldSamplerScene::PhysXMonitorFetch"
#define FSST_PHYSX_MONITOR_UPDATE	"FieldSamplerPhysXMonitor::Update"
}

} // end namespace nvidia::apex

#endif // FIELD_SAMPLER_SCENE_INTL_H
