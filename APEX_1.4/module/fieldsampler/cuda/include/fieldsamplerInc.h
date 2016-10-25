/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


APEX_CUDA_SURFACE_3D(surfRefFieldSamplerGridAccum)


#ifdef FIELD_SAMPLER_SEPARATE_KERNELS

APEX_CUDA_BOUND_KERNEL(FIELD_SAMPLER_POINTS_KERNEL_CONFIG, fieldSamplerPointsKernel,
                       ((nvidia::fieldsampler::FieldSamplerKernelParams, baseParams))
                       ((APEX_MEM_BLOCK(float4), accumField))
                       ((APEX_MEM_BLOCK(float4), accumVelocity))
                       ((APEX_MEM_BLOCK(const float4), positionMass))
                       ((APEX_MEM_BLOCK(const float4), velocity))
                       ((nvidia::fieldsampler::FieldSamplerParamsEx, paramsEx))
                       ((InplaceHandle<nvidia::fieldsampler::FieldSamplerQueryParams>, queryParamsHandle))
					   ((nvidia::fieldsampler::FieldSamplerKernelMode::Enum, kernelMode))
                      )
APEX_CUDA_FREE_KERNEL_3D(FIELD_SAMPLER_GRID_KERNEL_CONFIG, fieldSamplerGridKernel,
                       ((nvidia::fieldsampler::FieldSamplerKernelParams, baseParams))
                       ((nvidia::fieldsampler::FieldSamplerGridKernelParams, gridParams))
                       ((nvidia::fieldsampler::FieldSamplerParamsEx, paramsEx))
                       ((InplaceHandle<nvidia::fieldsampler::FieldSamplerQueryParams>, queryParamsHandle))
					   ((nvidia::fieldsampler::FieldSamplerKernelMode::Enum, kernelMode))
                      )

#ifndef __CUDACC__
#define LAUNCH_FIELD_SAMPLER_KERNEL( launchData ) \
	const ApexCudaConstStorage& _storage_ = *getFieldSamplerCudaConstStorage(); \
	InplaceHandle<nvidia::fieldsampler::FieldSamplerQueryParams> queryParamsHandle = _storage_.mappedHandle( launchData.queryParamsHandle ); \
	uint32_t fieldSamplerCount = launchData.fieldSamplerArray->size(); \
	switch( launchData.kernelType ) \
	{ \
	case nvidia::fieldsampler::FieldSamplerKernelType::POINTS: \
		{ \
			const nvidia::fieldsampler::FieldSamplerPointsKernelLaunchDataIntl& data = static_cast<const nvidia::fieldsampler::FieldSamplerPointsKernelLaunchDataIntl&>(launchData); \
			const nvidia::fieldsampler::FieldSamplerPointsKernelArgs* kernelArgs = static_cast<const nvidia::fieldsampler::FieldSamplerPointsKernelArgs*>(data.kernelArgs); \
			for (uint32_t i = 0, activeIdx = 0; i < fieldSamplerCount; ++i) \
			{ \
				const nvidia::fieldsampler::FieldSamplerWrapperGPU* wrapper = static_cast<const nvidia::fieldsampler::FieldSamplerWrapperGPU* >( (*data.fieldSamplerArray)[i].mFieldSamplerWrapper ); \
				if (wrapper->isEnabled()) \
				{ \
					nvidia::fieldsampler::FieldSamplerParamsEx paramsEx; \
					paramsEx.paramsHandle = _storage_.mappedHandle( wrapper->getParamsHandle() ); \
					paramsEx.multiplier = (*data.fieldSamplerArray)[i].mMultiplier; \
					nvidia::fieldsampler::FieldSamplerKernelMode::Enum kernelMode = (++activeIdx == data.activeFieldSamplerCount) ? data.kernelMode : nvidia::fieldsampler::FieldSamplerKernelMode::DEFAULT; \
					ON_LAUNCH_FIELD_SAMPLER_KERNEL( wrapper->getInternalFieldSampler(), wrapper->getInternalFieldSamplerDesc() ); \
					SCENE_CUDA_OBJ(this, fieldSamplerPointsKernel)( data.stream, data.threadCount, \
						*static_cast<const nvidia::fieldsampler::FieldSamplerKernelParams*>(kernelArgs), \
						nvidia::apex::createApexCudaMemRef(kernelArgs->accumField, size_t(data.memRefSize)), \
						nvidia::apex::createApexCudaMemRef(kernelArgs->accumVelocity, size_t(data.memRefSize)), \
						nvidia::apex::createApexCudaMemRef(kernelArgs->positionMass, size_t(data.memRefSize), ApexCudaMemFlags::IN), \
						nvidia::apex::createApexCudaMemRef(kernelArgs->velocity, size_t(data.memRefSize), ApexCudaMemFlags::IN), \
						paramsEx, queryParamsHandle, kernelMode ); \
				} \
			} \
		} \
		return true; \
	case nvidia::fieldsampler::FieldSamplerKernelType::GRID: \
		{ \
			const nvidia::fieldsampler::FieldSamplerGridKernelLaunchDataIntl& data = static_cast<const nvidia::fieldsampler::FieldSamplerGridKernelLaunchDataIntl&>(launchData); \
			const nvidia::fieldsampler::FieldSamplerGridKernelArgs* kernelArgs = static_cast<const nvidia::fieldsampler::FieldSamplerGridKernelArgs*>(data.kernelArgs); \
			SCENE_CUDA_OBJ(this, surfRefFieldSamplerGridAccum).bindTo(*data.accumArray, ApexCudaMemFlags::IN_OUT); \
			for (uint32_t i = 0, activeIdx = 0; i < fieldSamplerCount; ++i) \
			{ \
				const nvidia::fieldsampler::FieldSamplerWrapperGPU* wrapper = static_cast<const nvidia::fieldsampler::FieldSamplerWrapperGPU* >( (*data.fieldSamplerArray)[i].mFieldSamplerWrapper ); \
				if (wrapper->isEnabled() && wrapper->getInternalFieldSamplerDesc().gridSupportType == FieldSamplerGridSupportTypeIntl::VELOCITY_PER_CELL) \
				{ \
					nvidia::fieldsampler::FieldSamplerParamsEx paramsEx; \
					paramsEx.paramsHandle = _storage_.mappedHandle( wrapper->getParamsHandle() ); \
					paramsEx.multiplier = (*data.fieldSamplerArray)[i].mMultiplier; \
					nvidia::fieldsampler::FieldSamplerKernelMode::Enum kernelMode = (++activeIdx == data.activeFieldSamplerCount) ? data.kernelMode : nvidia::fieldsampler::FieldSamplerKernelMode::DEFAULT; \
					ON_LAUNCH_FIELD_SAMPLER_KERNEL( wrapper->getInternalFieldSampler(), wrapper->getInternalFieldSamplerDesc() ); \
					SCENE_CUDA_OBJ(this, fieldSamplerGridKernel)( data.stream, data.threadCountX, data.threadCountY, data.threadCountZ, \
						*static_cast<const nvidia::fieldsampler::FieldSamplerKernelParams*>(kernelArgs), \
						*static_cast<const nvidia::fieldsampler::FieldSamplerGridKernelParams*>(kernelArgs), \
						paramsEx, queryParamsHandle, kernelMode ); \
				} \
			} \
			SCENE_CUDA_OBJ(this, surfRefFieldSamplerGridAccum).unbind(); \
		} \
		return true; \
	default: \
		PX_ALWAYS_ASSERT(); \
		return false; \
	};
#endif

#else

APEX_CUDA_BOUND_KERNEL(FIELD_SAMPLER_POINTS_KERNEL_CONFIG, fieldSamplerPointsKernel,
                       ((nvidia::fieldsampler::FieldSamplerKernelParams, baseParams))
                       ((APEX_MEM_BLOCK(float4), accumField))
                       ((APEX_MEM_BLOCK(float4), accumVelocity))
                       ((APEX_MEM_BLOCK(const float4), positionMass))
                       ((APEX_MEM_BLOCK(const float4), velocity))
                       ((InplaceHandle<nvidia::fieldsampler::FieldSamplerParamsExArray>, paramsExArrayHandle))
                       ((InplaceHandle<nvidia::fieldsampler::FieldSamplerQueryParams>, queryParamsHandle))
					   ((nvidia::fieldsampler::FieldSamplerKernelMode::Enum, kernelMode))
                      )
APEX_CUDA_FREE_KERNEL_3D(FIELD_SAMPLER_GRID_KERNEL_CONFIG, fieldSamplerGridKernel,
                       ((nvidia::fieldsampler::FieldSamplerKernelParams, baseParams))
                       ((nvidia::fieldsampler::FieldSamplerGridKernelParams, gridParams))
                       ((InplaceHandle<nvidia::fieldsampler::FieldSamplerParamsExArray>, paramsExArrayHandle))
                       ((InplaceHandle<nvidia::fieldsampler::FieldSamplerQueryParams>, queryParamsHandle))
					   ((nvidia::fieldsampler::FieldSamplerKernelMode::Enum, kernelMode))
                      )

#ifndef __CUDACC__
#define LAUNCH_FIELD_SAMPLER_KERNEL( launchData ) \
	const ApexCudaConstStorage& _storage_ = *getFieldSamplerCudaConstStorage(); \
	InplaceHandle<nvidia::fieldsampler::FieldSamplerParamsExArray> paramsExArrayHandle = _storage_.mappedHandle( launchData.paramsExArrayHandle ); \
	InplaceHandle<nvidia::fieldsampler::FieldSamplerQueryParams> queryParamsHandle = _storage_.mappedHandle( launchData.queryParamsHandle ); \
	switch( launchData.kernelType ) \
	{ \
	case nvidia::fieldsampler::FieldSamplerKernelType::POINTS: \
		{ \
			const nvidia::fieldsampler::FieldSamplerPointsKernelLaunchDataIntl& data = static_cast<const nvidia::fieldsampler::FieldSamplerPointsKernelLaunchDataIntl&>(launchData); \
			const nvidia::fieldsampler::FieldSamplerPointsKernelArgs* kernelArgs = static_cast<const nvidia::fieldsampler::FieldSamplerPointsKernelArgs*>(data.kernelArgs); \
			SCENE_CUDA_OBJ(this, fieldSamplerPointsKernel)( data.stream, data.threadCount, \
				*static_cast<const nvidia::fieldsampler::FieldSamplerKernelParams*>(kernelArgs), \
				nvidia::apex::createApexCudaMemRef(kernelArgs->accumField, size_t(data.memRefSize)), \
				nvidia::apex::createApexCudaMemRef(kernelArgs->accumVelocity, size_t(data.memRefSize)), \
				nvidia::apex::createApexCudaMemRef(kernelArgs->positionMass, size_t(data.memRefSize), ApexCudaMemFlags::IN), \
				nvidia::apex::createApexCudaMemRef(kernelArgs->velocity, size_t(data.memRefSize), ApexCudaMemFlags::IN), \
				paramsExArrayHandle, queryParamsHandle, data.kernelMode ); \
		} \
		return true; \
	case nvidia::fieldsampler::FieldSamplerKernelType::GRID: \
		{ \
			const nvidia::fieldsampler::FieldSamplerGridKernelLaunchDataIntl& data = static_cast<const nvidia::fieldsampler::FieldSamplerGridKernelLaunchDataIntl&>(launchData); \
			const nvidia::fieldsampler::FieldSamplerGridKernelArgs* kernelArgs = static_cast<const nvidia::fieldsampler::FieldSamplerGridKernelArgs*>(data.kernelArgs); \
			SCENE_CUDA_OBJ(this, surfRefFieldSamplerGridAccum).bindTo(*data.accumArray, ApexCudaMemFlags::IN_OUT); \
			SCENE_CUDA_OBJ(this, fieldSamplerGridKernel)( data.stream, data.threadCountX, data.threadCountY, data.threadCountZ, \
				*static_cast<const nvidia::fieldsampler::FieldSamplerKernelParams*>(kernelArgs), \
				*static_cast<const nvidia::fieldsampler::FieldSamplerGridKernelParams*>(kernelArgs), \
				paramsExArrayHandle, queryParamsHandle, data.kernelMode ); \
			SCENE_CUDA_OBJ(this, surfRefFieldSamplerGridAccum).unbind(); \
		} \
		return true; \
	default: \
		PX_ALWAYS_ASSERT(); \
		return false; \
	};
#endif

#endif //FIELD_SAMPLER_SEPARATE_KERNELS
