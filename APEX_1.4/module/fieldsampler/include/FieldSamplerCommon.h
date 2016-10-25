/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __FIELD_SAMPLER_COMMON_H__
#define __FIELD_SAMPLER_COMMON_H__

#include "PxVec3.h"
#include "PxVec4.h"
#include <PxMat44.h>

#include <FieldSamplerIntl.h>
#include <FieldBoundaryIntl.h>

#include "PsArray.h"

#if APEX_CUDA_SUPPORT
#pragma warning(push)
#pragma warning(disable:4201)
#pragma warning(disable:4408)

#include <vector_types.h>

#pragma warning(pop)
#endif

#define FIELD_SAMPLER_MULTIPLIER_VALUE 1
#define FIELD_SAMPLER_MULTIPLIER_WEIGHT 2
//0, FIELD_SAMPLER_MULTIPLIER_VALUE or FIELD_SAMPLER_MULTIPLIER_WEIGHT
#define FIELD_SAMPLER_MULTIPLIER FIELD_SAMPLER_MULTIPLIER_WEIGHT

namespace nvidia
{
namespace apex
{
	class ApexCudaArray;
}

namespace fieldsampler
{

#define VELOCITY_WEIGHT_THRESHOLD 0.00001f

struct FieldSamplerExecuteArgs
{
	physx::PxVec3			position;
	float			mass;
	physx::PxVec3			velocity;

	float			elapsedTime;
	uint32_t			totalElapsedMS;
};

//struct FieldShapeParams
#define INPLACE_TYPE_STRUCT_NAME FieldShapeParams
#define INPLACE_TYPE_STRUCT_BASE FieldShapeDescIntl
#define INPLACE_TYPE_STRUCT_FIELDS \
	INPLACE_TYPE_FIELD(float,	fade)
#include INPLACE_TYPE_BUILD()


//struct FieldShapeGroupParams
#define INPLACE_TYPE_STRUCT_NAME FieldShapeGroupParams
#define INPLACE_TYPE_STRUCT_FIELDS \
	INPLACE_TYPE_FIELD(InplaceArray<FieldShapeParams>,	shapeArray)
#include INPLACE_TYPE_BUILD()


//struct FieldSamplerParams
#define INPLACE_TYPE_STRUCT_NAME FieldSamplerParams
#define INPLACE_TYPE_STRUCT_FIELDS \
	INPLACE_TYPE_FIELD(uint32_t,										executeType) \
	INPLACE_TYPE_FIELD(InplaceHandleBase,									executeParamsHandle) \
	INPLACE_TYPE_FIELD(InplaceEnum<FieldSamplerTypeIntl::Enum>,				type) \
	INPLACE_TYPE_FIELD(InplaceEnum<FieldSamplerGridSupportTypeIntl::Enum>,	gridSupportType) \
	INPLACE_TYPE_FIELD(float,										dragCoeff) \
	INPLACE_TYPE_FIELD(FieldShapeParams,									includeShape) \
	INPLACE_TYPE_FIELD(InplaceArray<InplaceHandle<FieldShapeGroupParams> >,	excludeShapeGroupHandleArray)
#include INPLACE_TYPE_BUILD()


//struct FieldSamplerQueryParams
#define INPLACE_TYPE_STRUCT_NAME FieldSamplerQueryParams
#include INPLACE_TYPE_BUILD()


//struct FieldSamplerParams
#define INPLACE_TYPE_STRUCT_NAME FieldSamplerParamsEx
#define INPLACE_TYPE_STRUCT_FIELDS \
	INPLACE_TYPE_FIELD(InplaceHandle<FieldSamplerParams>,	paramsHandle) \
	INPLACE_TYPE_FIELD(float,						multiplier)
#include INPLACE_TYPE_BUILD()

typedef InplaceArray< FieldSamplerParamsEx, false > FieldSamplerParamsExArray;


#if APEX_CUDA_SUPPORT || defined(__CUDACC__)

struct FieldSamplerKernelType
{
	enum Enum
	{
		POINTS,
		GRID
	};
};

struct FieldSamplerKernelParams
{
	float	elapsedTime;
	physx::PxVec3	cellSize;
	uint32_t	totalElapsedMS;
};

struct FieldSamplerGridKernelParams
{
	uint32_t numX, numY, numZ;

	PxMat44 gridToWorld;

	float mass;
};


struct FieldSamplerKernelArgs : FieldSamplerKernelParams
{
};

struct FieldSamplerPointsKernelArgs : FieldSamplerKernelArgs
{
	float4* accumField;
	float4* accumVelocity;
	const float4* positionMass;
	const float4* velocity;
};

struct FieldSamplerGridKernelArgs : FieldSamplerKernelArgs, FieldSamplerGridKernelParams
{
};

struct FieldSamplerKernelMode
{
	enum Enum
	{
		DEFAULT = 0,
		FINISH_PRIMARY = 1,
		FINISH_SECONDARY = 2
	};
};

#endif

class FieldSamplerWrapper;

struct FieldSamplerInfo
{
	FieldSamplerWrapper*	mFieldSamplerWrapper;
	float			mMultiplier;
};

#if APEX_CUDA_SUPPORT

struct FieldSamplerKernelLaunchDataIntl
{
	CUstream                                        stream;
	FieldSamplerKernelType::Enum                    kernelType;
	const FieldSamplerKernelArgs*                   kernelArgs;
	InplaceHandle<FieldSamplerQueryParams>          queryParamsHandle;
	InplaceHandle<FieldSamplerParamsExArray>        paramsExArrayHandle;
	const nvidia::Array<FieldSamplerInfo>*           fieldSamplerArray;
	uint32_t                                    activeFieldSamplerCount;
	FieldSamplerKernelMode::Enum                    kernelMode;
};

struct FieldSamplerPointsKernelLaunchDataIntl : FieldSamplerKernelLaunchDataIntl
{
	uint32_t                                    threadCount;
	uint32_t                                    memRefSize;
};

struct FieldSamplerGridKernelLaunchDataIntl : FieldSamplerKernelLaunchDataIntl
{
	uint32_t									threadCountX;
	uint32_t									threadCountY;
	uint32_t									threadCountZ;
	ApexCudaArray*								accumArray;
};

#endif

APEX_CUDA_CALLABLE PX_INLINE float evalFade(float dist, float fade)
{
	float x = (1 - dist) / (fade + 1e-5f);
	return physx::PxClamp<float>(x, 0, 1);
}

APEX_CUDA_CALLABLE PX_INLINE float evalFadeAntialiasing(float dist, float fade, float cellRadius)
{
	const float f = fade;
	const float r = cellRadius;
	const float x = dist - 1.0f;

	float res = 0.0f;
	//linear part
	//if (x - r < -f)
	{
		const float a = PxMin(x - r, -f);
		const float b = PxMin(x + r, -f);

		res += (b - a);
	}
	//quadratic part
	if (f >= 1e-5f)
	{
		//if (x - r < 0.0f && x + r > -f)
		{
			const float a = physx::PxClamp(x - r, -f, 0.0f);
			const float b = physx::PxClamp(x + r, -f, 0.0f);

			res += (a*a - b*b) / (2 * f);
		}
	}
	return res / (2 * r);
}


APEX_CUDA_CALLABLE PX_INLINE float evalDistInShapeNONE(const FieldShapeDescIntl& /*shapeParams*/, const physx::PxVec3& /*worldPos*/)
{
	return 0.0f; //always inside
}

APEX_CUDA_CALLABLE PX_INLINE float evalDistInShapeSPHERE(const FieldShapeDescIntl& shapeParams, const physx::PxVec3& worldPos)
{
	const physx::PxVec3 shapePos = shapeParams.worldToShape.transform(worldPos);
	const float radius = shapeParams.dimensions.x;
	return shapePos.magnitude() / radius;
}

APEX_CUDA_CALLABLE PX_INLINE float evalDistInShapeBOX(const FieldShapeDescIntl& shapeParams, const physx::PxVec3& worldPos)
{
	const physx::PxVec3 shapePos = shapeParams.worldToShape.transform(worldPos);
	const physx::PxVec3& halfSize = shapeParams.dimensions;
	physx::PxVec3 unitPos(shapePos.x / halfSize.x, shapePos.y / halfSize.y, shapePos.z / halfSize.z);
	return physx::PxVec3(physx::PxAbs(unitPos.x), physx::PxAbs(unitPos.y), physx::PxAbs(unitPos.z)).maxElement();
}

APEX_CUDA_CALLABLE PX_INLINE float evalDistInShapeCAPSULE(const FieldShapeDescIntl& shapeParams, const physx::PxVec3& worldPos)
{
	const physx::PxVec3 shapePos = shapeParams.worldToShape.transform(worldPos);
	const float radius = shapeParams.dimensions.x;
	const float halfHeight = shapeParams.dimensions.y * 0.5f;

	physx::PxVec3 clampPos = shapePos;
	clampPos.y -= physx::PxClamp(shapePos.y, -halfHeight, +halfHeight);

	return clampPos.magnitude() / radius;
}

APEX_CUDA_CALLABLE PX_INLINE float evalDistInShape(const FieldShapeDescIntl& shapeParams, const physx::PxVec3& worldPos)
{
	switch (shapeParams.type)
	{
	case FieldShapeTypeIntl::NONE:
		return evalDistInShapeNONE(shapeParams, worldPos);
	case FieldShapeTypeIntl::SPHERE:
		return evalDistInShapeSPHERE(shapeParams, worldPos);
	case FieldShapeTypeIntl::BOX:
		return evalDistInShapeBOX(shapeParams, worldPos);
	case FieldShapeTypeIntl::CAPSULE:
		return evalDistInShapeCAPSULE(shapeParams, worldPos);
	default:
		return 1.0f; //always outside
	};
}

APEX_CUDA_CALLABLE PX_INLINE physx::PxVec3 scaleToShape(const FieldShapeDescIntl& shapeParams, const physx::PxVec3& worldVec)
{
	switch (shapeParams.type)
	{
	case FieldShapeTypeIntl::SPHERE:
	case FieldShapeTypeIntl::CAPSULE:
	{
		const float radius = shapeParams.dimensions.x;
		return physx::PxVec3(worldVec.x / radius, worldVec.y / radius, worldVec.z / radius);
	}
	case FieldShapeTypeIntl::BOX:
	{
		const physx::PxVec3& halfSize = shapeParams.dimensions;
		return physx::PxVec3(worldVec.x / halfSize.x, worldVec.y / halfSize.y, worldVec.z / halfSize.z);
	}
	default:
		return worldVec;
	};
}


APEX_CUDA_CALLABLE PX_INLINE float evalWeightInShape(const FieldShapeParams& shapeParams, const physx::PxVec3& position)
{
	float dist = nvidia::fieldsampler::evalDistInShape(shapeParams, position);
	return nvidia::fieldsampler::evalFade(dist, shapeParams.fade) * shapeParams.weight;
}

APEX_CUDA_CALLABLE PX_INLINE void accumFORCE(const FieldSamplerExecuteArgs& args,
	const physx::PxVec3& field, float fieldW,
	physx::PxVec4& accumAccel, physx::PxVec4& accumVelocity)
{
	PX_UNUSED(accumVelocity);

	physx::PxVec3 newAccel = ((1 - accumAccel.w) * fieldW * args.elapsedTime / args.mass) * field;
	accumAccel.x += newAccel.x;
	accumAccel.y += newAccel.y;
	accumAccel.z += newAccel.z;
}

APEX_CUDA_CALLABLE PX_INLINE void accumACCELERATION(const FieldSamplerExecuteArgs& args,
	const physx::PxVec3& field, float fieldW,
	physx::PxVec4& accumAccel, physx::PxVec4& accumVelocity)
{
	PX_UNUSED(accumVelocity);

	physx::PxVec3 newAccel = ((1 - accumAccel.w) * fieldW * args.elapsedTime) * field;
	accumAccel.x += newAccel.x;
	accumAccel.y += newAccel.y;
	accumAccel.z += newAccel.z;
}

APEX_CUDA_CALLABLE PX_INLINE void accumVELOCITY_DIRECT(const FieldSamplerExecuteArgs& args,
	const physx::PxVec3& field, float fieldW,
	physx::PxVec4& accumAccel, physx::PxVec4& accumVelocity)
{
	PX_UNUSED(args);

	physx::PxVec3 newVelocity = ((1 - accumAccel.w) * fieldW) * field;
	accumVelocity.x += newVelocity.x;
	accumVelocity.y += newVelocity.y;
	accumVelocity.z += newVelocity.z;
	accumVelocity.w = PxMax(accumVelocity.w, fieldW);
}

APEX_CUDA_CALLABLE PX_INLINE void accumVELOCITY_DRAG(const FieldSamplerExecuteArgs& args, float dragCoeff,
	const physx::PxVec3& field, float fieldW,
	physx::PxVec4& accumAccel, physx::PxVec4& accumVelocity)
{
#if 1
	const float dragFieldW = PxMin(fieldW * dragCoeff * args.elapsedTime / args.mass, 1.0f);
	accumVELOCITY_DIRECT(args, field, dragFieldW, accumAccel, accumVelocity);
#else
	const physx::PxVec3 dragForce = (field - args.velocity) * dragCoeff;
	accumFORCE(args, dragForce, fieldW, accumAccel, accumVelocity);
#endif
}


} // namespace apex
}
#ifdef __CUDACC__

template <int queryType>
struct FieldSamplerExecutor;
/*
{
	INPLACE_TEMPL_ARGS_DEF
	static inline __device__ physx::PxVec3 func(const nvidia::fieldsampler::FieldSamplerParams* params, const nvidia::fieldsampler::FieldSamplerExecuteArgs& args, float& fieldWeight);
};
*/

template <int queryType>
struct FieldSamplerIncludeWeightEvaluator
{
	INPLACE_TEMPL_ARGS_DEF
	static inline __device__ float func(const nvidia::fieldsampler::FieldSamplerParams* params, const physx::PxVec3& position, const physx::PxVec3& cellSize)
	{
		return nvidia::fieldsampler::evalWeightInShape(params->includeShape, position);
	}
};

#endif

#endif
