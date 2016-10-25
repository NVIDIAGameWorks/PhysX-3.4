/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __FIELD_SAMPLER_WRAPPER_H__
#define __FIELD_SAMPLER_WRAPPER_H__

#include "Apex.h"
#include "ApexSDKHelpers.h"
#include "ApexActor.h"
#include "FieldSamplerIntl.h"

#if APEX_CUDA_SUPPORT
#include "ApexCudaWrapper.h"
#endif

#include "FieldSamplerSceneWrapper.h"
#include "FieldSamplerCommon.h"

namespace nvidia
{
namespace fieldsampler
{

class FieldSamplerManager;

class FieldBoundaryWrapper;

class FieldSamplerWrapper : public ApexResourceInterface, public ApexResource
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

	FieldSamplerWrapper(ResourceList& list, FieldSamplerManager* manager, FieldSamplerIntl* fieldSampler, const FieldSamplerDescIntl& fieldSamplerDesc, FieldSamplerSceneWrapper* fieldSamplerSceneWrapper);

	virtual void update();

	PX_INLINE FieldSamplerIntl* getInternalFieldSampler() const
	{
		return mFieldSampler;
	}
	PX_INLINE const FieldSamplerDescIntl& getInternalFieldSamplerDesc() const
	{
		return mFieldSamplerDesc;
	}
	PX_INLINE FieldSamplerSceneWrapper* getFieldSamplerSceneWrapper() const
	{
		return mSceneWrapper;
	}

	bool addFieldBoundary(FieldBoundaryWrapper* wrapper);
	bool removeFieldBoundary(FieldBoundaryWrapper* wrapper);

	uint32_t getFieldBoundaryCount() const
	{
		return mFieldBoundaryInfoArray.size();
	}
	FieldBoundaryWrapper* getFieldBoundaryWrapper(uint32_t index) const
	{
		return mFieldBoundaryInfoArray[index]->getFieldBoundaryWrapper();
	}

	PX_INLINE const FieldShapeDescIntl&   getInternalFieldSamplerShape() const
	{
		return mFieldSamplerShape;
	}
	PX_INLINE bool                      isFieldSamplerChanged() const
	{
		return mFieldSamplerShapeChanged;
	}
	PX_INLINE bool                      isEnabled() const
	{
		return mIsEnabled;
	}
	PX_INLINE bool                      isEnabledChanged() const
	{
		return (mIsEnabled != mIsEnabledLast);
	}

protected:
	FieldSamplerManager*		mManager;
	FieldSamplerIntl*				mFieldSampler;
	FieldSamplerDescIntl			mFieldSamplerDesc;

	FieldShapeDescIntl			mFieldSamplerShape;
	bool						mFieldSamplerShapeChanged;

	FieldSamplerSceneWrapper*	mSceneWrapper;
	uint32_t				mQueryRefCount;

	nvidia::Array<FieldSamplerSceneWrapper::FieldBoundaryInfo*>	mFieldBoundaryInfoArray;
	bool														mFieldBoundaryInfoArrayChanged;

	bool						mIsEnabled;
	bool						mIsEnabledLast;

	friend class FieldSamplerManager;
};


class FieldSamplerWrapperCPU : public FieldSamplerWrapper
{
public:
	FieldSamplerWrapperCPU(ResourceList& list, FieldSamplerManager* manager, FieldSamplerIntl* fieldSampler, const FieldSamplerDescIntl& fieldSamplerDesc, FieldSamplerSceneWrapper* fieldSamplerSceneWrapper);

private:
};

#if APEX_CUDA_SUPPORT
class FieldSamplerWrapperGPU : public FieldSamplerWrapperCPU
{
public:
	FieldSamplerWrapperGPU(ResourceList& list, FieldSamplerManager* manager, FieldSamplerIntl* fieldSampler, const FieldSamplerDescIntl& fieldSamplerDesc, FieldSamplerSceneWrapper* fieldSamplerSceneWrapper);

	virtual void update();

	PX_INLINE InplaceHandle<FieldSamplerParams>   getParamsHandle() const
	{
		PX_ASSERT(mFieldSamplerParamsHandle.isNull() == false);
		return mFieldSamplerParamsHandle;
	}

private:
	ApexCudaConstMemGroup               mConstMemGroup;
	InplaceHandle<FieldSamplerParams>   mFieldSamplerParamsHandle;
};
#endif

}
} // end namespace nvidia::apex

#endif
