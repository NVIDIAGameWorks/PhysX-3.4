/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __FIELD_SAMPLER_QUERY_H__
#define __FIELD_SAMPLER_QUERY_H__

#include "Apex.h"
#include "ApexSDKHelpers.h"
#include "FieldSamplerQueryIntl.h"
#include "FieldSamplerIntl.h"

#if APEX_CUDA_SUPPORT
#include "ApexCudaWrapper.h"
#endif

#include "FieldSamplerCommon.h"

namespace nvidia
{
namespace fieldsampler
{

class FieldSamplerManager;
class FieldSamplerSceneWrapper;
class FieldSamplerWrapper;

class FieldSamplerQuery : public FieldSamplerQueryIntl, public ApexResourceInterface, public ApexResource
{
protected:

	class SceneInfo : public ApexResourceInterface, public ApexResource
	{
	protected:
		FieldSamplerQuery* mQuery;
		FieldSamplerSceneWrapper* mSceneWrapper;
		nvidia::Array<FieldSamplerInfo> mFieldSamplerArray;
		bool mFieldSamplerArrayChanged;

		uint32_t mEnabledFieldSamplerCount;

		SceneInfo(ResourceList& list, FieldSamplerQuery* query, FieldSamplerSceneWrapper* sceneWrapper)
			: mQuery(query), mSceneWrapper(sceneWrapper), mFieldSamplerArrayChanged(false), mEnabledFieldSamplerCount(0)
		{
			list.add(*this);
		}

	public:
		FieldSamplerSceneWrapper* getSceneWrapper() const
		{
			return mSceneWrapper;
		}

		void addFieldSampler(FieldSamplerWrapper* fieldSamplerWrapper, float multiplier)
		{
			FieldSamplerInfo fsInfo;
			fsInfo.mFieldSamplerWrapper = fieldSamplerWrapper;
			fsInfo.mMultiplier = multiplier;
			mFieldSamplerArray.pushBack(fsInfo);
			mFieldSamplerArrayChanged = true;
		}
		bool removeFieldSampler(FieldSamplerWrapper* fieldSamplerWrapper)
		{
			const uint32_t size = mFieldSamplerArray.size();
			for (uint32_t index = 0; index < size; ++index)
			{
				if (mFieldSamplerArray[index].mFieldSamplerWrapper == fieldSamplerWrapper)
				{
					mFieldSamplerArray.replaceWithLast(index);
					mFieldSamplerArrayChanged = true;
					return true;
				}
			}
			return false;
		}
		void clearAllFieldSamplers()
		{
			mFieldSamplerArray.clear();
			mFieldSamplerArrayChanged = true;
		}
		uint32_t getEnabledFieldSamplerCount() const
		{
			return mEnabledFieldSamplerCount;
		}
		const nvidia::Array<FieldSamplerInfo>& getFieldSamplerArray() const
		{
			return mFieldSamplerArray;
		}

		virtual bool update();

		// ApexResourceInterface methods
		void						release()
		{
			delete this;
		}
		void						setListIndex(ResourceList& list, uint32_t index)
		{
			m_listIndex = index;
			m_list = &list;
		}
		uint32_t				getListIndex() const
		{
			return m_listIndex;
		}
	};

	SceneInfo* findSceneInfo(FieldSamplerSceneWrapper* sceneWrapper) const;

	virtual SceneInfo* createSceneInfo(FieldSamplerSceneWrapper* sceneWrapper) = 0;

	FieldSamplerQuery(const FieldSamplerQueryDescIntl& desc, ResourceList& list, FieldSamplerManager* manager);

public:
	virtual void update();
	
	PX_INLINE void setOnStartCallback(FieldSamplerCallbackIntl* callback)
	{
		if (mOnStartCallback) 
		{
			PX_DELETE(mOnStartCallback);
		}
		mOnStartCallback = callback;
	}
	PX_INLINE void setOnFinishCallback(FieldSamplerCallbackIntl* callback)
	{
		if (mOnFinishCallback) 
		{
			PX_DELETE(mOnFinishCallback);
		}
		mOnFinishCallback = callback;
	}

	virtual void		submitTasks() {}
	virtual void		setTaskDependencies() {}
	virtual void		fetchResults() {}

	void	release();
	void	destroy();

	// ApexResourceInterface methods
	void						setListIndex(ResourceList& list, uint32_t index)
	{
		m_listIndex = index;
		m_list = &list;
	}
	uint32_t				getListIndex() const
	{
		return m_listIndex;
	}

	void		setPhysXScene(PxScene*)	{}
	PxScene*	getPhysXScene() const
	{
		return NULL;
	}

	bool addFieldSampler(FieldSamplerWrapper*);
	bool removeFieldSampler(FieldSamplerWrapper*);
	void clearAllFieldSamplers();

	const FieldSamplerQueryDescIntl& getQueryDesc() const
	{
		return mQueryDesc;
	}

protected:
	void submitFieldSamplerQuery(const FieldSamplerQueryDataIntl& data, PxTask*, PxTask* );

	FieldSamplerManager* 				mManager;

	FieldSamplerQueryDescIntl				mQueryDesc;
	FieldSamplerQueryDataIntl				mQueryData;

	ResourceList						mSceneList;

	nvidia::Array<SceneInfo*>			mPrimarySceneList;
	nvidia::Array<SceneInfo*>			mSecondarySceneList;

	ApexMirroredArray<PxVec4>	mAccumVelocity;

	FieldSamplerCallbackIntl*				mOnStartCallback;
	FieldSamplerCallbackIntl*				mOnFinishCallback;

	friend class FieldSamplerManager;
};


class FieldSamplerQueryCPU : public FieldSamplerQuery
{
public:
	FieldSamplerQueryCPU(const FieldSamplerQueryDescIntl& desc, ResourceList& list, FieldSamplerManager* manager);
	~FieldSamplerQueryCPU();

	// FieldSamplerQueryIntl methods
	PxTaskID submitFieldSamplerQuery(const FieldSamplerQueryDataIntl& data, PxTaskID taskID);

protected:
	void		execute();
	void		executeScene(	const SceneInfo* sceneInfo, 
								const FieldSamplerIntl::ExecuteData& executeData, 
								PxVec4* accumField, 
								PxVec4* accumVelocity, 
								uint32_t positionStride, 
								uint32_t velocityStride, 
								uint32_t massStride);

	uint32_t				mExecuteCount;
	nvidia::Array<PxVec3>	mResultField;
	nvidia::Array<float>	mWeights;

	PxTask*				mTaskExecute;

	friend class TaskExecute;

	class SceneInfoCPU : public SceneInfo
	{
	public:
		SceneInfoCPU(ResourceList& list, FieldSamplerQuery* query, FieldSamplerSceneWrapper* sceneWrapper)
			: SceneInfo(list, query, sceneWrapper)
		{
		}
	};

	virtual SceneInfo* createSceneInfo(FieldSamplerSceneWrapper* sceneWrapper)
	{
		return PX_NEW(SceneInfoCPU)(mSceneList, this, sceneWrapper);
	}

};

#if APEX_CUDA_SUPPORT

class FieldSamplerQueryGPU : public FieldSamplerQueryCPU
{
public:
	FieldSamplerQueryGPU(const FieldSamplerQueryDescIntl& desc, ResourceList& list, FieldSamplerManager* manager);
	~FieldSamplerQueryGPU();

	// FieldSamplerQueryIntl methods
	PxTaskID submitFieldSamplerQuery(const FieldSamplerQueryDataIntl& data, PxTaskID taskID);

	PxVec3 executeFieldSamplerQueryOnGrid(const FieldSamplerQueryGridDataIntl&);

protected:
	bool		launch(CUstream stream, int kernelIndex);
	void		prepare();
	bool		copy(CUstream stream, int kernelIndex);
	void		fetch();

	PxTask*		mTaskLaunch;
	PxTask*		mTaskPrepare;
	PxTask*		mTaskCopy;
	PxTask*		mTaskFetch;

	ApexMirroredArray<PxVec4>	mPositionMass;
	ApexMirroredArray<PxVec4>	mVelocity;
	ApexMirroredArray<PxVec4>	mAccumField;
	PxGpuCopyDescQueue			mCopyQueue;

	friend class FieldSamplerQueryLaunchTask;
	friend class FieldSamplerQueryPrepareTask;
	friend class FieldSamplerQueryCopyTask;
	friend class FieldSamplerQueryFetchTask;

	class SceneInfoGPU : public SceneInfo
	{
	public:
		SceneInfoGPU(ResourceList& list, FieldSamplerQuery* query, FieldSamplerSceneWrapper* sceneWrapper);

		virtual bool update();

		PX_INLINE InplaceHandle<nvidia::fieldsampler::FieldSamplerParamsExArray> getParamsHandle() const
		{
			PX_ASSERT(mParamsExArrayHandle.isNull() == false);
			return mParamsExArrayHandle;
		}
		PX_INLINE InplaceHandle<nvidia::fieldsampler::FieldSamplerQueryParams> getQueryParamsHandle() const
		{
			PX_ASSERT(mQueryParamsHandle.isNull() == false);
			return mQueryParamsHandle;
		}

	private:
		ApexCudaConstMemGroup                           mConstMemGroup;
		InplaceHandle<nvidia::fieldsampler::FieldSamplerParamsExArray>    mParamsExArrayHandle;
		InplaceHandle<nvidia::fieldsampler::FieldSamplerQueryParams>      mQueryParamsHandle;
	};

	virtual SceneInfo* createSceneInfo(FieldSamplerSceneWrapper* sceneWrapper)
	{
		return PX_NEW(SceneInfoGPU)(mSceneList, this, sceneWrapper);
	}
};

#endif

}
} // end namespace nvidia::apex

#endif
