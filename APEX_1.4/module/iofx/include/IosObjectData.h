/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __IOS_OBJECT_DATA_H__
#define __IOS_OBJECT_DATA_H__

#include "ApexUsingNamespace.h"
#include "PsUserAllocated.h"
#include "PsSync.h"
#include "PxVec3.h"
#include "PxVec4.h"
#include "IofxManagerIntl.h"
#include "PxMat33.h"
#include "ApexMirroredArray.h"
#include "ApexMath.h"

#include "IofxRenderData.h"
#include "ModifierData.h"

namespace physx
{
class PxCudaContextManager;
}

namespace nvidia
{
namespace iofx
{

class IosObjectBaseData : public IofxManagerDescIntl, public IosBufferDescIntl, public UserAllocated
{
public:
	IosObjectBaseData(uint32_t instance)
		: instanceID(instance)
		, renderData(NULL)
		, maxStateID(0)
		, maxInputID(0)
		, numParticles(0)
	{
	}
	virtual ~IosObjectBaseData()
	{
		if (renderData)
		{
			renderData->release();
		}
	}

	void updateSemantics(IofxScene& scene, uint32_t semantics)
	{
		if (renderData)
		{
			renderData->updateSemantics(scene, semantics, maxObjectCount, numParticles);
		}
	}

	PX_INLINE ModifierCommonParams getCommonParams() const
	{
		ModifierCommonParams common;

		common.inputHasCollision = iosSupportsCollision;
		common.inputHasDensity = iosSupportsDensity;
		common.inputHasUserData = iosSupportsUserData;
		common.upVector = upVector;
		common.eyePosition = eyePosition;
		common.eyeDirection = eyeDirection;
		common.eyeAxisX = eyeAxisX;
		common.eyeAxisY = eyeAxisY;
		common.zNear = zNear;
		common.deltaTime = deltaTime;

		return common;
	}

	const uint32_t		instanceID;

	PxVec3			upVector;
	float			radius;
	float			gravity;
	float			restDensity; //!< resting density of simulation

	float			elapsedTime;			//!< total simulation time
	float			deltaTime;				//!< SceneIntl::getElapsedTime()
	PxVec3			eyePosition;			//!< Scene::getEyePosition()
	PxVec3			eyeDirection;			//!< Scene::getEyeDirection()
	PxVec3			eyeAxisX;
	PxVec3			eyeAxisY;
	float			zNear;

	uint32_t		maxStateID;				//!< From IOS each frame
	uint32_t		maxInputID;				//!< From IOS each frame
	uint32_t		numParticles;			//!< From IOS each frame

	IofxSharedRenderDataBase* renderData;

private:
	IosObjectBaseData& operator=(const IosObjectBaseData&);
};

class IosObjectCpuData : public IosObjectBaseData
{
public:
	IosObjectCpuData(uint32_t instance)
		: IosObjectBaseData(instance)
		, outputToState(NULL)
		, sortingKeys(NULL)
	{
	}

	IofxSlice**				inPubState;
	IofxSlice**				outPubState;
	uint32_t					numPubSlices;

	IofxSlice**				inPrivState;
	IofxSlice**				outPrivState;
	uint32_t					numPrivSlices;

	uint32_t*					outputToState;

	uint32_t*					sortingKeys;
};

#if APEX_CUDA_SUPPORT
class IosObjectGpuData : public IosObjectBaseData
{
public:
	IosObjectGpuData(uint32_t instance)
		: IosObjectBaseData(instance)
	{
	}
};
#endif

}
} // namespace nvidia

#endif /* __IOS_OBJECT_DATA_H__ */
