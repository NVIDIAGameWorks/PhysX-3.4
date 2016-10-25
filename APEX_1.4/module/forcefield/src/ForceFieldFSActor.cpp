/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "ApexDefs.h"
#include "ForceFieldActorImpl.h"
#include "ForceFieldAssetImpl.h"
#include "ForceFieldScene.h"
#include "FieldSamplerManagerIntl.h"
#include "ApexResourceHelper.h"
#include "ApexUsingNamespace.h"

#include "SceneIntl.h"

namespace nvidia
{
namespace forcefield
{

void ForceFieldActorImpl::initFieldSampler(const ForceFieldActorDesc& desc)
{
	FieldSamplerManagerIntl* fieldSamplerManager = mForceFieldScene->getInternalFieldSamplerManager();
	if (fieldSamplerManager != 0)
	{
		FieldSamplerDescIntl fieldSamplerDesc;
		fieldSamplerDesc.type = FieldSamplerTypeIntl::FORCE;
		fieldSamplerDesc.gridSupportType = FieldSamplerGridSupportTypeIntl::SINGLE_VELOCITY;

		fieldSamplerDesc.samplerFilterData = desc.samplerFilterData;
		fieldSamplerDesc.boundaryFilterData = desc.boundaryFilterData;
		
		fieldSamplerManager->registerFieldSampler(this, fieldSamplerDesc, mForceFieldScene);
		mFieldSamplerChanged = true;
	}
}

void ForceFieldActorImpl::releaseFieldSampler()
{
	FieldSamplerManagerIntl* fieldSamplerManager = mForceFieldScene->getInternalFieldSamplerManager();
	if (fieldSamplerManager != 0)
	{
		fieldSamplerManager->unregisterFieldSampler(this);
	}
}

bool ForceFieldActorImpl::updateFieldSampler(FieldShapeDescIntl& shapeDesc, bool& isEnabled)
{
	isEnabled = mEnable;
	if (mFieldSamplerChanged)
	{
		shapeDesc.type = FieldShapeTypeIntl::NONE;	//not using field sampler include shape (force field has its own implementation for shapes)

		//copy to buffered kernel data for execution
		memcpy(&mKernelExecutionParams, &mKernelParams, sizeof(ForceFieldFSKernelParamsUnion));
		mFieldSamplerChanged = false;
		return true;
	}
	return false;
}

/******************************** CPU Version ********************************/

ForceFieldActorCPU::ForceFieldActorCPU(const ForceFieldActorDesc& desc, ForceFieldAssetImpl& asset, ResourceList& list, ForceFieldScene& scene)
	: ForceFieldActorImpl(desc, asset, list, scene)
{
}

ForceFieldActorCPU::~ForceFieldActorCPU()
{
}

void ForceFieldActorCPU::executeFieldSampler(const ExecuteData& data)
{
	// totalElapsedMS is always 0 in PhysX 3
	uint32_t totalElapsedMS = mForceFieldScene->getApexScene().getTotalElapsedMS();

	if (mKernelParams.kernelType == ForceFieldKernelType::RADIAL)
	{
		for (uint32_t iter = 0; iter < data.count; ++iter)
		{
			uint32_t i = data.indices[iter & data.indicesMask] + (iter & ~data.indicesMask);
			PxVec3* pos = (PxVec3*)((uint8_t*)data.position + i * data.positionStride);
			data.resultField[iter] = executeForceFieldFS(mKernelExecutionParams.getRadialForceFieldFSKernelParams(), *pos, totalElapsedMS);
		}
	}
	else if (mKernelParams.kernelType == ForceFieldKernelType::GENERIC)
	{
		for (uint32_t iter = 0; iter < data.count; ++iter)
		{
			uint32_t i = data.indices[iter & data.indicesMask] + (iter & ~data.indicesMask);
			PxVec3* pos = (PxVec3*)((uint8_t*)data.position + i * data.positionStride);
			PxVec3* vel = (PxVec3*)((uint8_t*)data.velocity + i * data.velocityStride);

			data.resultField[iter] = executeForceFieldFS(mKernelExecutionParams.getGenericForceFieldFSKernelParams(), *pos, *vel, totalElapsedMS);
		}
	}
}

/******************************** GPU Version ********************************/

#if APEX_CUDA_SUPPORT

ForceFieldActorGPU::ForceFieldActorGPU(const ForceFieldActorDesc& desc, ForceFieldAssetImpl& asset, ResourceList& list, ForceFieldScene& scene)
	: ForceFieldActorCPU(desc, asset, list, scene)
	, mConstMemGroup(CUDA_OBJ(fieldSamplerStorage))
{
}

ForceFieldActorGPU::~ForceFieldActorGPU()
{
}

bool ForceFieldActorGPU::updateFieldSampler(FieldShapeDescIntl& shapeDesc, bool& isEnabled)
{
	if (ForceFieldActorImpl::updateFieldSampler(shapeDesc, isEnabled))
	{
		APEX_CUDA_CONST_MEM_GROUP_SCOPE(mConstMemGroup);

		if (mParamsHandle.isNull())
		{
			mParamsHandle.alloc(_storage_);
		}
		
		if (mKernelParams.kernelType == ForceFieldKernelType::GENERIC)
		{
			mParamsHandle.update(_storage_, mKernelExecutionParams.getGenericForceFieldFSKernelParams());
		}
		else if (mKernelParams.kernelType == ForceFieldKernelType::RADIAL)
		{
			mParamsHandle.update(_storage_, mKernelExecutionParams.getRadialForceFieldFSKernelParams());
		}
		else
		{
			PX_ASSERT("Wrong kernel type");
		}
		
		return true;
	}
	return false;
}

void ForceFieldActorGPU::getFieldSamplerCudaExecuteInfo(CudaExecuteInfo& info) const
{
	if (mKernelParams.kernelType == ForceFieldKernelType::GENERIC)
	{
		info.executeType = 1;
	}
	else if (mKernelParams.kernelType == ForceFieldKernelType::RADIAL)
	{
		info.executeType = 2;
	}
	else
	{
		PX_ASSERT("Wrong kernel type");
		info.executeType = 0;
	}
	info.executeParamsHandle = mParamsHandle;
}

#endif

}
} // namespace nvidia::apex

