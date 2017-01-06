/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef FIELD_SAMPLER_INTL_H
#define FIELD_SAMPLER_INTL_H

#include "InplaceTypes.h"
#include "FieldBoundaryIntl.h"

#ifndef __CUDACC__
#include "ApexSDKIntl.h"
#endif

namespace nvidia
{
namespace apex
{


struct FieldSamplerTypeIntl
{
	enum Enum
	{
		FORCE,
		ACCELERATION,
		VELOCITY_DRAG,
		VELOCITY_DIRECT,
	};
};

struct FieldSamplerGridSupportTypeIntl
{
	enum Enum
	{
		NONE = 0,
		SINGLE_VELOCITY,
		VELOCITY_PER_CELL,
	};
};

#ifndef __CUDACC__

struct FieldSamplerDescIntl
{
	FieldSamplerTypeIntl::Enum			type;
	FieldSamplerGridSupportTypeIntl::Enum	gridSupportType;
	bool								cpuSimulationSupport;
#if PX_PHYSICS_VERSION_MAJOR == 3
	PxFilterData						samplerFilterData;
	PxFilterData						boundaryFilterData;
#endif
	float						boundaryFadePercentage;

	float						dragCoeff; //only used then type is VELOCITY_DRAG

	void*								userData;

	FieldSamplerDescIntl()
	{
		type                     = FieldSamplerTypeIntl::FORCE;
		gridSupportType          = FieldSamplerGridSupportTypeIntl::NONE;
		cpuSimulationSupport = true;
#if PX_PHYSICS_VERSION_MAJOR == 3
		samplerFilterData.word0  = 0xFFFFFFFF;
		samplerFilterData.word1  = 0xFFFFFFFF;
		samplerFilterData.word2  = 0xFFFFFFFF;
		samplerFilterData.word3  = 0xFFFFFFFF;
		boundaryFilterData.word0 = 0xFFFFFFFF;
		boundaryFilterData.word1 = 0xFFFFFFFF;
		boundaryFilterData.word2 = 0xFFFFFFFF;
		boundaryFilterData.word3 = 0xFFFFFFFF;
#endif
		boundaryFadePercentage   = 0.1;
		dragCoeff                = 0;
		userData                 = NULL;
	}
};


class FieldSamplerIntl
{
public:
	//returns true if shape/params was changed
	//required to return true on first call!
	virtual bool updateFieldSampler(FieldShapeDescIntl& shapeDesc, bool& isEnabled) = 0;

	struct ExecuteData
	{
		uint32_t            count;
		uint32_t            positionStride;
		uint32_t            velocityStride;
		uint32_t            massStride;
		uint32_t            indicesMask;
		const float*	    position;
		const float*		velocity;
		const float*		mass;
		const uint32_t*		indices;
		PxVec3*	        resultField;
	};

	virtual void executeFieldSampler(const ExecuteData& data)
	{
		PX_UNUSED(data);
		APEX_INVALID_OPERATION("not implemented");
	}

#if APEX_CUDA_SUPPORT
	struct CudaExecuteInfo
	{
		uint32_t		executeType;
		InplaceHandleBase	executeParamsHandle;
	};

	virtual void getFieldSamplerCudaExecuteInfo(CudaExecuteInfo& info) const
	{
		PX_UNUSED(info);
		APEX_INVALID_OPERATION("not implemented");
	}
#endif

	virtual PxVec3 queryFieldSamplerVelocity() const
	{
		APEX_INVALID_OPERATION("not implemented");
		return PxVec3(0.0f);
	}

protected:
	virtual ~FieldSamplerIntl() {}
};

#endif // __CUDACC__

}
} // end namespace nvidia::apex

#endif // #ifndef FIELD_SAMPLER_INTL_H
