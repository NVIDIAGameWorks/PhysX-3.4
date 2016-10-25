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
#include "FieldSamplerWrapper.h"
#include "FieldBoundaryWrapper.h"
#include "FieldSamplerManager.h"
#include "FieldSamplerSceneWrapper.h"


namespace nvidia
{
namespace fieldsampler
{


FieldSamplerWrapper::FieldSamplerWrapper(ResourceList& list, FieldSamplerManager* manager, FieldSamplerIntl* fieldSampler, const FieldSamplerDescIntl& fieldSamplerDesc, FieldSamplerSceneWrapper* fieldSamplerSceneWrapper)
	: mManager(manager)
	, mFieldSampler(fieldSampler)
	, mFieldSamplerDesc(fieldSamplerDesc)
	, mFieldSamplerShapeChanged(false)
	, mSceneWrapper(fieldSamplerSceneWrapper)
	, mQueryRefCount(0)
	, mFieldBoundaryInfoArrayChanged(true)
	, mIsEnabled(false)
	, mIsEnabledLast(false)
{
	list.add(*this);

	//set default shape weight to 1
	mFieldSamplerShape.weight = 1;
}

void FieldSamplerWrapper::release()
{
	for (uint32_t i = 0; i < mFieldBoundaryInfoArray.size(); ++i)
	{
		FieldSamplerSceneWrapper::FieldBoundaryInfo* fieldBoundaryInfo = mFieldBoundaryInfoArray[i];
		mSceneWrapper->removeFieldBoundary(fieldBoundaryInfo);
	}

	delete this;
}

bool FieldSamplerWrapper::addFieldBoundary(FieldBoundaryWrapper* fieldBoundaryWrapper)
{
	const FieldBoundaryDescIntl& fieldBoundaryDesc = fieldBoundaryWrapper->getInternalFieldBoundaryDesc();

	float weight;
	if (mManager->getFieldSamplerGroupsFiltering(fieldBoundaryDesc.boundaryFilterData, mFieldSamplerDesc.boundaryFilterData,weight))
	{
		FieldSamplerSceneWrapper::FieldBoundaryInfo* fieldBoundaryInfo =
		    mSceneWrapper->addFieldBoundary(fieldBoundaryWrapper);

		mFieldBoundaryInfoArray.pushBack(fieldBoundaryInfo);
		mFieldBoundaryInfoArrayChanged = true;
		return true;
	}
	return false;
}

bool FieldSamplerWrapper::removeFieldBoundary(FieldBoundaryWrapper* fieldBoundaryWrapper)
{
	for (uint32_t i = 0; i < mFieldBoundaryInfoArray.size(); ++i)
	{
		FieldSamplerSceneWrapper::FieldBoundaryInfo* fieldBoundaryInfo = mFieldBoundaryInfoArray[i];
		if (fieldBoundaryInfo->getFieldBoundaryWrapper() == fieldBoundaryWrapper)
		{
			mSceneWrapper->removeFieldBoundary(fieldBoundaryInfo);

			mFieldBoundaryInfoArray.replaceWithLast(i);
			mFieldBoundaryInfoArrayChanged = true;
			return true;
		}
	}
	return false;
}

void FieldSamplerWrapper::update()
{
	mIsEnabledLast = mIsEnabled;
	mFieldSamplerShapeChanged = mFieldSampler->updateFieldSampler(mFieldSamplerShape, mIsEnabled);
	if (!mIsEnabledLast && mIsEnabled)
	{
		mFieldSamplerShapeChanged = true;
	}
	else if (!mIsEnabled)
	{
		mFieldSamplerShapeChanged = false;
	}
}

/******************************** CPU Version ********************************/

FieldSamplerWrapperCPU::FieldSamplerWrapperCPU(ResourceList& list, FieldSamplerManager* manager, FieldSamplerIntl* fieldSampler, const FieldSamplerDescIntl& fieldSamplerDesc, FieldSamplerSceneWrapper* fieldSamplerSceneWrapper)
	: FieldSamplerWrapper(list, manager, fieldSampler, fieldSamplerDesc, fieldSamplerSceneWrapper)
{
}

/******************************** GPU Version ********************************/
#if APEX_CUDA_SUPPORT

FieldSamplerWrapperGPU::FieldSamplerWrapperGPU(ResourceList& list, FieldSamplerManager* manager, FieldSamplerIntl* fieldSampler, const FieldSamplerDescIntl& fieldSamplerDesc, FieldSamplerSceneWrapper* fieldSamplerSceneWrapper)
	: FieldSamplerWrapperCPU(list, manager, fieldSampler, fieldSamplerDesc, fieldSamplerSceneWrapper)
	, mConstMemGroup(DYNAMIC_CAST(FieldSamplerSceneWrapperGPU*)(fieldSamplerSceneWrapper)->getConstStorage())
{
}

void FieldSamplerWrapperGPU::update()
{
	FieldSamplerWrapper::update();
	if (mFieldSamplerShapeChanged || mFieldBoundaryInfoArrayChanged)
	{
		APEX_CUDA_CONST_MEM_GROUP_SCOPE(mConstMemGroup);

		FieldSamplerParams params;
		if (mFieldSamplerParamsHandle.allocOrFetch(_storage_, params))
		{
			//only on alloc
			params.type = mFieldSamplerDesc.type;
			params.gridSupportType = mFieldSamplerDesc.gridSupportType;
			params.dragCoeff = mFieldSamplerDesc.dragCoeff;

			params.includeShape.fade           = PxClamp(mFieldSamplerDesc.boundaryFadePercentage, 0.0f, 1.0f);
		}
		
		if (mFieldSamplerShapeChanged)
		{
			FieldSamplerIntl::CudaExecuteInfo executeInfo;
			mFieldSampler->getFieldSamplerCudaExecuteInfo(executeInfo);

			params.executeType = executeInfo.executeType;
			params.executeParamsHandle = executeInfo.executeParamsHandle;

			params.includeShape.type           = mFieldSamplerShape.type;
			params.includeShape.dimensions     = mFieldSamplerShape.dimensions;
			params.includeShape.worldToShape   = mFieldSamplerShape.worldToShape;
			PX_ASSERT(mFieldSamplerShape.weight >= 0.0f && mFieldSamplerShape.weight <= 1.0f);
			params.includeShape.weight         = PxClamp(mFieldSamplerShape.weight, 0.0f, 1.0f);
		}

		if (mFieldBoundaryInfoArrayChanged)
		{
			uint32_t shapeGroupCount = mFieldBoundaryInfoArray.size();
			if (params.excludeShapeGroupHandleArray.resize(_storage_, shapeGroupCount))
			{
				for (uint32_t shapeGroupIndex = 0; shapeGroupIndex < shapeGroupCount; ++shapeGroupIndex)
				{
					FieldSamplerSceneWrapperGPU::FieldBoundaryInfoGPU* fieldBoundaryInfo =
						static_cast<FieldSamplerSceneWrapperGPU::FieldBoundaryInfoGPU*>(mFieldBoundaryInfoArray[shapeGroupIndex]);

					InplaceHandle<FieldShapeGroupParams> elem = fieldBoundaryInfo->getShapeGroupParamsHandle();
					params.excludeShapeGroupHandleArray.updateElem(_storage_, elem, shapeGroupIndex);
				}
			}

			mFieldBoundaryInfoArrayChanged = false;
		}

		mFieldSamplerParamsHandle.update(_storage_, params);
	}
}

#endif

}
} // end namespace nvidia::apex

