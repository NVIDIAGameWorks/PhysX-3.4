/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef FIELD_SAMPLER_QUERY_INTL_H
#define FIELD_SAMPLER_QUERY_INTL_H

#include "ApexDefs.h"
#include "ApexMirroredArray.h"

#include "PxTask.h"
#include "ApexActor.h"
#include "PxMat44.h"

namespace nvidia
{
namespace apex
{


class FieldSamplerSceneIntl;

struct FieldSamplerQueryDescIntl
{
	uint32_t					maxCount;
#if PX_PHYSICS_VERSION_MAJOR == 3
	PxFilterData			samplerFilterData;
#endif
	FieldSamplerSceneIntl*	ownerFieldSamplerScene;


	FieldSamplerQueryDescIntl()
	{
		maxCount                = 0;
#if PX_PHYSICS_VERSION_MAJOR == 3
		samplerFilterData.word0 = 0xFFFFFFFF;
		samplerFilterData.word1 = 0xFFFFFFFF;
		samplerFilterData.word2 = 0xFFFFFFFF;
		samplerFilterData.word3 = 0xFFFFFFFF;
#endif
		ownerFieldSamplerScene  = 0;
	}
};

struct FieldSamplerQueryDataIntl
{
	float						timeStep;
	uint32_t						count;
	bool						isDataOnDevice;

	uint32_t						positionStrideBytes; //Stride for position
	uint32_t						velocityStrideBytes; //Stride for velocity
	float*						pmaInPosition;
	float*						pmaInVelocity;
	PxVec4*						pmaOutField;

	uint32_t						massStrideBytes; //if massStride set to 0 supposed single mass for all objects
	float*						pmaInMass;

	uint32_t*						pmaInIndices;
};


#if APEX_CUDA_SUPPORT

class ApexCudaArray;

struct FieldSamplerQueryGridDataIntl
{
	uint32_t numX, numY, numZ;

	PxMat44			gridToWorld;

	float			mass;

	float			timeStep;

	PxVec3			cellSize;

	ApexCudaArray*	resultVelocity; //x, y, z = velocity vector, w = weight

	CUstream		stream;
};
#endif

class FieldSamplerCallbackIntl
{
public:
	virtual void operator()(void* stream = NULL) = 0;
};

class FieldSamplerQueryIntl : public ApexActor
{
public:
	virtual PxTaskID submitFieldSamplerQuery(const FieldSamplerQueryDataIntl& data, PxTaskID taskID) = 0;

	//! FieldSamplerCallbackIntl will be called before FieldSampler computations
	virtual void setOnStartCallback(FieldSamplerCallbackIntl*) = 0;
	//! FieldSamplerCallbackIntl will be called after FieldSampler computations
	virtual void setOnFinishCallback(FieldSamplerCallbackIntl*) = 0;

#if APEX_CUDA_SUPPORT
	virtual PxVec3 executeFieldSamplerQueryOnGrid(const FieldSamplerQueryGridDataIntl&)
	{
		APEX_INVALID_OPERATION("not implemented");
		return PxVec3(0.0f);
	}
#endif

protected:
	virtual ~FieldSamplerQueryIntl() {}
};

}
} // end namespace nvidia::apex

#endif // #ifndef FIELD_SAMPLER_QUERY_INTL_H
