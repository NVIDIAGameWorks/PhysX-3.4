/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __FIELD_SAMPLER_SCENE_WRAPPER_H__
#define __FIELD_SAMPLER_SCENE_WRAPPER_H__

#include "Apex.h"
#include "ApexSDKHelpers.h"
#include "ApexActor.h"
#include "FieldSamplerSceneIntl.h"

#if APEX_CUDA_SUPPORT
#include "ApexCudaWrapper.h"
#endif

#include "FieldSamplerCommon.h"

namespace nvidia
{
namespace fieldsampler
{

class FieldSamplerManager;
class FieldSamplerWrapper;
class FieldBoundaryWrapper;

class FieldSamplerSceneWrapper : public ApexResourceInterface, public ApexResource
{
public:
	// ApexResourceInterface methods
	void			release();
	void			setListIndex(ResourceList& list, uint32_t index)
	{
		m_listIndex = index;
		m_list = &list;
	}
	uint32_t	getListIndex() const
	{
		return m_listIndex;
	}

	FieldSamplerSceneWrapper(ResourceList& list, FieldSamplerManager* manager, FieldSamplerSceneIntl* fieldSamplerScene);

	FieldSamplerSceneIntl* getInternalFieldSamplerScene() const
	{
		return mFieldSamplerScene;
	}
	const FieldSamplerSceneDescIntl& getInternalFieldSamplerSceneDesc() const
	{
		return mFieldSamplerSceneDesc;
	}

	virtual void update();
	virtual void postUpdate() = 0;

	class FieldBoundaryInfo;

	FieldBoundaryInfo*	addFieldBoundary(FieldBoundaryWrapper* fieldBoundaryWrapper);
	void				removeFieldBoundary(FieldBoundaryInfo* fieldBoundaryInfo);

protected:
	virtual FieldBoundaryInfo* createFieldBoundaryInfo(FieldBoundaryWrapper* fieldBoundaryWrapper) = 0;

protected:
	FieldSamplerManager*	mManager;
	FieldSamplerSceneIntl*	mFieldSamplerScene;
	FieldSamplerSceneDescIntl	mFieldSamplerSceneDesc;

	ResourceList			mFieldBoundaryList;
	bool					mFieldBoundaryListChanged;
};

class FieldSamplerSceneWrapper::FieldBoundaryInfo : public ApexResourceInterface, public ApexResource
{
protected:
	FieldBoundaryWrapper*	mFieldBoundaryWrapper;
	uint32_t			mRefCount;

	FieldBoundaryInfo(ResourceList& list, FieldBoundaryWrapper* fieldBoundaryWrapper)
		: mFieldBoundaryWrapper(fieldBoundaryWrapper), mRefCount(0)
	{
		list.add(*this);
	}

public:
	FieldBoundaryWrapper* getFieldBoundaryWrapper() const
	{
		return mFieldBoundaryWrapper;
	}

	virtual void update() {}

	void addRef()
	{
		++mRefCount;
	}
	bool releaseRef()
	{
		if (--mRefCount == 0)
		{
			release();
			return true;
		}
		return false;
	}

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

/******************************** CPU Version ********************************/

class FieldSamplerSceneWrapperCPU : public FieldSamplerSceneWrapper
{
public:
	FieldSamplerSceneWrapperCPU(ResourceList& list, FieldSamplerManager* manager, FieldSamplerSceneIntl* fieldSamplerScene);

	virtual void postUpdate() {}

	class FieldBoundaryInfoCPU : public FieldBoundaryInfo
	{
	public:
		FieldBoundaryInfoCPU(ResourceList& list, FieldBoundaryWrapper* fieldBoundaryWrapper)
			: FieldBoundaryInfo(list, fieldBoundaryWrapper)
		{
		}
	};

protected:
	virtual FieldBoundaryInfo* createFieldBoundaryInfo(FieldBoundaryWrapper* fieldBoundaryWrapper)
	{
		return PX_NEW(FieldBoundaryInfoCPU)(mFieldBoundaryList, fieldBoundaryWrapper);
	}

};

/******************************** GPU Version ********************************/

#if APEX_CUDA_SUPPORT
class FieldSamplerSceneWrapperGPU : public FieldSamplerSceneWrapper
{
public:
	FieldSamplerSceneWrapperGPU(ResourceList& list, FieldSamplerManager* manager, FieldSamplerSceneIntl* fieldSamplerScene);

	virtual void postUpdate();

	PX_INLINE ApexCudaConstStorage& getConstStorage()
	{
		return mConstStorage;
	}

	class FieldBoundaryInfoGPU : public FieldBoundaryInfo
	{
	public:
		FieldBoundaryInfoGPU(ResourceList& list, FieldBoundaryWrapper* fieldBoundaryWrapper, ApexCudaConstStorage& constStorage);

		virtual void update();

		PX_INLINE InplaceHandle<FieldShapeGroupParams> getShapeGroupParamsHandle() const
		{
			PX_ASSERT(mFieldShapeGroupParamsHandle.isNull() == false);
			return mFieldShapeGroupParamsHandle;
		}

	private:
		ApexCudaConstMemGroup                   mConstMemGroup;
		InplaceHandle<FieldShapeGroupParams>    mFieldShapeGroupParamsHandle;
	};

protected:
	virtual FieldBoundaryInfo* createFieldBoundaryInfo(FieldBoundaryWrapper* fieldBoundaryWrapper)
	{
		return PX_NEW(FieldBoundaryInfoGPU)(mFieldBoundaryList, fieldBoundaryWrapper, getConstStorage());
	}

private:
	FieldSamplerSceneWrapperGPU& operator=(const FieldSamplerSceneWrapperGPU&);

	ApexCudaConstStorage& mConstStorage;
};
#endif // APEX_CUDA_SUPPORT

}
} // end namespace nvidia::apex

#endif
