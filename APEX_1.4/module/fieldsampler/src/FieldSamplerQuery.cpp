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
#include "Apex.h"
#include "FieldSamplerQuery.h"
#include "FieldSamplerManager.h"
#include "FieldSamplerWrapper.h"
#include "FieldSamplerSceneWrapper.h"
#include "FieldBoundaryWrapper.h"

#include "SceneIntl.h"

#if APEX_CUDA_SUPPORT
#include "PxGpuTask.h"
#endif

#include "FieldSamplerCommon.h"


namespace nvidia
{
namespace fieldsampler
{


FieldSamplerQuery::FieldSamplerQuery(const FieldSamplerQueryDescIntl& desc, ResourceList& list, FieldSamplerManager* manager)
	: mManager(manager)
	, mQueryDesc(desc)
	, mAccumVelocity(manager->getApexScene(), PX_ALLOC_INFO("mAccumVelocity", PARTICLES))
	, mOnStartCallback(NULL)
	, mOnFinishCallback(NULL)
{
	list.add(*this);
}

void FieldSamplerQuery::release()
{
	if (mInRelease)
	{
		return;
	}
	mInRelease = true;
	destroy();
}

void FieldSamplerQuery::destroy()
{
	delete this;
}


FieldSamplerQuery::SceneInfo* FieldSamplerQuery::findSceneInfo(FieldSamplerSceneWrapper* sceneWrapper) const
{
	for (uint32_t i = 0; i < mSceneList.getSize(); ++i)
	{
		SceneInfo* sceneInfo = DYNAMIC_CAST(SceneInfo*)(mSceneList.getResource(i));
		if (sceneInfo->getSceneWrapper() == sceneWrapper)
		{
			return sceneInfo;
		}
	}
	return NULL;
}


bool FieldSamplerQuery::addFieldSampler(FieldSamplerWrapper* fieldSamplerWrapper)
{
	const FieldSamplerDescIntl& fieldSamplerDesc = fieldSamplerWrapper->getInternalFieldSamplerDesc();
	float multiplier = 1.0f;
	bool result = mManager->getFieldSamplerGroupsFiltering(mQueryDesc.samplerFilterData, fieldSamplerDesc.samplerFilterData, multiplier);
	if (result)
	{
		FieldSamplerSceneWrapper* sceneWrapper = fieldSamplerWrapper->getFieldSamplerSceneWrapper();
		SceneInfo* sceneInfo = findSceneInfo(sceneWrapper);
		if (sceneInfo == NULL)
		{
			sceneInfo = createSceneInfo(sceneWrapper);
		}
		sceneInfo->addFieldSampler(fieldSamplerWrapper, multiplier);
	}
	return result;
}

bool FieldSamplerQuery::removeFieldSampler(FieldSamplerWrapper* fieldSamplerWrapper)
{
	FieldSamplerSceneWrapper* sceneWrapper = fieldSamplerWrapper->getFieldSamplerSceneWrapper();
	SceneInfo* sceneInfo = findSceneInfo(sceneWrapper);
	return (sceneInfo != NULL) ? sceneInfo->removeFieldSampler(fieldSamplerWrapper) : false;
}

void FieldSamplerQuery::clearAllFieldSamplers()
{
	for (uint32_t i = 0; i < mSceneList.getSize(); ++i)
	{
		SceneInfo* sceneInfo = DYNAMIC_CAST(SceneInfo*)(mSceneList.getResource(i));
		sceneInfo->clearAllFieldSamplers();
	}
}

void FieldSamplerQuery::submitFieldSamplerQuery(const FieldSamplerQueryDataIntl& data, PxTask* task, PxTask* readyTask)
{
	PX_UNUSED(readyTask);
	for (uint32_t i = 0; i < mSceneList.getSize(); ++i)
	{
		SceneInfo* sceneInfo = DYNAMIC_CAST(SceneInfo*)(mSceneList.getResource(i));
		FieldSamplerSceneIntl* niFieldSamplerScene = sceneInfo->getSceneWrapper()->getInternalFieldSamplerScene();
		const PxTask* fieldSamplerReadyTask = niFieldSamplerScene->onSubmitFieldSamplerQuery(data, readyTask);
		if (fieldSamplerReadyTask != 0)
		{
			task->startAfter(fieldSamplerReadyTask->getTaskID());
		}
	}
}

void FieldSamplerQuery::update()
{
	mPrimarySceneList.clear();
	mSecondarySceneList.clear();

	for (uint32_t i = 0; i < mSceneList.getSize(); ++i)
	{
		SceneInfo* sceneInfo = DYNAMIC_CAST(SceneInfo*)(mSceneList.getResource(i));
		sceneInfo->update();

		if (sceneInfo->getEnabledFieldSamplerCount() > 0 && (sceneInfo->getSceneWrapper()->getInternalFieldSamplerScene() != mQueryDesc.ownerFieldSamplerScene))
		{
			((sceneInfo->getSceneWrapper()->getInternalFieldSamplerSceneDesc().isPrimary) ? mPrimarySceneList : mSecondarySceneList).pushBack(sceneInfo);
		}
	}
}

bool FieldSamplerQuery::SceneInfo::update()
{
	mEnabledFieldSamplerCount = 0;
	for (uint32_t i = 0; i < mFieldSamplerArray.size(); ++i)
	{
		if (mFieldSamplerArray[i].mFieldSamplerWrapper->isEnabled())
		{
			++mEnabledFieldSamplerCount;
		}
		if (mFieldSamplerArray[i].mFieldSamplerWrapper->isEnabledChanged())
		{
			mFieldSamplerArrayChanged = true;
		}
	}

	if (mFieldSamplerArrayChanged)
	{
		mFieldSamplerArrayChanged = false;
		return true;
	}
	return false;
}
/******************************** CPU Version ********************************/
class TaskExecute : public PxTask, public UserAllocated
{
public:
	TaskExecute(FieldSamplerQueryCPU* query) : mQuery(query) {}

	const char* getName() const
	{
		return "FieldSamplerQueryCPU::TaskExecute";
	}
	void run()
	{
		mQuery->execute();
	}

protected:
	FieldSamplerQueryCPU* mQuery;
};

FieldSamplerQueryCPU::FieldSamplerQueryCPU(const FieldSamplerQueryDescIntl& desc, ResourceList& list, FieldSamplerManager* manager)
	: FieldSamplerQuery(desc, list, manager)
{
	mTaskExecute = PX_NEW(TaskExecute)(this);

	mExecuteCount = 256;
}

FieldSamplerQueryCPU::~FieldSamplerQueryCPU()
{
	delete mTaskExecute;
}

PxTaskID FieldSamplerQueryCPU::submitFieldSamplerQuery(const FieldSamplerQueryDataIntl& data, PxTaskID taskID)
{
	PX_ASSERT(data.isDataOnDevice == false);
	PX_ASSERT(data.count <= mQueryDesc.maxCount);
	if (data.count == 0)
	{
		return taskID;
	}
	mQueryData = data;

	mResultField.resize(mExecuteCount);
	mWeights.resize(mExecuteCount);
	mAccumVelocity.reserve(mQueryDesc.maxCount);

	PxTaskManager* tm = mManager->getApexScene().getTaskManager();
	tm->submitUnnamedTask(*mTaskExecute);

	FieldSamplerQuery::submitFieldSamplerQuery(data, mTaskExecute, NULL);

	mTaskExecute->finishBefore(taskID);
	return mTaskExecute->getTaskID();
}

void FieldSamplerQueryCPU::execute()
{
	if (mOnStartCallback)
	{
		(*mOnStartCallback)(NULL);
	}

	FieldSamplerIntl::ExecuteData executeData;
	
	executeData.position		= mQueryData.pmaInPosition;
	executeData.velocity		= mQueryData.pmaInVelocity;
	executeData.mass			= mQueryData.pmaInMass;//  + massOffset;
	executeData.resultField		= mResultField.begin();
	executeData.positionStride	= mQueryData.positionStrideBytes;
	executeData.velocityStride	= mQueryData.velocityStrideBytes;
	executeData.massStride		= mQueryData.massStrideBytes;
	executeData.indicesMask		= 0;

	uint32_t beginIndex;
	uint32_t* indices = &beginIndex;
	if (mQueryData.pmaInIndices)
	{
		indices = mQueryData.pmaInIndices;
		executeData.indicesMask = ~executeData.indicesMask;
	}

	for (uint32_t executeOffset = 0; executeOffset < mQueryData.count; executeOffset += mExecuteCount)
	{
		const uint32_t positionStride = mQueryData.positionStrideBytes / 4;
		const uint32_t velocityStride = mQueryData.velocityStrideBytes / 4;
		const uint32_t massStride = mQueryData.massStrideBytes / 4;
		//const uint32_t offset = executeOffset * stride;
		//const uint32_t massOffset = executeOffset * massStride;

		beginIndex = executeOffset;
		executeData.count        = PxMin(mExecuteCount, mQueryData.count - executeOffset);
		executeData.indices		 = indices + (executeOffset & executeData.indicesMask);

		PxVec4* accumField = (PxVec4*)(mQueryData.pmaOutField);
		PxVec4* accumVelocity = mAccumVelocity.getPtr() + executeOffset;
		//clear accum
		for (uint32_t i = 0; i < executeData.count; ++i)
		{
			uint32_t j = executeData.indices[i & executeData.indicesMask] + (i & ~executeData.indicesMask);
			accumField[j] = PxVec4(0.0f);
			accumVelocity[i] = PxVec4(0.0f);
		}
		for (uint32_t sceneIdx = 0; sceneIdx < mPrimarySceneList.size(); ++sceneIdx)
		{
			executeScene(mPrimarySceneList[sceneIdx], executeData, accumField, accumVelocity, positionStride, velocityStride, massStride);
		}

		//setup weights for secondary scenes
		for (uint32_t i = 0; i < executeData.count; ++i)
		{
			uint32_t j = executeData.indices[i & executeData.indicesMask] + (i & ~executeData.indicesMask);
			accumField[j].w = accumVelocity[i].w;
			accumVelocity[i].w = 0.0f;
		}
		for (uint32_t sceneIdx = 0; sceneIdx < mSecondarySceneList.size(); ++sceneIdx)
		{
			executeScene(mSecondarySceneList[sceneIdx], executeData, accumField, accumVelocity, positionStride, velocityStride, massStride);
		}

		//compose accum field
		for (uint32_t i = 0; i < executeData.count; ++i)
		{
			uint32_t j = executeData.indices[i & executeData.indicesMask] + (i & ~executeData.indicesMask);
			float blend = accumField[j].w;
			float velW = accumVelocity[i].w;
			float weight = blend + velW * (1 - blend);
			if (weight >= VELOCITY_WEIGHT_THRESHOLD)
			{
				PxVec3 result = accumField[j].getXYZ();
				const PxVec3& velocity = *(PxVec3*)(executeData.velocity + j * velocityStride);
				result += (accumVelocity[i].getXYZ() - weight * velocity);
				accumField[j] = PxVec4(result, 0);
			}
		}
	}

	if (mOnFinishCallback)
	{
		(*mOnFinishCallback)(NULL);
	}
}

void FieldSamplerQueryCPU::executeScene(const SceneInfo* sceneInfo, 
										const FieldSamplerIntl::ExecuteData& executeData, 
										PxVec4* accumField, 
										PxVec4* accumVelocity, 
										uint32_t positionStride, 
										uint32_t velocityStride, 
										uint32_t massStride)
{
	FieldSamplerExecuteArgs execArgs;
	execArgs.elapsedTime = mQueryData.timeStep;
	execArgs.totalElapsedMS = mManager->getApexScene().getTotalElapsedMS();

	const nvidia::Array<FieldSamplerInfo>& fieldSamplerArray = sceneInfo->getFieldSamplerArray();
	for (uint32_t fieldSamplerIdx = 0; fieldSamplerIdx < fieldSamplerArray.size(); ++fieldSamplerIdx)
	{
		const FieldSamplerWrapperCPU* fieldSampler = DYNAMIC_CAST(FieldSamplerWrapperCPU*)(fieldSamplerArray[fieldSamplerIdx].mFieldSamplerWrapper);
		if (fieldSampler->isEnabled())
		{
			const float multiplier = fieldSamplerArray[fieldSamplerIdx].mMultiplier;
			PX_UNUSED(multiplier);

			const FieldSamplerDescIntl& desc = fieldSampler->getInternalFieldSamplerDesc();
			if (desc.cpuSimulationSupport)
			{
				const FieldShapeDescIntl& shapeDesc = fieldSampler->getInternalFieldSamplerShape();
				PX_ASSERT(shapeDesc.weight >= 0.0f && shapeDesc.weight <= 1.0f);

				for (uint32_t i = 0; i < executeData.count; ++i)
				{
					mWeights[i] = 0;
				}

				uint32_t boundaryCount = fieldSampler->getFieldBoundaryCount();
				for (uint32_t boundaryIndex = 0; boundaryIndex < boundaryCount; ++boundaryIndex)
				{
					FieldBoundaryWrapper* fieldBoundaryWrapper = fieldSampler->getFieldBoundaryWrapper(boundaryIndex);

					const nvidia::Array<FieldShapeDescIntl>& fieldShapes = fieldBoundaryWrapper->getFieldShapes();
					for (uint32_t shapeIndex = 0; shapeIndex < fieldShapes.size(); ++shapeIndex)
					{
						const FieldShapeDescIntl& boundaryShapeDesc = fieldShapes[shapeIndex];
						PX_ASSERT(boundaryShapeDesc.weight >= 0.0f && boundaryShapeDesc.weight <= 1.0f);

						for (uint32_t i = 0; i < executeData.count; ++i)
						{
							uint32_t j = executeData.indices[i & executeData.indicesMask] + (i & ~executeData.indicesMask);
							PxVec3* pos = (PxVec3*)(executeData.position + j * positionStride);
							const float excludeWeight = evalFade(evalDistInShape(boundaryShapeDesc, *pos), 0.0f) * boundaryShapeDesc.weight;
							mWeights[i] = PxMax(mWeights[i], excludeWeight);
						}
					}
				}

				for (uint32_t i = 0; i < executeData.count; ++i)
				{
					uint32_t j = executeData.indices[i & executeData.indicesMask] + (i & ~executeData.indicesMask);
					PxVec3* pos = (PxVec3*)(executeData.position + j * positionStride);
					const float includeWeight = evalFade(evalDistInShape(shapeDesc, *pos), desc.boundaryFadePercentage) * shapeDesc.weight;
					const float excludeWeight = mWeights[i];
					mWeights[i] = includeWeight * (1.0f - excludeWeight);
#if FIELD_SAMPLER_MULTIPLIER == FIELD_SAMPLER_MULTIPLIER_WEIGHT
					mWeights[i] *= multiplier;
#endif
				}

				//execute field
				fieldSampler->getInternalFieldSampler()->executeFieldSampler(executeData);

#if FIELD_SAMPLER_MULTIPLIER == FIELD_SAMPLER_MULTIPLIER_VALUE
				const float multiplier = fieldSamplerArray[fieldSamplerIdx].mMultiplier;
				for (uint32_t i = 0; i < executeData.count; ++i)
				{
					executeData.resultField[i] *= multiplier;
				}
#endif

				//accum field
				switch (desc.type)
				{
				case FieldSamplerTypeIntl::FORCE:
					for (uint32_t i = 0; i < executeData.count; ++i)
					{
						uint32_t j = executeData.indices[i & executeData.indicesMask] + (i & ~executeData.indicesMask);
						execArgs.position = *(PxVec3*)(executeData.position + j * positionStride);
						execArgs.velocity = *(PxVec3*)(executeData.velocity + j * velocityStride);
						execArgs.mass     = *(executeData.mass + massStride * j);

						accumFORCE(execArgs, executeData.resultField[i], mWeights[i], accumField[j], accumVelocity[i]);
					}
					break;
				case FieldSamplerTypeIntl::ACCELERATION:
					for (uint32_t i = 0; i < executeData.count; ++i)
					{
						uint32_t j = executeData.indices[i & executeData.indicesMask] + (i & ~executeData.indicesMask);
						execArgs.position = *(PxVec3*)(executeData.position + j * positionStride);
						execArgs.velocity = *(PxVec3*)(executeData.velocity + j * velocityStride);
						execArgs.mass     = *(executeData.mass + massStride * j);

						accumACCELERATION(execArgs, executeData.resultField[i], mWeights[i], accumField[j], accumVelocity[i]);
					}
					break;
				case FieldSamplerTypeIntl::VELOCITY_DRAG:
					for (uint32_t i = 0; i < executeData.count; ++i)
					{
						uint32_t j = executeData.indices[i & executeData.indicesMask] + (i & ~executeData.indicesMask);
						execArgs.position = *(PxVec3*)(executeData.position + j * positionStride);
						execArgs.velocity = *(PxVec3*)(executeData.velocity + j * velocityStride);
						execArgs.mass     = *(executeData.mass + massStride * j);

						accumVELOCITY_DRAG(execArgs, desc.dragCoeff, executeData.resultField[i], mWeights[i], accumField[j], accumVelocity[i]);
					}
					break;
				case FieldSamplerTypeIntl::VELOCITY_DIRECT:
					for (uint32_t i = 0; i < executeData.count; ++i)
					{
						uint32_t j = executeData.indices[i & executeData.indicesMask] + (i & ~executeData.indicesMask);
						execArgs.position = *(PxVec3*)(executeData.position + j * positionStride);
						execArgs.velocity = *(PxVec3*)(executeData.velocity + j * velocityStride);
						execArgs.mass     = *(executeData.mass + massStride * j);

						accumVELOCITY_DIRECT(execArgs, executeData.resultField[i], mWeights[i], accumField[j], accumVelocity[i]);
					}
					break;
				};
			}
		}
	}

}


/******************************** GPU Version ********************************/
#if APEX_CUDA_SUPPORT

class FieldSamplerQueryLaunchTask : public PxGpuTask, public UserAllocated
{
public:
	FieldSamplerQueryLaunchTask(FieldSamplerQueryGPU* query) : mQuery(query) {}
	const char* getName() const
	{
		return "FieldSamplerQueryLaunchTask";
	}
	void         run()
	{
		PX_ALWAYS_ASSERT();
	}
	bool         launchInstance(CUstream stream, int kernelIndex)
	{
		return mQuery->launch(stream, kernelIndex);
	}
	PxGpuTaskHint::Enum getTaskHint() const
	{
		return PxGpuTaskHint::Kernel;
	}

protected:
	FieldSamplerQueryGPU* mQuery;
};

class FieldSamplerQueryPrepareTask : public PxTask, public UserAllocated
{
public:
	FieldSamplerQueryPrepareTask(FieldSamplerQueryGPU* query) : mQuery(query) {}

	const char* getName() const
	{
		return "FieldSamplerQueryPrepareTask";
	}
	void run()
	{
		mQuery->prepare();
	}

protected:
	FieldSamplerQueryGPU* mQuery;
};

class FieldSamplerQueryCopyTask : public PxGpuTask, public UserAllocated
{
public:
	FieldSamplerQueryCopyTask(FieldSamplerQueryGPU* query) : mQuery(query) {}
	const char* getName() const
	{
		return "FieldSamplerQueryCopyTask";
	}
	void         run()
	{
		PX_ALWAYS_ASSERT();
	}
	bool         launchInstance(CUstream stream, int kernelIndex)
	{
		return mQuery->copy(stream, kernelIndex);
	}
	PxGpuTaskHint::Enum getTaskHint() const
	{
		return PxGpuTaskHint::Kernel;
	}

protected:
	FieldSamplerQueryGPU* mQuery;
};

class FieldSamplerQueryFetchTask : public PxTask, public UserAllocated
{
public:
	FieldSamplerQueryFetchTask(FieldSamplerQueryGPU* query) : mQuery(query) {}

	const char* getName() const
	{
		return "FieldSamplerQueryFetchTask";
	}
	void run()
	{
		mQuery->fetch();
	}

protected:
	FieldSamplerQueryGPU* mQuery;
};


FieldSamplerQueryGPU::FieldSamplerQueryGPU(const FieldSamplerQueryDescIntl& desc, ResourceList& list, FieldSamplerManager* manager)
	: FieldSamplerQueryCPU(desc, list, manager)
	, mPositionMass(manager->getApexScene(), PX_ALLOC_INFO("mPositionMass", PARTICLES))
	, mVelocity(manager->getApexScene(), PX_ALLOC_INFO("mVelocity", PARTICLES))
	, mAccumField(manager->getApexScene(), PX_ALLOC_INFO("mAccumField", PARTICLES))
	, mCopyQueue(*manager->getApexScene().getTaskManager()->getGpuDispatcher())
{
	mTaskLaunch   = PX_NEW(FieldSamplerQueryLaunchTask)(this);
	mTaskPrepare  = PX_NEW(FieldSamplerQueryPrepareTask)(this);
	mTaskCopy     = PX_NEW(FieldSamplerQueryCopyTask)(this);
	mTaskFetch    = PX_NEW(FieldSamplerQueryFetchTask)(this);
}

FieldSamplerQueryGPU::~FieldSamplerQueryGPU()
{
	PX_DELETE(mTaskFetch);
	PX_DELETE(mTaskCopy);
	PX_DELETE(mTaskPrepare);
	PX_DELETE(mTaskLaunch);
}

PxTaskID FieldSamplerQueryGPU::submitFieldSamplerQuery(const FieldSamplerQueryDataIntl& data, PxTaskID taskID)
{
	PX_ASSERT(data.count <= mQueryDesc.maxCount);
	if (data.count == 0)
	{
		return taskID;
	}
	mQueryData = data;

	if (!data.isDataOnDevice)
	{
		bool isWorkOnCPU = true;
		// try to find FieldSampler which has no CPU implemntation (Turbulence for example)
		for (uint32_t sceneIdx = 0; (sceneIdx < mPrimarySceneList.size() + mSecondarySceneList.size()) && isWorkOnCPU; ++sceneIdx)
		{
			const nvidia::Array<FieldSamplerInfo>& fsArray = sceneIdx < mPrimarySceneList.size() 
				? mPrimarySceneList[sceneIdx]->getFieldSamplerArray() 
				: mSecondarySceneList[sceneIdx-mPrimarySceneList.size()]->getFieldSamplerArray();
			for (uint32_t fsIdx = 0; fsIdx < fsArray.size() && isWorkOnCPU; fsIdx++)
			{
				if (fsArray[fsIdx].mFieldSamplerWrapper->isEnabled())
				{
					isWorkOnCPU = fsArray[fsIdx].mFieldSamplerWrapper->getInternalFieldSamplerDesc().cpuSimulationSupport;
				}
			}
		}

		// if all FSs can work on CPU we will execute FieldSamplerQuery on CPU
		if (isWorkOnCPU)
		{
			return FieldSamplerQueryCPU::submitFieldSamplerQuery(data, taskID);
		}

		mPositionMass.reserve(mQueryDesc.maxCount, ApexMirroredPlace::CPU_GPU);
		mVelocity.reserve(mQueryDesc.maxCount, ApexMirroredPlace::CPU_GPU);
		mAccumField.reserve(mQueryDesc.maxCount, ApexMirroredPlace::CPU_GPU);
	}
	mAccumVelocity.reserve(mQueryDesc.maxCount, ApexMirroredPlace::CPU_GPU);
	
	// if data on device or some FS can't work on CPU we will launch FieldSamplerQuery on GPU
	PxTaskManager* tm = mManager->getApexScene().getTaskManager();
	tm->submitUnnamedTask(*mTaskLaunch, PxTaskType::TT_GPU);

	if (data.isDataOnDevice)
	{
		FieldSamplerQuery::submitFieldSamplerQuery(data, mTaskLaunch, NULL);

		mTaskLaunch->finishBefore(taskID);
		return mTaskLaunch->getTaskID();
	}
	else
	{
		FieldSamplerQueryDataIntl data4Device;
		data4Device.timeStep = data.timeStep;
		data4Device.count = data.count;
		data4Device.isDataOnDevice = true;
		data4Device.positionStrideBytes = sizeof(PxVec4);
		data4Device.velocityStrideBytes = sizeof(PxVec4);
		data4Device.massStrideBytes = sizeof(PxVec4);
		data4Device.pmaInPosition = (float*)mPositionMass.getGpuPtr();
		data4Device.pmaInVelocity = (float*)mVelocity.getGpuPtr();
		data4Device.pmaInMass = &mPositionMass.getGpuPtr()->w;
		data4Device.pmaOutField = mAccumField.getGpuPtr();
		data4Device.pmaInIndices = 0;

		FieldSamplerQuery::submitFieldSamplerQuery(data4Device, mTaskLaunch, mTaskCopy);

		tm->submitUnnamedTask(*mTaskPrepare);
		tm->submitUnnamedTask(*mTaskCopy, PxTaskType::TT_GPU);
		tm->submitUnnamedTask(*mTaskFetch);

		mTaskPrepare->finishBefore(mTaskCopy->getTaskID());
		mTaskCopy->finishBefore(mTaskLaunch->getTaskID());
		mTaskLaunch->finishBefore(mTaskFetch->getTaskID());
		mTaskFetch->finishBefore(taskID);
		return mTaskPrepare->getTaskID();
	}
}

void FieldSamplerQueryGPU::prepare()
{
	const uint32_t positionStride = mQueryData.positionStrideBytes / sizeof(float);
	const uint32_t velocityStride = mQueryData.velocityStrideBytes / sizeof(float);
	const uint32_t massStride = mQueryData.massStrideBytes / sizeof(float);
	for (uint32_t idx = 0; idx < mQueryData.count; idx++)
	{
		mPositionMass[idx] = PxVec4(*(PxVec3*)(mQueryData.pmaInPosition + idx * positionStride), *(mQueryData.pmaInMass + idx * massStride));
		mVelocity[idx] = PxVec4(*(PxVec3*)(mQueryData.pmaInVelocity + idx * velocityStride), 0.f);
	}
}

void FieldSamplerQueryGPU::fetch()
{
	for (uint32_t idx = 0; idx < mQueryData.count; idx++)
	{
		mQueryData.pmaOutField[idx] = mAccumField[idx];
	}
}

bool FieldSamplerQueryGPU::copy(CUstream stream, int kernelIndex)
{
	if (kernelIndex == 0)
	{
		mCopyQueue.reset(stream, 4);
		mPositionMass.copyHostToDeviceQ(mCopyQueue, mQueryData.count);
		mVelocity.copyHostToDeviceQ(mCopyQueue, mQueryData.count);
		mCopyQueue.flushEnqueued();
	}
	return false;
}

bool FieldSamplerQueryGPU::launch(CUstream stream, int kernelIndex)
{
	FieldSamplerPointsKernelArgs args;
	args.elapsedTime        = mQueryData.timeStep;
	args.totalElapsedMS     = mManager->getApexScene().getTotalElapsedMS();
	if (mQueryData.isDataOnDevice)
	{
		args.positionMass   = (float4*)mQueryData.pmaInPosition;
		args.velocity       = (float4*)mQueryData.pmaInVelocity;
		args.accumField     = (float4*)mQueryData.pmaOutField;
	}
	else
	{
		args.positionMass   = (float4*)mPositionMass.getGpuPtr();
		args.velocity       = (float4*)mVelocity.getGpuPtr();
		args.accumField     = (float4*)mAccumField.getGpuPtr();
	}
	args.accumVelocity      = (float4*)mAccumVelocity.getGpuPtr();

	FieldSamplerPointsKernelLaunchDataIntl launchData;
	launchData.stream       = stream;
	launchData.kernelType   = FieldSamplerKernelType::POINTS;
	launchData.kernelArgs   = &args;
	launchData.threadCount  = mQueryData.count;
	launchData.memRefSize   = mQueryData.count;

	if (kernelIndex == 0 && mOnStartCallback)
	{
		(*mOnStartCallback)(stream);
	}

	if (kernelIndex == 0)
	{
		CUDA_OBJ(clearKernel)(stream, mQueryData.count,
		                      createApexCudaMemRef(args.accumField, launchData.memRefSize, ApexCudaMemFlags::OUT),
		                      createApexCudaMemRef(args.accumVelocity, launchData.memRefSize, ApexCudaMemFlags::OUT));
		return true;
	}
	--kernelIndex;

	const uint32_t bothSceneCount = mPrimarySceneList.size() + mSecondarySceneList.size();
	if (kernelIndex < (int) bothSceneCount)
	{
		SceneInfo* sceneInfo = (kernelIndex < (int) mPrimarySceneList.size()) 
			? mPrimarySceneList[(uint32_t)kernelIndex] 
			: mSecondarySceneList[(uint32_t)kernelIndex - mPrimarySceneList.size()];
		SceneInfoGPU* sceneInfoGPU = DYNAMIC_CAST(SceneInfoGPU*)(sceneInfo);

		launchData.kernelMode = FieldSamplerKernelMode::DEFAULT;
		if (kernelIndex == (int) mPrimarySceneList.size() - 1)
		{
			launchData.kernelMode = FieldSamplerKernelMode::FINISH_PRIMARY;
		}
		if ((kernelIndex == (int) bothSceneCount - 1))
		{
			launchData.kernelMode = FieldSamplerKernelMode::FINISH_SECONDARY;
		}

		FieldSamplerSceneWrapperGPU* sceneWrapper = DYNAMIC_CAST(FieldSamplerSceneWrapperGPU*)(sceneInfo->getSceneWrapper());

		launchData.queryParamsHandle = sceneInfoGPU->getQueryParamsHandle();
		launchData.paramsExArrayHandle = sceneInfoGPU->getParamsHandle();
		launchData.fieldSamplerArray = &sceneInfo->getFieldSamplerArray();
		launchData.activeFieldSamplerCount = sceneInfo->getEnabledFieldSamplerCount();

		sceneWrapper->getInternalFieldSamplerScene()->launchFieldSamplerCudaKernel(launchData);
		return true;
	}
	kernelIndex -= bothSceneCount;

	if (kernelIndex == 0)
	{
		CUDA_OBJ(composeKernel)(stream, mQueryData.count,
								createApexCudaMemRef(args.accumField, launchData.memRefSize, ApexCudaMemFlags::IN_OUT),
								createApexCudaMemRef((const float4*)args.accumVelocity, launchData.memRefSize, ApexCudaMemFlags::IN),
								createApexCudaMemRef(args.velocity, launchData.memRefSize, ApexCudaMemFlags::IN),
								args.elapsedTime);
		return true;
	}
	--kernelIndex;

	if (!mQueryData.isDataOnDevice)
	{
		mAccumField.copyDeviceToHostQ(mCopyQueue, mQueryData.count);
		mCopyQueue.flushEnqueued();

		PxTaskManager* tm = mManager->getApexScene().getTaskManager();
		tm->getGpuDispatcher()->addCompletionPrereq(*mTaskFetch);
	}

	if (mOnFinishCallback)
	{
		(*mOnFinishCallback)(stream);
	}
	return false;
}

FieldSamplerQueryGPU::SceneInfoGPU::SceneInfoGPU(ResourceList& list, FieldSamplerQuery* query, FieldSamplerSceneWrapper* sceneWrapper)
	: SceneInfo(list, query, sceneWrapper)
	, mConstMemGroup(DYNAMIC_CAST(FieldSamplerSceneWrapperGPU*)(sceneWrapper)->getConstStorage())
{
	APEX_CUDA_CONST_MEM_GROUP_SCOPE(mConstMemGroup);

	mQueryParamsHandle.alloc(_storage_);
}

bool FieldSamplerQueryGPU::SceneInfoGPU::update()
{
	if (FieldSamplerQuery::SceneInfo::update())
	{
		APEX_CUDA_CONST_MEM_GROUP_SCOPE(mConstMemGroup);

		FieldSamplerParamsExArray paramsExArray;
		mParamsExArrayHandle.allocOrFetch(_storage_, paramsExArray);
		if (paramsExArray.resize(_storage_, mEnabledFieldSamplerCount))
		{
			for (uint32_t i = 0, enabledIdx = 0; i < mFieldSamplerArray.size(); ++i)
			{
				FieldSamplerWrapperGPU* fieldSamplerWrapper = DYNAMIC_CAST(FieldSamplerWrapperGPU*)(mFieldSamplerArray[i].mFieldSamplerWrapper);
				if (fieldSamplerWrapper->isEnabled())
				{
					FieldSamplerParamsEx fsParamsEx;
					fsParamsEx.paramsHandle = fieldSamplerWrapper->getParamsHandle();
					fsParamsEx.multiplier = mFieldSamplerArray[i].mMultiplier;
					PX_ASSERT(enabledIdx < mEnabledFieldSamplerCount);
					paramsExArray.updateElem(_storage_, fsParamsEx, enabledIdx++);
				}
			}
			mParamsExArrayHandle.update(_storage_, paramsExArray);
		}
		return true;
	}
	return false;
}

PxVec3 FieldSamplerQueryGPU::executeFieldSamplerQueryOnGrid(const FieldSamplerQueryGridDataIntl& data)
{
	FieldSamplerGridKernelArgs args;

	args.numX           = data.numX;
	args.numY           = data.numY;
	args.numZ           = data.numZ;

	args.gridToWorld    = data.gridToWorld;

	args.mass           = data.mass;
	args.elapsedTime    = data.timeStep;
	args.cellSize		= data.cellSize;
	args.totalElapsedMS = mManager->getApexScene().getTotalElapsedMS();

	FieldSamplerGridKernelLaunchDataIntl launchData;
	launchData.stream       = data.stream;
	launchData.kernelType   = FieldSamplerKernelType::GRID;
	launchData.kernelArgs   = &args;
	launchData.threadCountX	= data.numX;
	launchData.threadCountY	= data.numY;
	launchData.threadCountZ	= data.numZ;
	launchData.accumArray	= data.resultVelocity;


	{
		APEX_CUDA_SURFACE_SCOPE_BIND(surfRefGridAccum, *launchData.accumArray, ApexCudaMemFlags::OUT);

		CUDA_OBJ(clearGridKernel)(data.stream, launchData.threadCountX, launchData.threadCountY, launchData.threadCountZ,
								  args.numX, args.numY, args.numZ);
	}

	PxVec3 velocity(0.0f);
	for (uint32_t i = 0; i < mSecondarySceneList.size(); ++i)
	{
		SceneInfoGPU* sceneInfo = DYNAMIC_CAST(SceneInfoGPU*)(mSecondarySceneList[i]);
		FieldSamplerSceneWrapperGPU* sceneWrapper = DYNAMIC_CAST(FieldSamplerSceneWrapperGPU*)(sceneInfo->getSceneWrapper());

		launchData.activeFieldSamplerCount = 0;

		const nvidia::Array<FieldSamplerInfo>& fieldSamplerArray = sceneInfo->getFieldSamplerArray();
		for (uint32_t fieldSamplerIdx = 0; fieldSamplerIdx < fieldSamplerArray.size(); ++fieldSamplerIdx)
		{
			const FieldSamplerWrapperGPU* wrapper = static_cast<const FieldSamplerWrapperGPU* >( fieldSamplerArray[fieldSamplerIdx].mFieldSamplerWrapper );
			if (wrapper->isEnabled())
			{
				switch (wrapper->getInternalFieldSamplerDesc().gridSupportType)
				{
					case FieldSamplerGridSupportTypeIntl::SINGLE_VELOCITY:
					{
						const FieldSamplerIntl* fieldSampler = wrapper->getInternalFieldSampler();
						velocity += fieldSampler->queryFieldSamplerVelocity();
					}
					break;
					case FieldSamplerGridSupportTypeIntl::VELOCITY_PER_CELL:
					{
						launchData.activeFieldSamplerCount += 1;
					}
					break;
					default:
						break;
				}
			}
		}

		if (launchData.activeFieldSamplerCount > 0)
		{
			launchData.queryParamsHandle = sceneInfo->getQueryParamsHandle();
			launchData.paramsExArrayHandle = sceneInfo->getParamsHandle();
			launchData.fieldSamplerArray = &sceneInfo->getFieldSamplerArray();
			launchData.kernelMode = FieldSamplerKernelMode::DEFAULT;

			sceneWrapper->getInternalFieldSamplerScene()->launchFieldSamplerCudaKernel(launchData);
		}
	}
	return velocity;
}


#endif

}
} // end namespace nvidia::apex

